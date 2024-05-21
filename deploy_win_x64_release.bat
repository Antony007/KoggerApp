@echo off
REM locat paths:
set binPath=build\MinGW_8_1_0_x64_desktop-Release\release\KoggerApp.exe
set qmlPath=QML
set outPath=out_x64
set zip_file=out_x64\out_x64.zip

echo executable file path: "%binPath%"
echo QML project files path: "%qmlPath%"
echo output path: "%outPath%"

if exist "%outPath%" (
    rmdir /s /q "%outPath%"
    echo the output directory will be overwritten
)
REM deploy and copy binary file:
windeployqt %binPath% -qmldir %qmlPath% -dir %outPath% -no-translations -no-virtualkeyboard
xcopy "%binPath%" "%outPath%"

REM .zip archiving
7z a "%zip_file%" "%outPath%" %compression_level%
pause