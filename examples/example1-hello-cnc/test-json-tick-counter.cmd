@echo off
echo Testing CNC Web API with JSON and Automatic Type Conversion
echo =========================================================
echo.
echo Reading tick counter using JSON API...
echo.

curl -X POST "http://localhost:8080/read" ^
  -H "Content-Type: application/json" ^
  -d "{\"thread\": 1, \"group\": 131840, \"offset\": 7}"

echo.
echo.
echo Done! The API automatically:
echo - Detected the data type (should be UNS32)
echo - Converted the raw bytes to proper JSON number
echo - No need to specify length or handle hex conversion!
echo.
pause