"""
Windows kayıt defteri tabanlı tekil kurulum kilidi.

OOP Rolü (Yardımcı Modül):
    Bu modül kurulum sürecinde kullanılır; oyunun zaten kurulup
    kurulmadığını HKLM kayıt defteri anahtarı üzerinden doğrular.
    Kaldırma işlemi sonrası bile kilit geçerliliğini korur.

Teknik Detay:
    HKLM (HKEY_LOCAL_MACHINE) tüm kullanıcılar için geçerlidir.
    Kaldırma işlemi bu anahtarı silmez — kalıcı kurulum kilidi
    böylece sağlanır.  İzin hatası durumunda HKCU'ya (mevcut kullanıcı)
    geri düşülür.
"""

import sys

# Yalnızca Windows'ta çalışır
if sys.platform != 'win32':
    raise ImportError("registry_check yalnızca Windows işletim sisteminde kullanılabilir.")

import winreg

REG_PATH  = r"SOFTWARE\TowerDefenseGame"
REG_KEY   = "InstallationID"
REG_VALUE = "INSTALLED_2025_PERMANENT"


def is_already_installed() -> bool:
    """
    Oyunun daha önce bu bilgisayara kurulup kurulmadığını kontrol eder.

    Önce HKLM'yi kontrol eder (yönetici kurulumu);
    bulunamazsa HKCU'ya (kullanıcı kurulumu) bakar.

    Returns:
        Kayıt defteri anahtarı ve değeri mevcutsa True.
    """
    for hive in (winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER):
        try:
            key   = winreg.OpenKey(hive, REG_PATH)
            value, _ = winreg.QueryValueEx(key, REG_KEY)
            winreg.CloseKey(key)
            if value == REG_VALUE:
                return True
        except FileNotFoundError:
            continue
        except OSError:
            continue
    return False


def mark_as_installed() -> None:
    """
    Kurulum başarıyla tamamlandıktan sonra kayıt defteri anahtarını yazar.

    HKLM'ye yazmayı dener; izin yetersizse HKCU'ya geri düşer.
    Raises:
        PermissionError: Her iki hive'a da yazılamazsa.
    """
    for hive in (winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER):
        try:
            key = winreg.CreateKey(hive, REG_PATH)
            winreg.SetValueEx(key, REG_KEY, 0, winreg.REG_SZ, REG_VALUE)
            winreg.CloseKey(key)
            return
        except PermissionError:
            continue
        except OSError:
            continue
    raise PermissionError(
        "Kayıt defterine yazılamadı: HKLM ve HKCU erişimi reddedildi."
    )


def remove_installation_mark() -> None:
    """
    (İsteğe bağlı) Kayıt defteri anahtarını siler.
    Normal kaldırma işleminde çağrılmaz — kalıcı kilit mekanizması budur.
    Bu fonksiyon yalnızca geliştirici/test ortamı için mevcuttur.
    """
    for hive in (winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER):
        try:
            winreg.DeleteKey(hive, REG_PATH)
        except FileNotFoundError:
            pass
        except OSError:
            pass


# ── Komut satırı testi ────────────────────────────────────────────────────────

if __name__ == '__main__':
    if len(sys.argv) > 1:
        cmd = sys.argv[1]
        if cmd == '--check':
            installed = is_already_installed()
            print(f"Kurulu: {installed}")
            sys.exit(0 if installed else 1)
        elif cmd == '--mark':
            mark_as_installed()
            print("Kurulum kaydı yapıldı.")
        elif cmd == '--remove':
            remove_installation_mark()
            print("Kurulum kaydı silindi (test modu).")
    else:
        print(f"Kurulum durumu: {is_already_installed()}")
