@echo off
echo Uploading ESP32 DCC Servo Controller...
"%USERPROFILE%\.platformio\penv\Scripts\platformio.exe" run --target upload
pause
