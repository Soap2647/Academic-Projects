// ============================================================
// Value.cpp — DorukDeger implementasyonu
// ============================================================
#include "Value.h"

#include <sstream>
#include <cmath>
#include <stdexcept>

namespace Doruk {

// ============================================================
// Fabrika fonksiyonları
// ============================================================

DegerRef DorukDeger::bos() {
    return BOSDeger();
}

DegerRef DorukDeger::tamSayiOlstur(int64_t v) {
    auto d = std::make_shared<DorukDeger>();
    d->tur      = Tur::TAM_SAYI;
    d->tamSayi  = v;
    return d;
}

DegerRef DorukDeger::ondalikOlstur(double v) {
    auto d = std::make_shared<DorukDeger>();
    d->tur     = Tur::ONDALIK;
    d->ondalik = v;
    return d;
}

DegerRef DorukDeger::metinOlstur(std::string v) {
    auto d = std::make_shared<DorukDeger>();
    d->tur   = Tur::METIN;
    d->metin = std::move(v);
    return d;
}

DegerRef DorukDeger::mantiksalOlstur(bool v) {
    return v ? DOGRUDeger() : YANLISDeger();
}

DegerRef DorukDeger::listeOlstur(std::vector<DegerRef> elemanlar) {
    auto d  = std::make_shared<DorukDeger>();
    d->tur  = Tur::LISTE;
    d->liste = std::make_shared<DorukListe>();
    d->liste->elemanlar = std::move(elemanlar);
    return d;
}

DegerRef DorukDeger::sozlukOlstur() {
    auto d    = std::make_shared<DorukDeger>();
    d->tur    = Tur::SOZLUK;
    d->sozluk = std::make_shared<DorukSozluk>();
    return d;
}

DegerRef DorukDeger::fonksiyonOlstur(DorukFonksiyon fn) {
    auto d       = std::make_shared<DorukDeger>();
    d->tur       = Tur::FONKSIYON;
    d->fonksiyon = std::make_shared<DorukFonksiyon>(std::move(fn));
    return d;
}

DegerRef DorukDeger::yerlesikOlstur(std::string isim, YerlesikFn fn) {
    auto d        = std::make_shared<DorukDeger>();
    d->tur        = Tur::YERLESIK;
    d->yerlesik   = std::make_shared<DorukYerlesik>();
    d->yerlesik->isim = std::move(isim);
    d->yerlesik->fn   = std::move(fn);
    return d;
}

DegerRef DorukDeger::sinifOlstur(std::shared_ptr<DorukSinif> s) {
    auto d   = std::make_shared<DorukDeger>();
    d->tur   = Tur::SINIF;
    d->sinif = std::move(s);
    return d;
}

DegerRef DorukDeger::nesneOlstur(std::shared_ptr<DorukNesne> n) {
    auto d   = std::make_shared<DorukDeger>();
    d->tur   = Tur::NESNE;
    d->nesne = std::move(n);
    return d;
}

// ============================================================
// Singleton önbellek değerleri
// ============================================================

const DegerRef& BOSDeger() {
    static DegerRef v = [](){
        auto d = std::make_shared<DorukDeger>();
        d->tur = DorukDeger::Tur::BOS;
        return d;
    }();
    return v;
}

const DegerRef& DOGRUDeger() {
    static DegerRef v = [](){
        auto d = std::make_shared<DorukDeger>();
        d->tur       = DorukDeger::Tur::MANTIKSAL;
        d->mantiksal = true;
        return d;
    }();
    return v;
}

const DegerRef& YANLISDeger() {
    static DegerRef v = [](){
        auto d = std::make_shared<DorukDeger>();
        d->tur       = DorukDeger::Tur::MANTIKSAL;
        d->mantiksal = false;
        return d;
    }();
    return v;
}

// ============================================================
// gercegiDeger — doğruluk değeri (falsy: boş, 0, "", yanlış)
// ============================================================

bool DorukDeger::gercegiDeger() const {
    switch (tur) {
        case Tur::BOS:        return false;
        case Tur::MANTIKSAL:  return mantiksal;
        case Tur::TAM_SAYI:   return tamSayi != 0;
        case Tur::ONDALIK:    return ondalik != 0.0;
        case Tur::METIN:      return !metin.empty();
        case Tur::LISTE:      return liste && !liste->elemanlar.empty();
        case Tur::SOZLUK:     return sozluk && !sozluk->cifter.empty();
        case Tur::FONKSIYON:  return true;
        case Tur::YERLESIK:   return true;
        case Tur::SINIF:      return true;
        case Tur::NESNE:      return true;
    }
    return false;
}

// ============================================================
// esittir — değer eşitliği
// ============================================================

bool DorukDeger::esittir(const DorukDeger& diger) const {
    if (tur != diger.tur) {
        // TAM_SAYI == ONDALIK karşılaştırması
        if (tur == Tur::TAM_SAYI && diger.tur == Tur::ONDALIK)
            return static_cast<double>(tamSayi) == diger.ondalik;
        if (tur == Tur::ONDALIK && diger.tur == Tur::TAM_SAYI)
            return ondalik == static_cast<double>(diger.tamSayi);
        return false;
    }
    switch (tur) {
        case Tur::BOS:       return true;
        case Tur::MANTIKSAL: return mantiksal == diger.mantiksal;
        case Tur::TAM_SAYI:  return tamSayi   == diger.tamSayi;
        case Tur::ONDALIK:   return ondalik   == diger.ondalik;
        case Tur::METIN:     return metin     == diger.metin;
        // Koleksiyonlar ve nesneler referans eşitliği
        case Tur::LISTE:     return liste.get()    == diger.liste.get();
        case Tur::SOZLUK:    return sozluk.get()   == diger.sozluk.get();
        case Tur::FONKSIYON: return fonksiyon.get()== diger.fonksiyon.get();
        case Tur::YERLESIK:  return yerlesik.get() == diger.yerlesik.get();
        case Tur::SINIF:     return sinif.get()    == diger.sinif.get();
        case Tur::NESNE:     return nesne.get()    == diger.nesne.get();
    }
    return false;
}

// ============================================================
// metineCevir — yazdırma için string gösterimi
// ============================================================

std::string DorukDeger::metineCevir() const {
    switch (tur) {
        case Tur::BOS:
            return "boş"; // boş
        case Tur::MANTIKSAL:
            return mantiksal
                ? "doğru"          // doğru
                : "yanlış"; // yanlış
        case Tur::TAM_SAYI:
            return std::to_string(tamSayi);
        case Tur::ONDALIK: {
            // Tam sayıya denk geliyorsa ".0" ekle
            std::ostringstream oss;
            oss << ondalik;
            std::string s = oss.str();
            if (s.find('.') == std::string::npos &&
                s.find('e') == std::string::npos &&
                s.find('E') == std::string::npos)
                s += ".0";
            return s;
        }
        case Tur::METIN:
            return metin;
        case Tur::LISTE: {
            std::string s = "[";
            if (liste) {
                for (size_t i = 0; i < liste->elemanlar.size(); ++i) {
                    if (i) s += ", ";
                    if (liste->elemanlar[i])
                        s += liste->elemanlar[i]->metineCevir();
                    else
                        s += "boş";
                }
            }
            s += "]";
            return s;
        }
        case Tur::SOZLUK: {
            std::string s = "{";
            if (sozluk) {
                bool ilk = true;
                for (const auto& k : sozluk->anahtarSirasi) {
                    if (!ilk) s += ", ";
                    ilk = false;
                    s += "\"" + k + "\": ";
                    auto it = sozluk->cifter.find(k);
                    if (it != sozluk->cifter.end() && it->second)
                        s += it->second->metineCevir();
                    else
                        s += "boş";
                }
            }
            s += "}";
            return s;
        }
        case Tur::FONKSIYON:
            return "<fonksiyon " +
                   (fonksiyon ? fonksiyon->isim : "?") + ">";
        case Tur::YERLESIK:
            return "<yerleşik " +           // yerleşik
                   (yerlesik ? yerlesik->isim : "?") + ">";
        case Tur::SINIF:
            return "<sınıf " +       // sınıf
                   (sinif ? sinif->isim : "?") + ">";
        case Tur::NESNE:
            return "<nesne:" +
                   (nesne && nesne->sinif ? nesne->sinif->isim : "?") + ">";
    }
    return "?";
}

// ============================================================
// turAdi — tip adı (hata mesajları için)
// ============================================================

std::string DorukDeger::turAdi() const {
    switch (tur) {
        case Tur::BOS:       return "boş";
        case Tur::TAM_SAYI:  return "tam_sayı";  // tam_sayı
        case Tur::ONDALIK:   return "ondalik";
        case Tur::METIN:     return "metin";
        case Tur::MANTIKSAL: return "mantıksal"; // mantıksal
        case Tur::LISTE:     return "liste";
        case Tur::SOZLUK:    return "sözlük"; // sözlük
        case Tur::FONKSIYON: return "fonksiyon";
        case Tur::YERLESIK:  return "yerleşik";   // yerleşik
        case Tur::SINIF:     return "sınıf"; // sınıf
        case Tur::NESNE:     return "nesne";
    }
    return "bilinmeyen";
}

} // namespace Doruk


