@echo off
echo ========================================
echo ESP32 DCC Servo Controller Build Menu
echo ========================================
echo.
echo 1. Build project
echo 2. Upload to ESP32
echo 3. Clean build files
echo 4. Build and upload
echo 5. Open serial monitor
echo 6. Build, upload and monitor
echo 7. Exit
echo.
set /p choice="Enter your choice (1-7): "

set PIO_PATH="%USERPROFILE%\.platformio\penv\Scripts\platformio.exe"

if "%choice%"=="1" (
    echo Building project...
    %PIO_PATH% run
) else if "%choice%"=="2" (
    echo Uploading to ESP32...
    %PIO_PATH% run --target upload
) else if "%choice%"=="3" (
    echo Cleaning build files...
    %PIO_PATH% run --target clean
) else if "%choice%"=="4" (
    echo Building and uploading...
    %PIO_PATH% run --target upload
) else if "%choice%"=="5" (
    echo Opening serial monitor...
    echo Press Ctrl+C to exit monitor
    %PIO_PATH% device monitor
) else if "%choice%"=="6" (
    echo Building, uploading and opening monitor...
    %PIO_PATH% run --target upload
    echo Starting serial monitor...
    echo Press Ctrl+C to exit monitor
    %PIO_PATH% device monitor
) else if "%choice%"=="7" (
    echo Goodbye!
    exit /b 0
) else (
    echo Invalid choice. Please try again.
)

echo.
echo Press any key to return to menu...
pause > nul
goto :eof
