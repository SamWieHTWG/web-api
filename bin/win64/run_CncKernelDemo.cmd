copy "..\..\lib\win64\libCncSDK.dll" "."

xcopy "..\..\examples\example2-full-demo\cfg" ".\listen" /s /e /i /y
xcopy "..\..\examples\components\example-nc-programs" ".\prg" /s /e /i /y
xcopy "..\..\components\error" ".\error" /s /e /i /y

CncKernelDemo.exe