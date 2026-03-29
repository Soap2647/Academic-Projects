// ============================================================
// Token.cpp — Token yapısının implementasyonu
// ============================================================
#include "Token.h"
#include <sstream>

namespace Doruk {

// ============================================================
// tokenTuruAdi — Token türünün Türkçe adı
// ============================================================
const char* tokenTuruAdi(TokenTuru tur) noexcept {
    switch (tur) {
        // Literaller
        case TokenTuru::TAM_SAYI_LIT:    return "TAM_SAYI_LIT";
        case TokenTuru::ONDALIK_LIT:     return "ONDALIK_LIT";
        case TokenTuru::METIN_LIT:       return "METIN_LIT";
        // Anahtar kelimeler
        case TokenTuru::DEGISKEN:        return "DEGISKEN";
        case TokenTuru::EGER:            return "EGER";
        case TokenTuru::DEGILSE:         return "DEGILSE";
        case TokenTuru::DONGU:           return "DONGU";
        case TokenTuru::ICIN:            return "ICIN";
        case TokenTuru::FONKSIYON:       return "FONKSIYON";
        case TokenTuru::DONDUR:          return "DONDUR";
        case TokenTuru::YAZDIR:          return "YAZDIR";
        case TokenTuru::OKU:             return "OKU";
        case TokenTuru::DOGRU:           return "DOGRU";
        case TokenTuru::YANLIS:          return "YANLIS";
        case TokenTuru::BOS:             return "BOS";
        case TokenTuru::VE:              return "VE";
        case TokenTuru::VEYA:            return "VEYA";
        case TokenTuru::DEGIL:           return "DEGIL";
        case TokenTuru::KIR:             return "KIR";
        case TokenTuru::DEVAM:           return "DEVAM";
        case TokenTuru::SINIF:           return "SINIF";
        case TokenTuru::YENI:            return "YENI";
        case TokenTuru::BU:              return "BU";
        // Tanımlayıcı
        case TokenTuru::TANIMLAYICI:     return "TANIMLAYICI";
        // Aritmetik
        case TokenTuru::ARTI:            return "ARTI";
        case TokenTuru::EKSI:            return "EKSI";
        case TokenTuru::CARPIM:          return "CARPIM";
        case TokenTuru::BOLUM:           return "BOLUM";
        case TokenTuru::MOD:             return "MOD";
        // Atama
        case TokenTuru::ESIT:            return "ESIT";
        // Karşılaştırma
        case TokenTuru::ESIT_ESIT:       return "ESIT_ESIT";
        case TokenTuru::ESIT_DEGIL:      return "ESIT_DEGIL";
        case TokenTuru::KUCUK:           return "KUCUK";
        case TokenTuru::KUCUK_ESIT:      return "KUCUK_ESIT";
        case TokenTuru::BUYUK:           return "BUYUK";
        case TokenTuru::BUYUK_ESIT:      return "BUYUK_ESIT";
        // Mantıksal
        case TokenTuru::VE_VE:           return "VE_VE";
        case TokenTuru::VEYA_VEYA:       return "VEYA_VEYA";
        case TokenTuru::UNLEM:           return "UNLEM";
        // Noktalama
        case TokenTuru::SOL_PAREN:       return "SOL_PAREN";
        case TokenTuru::SAG_PAREN:       return "SAG_PAREN";
        case TokenTuru::SOL_SUSE:        return "SOL_SUSE";
        case TokenTuru::SAG_SUSE:        return "SAG_SUSE";
        case TokenTuru::SOL_KOSELI:      return "SOL_KOSELI";
        case TokenTuru::SAG_KOSELI:      return "SAG_KOSELI";
        case TokenTuru::NOKTALI_VIRGUL:  return "NOKTALI_VIRGUL";
        case TokenTuru::VIRGUL:          return "VIRGUL";
        case TokenTuru::NOKTA:           return "NOKTA";
        case TokenTuru::IKI_NOKTA:       return "IKI_NOKTA";
        // Artırma/azaltma
        case TokenTuru::ARTI_ARTI:       return "ARTI_ARTI";
        case TokenTuru::EKSI_EKSI:       return "EKSI_EKSI";
        case TokenTuru::ARTI_ESIT:       return "ARTI_ESIT";
        case TokenTuru::EKSI_ESIT:       return "EKSI_ESIT";
        case TokenTuru::CARPIM_ESIT:     return "CARPIM_ESIT";
        case TokenTuru::BOLUM_ESIT:      return "BOLUM_ESIT";
        // Özel
        case TokenTuru::DOSYA_SONU:      return "DOSYA_SONU";
        case TokenTuru::BILINMEYEN:      return "BILINMEYEN";
    }
    return "???";
}

// ============================================================
// tokenTuruGoruntu — Kaynak kodda nasıl görünür?
// ============================================================
const char* tokenTuruGoruntu(TokenTuru tur) noexcept {
    switch (tur) {
        case TokenTuru::TAM_SAYI_LIT:    return "<tam sayı>";
        case TokenTuru::ONDALIK_LIT:     return "<ondalık>";
        case TokenTuru::METIN_LIT:       return "<metin>";
        case TokenTuru::DEGISKEN:        return "'de\u011fi\u015fken'";
        case TokenTuru::EGER:            return "'\u0065\u011fer'";
        case TokenTuru::DEGILSE:         return "'de\u011filse'";
        case TokenTuru::DONGU:           return "'d\u00f6ng\u00fc'";
        case TokenTuru::ICIN:            return "'\u0069\u00e7in'";
        case TokenTuru::FONKSIYON:       return "'fonksiyon'";
        case TokenTuru::DONDUR:          return "'d\u00f6nd\u00fcr'";
        case TokenTuru::YAZDIR:          return "'yazd\u0131r'";
        case TokenTuru::OKU:             return "'oku'";
        case TokenTuru::DOGRU:           return "'do\u011fru'";
        case TokenTuru::YANLIS:          return "'yanl\u0131\u015f'";
        case TokenTuru::BOS:             return "'bo\u015f'";
        case TokenTuru::VE:              return "'ve'";
        case TokenTuru::VEYA:            return "'veya'";
        case TokenTuru::DEGIL:           return "'de\u011fil'";
        case TokenTuru::KIR:             return "'k\u0131r'";
        case TokenTuru::DEVAM:           return "'devam'";
        case TokenTuru::SINIF:           return "'s\u0131n\u0131f'";
        case TokenTuru::YENI:            return "'yeni'";
        case TokenTuru::BU:              return "'bu'";
        case TokenTuru::TANIMLAYICI:     return "<tan\u0131mlay\u0131c\u0131>";
        case TokenTuru::ARTI:            return "'+'";
        case TokenTuru::EKSI:            return "'-'";
        case TokenTuru::CARPIM:          return "'*'";
        case TokenTuru::BOLUM:           return "'/'";
        case TokenTuru::MOD:             return "'%'";
        case TokenTuru::ESIT:            return "'='";
        case TokenTuru::ESIT_ESIT:       return "'=='";
        case TokenTuru::ESIT_DEGIL:      return "'!='";
        case TokenTuru::KUCUK:           return "'<'";
        case TokenTuru::KUCUK_ESIT:      return "'<='";
        case TokenTuru::BUYUK:           return "'>'";
        case TokenTuru::BUYUK_ESIT:      return "'>='";
        case TokenTuru::VE_VE:           return "'&&'";
        case TokenTuru::VEYA_VEYA:       return "'||'";
        case TokenTuru::UNLEM:           return "'!'";
        case TokenTuru::SOL_PAREN:       return "'('";
        case TokenTuru::SAG_PAREN:       return "')'";
        case TokenTuru::SOL_SUSE:        return "'{'";
        case TokenTuru::SAG_SUSE:        return "'}'";
        case TokenTuru::SOL_KOSELI:      return "'['";
        case TokenTuru::SAG_KOSELI:      return "']'";
        case TokenTuru::NOKTALI_VIRGUL:  return "';'";
        case TokenTuru::VIRGUL:          return "','";
        case TokenTuru::NOKTA:           return "'.'";
        case TokenTuru::IKI_NOKTA:       return "':'";
        case TokenTuru::ARTI_ARTI:       return "'++'";
        case TokenTuru::EKSI_EKSI:       return "'--'";
        case TokenTuru::ARTI_ESIT:       return "'+='";
        case TokenTuru::EKSI_ESIT:       return "'-='";
        case TokenTuru::CARPIM_ESIT:     return "'*='";
        case TokenTuru::BOLUM_ESIT:      return "'/='";
        case TokenTuru::DOSYA_SONU:      return "<dosya sonu>";
        case TokenTuru::BILINMEYEN:      return "<bilinmeyen>";
    }
    return "???";
}

// ============================================================
// Token::kapaticiMi
// Panik modlu hata kurtarmada "güvenli nokta" token'ları
// ============================================================
bool Token::kapaticiMi() const {
    switch (tur) {
        case TokenTuru::NOKTALI_VIRGUL:
        case TokenTuru::SAG_SUSE:
        case TokenTuru::SAG_PAREN:
        case TokenTuru::SAG_KOSELI:
        case TokenTuru::FONKSIYON:
        case TokenTuru::EGER:
        case TokenTuru::ICIN:
        case TokenTuru::DONGU:
        case TokenTuru::DONDUR:
        case TokenTuru::SINIF:
        case TokenTuru::DEGISKEN:
        case TokenTuru::DOSYA_SONU:
            return true;
        default:
            return false;
    }
}

// ============================================================
// Token::str — Debug/log çıktısı
// ============================================================
std::string Token::str() const {
    std::ostringstream ss;
    ss << "Token{"
       << tokenTuruAdi(tur)
       << ", \"" << lexem << "\""
       << ", " << satir << ":" << sutun
       << "}";
    return ss.str();
}

} // namespace Doruk


