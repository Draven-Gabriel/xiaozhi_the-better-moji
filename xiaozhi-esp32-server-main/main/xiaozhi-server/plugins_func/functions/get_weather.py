import httpx
from config.logger import setup_logging
from plugins_func.register import register_function, ToolType, ActionResponse, Action
from core.utils.util import get_ip_info
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from core.connection import ConnectionHandler

TAG = __name__
logger = setup_logging()

GET_WEATHER_FUNCTION_DESC = {
    "type": "function",
    "function": {
        "name": "get_weather",
        "description": (
            "获取某个地点的天气。规则："
            "1. 用户指定了城市时，参数 location 填城市名（如：杭州）；"
            "2. 用户没有指定城市时（如只说'今天天气'），也必须直接调用本工具，参数 location 留空即可"
            "——代码会自动根据用户IP定位城市，定位失败则使用默认城市。"
            "禁止反问用户'你在哪个城市'。"
        ),
        "parameters": {
            "type": "object",
            "properties": {
                "location": {
                    "type": "string",
                    "description": "地点名，例如杭州。可选参数，如果不提供则不传",
                },
                "lang": {
                    "type": "string",
                    "description": "返回用户使用的语言code，例如zh_CN/zh_HK/en_US/ja_JP等，默认zh_CN",
                },
            },
            "required": ["lang"],
        },
    },
}

HEADERS = {
    "User-Agent": (
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
        "(KHTML, like Gecko) Chrome/92.0.4515.107 Safari/537.36"
    )
}

# WMO 天气代码映射（Open-Meteo 标准，免费无需 key）
# 参考: https://open-meteo.com/en/docs 的 WMO Weather interpretation codes
WEATHER_CODE_MAP = {
    0: "晴",
    1: "基本晴朗",
    2: "多云",
    3: "阴",
    45: "雾",
    48: "冻雾",
    51: "毛毛雨",
    53: "中等毛毛雨",
    55: "密集毛毛雨",
    56: "冻毛毛雨",
    57: "强冻毛毛雨",
    61: "小雨",
    63: "中雨",
    65: "大雨",
    66: "冻雨",
    67: "强冻雨",
    71: "小雪",
    73: "中雪",
    75: "大雪",
    77: "雪粒",
    80: "小阵雨",
    81: "中阵雨",
    82: "强阵雨",
    85: "小阵雪",
    86: "强阵雪",
    95: "雷暴",
    96: "雷暴伴冰雹",
    99: "强雷暴伴冰雹",
}


async def fetch_city_info(location, api_key="", api_host=""):
    """通过 Open-Meteo 地理编码查询城市（免费，无需 key）"""
    url = f"https://geocoding-api.open-meteo.com/v1/search?name={location}&count=1&language=zh&format=json"
    async with httpx.AsyncClient(timeout=httpx.Timeout(5.0, connect=3.0)) as client:
        response = await client.get(url, headers=HEADERS)
    data = response.json()
    if data.get("results") is None or len(data.get("results", [])) == 0:
        logger.bind(tag=TAG).error(f"未找到城市: {location}")
        return None
    result = data["results"][0]
    return {
        "name": result.get("name", location),
        "latitude": result.get("latitude"),
        "longitude": result.get("longitude"),
        "admin1": result.get("admin1", ""),
        "country": result.get("country", ""),
    }


async def fetch_weather_page(city_info):
    """通过 Open-Meteo 获取当前天气 + 7天预报（免费，无需 key）"""
    url = (
        f"https://api.open-meteo.com/v1/forecast?"
        f"latitude={city_info['latitude']}&longitude={city_info['longitude']}"
        f"&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,wind_speed_10m"
        f"&daily=weather_code,temperature_2m_max,temperature_2m_min"
        f"&timezone=auto&forecast_days=7"
    )
    async with httpx.AsyncClient(timeout=httpx.Timeout(10.0, connect=3.0)) as client:
        response = await client.get(url, headers=HEADERS)
    if response.status_code != 200:
        return None
    return response.json()


