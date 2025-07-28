@echo off
echo Starting PlatformIO Serial Monitor...
echo Connecting to ESP32 at 115200 baud
echo Press Ctrl+C to exit monitor
echo.
echo ===== Serial Monitor Output =====
echo.

set PIO_PATH="%USERPROFILE%\.platformio\penv\Scripts\platformio.exe"
%PIO_PATH% device monitor
