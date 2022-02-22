@echo off
TITLE Schmurtz ESP Flasher

:com_port
:: empty com variable
set "com="
cls
Echo+
Echo --------------------------------------------------------------------------------
Echo    Schmurtz ESP Flasher for MrDiy-Audio-Notifier               Version: Jan 2022
Echo --------------------------------------------------------------------------------
Echo+
echo ================================================================================
echo. & echo List of current COM ports : & echo.
wmic path win32_pnpentity get caption | find "(COM"
echo.
echo ================================================================================

echo. & echo Please enter the number of your ESP COM port (only the number, without "COM")
echo if you don't know just press enter
echo.
echo ================================================================================
set /p com=
cls
if "%com%"=="" (
echo. & echo This will let ESPtool flash an ESP connected to your computer.
echo Do not use this if you have another ESP device connected (it could flash the wrong one !^) & echo.
set com=auto
pause
)

if "%com%"=="auto" (
	set ComCmd=
) ELSE (
	set ComCmd=--port COM%com%
)



:Menu
set userinp=
cls
Echo+
Echo --------------------------------------------------------------------------------
Echo    Schmurtz ESP Flasher for MrDiy-Audio-Notifier               Version: Jan 2022
Echo --------------------------------------------------------------------------------
Echo+
echo Hello %USERNAME%, what do you want to do?                    Current COM port : %com% 
echo+
echo    1) Flash ESP32
echo    2) Flash ESP8266
echo+
echo    ---------------------------------
echo    X) Erase flash of the ESP
echo    ---------------------------------
echo+
echo    C) Choose another COM port
echo    Q) Exit
echo+
::Menu Choose Option Code
set /p userinp= ^> Select Option : 


if /i "%userinp%"=="1" set Selected_Firmware=FW_v0.61_ESP32_esp32doit-devkit-v4.bin & goto :FlashESP32
if /i "%userinp%"=="2" set Selected_Firmware=FW_v0.61_ESP8266_d1_mini.bin & goto :FlashESP8266
if /i "%userinp%"=="X" goto :EraseFlash
if /i "%userinp%"=="C" goto :com_port
if /i "%userinp%"=="Q" exit
goto :Menu




:FlashESP32
cls
echo  You have selected : %Selected_Firmware% & pause
echo.
tools\esptool.exe %ComCmd% --chip esp32 --baud 921600  write_flash --flash_mode dio --flash_freq 80m --flash_size detect 0x1000 firmwares\bootloader_qio_80m.bin 0x8000 firmwares\partitions.bin 0xe000 firmwares\boot_app0.bin 0x10000 firmwares\%Selected_Firmware%
echo  Press a key to return to main menu.
pause > NUL
goto :Menu


:FlashESP8266
cls
echo  You have selected : %Selected_Firmware% & pause
echo.
tools\esptool.exe %ComCmd% --chip esp8266 --baud 921600 write_flash 0x0 firmwares\%Selected_Firmware%
echo  Press a key to return to main menu.
pause > NUL
goto :Menu

:EraseFlash
cls
echo.
echo    !!!!!! You are about to erase your ESP !!!!!! & echo.
echo    ==============================================
echo.
echo  All data will be deleted (wifi settings too).
echo  Please connect only one ESP (to avoid to erase the wrong one).
echo.
echo  Press a key to start (or close this windows to avoid flash).
pause > NUL
cls
echo    !!!!!! Erasing ESP !!!!!! & echo.
tools\esptool.exe erase_flash
echo.
echo  Press a key to return to main menu.
pause > NUL
goto :Menu




