// ============================================================
// Value.h — DORUK çalışma zamanı değer tipi sistemi
//
// DorukDeger: dinamik tipli tek değer kabı
//   BOS | TAM_SAYI | ONDALIK | METIN | MANTIKSAL
//   LISTE | SOZLUK | FONKSIYON | YERLESIK | SINIF | NESNE
//
// Bellek: shared_ptr ile referans sayımı (GC yerine)
// Kural: C++ tanımlayıcılarında Türkçe karakter YOK.
// ============================================================
#pragma once

#include "AST.h"

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace Doruk {

// ── İleri bildirimler ────────────────────────────────────────
struct DorukDeger;
class  Environment;

using DegerRef = std::shared_ptr<DorukDeger>;

// ============================================================
// Koleksiyon türleri
// ============================================================

struct DorukListe {
    std::vector<DegerRef> elemanlar;
};

struct DorukSozluk {
    std::unordered_map<std::string, DegerRef> cifter;
    std::vector<std::string> anahtarSirasi; // ekleme sırası
};

// ============================================================
// Fonksiyon türleri
// ============================================================

// Kullanıcı tanımlı fonksiyon
struct DorukFonksiyon {
    std::string                  isim;
    std::vector<std::string>     parametreler;
    BlokDeyim*                   govde;    // non-owning (AST tarafından sahiplenilir)
    std::shared_ptr<Environment> kapatma;  // closure kapsamı
};

// Yerleşik (C++) fonksiyon
using YerlesikFn = std::function<DegerRef(std::vector<DegerRef>)>;

struct DorukYerlesik {
    std::string isim;
    YerlesikFn  fn;
};

// ============================================================
// Sınıf + nesne türleri
// ============================================================

struct DorukSinif {
    std::string                                    isim;
    std::string                                    ebeveyn;        // boş = yok
    std::shared_ptr<DorukSinif>                    ebeveynSinif;   // nullptr = yok
    std::unordered_map<std::string, DorukFonksiyon> metodlar;
};

struct DorukNesne {
    std::shared_ptr<DorukSinif>               sinif;
    std::unordered_map<std::string, DegerRef> alanlar;
};

// ============================================================
// DorukDeger — ana değer tipi
// ============================================================

struct DorukDeger {
    enum class Tur {
        BOS,
        TAM_SAYI,
        ONDALIK,
        METIN,
        MANTIKSAL,
        LISTE,
        SOZLUK,
        FONKSIYON,
        YERLESIK,
        SINIF,
        NESNE
    };

    Tur tur = Tur::BOS;

    // Primitif alanlar
    int64_t     tamSayi   = 0;
    double      ondalik   = 0.0;
    bool        mantiksal = false;
    std::string metin;

    // Karmaşık alanlar
    std::shared_ptr<DorukListe>     liste;
    std::shared_ptr<DorukSozluk>    sozluk;
    std::shared_ptr<DorukFonksiyon> fonksiyon;
    std::shared_ptr<DorukYerlesik>  yerlesik;
    std::shared_ptr<DorukSinif>     sinif;
    std::shared_ptr<DorukNesne>     nesne;

    // ── Fabrika fonksiyonları ─────────────────────────────
    static DegerRef bos();
    static DegerRef tamSayiOlstur(int64_t v);
    static DegerRef ondalikOlstur(double v);
    static DegerRef metinOlstur(std::string v);
    static DegerRef mantiksalOlstur(bool v);
    static DegerRef listeOlstur(std::vector<DegerRef> elemanlar = {});
    static DegerRef sozlukOlstur();
    static DegerRef fonksiyonOlstur(DorukFonksiyon fn);
    static DegerRef yerlesikOlstur(std::string isim, YerlesikFn fn);
    static DegerRef sinifOlstur(std::shared_ptr<DorukSinif> s);
    static DegerRef nesneOlstur(std::shared_ptr<DorukNesne> n);

    // ── Sorgulama ─────────────────────────────────────────

    // JavaScript tarzı doğruluk değeri
    bool gercegiDeger() const;

    // Değer eşitliği (==)
    bool esittir(const DorukDeger& diger) const;

    // Yazdırma için metin gösterimi
    std::string metineCevir() const;

    // Tip adı (hata mesajları için)
    std::string turAdi() const;
};

// ── Önbellek (singleton) değerler ────────────────────────────
const DegerRef& BOSDeger();
const DegerRef& DOGRUDeger();
const DegerRef& YANLISDeger();

} // namespace Doruk

