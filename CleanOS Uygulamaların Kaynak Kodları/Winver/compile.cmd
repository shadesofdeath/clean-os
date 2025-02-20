@echo off
setlocal enabledelayedexpansion

REM Eski dosyaları temizle
if exist Winver.exe del Winver.exe
if exist *.obj del *.obj
if exist *.res del *.res

REM Visual Studio ortamını yükle
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

echo Kaynak dosyası derleniyor...
rc /v /nologo /c65001 resource.rc
if errorlevel 1 (
    echo Kaynak derlemesi başarısız oldu!
    pause
    exit /b 1
)

echo resource.res dosyası oluşturuldu...

echo Derleniyor...

REM C++ dosyasını derle
cl /nologo /EHsc /W4 /std:c++17 /DUNICODE /D_UNICODE /utf-8 /O2 /GL ^
    winver.cpp resource.res ^
    /link /SUBSYSTEM:WINDOWS /LTCG ^
    user32.lib ^
    gdi32.lib ^
    dwmapi.lib ^
    uxtheme.lib ^
    advapi32.lib ^
    shell32.lib

if errorlevel 1 (
    echo Derleme başarısız oldu!
    pause
    exit /b 1
)

REM Başarılı
echo.
echo Derleme başarıyla tamamlandı!

REM Geçici dosyaları temizle
del *.obj
del *.res

pause
