@echo off
echo Testing Axis 1 Position via CNC Web API
echo ========================================
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
    echo.
    echo You can start the server by running: CncSdkExample.exe
    pause
    exit /b 1
)
echo ✓ Server is responding

echo.
echo 2. Reading Axis 1 Position...
echo.
echo Object details:
echo - Thread: 3 (CNC_TASK_HMI)
echo - Group: 0x20201 (131585 - Axis 1 position group)
echo - Offset: 0x00030 (48 - ac_1_active_position_acs_hr_r)
echo - Type: REAL64 (datatype string, 8 bytes)
echo.

curl -X POST "http://localhost:8080/read" ^
  -H "Content-Type: application/json" ^
  -d "{\"thread\": 3, \"group\": 131585, \"offset\": 48, \"datatype\": \"REAL64\", \"length\": 8}" ^
  --silent --show-error
echo.

echo.
echo 3. Reading with automatic type detection...
curl -X POST "http://localhost:8080/read" ^
  -H "Content-Type: application/json" ^
  -d "{\"thread\": 3, \"group\": 131585, \"offset\": 48}" ^
  --silent --show-error
echo.

echo.
echo ========================================
echo Test Summary:
echo - Object: ac_1_active_position_acs_hr_r
echo - This reads the current position of Axis 1
echo - Value is in machine coordinates (typically mm)
echo - Type REAL64 provides high precision positioning
echo.
echo For other axes, change the group:
echo - Axis 1: Group 131585 (0x20201)
echo - Axis 2: Group 131586 (0x20202)
echo - Axis 3: Group 131587 (0x20203)
echo - etc...
echo.
pause