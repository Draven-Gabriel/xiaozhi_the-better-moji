$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
    $python = Get-Command py -ErrorAction SilentlyContinue
}
if (-not $python) {
    Write-Host "Python not found. Install Python 3 or run:"
    Write-Host "  C:\Users\10984\AppData\Local\Programs\Python\Python313\python.exe -m http.server 8011"
    exit 1
}

function Test-PortFree([int]$p) {
    try {
        $listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, $p)
        $listener.Start()
        $listener.Stop()
        return $true
    } catch {
        return $false
    }
}

$port = 8011
while (-not (Test-PortFree $port)) {
    $port++
}

Write-Host "XiaoZhi platform: http://localhost:$port  (Ctrl+C to stop)"
Start-Sleep -Milliseconds 500
Start-Process "http://localhost:$port"
& $python.Source -m http.server $port
