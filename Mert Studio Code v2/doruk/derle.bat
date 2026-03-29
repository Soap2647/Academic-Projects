@echo off
chcp 65001 >nul
title DORUK - C++ Derleme

echo.
echo ████████████████████████████████████████████████
echo        DORUK C++ Backend Derleniyor...
echo ████████████████████████████████████████████████
echo.

:: CMake yolu (VS 2026 içindeki)
set CMAKE="C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

:: Proje kök dizini (bu .bat dosyasının bulunduğu yer)
set PROJE_DIR=%~dp0
set BUILD_DIR=%PROJE_DIR%build

:: Build klasörünü oluştur
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cd /d "%BUILD_DIR%"

echo [1/3] CMake yapılandırılıyor...
%CMAKE% .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 (
    echo.
    echo [HATA] CMake yapılandırması başarısız!
    pause
    exit /b 1
)

echo.
echo [2/3] Derleniyor (Release)...
%CMAKE% --build . --config Release --parallel

if errorlevel 1 (
    echo.
    echo [HATA] Derleme başarısız!
    pause
    exit /b 1
)

echo.
echo [3/3] doruk.exe kopyalanıyor...

:: Çeşitli olası konumlardan kopyala
if exist "%BUILD_DIR%\Release\doruk.exe" (
    copy /y "%BUILD_DIR%\Release\doruk.exe" "%PROJE_DIR%doruk.exe"
    echo [OK] %PROJE_DIR%doruk.exe oluşturuldu!
) else if exist "%BUILD_DIR%\cli\Release\doruk.exe" (
    copy /y "%BUILD_DIR%\cli\Release\doruk.exe" "%PROJE_DIR%doruk.exe"
    echo [OK] %PROJE_DIR%doruk.exe oluşturuldu!
) else (
    echo [UYARI] doruk.exe Release klasöründe bulunamadı, aranıyor...
    for /r "%BUILD_DIR%" %%f in (doruk.exe) do (
        copy /y "%%f" "%PROJE_DIR%doruk.exe"
        echo [OK] %%f bulundu ve kopyalandı!
        goto bitti
    )
    echo [HATA] doruk.exe hiçbir yerde bulunamadı!
    pause
    exit /b 1
)

:bitti
echo.
echo ████████████████████████████████████████████████
echo   DERLEME BAŞARILI! doruk.exe hazır.
echo   Şimdi Mert Studio Code'u açıp F5'e basın!
echo ████████████████████████████████████████████████
echo.
pause