def parse_weather_info(data):
    """从 Open-Meteo JSON 解析天气信息"""
    city_name = data["city_name"]
    current = data["current"]
    daily = data["daily"]

    # 当前天气描述
    code = current.get("weather_code")
    weather_desc = WEATHER_CODE_MAP.get(code, "未知")
    current_abstract = (
        f"{weather_desc}，气温 {current.get('temperature_2m', '?')}°C"
    )

    # 详细参数
    current_basic = {
        "温度": f"{current.get('temperature_2m', '?')}°C",
        "体感": f"{current.get('apparent_temperature', '?')}°C",
        "湿度": f"{current.get('relative_humidity_2m', '?')}%",
        "风速": f"{current.get('wind_speed_10m', '?')}km/h",
    }

    # 7天预报
    temps_list = []
    dates = daily.get("time", [])
    codes = daily.get("weather_code", [])
    highs = daily.get("temperature_2m_max", [])
    lows = daily.get("temperature_2m_min", [])
    for i in range(len(dates)):
        desc = WEATHER_CODE_MAP.get(codes[i], "未知") if i < len(codes) else "未知"
        high = highs[i] if i < len(highs) else "?"
        low = lows[i] if i < len(lows) else "?"
        temps_list.append((dates[i], desc, high, low))

    return city_name, current_abstract, current_basic, temps_list


@register_function("get_weather", GET_WEATHER_FUNCTION_DESC, ToolType.SYSTEM_CTL)
async def get_weather(conn: "ConnectionHandler", location: str = None, lang: str = "zh_CN"):
    from core.utils.cache.manager import cache_manager, CacheType

    weather_config = conn.config.get("plugins", {}).get("get_weather", {})
    default_location = weather_config.get("default_location", "广州")
    client_ip = conn.client_ip

    # 优先使用用户提供的location参数
    if not location:
        # 通过客户端IP解析城市
        if client_ip:
            # 先从缓存获取IP对应的城市信息
            cached_ip_info = cache_manager.get(CacheType.IP_INFO, client_ip)
            if cached_ip_info:
                location = cached_ip_info.get("city")
            else:
                # 缓存未命中，调用API获取
                ip_info = get_ip_info(client_ip, logger)
                if ip_info:
                    cache_manager.set(CacheType.IP_INFO, client_ip, ip_info)
                    location = ip_info.get("city")

            if not location:
                location = default_location
        else:
            # 若无IP，使用默认位置
            location = default_location
    # 尝试从缓存获取完整天气报告
    weather_cache_key = f"full_weather_{location}_{lang}"
    cached_weather_report = cache_manager.get(CacheType.WEATHER, weather_cache_key)
    if cached_weather_report:
        return ActionResponse(Action.REQLLM, cached_weather_report, None)

    # 缓存未命中，获取实时天气数据（Open-Meteo，免费无需key）
    city_info = await fetch_city_info(location)
    if not city_info:
        return ActionResponse(
            Action.REQLLM, f"未找到相关的城市: {location}，请确认地点是否正确", None
        )
    weather_data = await fetch_weather_page(city_info)
    if not weather_data:
        return ActionResponse(Action.REQLLM, None, "请求失败")
    # 注入城市名（含省份），如"广东广州"
    weather_data["city_name"] = (
        f"{city_info.get('admin1', '')}{city_info.get('name', location)}"
    )
    city_name, current_abstract, current_basic, temps_list = parse_weather_info(weather_data)

    weather_report = f"您查询的位置是：{city_name}\n\n当前天气: {current_abstract}\n"

    # 添加有效的当前天气参数
    if current_basic:
        weather_report += "详细参数：\n"
        for key, value in current_basic.items():
            if value != "0":  # 过滤无效值
                weather_report += f"  · {key}: {value}\n"

    # 添加7天预报
    weather_report += "\n未来7天预报：\n"
    for date, weather, high, low in temps_list:
        weather_report += f"{date}: {weather}，气温 {low}~{high}\n"

    # 提示语
    weather_report += "\n（如需某一天的具体天气，请告诉我日期）"

    # 缓存完整的天气报告
    cache_manager.set(CacheType.WEATHER, weather_cache_key, weather_report)

    return ActionResponse(Action.REQLLM, weather_report, None)
