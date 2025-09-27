@echo off
echo Testing CNC Tick Counter via HTTP API
echo =====================================
echo.
echo Making HTTP request to read tick counter...
echo URL: http://localhost:8080/read?thread=1&group=0x20300&offset=0x7&length=4
echo.

curl "http://localhost:8080/read?thread=1&group=0x20300&offset=0x7&length=4"

echo.
echo.
echo Done! The 'data' field contains the hex representation of the current tick count.
echo If result=0, the read was successful. If result=-1, check if CNC is running.
echo.
pause