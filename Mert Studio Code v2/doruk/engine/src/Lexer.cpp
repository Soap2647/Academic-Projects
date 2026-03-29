// ============================================================
// Lexer.cpp — DORUK Dili Sözdizimsel Tarayıcı
//
// Tasarım notları:
//   • Kaynak metin UTF-8 olarak işlenir. Türkçe karakterler
//     (ğ, ş, ı, ö, ü, ç, İ, Ğ, Ş, Ö, Ü, Ç) hem anahtar
//     kelimelerde hem de tanımlayıcılarda tam olarak desteklenir.
//   • İlerleme (ilerle()) her zaman satır/sütun sayacını günceller.
//   • Hata kurtarma: hatalı token BILINMEYEN olarak döndürülür,
//     tarama durdurmadan devam eder.
//   • Sütun numarası byte-offset'e dayanır (görsel değil).
// ============================================================
#include "Lexer.h"

#include <cassert>
#include <cctype>
#include <cstring>
#include <stdexcept>
#include <sstream>

namespace Doruk {

// ============================================================
// Anahtar kelime tablosu (statik, program ömrü boyunca yaşar)
// UTF-8 kodlu Türkçe → TokenTuru eşlemesi
// ============================================================
const std::unordered_map<std::string, TokenTuru>& Lexer::anahtarKelimeler() {
    // UTF-8 literalleri doğrudan C++ string olarak yazıyoruz.
    // Editör UTF-8 kaydettiğinde bu byte'lar doğru olacak.
    static const std::unordered_map<std::string, TokenTuru> tablo = {
        // Türkçe karşılıkları — her biri kaynak dosyadaki tam byte dizisi
        { "değişken",  TokenTuru::DEGISKEN   }, // değişken
        { "eğer",             TokenTuru::EGER        }, // eğer
        { "değilse",          TokenTuru::DEGILSE     }, // değilse
        { "döngü",     TokenTuru::DONGU       }, // döngü
        { "için",             TokenTuru::ICIN        }, // için
        { "fonksiyon",               TokenTuru::FONKSIYON   }, // fonksiyon
        { "döndür",    TokenTuru::DONDUR      }, // döndür
        { "yazdır",           TokenTuru::YAZDIR      }, // yazdır
        { "oku",                     TokenTuru::OKU         }, // oku
        { "doğru",            TokenTuru::DOGRU       }, // doğru
        { "yanlış",    TokenTuru::YANLIS      }, // yanlış
        { "boş",              TokenTuru::BOS         }, // boş
        { "ve",                      TokenTuru::VE          }, // ve
        { "veya",                    TokenTuru::VEYA        }, // veya
        { "değil",            TokenTuru::DEGIL       }, // değil
        { "kır",              TokenTuru::KIR         }, // kır
        { "devam",                   TokenTuru::DEVAM       }, // devam
        { "sınıf",     TokenTuru::SINIF       }, // sınıf
        { "yeni",                    TokenTuru::YENI        }, // yeni
        { "bu",                      TokenTuru::BU          }, // bu
    };
    return tablo;
}

// ============================================================
// Yapıcı ve sıfırlama
// ============================================================
Lexer::Lexer(std::string kaynak, std::string dosya)
    : kaynak_(std::move(kaynak))
    , dosya_(std::move(dosya))
    , konum_(0)
    , satir_(1)
    , sutun_(1)
    , satirBaslangic_(0)
{}

void Lexer::sifirla() {
    konum_          = 0;
    satir_          = 1;
    sutun_           = 1;
    satirBaslangic_  = 0;
    tokenler_.clear();
    tanilar_ = TaniListesi{};
}

// ============================================================
// Tara — ana giriş noktası
// ============================================================
LexerSonucu Lexer::tara() {
    sifirla();

    while (true) {
        Token tok = sonrakiToken();
        tokenler_.push_back(tok);
        if (tok.tur == TokenTuru::DOSYA_SONU) break;
    }

    return LexerSonucu{std::move(tokenler_), std::move(tanilar_)};
}

// ============================================================
// Yardımcı: Belirli bir satırın içeriğini döndür
// ============================================================
std::string Lexer::satirGetir(uint32_t satirNo) const {
    size_t pos   = 0;
    uint32_t cur = 1;
    while (pos < kaynak_.size() && cur < satirNo) {
        if (kaynak_[pos] == '\n') ++cur;
        ++pos;
    }
    size_t bas = pos;
    while (pos < kaynak_.size() && kaynak_[pos] != '\n') ++pos;
    return kaynak_.substr(bas, pos - bas);
}

// ============================================================
// İlerleme ve bakış yardımcıları
// ============================================================
bool Lexer::bitti() const noexcept {
    return konum_ >= kaynak_.size();
}

char Lexer::mevcutKar() const noexcept {
    if (bitti()) return '\0';
    return kaynak_[konum_];
}

char Lexer::sonrakiKar() const noexcept {
    if (konum_ + 1 >= kaynak_.size()) return '\0';
    return kaynak_[konum_ + 1];
}

char Lexer::sonrakiSonrakiKar() const noexcept {
    if (konum_ + 2 >= kaynak_.size()) return '\0';
    return kaynak_[konum_ + 2];
}

// ilerle — mevcut karakteri tüketir, konum günceller
char Lexer::ilerle() {
    char c = kaynak_[konum_++];
    if (c == '\n') {
        ++satir_;
        sutun_          = 1;
        satirBaslangic_ = konum_;
    } else {
        ++sutun_;
    }
    return c;
}

// eslestir — mevcut karakter beklenenle eşleşiyorsa tüket
bool Lexer::eslestir(char beklenen) {
    if (bitti() || mevcutKar() != beklenen) return false;
    ilerle();
    return true;
}

// ============================================================
// UTF-8 yardımcıları
// ============================================================

// utf8CokByteMi — çok-byte'lı UTF-8 dizisinin başlangıç byte'ı mı?
bool Lexer::utf8CokByteMi(unsigned char b) noexcept {
    // 110xxxxx (2-byte), 1110xxxx (3-byte), 11110xxx (4-byte)
    return (b & 0xC0) == 0xC0;
}

// utf8MevcutBak — ilerletmeden mevcut kod noktasını oku
uint32_t Lexer::utf8MevcutBak() const {
    if (bitti()) return 0;

    unsigned char b0 = static_cast<unsigned char>(kaynak_[konum_]);

    // 1-byte (ASCII)
    if ((b0 & 0x80) == 0) return b0;

    // 2-byte: 110xxxxx 10xxxxxx
    if ((b0 & 0xE0) == 0xC0) {
        if (konum_ + 1 >= kaynak_.size()) return 0xFFFD;
        unsigned char b1 = static_cast<unsigned char>(kaynak_[konum_ + 1]);
        if ((b1 & 0xC0) != 0x80) return 0xFFFD;
        return ((b0 & 0x1F) << 6) | (b1 & 0x3F);
    }

    // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
    if ((b0 & 0xF0) == 0xE0) {
        if (konum_ + 2 >= kaynak_.size()) return 0xFFFD;
        unsigned char b1 = static_cast<unsigned char>(kaynak_[konum_ + 1]);
        unsigned char b2 = static_cast<unsigned char>(kaynak_[konum_ + 2]);
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) return 0xFFFD;
        return ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
    }

    // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if ((b0 & 0xF8) == 0xF0) {
        if (konum_ + 3 >= kaynak_.size()) return 0xFFFD;
        unsigned char b1 = static_cast<unsigned char>(kaynak_[konum_ + 1]);
        unsigned char b2 = static_cast<unsigned char>(kaynak_[konum_ + 2]);
        unsigned char b3 = static_cast<unsigned char>(kaynak_[konum_ + 3]);
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80)
            return 0xFFFD;
        return ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) |
               ((b2 & 0x3F) << 6)  |  (b3 & 0x3F);
    }

    return 0xFFFD; // geçersiz UTF-8
}

// utf8OkuIlerle — bir kod noktasını okur ve konumu ilerletir
uint32_t Lexer::utf8OkuIlerle() {
    if (bitti()) return 0;

    unsigned char b0 = static_cast<unsigned char>(mevcutKar());

    // 1-byte ASCII
    if ((b0 & 0x80) == 0) {
        ilerle();
        return b0;
    }

    // 2-byte
    if ((b0 & 0xE0) == 0xC0) {
        ilerle(); // b0
        if (bitti()) return 0xFFFD;
        unsigned char b1 = static_cast<unsigned char>(mevcutKar());
        if ((b1 & 0xC0) != 0x80) return 0xFFFD;
        ilerle(); // b1
        return ((b0 & 0x1F) << 6) | (b1 & 0x3F);
    }

    // 3-byte
    if ((b0 & 0xF0) == 0xE0) {
        ilerle(); // b0
        if (bitti()) return 0xFFFD;
        unsigned char b1 = static_cast<unsigned char>(mevcutKar());
        if ((b1 & 0xC0) != 0x80) return 0xFFFD;
        ilerle(); // b1
        if (bitti()) return 0xFFFD;
        unsigned char b2 = static_cast<unsigned char>(mevcutKar());
        if ((b2 & 0xC0) != 0x80) return 0xFFFD;
        ilerle(); // b2
        return ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
    }

    // 4-byte
    if ((b0 & 0xF8) == 0xF0) {
        ilerle(); // b0
        if (bitti()) return 0xFFFD;
        unsigned char b1 = static_cast<unsigned char>(mevcutKar());
        if ((b1 & 0xC0) != 0x80) return 0xFFFD;
        ilerle(); // b1
        if (bitti()) return 0xFFFD;
        unsigned char b2 = static_cast<unsigned char>(mevcutKar());
        if ((b2 & 0xC0) != 0x80) return 0xFFFD;
        ilerle(); // b2
        if (bitti()) return 0xFFFD;
        unsigned char b3 = static_cast<unsigned char>(mevcutKar());
        if ((b3 & 0xC0) != 0x80) return 0xFFFD;
        ilerle(); // b3
        return ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) |
               ((b2 & 0x3F) << 6)  |  (b3 & 0x3F);
    }

    // Geçersiz byte — atla
    ilerle();
    return 0xFFFD;
}

