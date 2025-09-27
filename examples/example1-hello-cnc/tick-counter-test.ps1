# Tick Counter API Performance Test
# Tests the exact same call that the web browser makes

$url = "http://localhost:8080/read"

Write-Host "Testing Tick Counter API Call..." -ForegroundColor Yellow
Write-Host "URL: $url" -ForegroundColor Gray
Write-Host ""

# Measure time using high-precision stopwatch
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

try {
    # Use Invoke-WebRequest instead of curl for better PowerShell compatibility
    $body = @{
        thread = 1
        group = 131840
        offset = 7
        length = 4
        datatype = 6
    } | ConvertTo-Json -Compress

    Write-Host "Body: $body" -ForegroundColor Gray

    $response = Invoke-WebRequest -Uri $url -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
    $stopwatch.Stop()

    Write-Host "API Call Completed" -ForegroundColor Green
    Write-Host "Response Time: $($stopwatch.ElapsedMilliseconds)ms" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Response Data:" -ForegroundColor Yellow
    Write-Host $response.Content -ForegroundColor White

} catch {
    $stopwatch.Stop()
    Write-Host "API Call Failed after $($stopwatch.ElapsedMilliseconds)ms" -ForegroundColor Red
    Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host ""
Write-Host "Note: Browser shows ~311ms, this should show true PowerShell performance" -ForegroundColor Magenta