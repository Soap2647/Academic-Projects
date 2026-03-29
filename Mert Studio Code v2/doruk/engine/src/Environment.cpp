// ============================================================
// Environment.cpp — kapsam implementasyonu
// ============================================================
#include "Environment.h"

namespace Doruk {

// ============================================================
// Kurucu
// ============================================================

Environment::Environment(std::shared_ptr<Environment> ust)
    : ust_(std::move(ust))
{}

// ============================================================
// tanimla — bu kapsamda yeni değişken oluştur
// ============================================================

void Environment::tanimla(const std::string& isim, DegerRef deger) {
    degerler_[isim] = deger ? deger : BOSDeger();
}

// ============================================================
// oku — değişken değerini oku (yukarı zincirle)
// ============================================================

DegerRef Environment::oku(const std::string& isim,
                           uint32_t satir, uint32_t sutun) const {
    auto it = degerler_.find(isim);
    if (it != degerler_.end()) return it->second;

    if (ust_) return ust_->oku(isim, satir, sutun);

    throw CalismaHatasi(
        "Tanımsız değişken: '" + isim + "'",
        // Tanımsız değişken
        satir, sutun
    );
}

// ============================================================
// guncelle — değişkeni tanımlı olduğu kapsamda güncelle
// ============================================================

void Environment::guncelle(const std::string& isim, DegerRef deger,
                            uint32_t satir, uint32_t sutun) {
    auto it = degerler_.find(isim);
    if (it != degerler_.end()) {
        it->second = deger ? deger : BOSDeger();
        return;
    }

    if (ust_) {
        ust_->guncelle(isim, deger, satir, sutun);
        return;
    }

    throw CalismaHatasi(
        "Tanımsız değişken: '" + isim + "'",
        satir, sutun
    );
}

// ============================================================
// yerelTanimliMi
// ============================================================

bool Environment::yerelTanimliMi(const std::string& isim) const {
    return degerler_.count(isim) > 0;
}

// ============================================================
// cocukOlustur — iç kapsam
// ============================================================

std::shared_ptr<Environment> Environment::cocukOlustur() {
    return std::make_shared<Environment>(shared_from_this());
}

} // namespace Doruk


