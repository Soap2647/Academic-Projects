// ============================================================
// Diagnostics.h — DORUK Dili Tanı Sistemi
// Açıklama: Lexer, Parser, Semantik Analizör ve Yorumlayıcıdan
//           gelen hata/uyarı mesajlarını temsil eden yapılar.
//           Hiçbir zaman exception fırlatılmaz; hata bilgileri
//           Tani yapıları aracılığıyla iletilir.
// ============================================================
#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <sstream>

namespace Doruk {

// ============================================================
// Siddet — Tanının ciddiyeti
// ============================================================
enum class Siddet {
    BILGI,   // informational — çalışmayı durdurmaz
    UYARI,   // warning       — çalışmayı durdurmaz
    HATA,    // error         — çalışmayı durdurur
};

// ============================================================
// TaniKaynagi — Hangi bileşen bu hatayı üretti?
// ============================================================
enum class TaniKaynagi {
    LEXER,            // Sözdizimsel tarayıcı
    PARSER,           // Sözdizim çözümleyici
    ANLAM_ANALIZI,    // Semantik analizör
    CALISMA_ZAMANI,   // Yorumlayıcı/runtime
};

// ============================================================
// Tani — Tek bir hata/uyarı/bilgi birimi
// ============================================================
struct Tani {
    Siddet       siddet;      // hata ciddiyeti
    TaniKaynagi  kaynak;      // üreten bileşen
    std::string  mesaj;       // ana hata mesajı (Türkçe)
    std::string  dosyaAdi;    // kaynak dosya adı
    uint32_t     satir;       // 1-tabanlı satır numarası
    uint32_t     sutun;       // 1-tabanlı sütun numarası
    uint32_t     uzunluk;     // hatanın kaynak kodda kapladığı karakter sayısı
    std::string  ipucu;       // isteğe bağlı düzeltme ipucu

    // Tüm alanları dolduran ana yapıcı
    Tani(Siddet       siddet,
         TaniKaynagi  kaynak,
         std::string  mesaj,
         std::string  dosyaAdi,
         uint32_t     satir,
         uint32_t     sutun,
         uint32_t     uzunluk,
         std::string  ipucu = "")
        : siddet(siddet)
        , kaynak(kaynak)
        , mesaj(std::move(mesaj))
        , dosyaAdi(std::move(dosyaAdi))
        , satir(satir)
        , sutun(sutun)
        , uzunluk(uzunluk)
        , ipucu(std::move(ipucu))
    {}

    // ──────────────────────────────────────────────────────────
    // Yardımcı erişimciler
    // ──────────────────────────────────────────────────────────
    bool hataVar()  const { return siddet == Siddet::HATA;  }
    bool uyariVar() const { return siddet == Siddet::UYARI; }
    bool bilgiMi()  const { return siddet == Siddet::BILGI; }

    // Siddetin Türkçe adı
    const char* siddetAdi() const noexcept;

    // Kaynak bileşenin Türkçe adı
    const char* kaynakAdi() const noexcept;

    // ──────────────────────────────────────────────────────────
    // Biçimlendirilmiş hata mesajı
    // Örn:
    //   [HATA] dosya.drk:12:5 → Tanımsız değişken: 'sayaç'
    //          ╰─ Bir değişken önce 'değişken' ile tanımlanmalıdır.
    // ──────────────────────────────────────────────────────────
    std::string bicimle() const;

    // Satır/sütun ikilisini "12:5" formatında döndürür
    std::string konumStr() const;
};

// ============================================================
// TaniListesi — Birden fazla tanıyı tutan yardımcı konteyner
// ============================================================
class TaniListesi {
public:
    TaniListesi() = default;

    // Tanı ekle
    void ekle(Tani tani);

    // Başka bir listenin tüm tanılarını birleştir
    void birlestir(const TaniListesi& diger);

    // Sorgular
    bool        hatalarVar()   const;
    bool        uyarilarVar()  const;
    size_t      hataSayisi()   const;
    size_t      uyariSayisi()  const;
    size_t      toplamSayi()   const { return tanilar_.size(); }
    bool        bos()          const { return tanilar_.empty(); }

    // Erişim
    const std::vector<Tani>& tanilar() const { return tanilar_; }
    const Tani& operator[](size_t i)   const { return tanilar_[i]; }

    // Tüm tanıları biçimlendirilmiş tek bir string olarak döndürür
    std::string hepsiBicimle() const;

    // Tüm tanılara erişim (alias)
    const std::vector<Tani>& hepsi() const { return tanilar_; }

    // Toplam tanı sayısı (alias)
    size_t taniSayisi() const { return tanilar_.size(); }

    // Iterator desteği (range-for için)
    auto begin() const { return tanilar_.begin(); }
    auto end()   const { return tanilar_.end();   }

private:
    std::vector<Tani> tanilar_;
};

// ============================================================
// Kolaylık Fabrika Fonksiyonları
// ============================================================

inline Tani lexerHatasi(std::string mesaj,
                        std::string dosya,
                        uint32_t    satir,
                        uint32_t    sutun,
                        uint32_t    uzunluk = 1,
                        std::string ipucu   = "")
{
    return Tani(Siddet::HATA, TaniKaynagi::LEXER,
                std::move(mesaj), std::move(dosya),
                satir, sutun, uzunluk, std::move(ipucu));
}

inline Tani parserHatasi(std::string mesaj,
                         std::string dosya,
                         uint32_t    satir,
                         uint32_t    sutun,
                         uint32_t    uzunluk = 1,
                         std::string ipucu   = "")
{
    return Tani(Siddet::HATA, TaniKaynagi::PARSER,
                std::move(mesaj), std::move(dosya),
                satir, sutun, uzunluk, std::move(ipucu));
}

inline Tani anlamHatasi(std::string mesaj,
                        std::string dosya,
                        uint32_t    satir,
                        uint32_t    sutun,
                        uint32_t    uzunluk = 1,
                        std::string ipucu   = "")
{
    return Tani(Siddet::HATA, TaniKaynagi::ANLAM_ANALIZI,
                std::move(mesaj), std::move(dosya),
                satir, sutun, uzunluk, std::move(ipucu));
}

inline Tani anlamUyarisi(std::string dosya,
                         uint32_t    satir,
                         uint32_t    sutun,
                         std::string mesaj,
                         uint32_t    uzunluk = 1,
                         std::string ipucu   = "")
{
    return Tani(Siddet::UYARI, TaniKaynagi::ANLAM_ANALIZI,
                std::move(mesaj), std::move(dosya),
                satir, sutun, uzunluk, std::move(ipucu));
}

inline Tani calismaHatasi(std::string mesaj,
                          std::string dosya,
                          uint32_t    satir,
                          uint32_t    sutun,
                          uint32_t    uzunluk = 1,
                          std::string ipucu   = "")
{
    return Tani(Siddet::HATA, TaniKaynagi::CALISMA_ZAMANI,
                std::move(mesaj), std::move(dosya),
                satir, sutun, uzunluk, std::move(ipucu));
}

} // namespace Doruk

