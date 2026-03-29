// ============================================================
// Environment.h — DORUK kapsam (scope) ve closure sistemi
//
// Lexical scoping: her blok yeni bir Environment oluşturur.
// Kapatma (closure): fonksiyon kendi kapsamını shared_ptr ile tutar.
//
// Hiyerarşi:
//   küresel → fonksiyon → blok → iç blok...
//   Her kapsamda tanımlı değişken bulunamazsa üst kapsama bakılır.
// ============================================================
#pragma once

#include "Value.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>

namespace Doruk {

// ============================================================
// CalismaHatasi — yorumlayıcı çalışma zamanı hatası
// ============================================================
struct CalismaHatasi {
    std::string mesaj;
    uint32_t    satir = 0;
    uint32_t    sutun = 0;

    CalismaHatasi(std::string msg, uint32_t s = 0, uint32_t c = 0)
        : mesaj(std::move(msg)), satir(s), sutun(c) {}
};

// ============================================================
// Environment — tek kapsam katmanı
// ============================================================
class Environment : public std::enable_shared_from_this<Environment> {
public:
    // ust = nullptr → küresel kapsam
    explicit Environment(std::shared_ptr<Environment> ust = nullptr);

    // Yeni değişken tanımla (mevcut kapsama)
    void tanimla(const std::string& isim, DegerRef deger);

    // Değişken oku — bulamazsa üst kapsamlara bak
    // Bulunamazsa CalismaHatasi fırlatır
    DegerRef oku(const std::string& isim, uint32_t satir, uint32_t sutun) const;

    // Değişken güncelle — tanımlı kapsama yaz
    // Bulunamazsa CalismaHatasi fırlatır
    void guncelle(const std::string& isim, DegerRef deger,
                  uint32_t satir, uint32_t sutun);

    // Bu kapsamda doğrudan tanımlı mı?
    bool yerelTanimliMi(const std::string& isim) const;

    // Üst kapsama erişim (closure zinciri için)
    std::shared_ptr<Environment> ust() const { return ust_; }

    // Çocuk kapsam oluştur
    std::shared_ptr<Environment> cocukOlustur();

private:
    std::unordered_map<std::string, DegerRef> degerler_;
    std::shared_ptr<Environment>              ust_;
};

} // namespace Doruk

