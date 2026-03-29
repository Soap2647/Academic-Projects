// ============================================================
// Diagnostics.cpp — Tanı sistemi implementasyonu
// ============================================================
#include "Diagnostics.h"
#include <sstream>
#include <algorithm>

namespace Doruk {

// ============================================================
// Tani::siddetAdi
// ============================================================
const char* Tani::siddetAdi() const noexcept {
    switch (siddet) {
        case Siddet::BILGI:  return "BİLGİ";   // BİLGİ
        case Siddet::UYARI:  return "UYARI";
        case Siddet::HATA:   return "HATA";
    }
    return "???";
}

// ============================================================
// Tani::kaynakAdi
// ============================================================
const char* Tani::kaynakAdi() const noexcept {
    switch (kaynak) {
        case TaniKaynagi::LEXER:          return "LEXER";
        case TaniKaynagi::PARSER:         return "PARSER";
        case TaniKaynagi::ANLAM_ANALIZI:  return "ANALİZ";     // ANALİZ
        case TaniKaynagi::CALISMA_ZAMANI: return "ÇALIŞMA"; // ÇALIŞMA
    }
    return "???";
}

// ============================================================
// Tani::konumStr
// ============================================================
std::string Tani::konumStr() const {
    return std::to_string(satir) + ":" + std::to_string(sutun);
}

// ============================================================
// Tani::bicimle
// Örnek çıktı:
//   [HATA] dosya.drk:12:5 → Tanımsız değişken: 'sayaç'
//          ╰─ Bir değişken önce 'değişken' ile tanımlanmalıdır.
// ============================================================
std::string Tani::bicimle() const {
    std::ostringstream ss;

    // Başlık satırı: [HATA] dosya.drk:12:5 → mesaj
    ss << "["
       << siddetAdi()
       << "] "
       << dosyaAdi
       << ":"
       << satir
       << ":"
       << sutun
       << "  "   // → (U+2192, UTF-8: E2 86 92)
       << mesaj;

    // İpucu varsa ikinci satıra yaz
    if (!ipucu.empty()) {
        // Hizalama için başlık kısmı kadar boşluk
        // "[HATA] " = 7 char (ASCII), dosya:sat:sut boşluk atlanır,
        // sadece sabit girinti kullanıyoruz
        ss << "\n         "
           << " "  // ╰─ (U+2570 U+2500, UTF-8)
           << ipucu;
    }

    return ss.str();
}

// ============================================================
// TaniListesi
// ============================================================
void TaniListesi::ekle(Tani tani) {
    tanilar_.push_back(std::move(tani));
}

void TaniListesi::birlestir(const TaniListesi& diger) {
    tanilar_.insert(tanilar_.end(),
                    diger.tanilar_.begin(),
                    diger.tanilar_.end());
}

bool TaniListesi::hatalarVar() const {
    return std::any_of(tanilar_.begin(), tanilar_.end(),
                       [](const Tani& t) { return t.hataVar(); });
}

bool TaniListesi::uyarilarVar() const {
    return std::any_of(tanilar_.begin(), tanilar_.end(),
                       [](const Tani& t) { return t.uyariVar(); });
}

size_t TaniListesi::hataSayisi() const {
    return static_cast<size_t>(
        std::count_if(tanilar_.begin(), tanilar_.end(),
                      [](const Tani& t) { return t.hataVar(); }));
}

size_t TaniListesi::uyariSayisi() const {
    return static_cast<size_t>(
        std::count_if(tanilar_.begin(), tanilar_.end(),
                      [](const Tani& t) { return t.uyariVar(); }));
}

std::string TaniListesi::hepsiBicimle() const {
    std::ostringstream ss;
    for (size_t i = 0; i < tanilar_.size(); ++i) {
        if (i > 0) ss << "\n";
        ss << tanilar_[i].bicimle();
    }
    return ss.str();
}

} // namespace Doruk


