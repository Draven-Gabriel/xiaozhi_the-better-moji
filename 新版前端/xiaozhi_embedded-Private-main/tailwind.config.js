module.exports = {
  content: ['./src/**/*.{vue,js,jsx,ts,tsx}', './public/index.html'],
  corePlugins: {
    preflight: false
  },
  theme: {
    extend: {
      colors: {
        apple: {
          bg: '#f5f5f7',
          surface: '#ffffff',
          dark: '#1d1d1f',
          gray: '#6e6e73',
          blue: '#0071e3',
          bluehover: '#0077ed',
          blueactive: '#006edb',
          tint: '#e8f1fd',
          success: '#34c759',
          warning: '#ff9f0a',
          danger: '#ff3b30'
        }
      },
      borderRadius: {
        sm: '8px',
        md: '12px',
        lg: '16px',
        xl: '24px',
        '2xl': '32px'
      },
      boxShadow: {
        card: '0 1px 2px rgba(0,0,0,0.04)',
        raised: '0 12px 32px 0 rgba(0,0,0,0.10), 0 2px 8px rgba(0,0,0,0.06)',
        overlay: '0 24px 48px rgba(0,0,0,0.16)'
      },
      fontFamily: {
        apple: ['-apple-system', 'BlinkMacSystemFont', '"SF Pro Display"', '"SF Pro Text"', '"Helvetica Neue"', '"PingFang SC"', '"Microsoft YaHei"', 'sans-serif']
      },
      transitionTimingFunction: {
        apple: 'cubic-bezier(0.32, 0.72, 0, 1)'
      }
    }
  },
  plugins: []
}