// kimlikBaslangiciMi — tanımlayıcı başlangıç karakteri mi?
// ASCII: a-z A-Z _
// Latin Supplement + Extended: U+00C0-U+024F (Türkçe karakterler dahil)
// Genel kural: BMP harfler (basit kontrol)
bool Lexer::kimlikBaslangiciMi(uint32_t cp) noexcept {
    if (cp < 0x80) {
        // ASCII
        return std::isalpha(static_cast<unsigned char>(cp)) || cp == '_';
    }
    // Latin-1 Supplement (U+00C0..U+00FF) — Türkçe büyük/küçük
    // Latin Extended-A (U+0100..U+017F) — ğ Ğ ş Ş ı İ gibi karakterler
    // Latin Extended-B (U+0180..U+024F)
    // Basic Latin tabanlı dil karakterleri
    if (cp >= 0x00C0 && cp <= 0x024F) return true;
    // Kiril, Çince vb. için genel BMP harf kontrolü
    // (0x0250 ve üzeri — ileriki sürümlerde genişletilebilir)
    if (cp >= 0x0370 && cp <= 0xFFFF) return true;
    return false;
}

// kimlikDevamMi — tanımlayıcı devam karakteri mi?
bool Lexer::kimlikDevamMi(uint32_t cp) noexcept {
    if (cp < 0x80) {
        return std::isalnum(static_cast<unsigned char>(cp)) || cp == '_';
    }
    return kimlikBaslangiciMi(cp);
}

// ============================================================
// Boşluk ve yorum atlama
// ============================================================
void Lexer::bosluklarAtla() {
    while (!bitti()) {
        char c = mevcutKar();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            ilerle();
        } else if (c == '/' && sonrakiKar() == '/') {
            satirYorumAtla();
        } else if (c == '/' && sonrakiKar() == '*') {
            blokYorumAtla();
        } else {
            break;
        }
    }
}

