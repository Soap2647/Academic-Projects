@echo off
REM ════════════════════════════════════════════════════════════
REM  Tower Defense — Derleme ve Paketleme Betiği
REM  Kullanım: installer\build.bat (proje kökünden çalıştırın)
REM ════════════════════════════════════════════════════════════

echo [1/4] Bağımlılıklar kontrol ediliyor...
pip show pygame >nul 2>&1
if errorlevel 1 (
    echo pygame bulunamadı — yükleniyor...
    pip install pygame
)
pip show pyinstaller >nul 2>&1
if errorlevel 1 (
    echo PyInstaller bulunamadı — yükleniyor...
    pip install pyinstaller
)

echo [2/4] PyInstaller ile tek dosya EXE oluşturuluyor...
pyinstaller ^
    --onefile ^
    --windowed ^
    --name="TowerDefense" ^
    --distpath="dist" ^
    --workpath="build" ^
    --specpath="build" ^
    main.py

if errorlevel 1 (
    echo HATA: PyInstaller derlemesi basarisiz!
    exit /b 1
)

echo [3/4] dist\TowerDefense.exe oluşturuldu.

echo [4/4] Inno Setup kurulum paketi oluşturuluyor...
REM Inno Setup iscc.exe'nin PATH'de olduğunu varsayar.
REM Varsayılan konum: C:\Program Files (x86)\Inno Setup 6\
where iscc >nul 2>&1
if errorlevel 1 (
    echo UYARI: Inno Setup bulunamadı. Kurulum paketi atlanıyor.
    echo        Inno Setup'ı https://jrsoftware.org/isinfo.php adresinden indirin.
) else (
    iscc installer\installer_script.iss
    if errorlevel 1 (
        echo HATA: Inno Setup derlemesi basarisiz!
        exit /b 1
    )
    echo Kurulum paketi dist\TowerDefenseSetup_v1.0.exe olarak oluşturuldu.
)

echo.
echo ════ Derleme tamamlandı ════
echo Çalıştırmak için: dist\TowerDefense.exe
pause
