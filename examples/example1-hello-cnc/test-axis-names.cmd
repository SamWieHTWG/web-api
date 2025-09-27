@echo off
echo Testing CNC Axis Names via JSON API with Automatic Type Conversion
echo ================================================================
echo.
echo Reading axis names using JSON API...
echo.
echo Object details:
echo - Thread: 3 (CNC_TASK_HMI)
echo - Group: 131585 (0x20201 = CNC_IGRP_AC_AXIS + 1)
echo - Offset: 4355 (0x1103 = AC_X_AXES_NAMES)
echo - Type: STRUCT (automatically detected)
echo - Length: 68 bytes (automatically calculated)
echo.

curl -X POST "http://localhost:8080/read" ^
  -H "Content-Type: application/json" ^
  -d "{\"thread\": 3, \"group\": 131585, \"offset\": 4355, \"length\": 68}"

echo.
echo.
echo Done! The JSON API automatically:
echo - Detected the data type (STRUCT for axis names)
echo - Provided the raw structure data
echo - No manual hex conversion needed!
echo.
echo The 'value' field contains the axis names structure:
echo - Use a hex-to-ASCII converter to decode axis names
echo - First 4 bytes = number of axes
echo - Next 64 bytes = axis names (4 axes * 16 chars each)
echo.
pause