void Lexer::satirYorumAtla() {
    // // ile başlayan; satır sonuna kadar tüket
    while (!bitti() && mevcutKar() != '\n') {
        ilerle();
    }
}

void Lexer::blokYorumAtla() {
    uint32_t basSatir = satir_;
    uint32_t basSutun = sutun_;

    ilerle(); // '/'
    ilerle(); // '*'

    while (!bitti()) {
        if (mevcutKar() == '*' && sonrakiKar() == '/') {
            ilerle(); // '*'
            ilerle(); // '/'
            return;
        }
        ilerle();
    }

    // Kapatılmamış blok yorumu
    hataEkle(basSatir, basSutun, 2,
              "Kapatilmamiş blok yorumu",       // Kapatılmamış blok yorumu
              "'*/' ile kapatilmali.");                  // '*/' ile kapatılmalı.
}

// ============================================================
// Hata ekleme yardımcısı
// ============================================================
void Lexer::hataEkle(uint32_t satir,
                     uint32_t sutun,
                     uint32_t uzunluk,
                     std::string mesaj,
                     std::string ipucu) {
    tanilar_.ekle(lexerHatasi(std::move(mesaj), dosya_,
                               satir, sutun, uzunluk,
                               std::move(ipucu)));
}

// ============================================================
// Ana token üreticisi
// ============================================================
Token Lexer::sonrakiToken() {
    bosluklarAtla();

    if (bitti()) {
        return Token(TokenTuru::DOSYA_SONU, "", satir_, sutun_);
    }

    uint32_t basSatir = satir_;
    uint32_t basSutun = sutun_;
    char     c        = mevcutKar();

    // ── Metin literal ──────────────────────────────────────
    if (c == '"' || c == '\'') {
        return metinTara(basSatir, basSutun);
    }

    // ── Sayı literal ────────────────────────────────────────
    if (std::isdigit(static_cast<unsigned char>(c))) {
        return sayiTara(basSatir, basSutun);
    }

    // ── Tanımlayıcı veya anahtar kelime ─────────────────────
    uint32_t cp = utf8MevcutBak();
    if (kimlikBaslangiciMi(cp)) {
        return kimlikTara(basSatir, basSutun);
    }

    // ── Operatörler ve noktalama ─────────────────────────────
    ilerle(); // karakteri tüket

    switch (c) {
        // Tek karakterlik
        case '(': return Token(TokenTuru::SOL_PAREN,      "(", basSatir, basSutun);
        case ')': return Token(TokenTuru::SAG_PAREN,      ")", basSatir, basSutun);
        case '{': return Token(TokenTuru::SOL_SUSE,       "{", basSatir, basSutun);
        case '}': return Token(TokenTuru::SAG_SUSE,       "}", basSatir, basSutun);
        case '[': return Token(TokenTuru::SOL_KOSELI,     "[", basSatir, basSutun);
        case ']': return Token(TokenTuru::SAG_KOSELI,     "]", basSatir, basSutun);
        case ';': return Token(TokenTuru::NOKTALI_VIRGUL, ";", basSatir, basSutun);
        case ',': return Token(TokenTuru::VIRGUL,         ",", basSatir, basSutun);
        case '.': return Token(TokenTuru::NOKTA,          ".", basSatir, basSutun);
        case ':': return Token(TokenTuru::IKI_NOKTA,      ":", basSatir, basSutun);
        case '%': return Token(TokenTuru::MOD,            "%", basSatir, basSutun);

        // + veya ++ veya +=
        case '+':
            if (eslestir('+')) return Token(TokenTuru::ARTI_ARTI,  "++", basSatir, basSutun);
            if (eslestir('=')) return Token(TokenTuru::ARTI_ESIT,  "+=", basSatir, basSutun);
            return Token(TokenTuru::ARTI, "+", basSatir, basSutun);

        // - veya -- veya -=
        case '-':
            if (eslestir('-')) return Token(TokenTuru::EKSI_EKSI,  "--", basSatir, basSutun);
            if (eslestir('=')) return Token(TokenTuru::EKSI_ESIT,  "-=", basSatir, basSutun);
            return Token(TokenTuru::EKSI, "-", basSatir, basSutun);

        // * veya *=
        case '*':
            if (eslestir('=')) return Token(TokenTuru::CARPIM_ESIT, "*=", basSatir, basSutun);
            return Token(TokenTuru::CARPIM, "*", basSatir, basSutun);

        // / veya /=  (// ve /* zaten yukarıda atlandı)
        case '/':
            if (eslestir('=')) return Token(TokenTuru::BOLUM_ESIT, "/=", basSatir, basSutun);
            return Token(TokenTuru::BOLUM, "/", basSatir, basSutun);

        // = veya ==
        case '=':
            if (eslestir('=')) return Token(TokenTuru::ESIT_ESIT, "==", basSatir, basSutun);
            return Token(TokenTuru::ESIT, "=", basSatir, basSutun);

        // ! veya !=
        case '!':
            if (eslestir('=')) return Token(TokenTuru::ESIT_DEGIL, "!=", basSatir, basSutun);
            return Token(TokenTuru::UNLEM, "!", basSatir, basSutun);

        // < veya <=
        case '<':
            if (eslestir('=')) return Token(TokenTuru::KUCUK_ESIT, "<=", basSatir, basSutun);
            return Token(TokenTuru::KUCUK, "<", basSatir, basSutun);

        // > veya >=
        case '>':
            if (eslestir('=')) return Token(TokenTuru::BUYUK_ESIT, ">=", basSatir, basSutun);
            return Token(TokenTuru::BUYUK, ">", basSatir, basSutun);

        // && (sembolik AND)
        case '&':
            if (eslestir('&')) return Token(TokenTuru::VE_VE, "&&", basSatir, basSutun);
            {
                std::string lexem(1, c);
                hataEkle(basSatir, basSutun, 1,
                         "Beklenmeyen karakter: '&'",
                         "Mantıksal VE için '&&' veya 've' kullanın.");
                return Token(TokenTuru::BILINMEYEN, lexem, basSatir, basSutun);
            }

        // || (sembolik OR)
        case '|':
            if (eslestir('|')) return Token(TokenTuru::VEYA_VEYA, "||", basSatir, basSutun);
            {
                std::string lexem(1, c);
                hataEkle(basSatir, basSutun, 1,
                         "Beklenmeyen karakter: '|'",
                         "Mantıksal VEYA için '||' veya 'veya' kullanın.");
                return Token(TokenTuru::BILINMEYEN, lexem, basSatir, basSutun);
            }

        // Tanınmayan ASCII karakter
        default: {
            std::string lexem(1, c);
            std::ostringstream msg;
            msg << "Beklenmeyen karakter: '"
                << c
                << "' (ASCII: "
                << static_cast<int>(static_cast<unsigned char>(c))
                << ")";
            hataEkle(basSatir, basSutun, 1, msg.str());
            return Token(TokenTuru::BILINMEYEN, lexem, basSatir, basSutun);
        }
    }
}

