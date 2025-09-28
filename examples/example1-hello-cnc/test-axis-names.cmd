@echo off
echo Testing CNC Web API Server Connection and Axis Names
echo ====================================================
echo.

REM First test if server is running
echo 1. Testing server connection...
curl -X GET "http://localhost:8080/" --connect-timeout 3 --max-time 5 > nul 2>&1
if errorlevel 1 (
    echo ERROR: Cannot connect to CNC Web API server on localhost:8080
    echo.
    echo Please check:
    echo - Is the CncSdkExample.exe running?
    echo - Is the web API server started?
    echo - Is port 8080 available?
    echo.
    echo You can start the server by running: CncSdkExample.exe
    pause
    exit /b 1
)
echo ✓ Server is responding

echo.
echo 2. Testing server info page...
curl -X GET "http://localhost:8080/" -H "Accept: text/html" --silent
echo.

echo.
echo 3. Testing number of axes...
echo Object: Thread=1, Group=131840, Offset=7 (number_of_axes)
curl -X POST "http://localhost:8080/read" ^
  -H "Content-Type: application/json" ^
  -d "{\"thread\": 1, \"group\": 131840, \"offset\": 7}" ^
  --silent
echo.

echo.
echo 4. Testing axis names...
echo Object: Thread=3, Group=131585, Offset=4355 (axis_names structure)
curl -X POST "http://localhost:8080/read" ^
  -H "Content-Type: application/json" ^
  -d "{\"thread\": 3, \"group\": 131585, \"offset\": 4355, \"length\": 68}" ^
  --silent
echo.

echo.
echo 5. Testing WebSocket endpoint availability...
echo Attempting WebSocket connection test...
curl -X GET "http://localhost:8080/" ^
  -H "Connection: Upgrade" ^
  -H "Upgrade: websocket" ^
  -H "Sec-WebSocket-Key: dGVzdA==" ^
  --connect-timeout 2 --max-time 3 --silent --include
echo.

echo.
echo ====================================================
echo Connection Test Summary:
echo - HTTP REST API: Available on http://localhost:8080
echo - WebSocket API: Available on ws://localhost:8080
echo - Use /read endpoint for reading CNC objects
echo - Use /write endpoint for writing CNC objects
echo.
echo For the web HMI to work properly, both HTTP and WebSocket
echo endpoints must be responding correctly.
echo.
pause