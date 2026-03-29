@echo off
chcp 65001 >nul
title Doruk IDE - Kurulum ve Başlatma

echo.
echo ████████████████████████████████████████████████████
echo          DORUK IDE - Kurulum Başlatılıyor
echo ████████████████████████████████████████████████████
echo.

:: Node.js kontrolü
node --version >nul 2>&1
if errorlevel 1 (
    echo [HATA] Node.js bulunamadı!
    echo Node.js'i buradan indirin: https://nodejs.org
    echo Minimum sürüm: Node.js 18 LTS
    pause
    exit /b 1
)

echo [OK] Node.js:
node --version

:: npm kontrolü
npm --version >nul 2>&1
if errorlevel 1 (
    echo [HATA] npm bulunamadı!
    pause
    exit /b 1
)

echo [OK] npm:
npm --version
echo.

:: Bağımlılıkları yükle
echo [1/2] Bağımlılıklar yükleniyor (ilk kurulumda 2-3 dakika sürebilir)...
echo       monaco-editor + electron indiriliyor...
echo.

npm install

if errorlevel 1 (
    echo.
    echo [HATA] npm install başarısız!
    echo İnternet bağlantınızı kontrol edin.
    pause
    exit /b 1
)

echo.
echo [OK] Bağımlılıklar yüklendi!
echo.

:: Başlat
echo [2/2] Doruk IDE başlatılıyor...
echo.
npm start