// ============================================================
// Metin literal tarayıcı
// Desteklenen çıkış dizileri: \n \t \r \\ \" \' \0 \uXXXX
// ============================================================
Token Lexer::metinTara(uint32_t basSatir, uint32_t basSutun) {
    char kapatici = mevcutKar(); // " veya '
    ilerle();                    // açış tırnağını tüket

    std::string deger;
    deger.reserve(32);

    while (!bitti() && mevcutKar() != kapatici) {
        char c = mevcutKar();

        if (c == '\n' || c == '\r') {
            // Çok satırlı metin desteklenmiyor (şimdilik)
            hataEkle(basSatir, basSutun, 1,
                     "Metin literali satır sonundan önce kapatılmadı.",
                     "Metni aynı satırda kapatın.");
            break;
        }

        if (c == '\\') {
            ilerle(); // '\' tüket
            if (bitti()) {
                hataEkle(basSatir, basSutun, 1,
                         "Çıkış dizisi dosya sonunda bitti.");
                break;
            }
            char esc = ilerle();
            switch (esc) {
                case 'n':  deger += '\n'; break;
                case 't':  deger += '\t'; break;
                case 'r':  deger += '\r'; break;
                case '\\': deger += '\\'; break;
                case '"':  deger += '"';  break;
                case '\'': deger += '\''; break;
                case '0':  deger += '\0'; break;
                case 'u':
                case 'U': {
                    // \uXXXX veya \UXXXXXXXX — Unicode kod noktası
                    int maxDigit = (esc == 'u') ? 4 : 8;
                    uint32_t kodNoktasi = 0;
                    int okunan = 0;
                    for (; okunan < maxDigit && !bitti(); ++okunan) {
                        char h = mevcutKar();
                        uint32_t dgt = 0;
                        if (h >= '0' && h <= '9')      dgt = h - '0';
                        else if (h >= 'a' && h <= 'f') dgt = h - 'a' + 10;
                        else if (h >= 'A' && h <= 'F') dgt = h - 'A' + 10;
                        else break;
                        ilerle();
                        kodNoktasi = (kodNoktasi << 4) | dgt;
                    }
                    if (okunan == 0) {
                        hataEkle(satir_, sutun_, 1,
                                 "\\u sonrasında hex basamak bekleniyor.");
                        break;
                    }
                    // UTF-8 olarak encode et
                    if (kodNoktasi < 0x80) {
                        deger += static_cast<char>(kodNoktasi);
                    } else if (kodNoktasi < 0x800) {
                        deger += static_cast<char>(0xC0 | (kodNoktasi >> 6));
                        deger += static_cast<char>(0x80 | (kodNoktasi & 0x3F));
                    } else if (kodNoktasi < 0x10000) {
                        deger += static_cast<char>(0xE0 | (kodNoktasi >> 12));
                        deger += static_cast<char>(0x80 | ((kodNoktasi >> 6) & 0x3F));
                        deger += static_cast<char>(0x80 | (kodNoktasi & 0x3F));
                    } else {
                        deger += static_cast<char>(0xF0 | (kodNoktasi >> 18));
                        deger += static_cast<char>(0x80 | ((kodNoktasi >> 12) & 0x3F));
                        deger += static_cast<char>(0x80 | ((kodNoktasi >> 6)  & 0x3F));
                        deger += static_cast<char>(0x80 | (kodNoktasi & 0x3F));
                    }
                    break;
                }
                default: {
                    std::ostringstream msg;
                    msg << "Bilinmeyen kaçış dizisi: '\\" << esc << "'";
                    hataEkle(satir_, sutun_, 2, msg.str());
                    deger += esc; // kurtarma: karakteri olduğu gibi ekle
                    break;
                }
            }
        } else if ((static_cast<unsigned char>(c) & 0x80) != 0) {
            // UTF-8 çok-byte'lı karakter — olduğu gibi kopyala
            uint32_t cp = utf8OkuIlerle();
            // UTF-8 geri encode et (zaten kaynak byte'ları kopyalamak daha basit)
            // utf8OkuIlerle zaten konumu ilerletti; deger'e byte'ları ekle
            // Aslında kaynak byte'larını doğrudan kopyalamak daha verimli.
            // Yukarıdaki utf8OkuIlerle çağrısından önce konum zaten ilerledi.
            // Kod noktasını geri UTF-8'e çevirelim:
            if (cp < 0x80) {
                deger += static_cast<char>(cp);
            } else if (cp < 0x800) {
                deger += static_cast<char>(0xC0 | (cp >> 6));
                deger += static_cast<char>(0x80 | (cp & 0x3F));
            } else if (cp < 0x10000) {
                deger += static_cast<char>(0xE0 | (cp >> 12));
                deger += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                deger += static_cast<char>(0x80 | (cp & 0x3F));
            } else {
                deger += static_cast<char>(0xF0 | (cp >> 18));
                deger += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                deger += static_cast<char>(0x80 | ((cp >> 6)  & 0x3F));
                deger += static_cast<char>(0x80 | (cp & 0x3F));
            }
        } else {
            deger += c;
            ilerle();
        }
    }

    if (bitti() && mevcutKar() != kapatici) {
        hataEkle(basSatir, basSutun, 1,
                 "Metin literali dosya sonunda kapatılmadı.",
                 "Eksik kapatış tırnağı.");
    } else {
        ilerle(); // kapanış tırnağını tüket
    }

    return Token(TokenTuru::METIN_LIT, std::move(deger), basSatir, basSutun);
}

// ============================================================
// Sayı literal tarayıcı
// Destekler: 42, 0, 3.14, 0.5, 1_000_000 (alt çizgi ayırıcı)
// ============================================================
Token Lexer::sayiTara(uint32_t basSatir, uint32_t basSutun) {
    std::string lexem;
    bool ondalikVar = false;

    // Hex literal: 0x veya 0X
    if (mevcutKar() == '0' &&
        (sonrakiKar() == 'x' || sonrakiKar() == 'X')) {
        lexem += ilerle(); // '0'
        lexem += ilerle(); // 'x'
        while (!bitti()) {
            char c = mevcutKar();
            if ((c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F') ||
                c == '_') {
                if (c != '_') lexem += c;
                ilerle();
            } else {
                break;
            }
        }
        return Token(TokenTuru::TAM_SAYI_LIT, std::move(lexem), basSatir, basSutun);
    }

    // Ondalık/tam sayı
    while (!bitti()) {
        char c = mevcutKar();
        if (std::isdigit(static_cast<unsigned char>(c))) {
            lexem += c;
            ilerle();
        } else if (c == '_') {
            // Alt çizgi ayırıcı — lexem'e ekleme, sadece atla
            ilerle();
        } else if (c == '.' && !ondalikVar) {
            // Ondalık nokta: sonraki karakter de rakam olmalı
            if (std::isdigit(static_cast<unsigned char>(sonrakiKar()))) {
                ondalikVar = true;
                lexem += c;
                ilerle();
            } else {
                break; // nokta sayının parçası değil
            }
        } else if ((c == 'e' || c == 'E') && !lexem.empty()) {
            // Bilimsel gösterim: 1.5e10 veya 1e-3
            lexem += c;
            ilerle();
            ondalikVar = true;
            if (!bitti() && (mevcutKar() == '+' || mevcutKar() == '-')) {
                lexem += ilerle();
            }
            // Üs rakamları
            while (!bitti() && std::isdigit(static_cast<unsigned char>(mevcutKar()))) {
                lexem += ilerle();
            }
            break;
        } else {
            break;
        }
    }

    if (ondalikVar) {
        return Token(TokenTuru::ONDALIK_LIT, std::move(lexem), basSatir, basSutun);
    }
    return Token(TokenTuru::TAM_SAYI_LIT, std::move(lexem), basSatir, basSutun);
}

// ============================================================
// Tanımlayıcı veya anahtar kelime tarayıcı
// ============================================================
Token Lexer::kimlikTara(uint32_t basSatir, uint32_t basSutun) {
    // Kaynak byte konumunu kaydet — lexem için slice alacağız
    size_t bas = konum_;

    // İlk kod noktasını tüket (zaten başlangıç olduğu doğrulandı)
    utf8OkuIlerle();

    // Devam karakterlerini tüket
    while (!bitti() && kimlikDevamMi(utf8MevcutBak())) {
        utf8OkuIlerle();
    }

    // Kaynak metinden slice al — ham byte'lar (UTF-8)
    std::string lexem = kaynak_.substr(bas, konum_ - bas);

    // Anahtar kelime tablosunda ara
    const auto& tablo = anahtarKelimeler();
    auto it = tablo.find(lexem);
    if (it != tablo.end()) {
        return Token(it->second, std::move(lexem), basSatir, basSutun);
    }

    return Token(TokenTuru::TANIMLAYICI, std::move(lexem), basSatir, basSutun);
}

} // namespace Doruk


