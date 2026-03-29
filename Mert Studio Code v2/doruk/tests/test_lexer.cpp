// ============================================================
// test_lexer.cpp — DORUK Lexer Test Paketi
// Framework: Catch2 v3
//
// Test kategorileri:
//   [boşluk]     — boşluk ve yorum atlama
//   [anahtar]    — Türkçe anahtar kelimeler
//   [kimlik]     — tanımlayıcılar (ASCII + Türkçe)
//   [tam_sayi]   — tam sayı literalleri
//   [ondalik]    — ondalık literalleri
//   [metin]      — metin literalleri ve kaçış dizileri
//   [operatör]   — operatörler
//   [noktalama]  — noktalama işaretleri
//   [hata]       — hata kurtarma
//   [konum]      — satır/sütun takibi
//   [entegrasyon]— gerçek DORUK kodu parçaları
// ============================================================
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "Lexer.h"
#include "Token.h"
#include "Diagnostics.h"

using namespace Doruk;

// ============================================================
// Test yardımcıları
// ============================================================
static LexerSonucu tara(const std::string& kaynak) {
    Lexer lx(kaynak, "test.drk");
    return lx.tara();
}

// Bir kaynaktaki tüm token türlerini döndürür (DOSYA_SONU hariç)
static std::vector<TokenTuru> tokenTurleri(const std::string& kaynak) {
    auto sonuc = tara(kaynak);
    std::vector<TokenTuru> turler;
    for (const auto& tok : sonuc.tokenler) {
        if (tok.tur != TokenTuru::DOSYA_SONU)
            turler.push_back(tok.tur);
    }
    return turler;
}

// Tek token için kısayol — lexem döndürür
static std::string ilkLexem(const std::string& kaynak) {
    auto sonuc = tara(kaynak);
    if (sonuc.tokenler.empty()) return "";
    return sonuc.tokenler[0].lexem;
}

// ============================================================
// BÖLÜM 1: Temel tarama
// ============================================================
TEST_CASE("Boş kaynak yalnızca DOSYA_SONU döndürür", "[temel]") {
    auto sonuc = tara("");
    REQUIRE(sonuc.tokenler.size() == 1);
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::DOSYA_SONU);
    REQUIRE(sonuc.basarili());
}

TEST_CASE("Yalnızca boşluk yalnızca DOSYA_SONU döndürür", "[boşluk]") {
    auto sonuc = tara("   \t\n\r\n   ");
    REQUIRE(sonuc.tokenler.size() == 1);
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::DOSYA_SONU);
}

TEST_CASE("Satır yorumu atlanır", "[boşluk]") {
    auto turler = tokenTurleri("// bu bir yorum\n42");
    REQUIRE(turler.size() == 1);
    REQUIRE(turler[0] == TokenTuru::TAM_SAYI_LIT);
}

TEST_CASE("Birden fazla satır yorumu atlanır", "[boşluk]") {
    const char* kaynak =
        "// birinci yorum\n"
        "// ikinci yorum\n"
        "123\n"
        "// üçüncü yorum\n";
    auto turler = tokenTurleri(kaynak);
    REQUIRE(turler.size() == 1);
    REQUIRE(turler[0] == TokenTuru::TAM_SAYI_LIT);
}

TEST_CASE("Blok yorumu atlanır", "[boşluk]") {
    auto turler = tokenTurleri("/* bu bir blok */ 42");
    REQUIRE(turler.size() == 1);
    REQUIRE(turler[0] == TokenTuru::TAM_SAYI_LIT);
}

TEST_CASE("Çok satırlı blok yorumu atlanır", "[boşluk]") {
    const char* kaynak =
        "/* \n"
        "   bu çok satırlı\n"
        "   bir yorum\n"
        "*/ 99";
    auto turler = tokenTurleri(kaynak);
    REQUIRE(turler.size() == 1);
    REQUIRE(turler[0] == TokenTuru::TAM_SAYI_LIT);
}

TEST_CASE("Kapatılmamış blok yorumu hata üretir", "[boşluk][hata]") {
    auto sonuc = tara("/* kapatılmamış yorum 42");
    REQUIRE(sonuc.tanilar.hatalarVar());
    REQUIRE(sonuc.tanilar.hataSayisi() == 1);
}

// ============================================================
// BÖLÜM 2: Anahtar Kelimeler
// ============================================================
TEST_CASE("Tüm Türkçe anahtar kelimeler tanınır", "[anahtar]") {
    struct Beklenti { const char* kaynak; TokenTuru beklenenTur; };

    // UTF-8 encoded Türkçe anahtar kelimeler
    static const Beklenti beklentiler[] = {
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

    for (const auto& b : beklentiler) {
        CAPTURE(b.kaynak);
        auto sonuc = tara(b.kaynak);
        REQUIRE(!sonuc.tokenler.empty());
        REQUIRE(sonuc.tokenler[0].tur == b.beklenenTur);
        REQUIRE(sonuc.basarili());
    }
}

TEST_CASE("Anahtar kelime lexemi kaynak metniyle eşleşir", "[anahtar]") {
    // değişken
    auto sonuc = tara("değişken");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::DEGISKEN);
    REQUIRE(sonuc.tokenler[0].lexem == "değişken");
}

TEST_CASE("Anahtar kelimeden büyük harfli identifier farklı tanınır", "[anahtar]") {
    // "VE" → identifier (büyük harf farklı)
    auto sonuc = tara("VE");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TANIMLAYICI);
}

TEST_CASE("doğru ve yanlış boolean literalleri", "[anahtar]") {
    auto sonuc = tara("doğru yanlış");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::DOGRU);
    REQUIRE(sonuc.tokenler[1].tur == TokenTuru::YANLIS);
}

TEST_CASE("boş null token", "[anahtar]") {
    auto sonuc = tara("boş");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::BOS);
}

// ============================================================
// BÖLÜM 3: Tanımlayıcılar
// ============================================================
TEST_CASE("ASCII tanımlayıcı", "[kimlik]") {
    auto sonuc = tara("sayi");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(sonuc.tokenler[0].lexem == "sayi");
}

TEST_CASE("Alt çizgili tanımlayıcı", "[kimlik]") {
    auto sonuc = tara("_gizli_degisken");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(sonuc.tokenler[0].lexem == "_gizli_degisken");
}

TEST_CASE("Alt çizgi ile başlayan tanımlayıcı", "[kimlik]") {
    auto sonuc = tara("_x");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TANIMLAYICI);
}

TEST_CASE("Türkçe karakterli tanımlayıcı", "[kimlik]") {
    // sayaç — s a y a ç (UTF-8: 73 61 79 61 C3 A7)
    const char* kimlik = "sayaç";
    auto sonuc = tara(kimlik);
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(sonuc.tokenler[0].lexem == kimlik);
}

TEST_CASE("Türkçe büyük harfli tanımlayıcı", "[kimlik]") {
    // İsim — UTF-8: C4 B0 73 69 6D
    const char* kimlik = "İsim";
    auto sonuc = tara(kimlik);
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(sonuc.tokenler[0].lexem == kimlik);
}

TEST_CASE("Rakamla bitmek üzere tanımlayıcı", "[kimlik]") {
    auto sonuc = tara("x1y2z3");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(sonuc.tokenler[0].lexem == "x1y2z3");
}

TEST_CASE("Rakamla başlayan string identifier değil, sayı", "[kimlik]") {
    auto turler = tokenTurleri("123abc");
    // 123 = TAM_SAYI, abc = TANIMLAYICI
    REQUIRE(turler.size() == 2);
    REQUIRE(turler[0] == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(turler[1] == TokenTuru::TANIMLAYICI);
}

TEST_CASE("Birden fazla tanımlayıcı boşlukla ayrılır", "[kimlik]") {
    auto sonuc = tara("x y z");
    REQUIRE(sonuc.tokenler.size() == 4); // x y z DOSYA_SONU
    REQUIRE(sonuc.tokenler[0].lexem == "x");
    REQUIRE(sonuc.tokenler[1].lexem == "y");
    REQUIRE(sonuc.tokenler[2].lexem == "z");
}

// ============================================================
// BÖLÜM 4: Tam Sayı Literalleri
// ============================================================
TEST_CASE("Sıfır tam sayı", "[tam_sayi]") {
    auto sonuc = tara("0");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "0");
}

TEST_CASE("Pozitif tam sayı", "[tam_sayi]") {
    auto sonuc = tara("42");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "42");
}

TEST_CASE("Büyük tam sayı", "[tam_sayi]") {
    auto sonuc = tara("9999999999");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "9999999999");
}

TEST_CASE("Alt çizgi ayırıcılı sayı", "[tam_sayi]") {
    // 1_000_000 → lexem = "1000000" (alt çizgiler çıkarılır)
    auto sonuc = tara("1_000_000");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "1000000");
}

TEST_CASE("Hex literal", "[tam_sayi]") {
    auto sonuc = tara("0xFF");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "0xFF");
}

TEST_CASE("Hex literal küçük harf", "[tam_sayi]") {
    auto sonuc = tara("0xdeadbeef");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "0xdeadbeef");
}

// ============================================================
// BÖLÜM 5: Ondalık Literalleri
// ============================================================
TEST_CASE("Basit ondalık", "[ondalik]") {
    auto sonuc = tara("3.14");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::ONDALIK_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "3.14");
}

TEST_CASE("Sıfır nokta değer", "[ondalik]") {
    auto sonuc = tara("0.5");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::ONDALIK_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "0.5");
}

TEST_CASE("Bilimsel gösterim", "[ondalik]") {
    auto sonuc = tara("1.5e10");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::ONDALIK_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "1.5e10");
}

TEST_CASE("Negatif üslü bilimsel gösterim", "[ondalik]") {
    auto sonuc = tara("2.718e-5");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::ONDALIK_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "2.718e-5");
}

TEST_CASE("Bilimsel gösterim E büyük", "[ondalik]") {
    auto sonuc = tara("1E3");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::ONDALIK_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "1E3");
}

TEST_CASE("Nokta sayının parçası değilse ayrı token", "[ondalik]") {
    // 42. → TAM_SAYI . (nokta sayının parçası değil, sonraki char rakam değil)
    auto turler = tokenTurleri("42.");
    REQUIRE(turler.size() == 2);
    REQUIRE(turler[0] == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(turler[1] == TokenTuru::NOKTA);
}

// ============================================================
// BÖLÜM 6: Metin Literalleri
// ============================================================
TEST_CASE("Boş metin", "[metin]") {
    auto sonuc = tara("\"\"");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "");
}

TEST_CASE("Basit ASCII metin", "[metin]") {
    auto sonuc = tara("\"merhaba\"");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "merhaba");
}

TEST_CASE("Türkçe karakterli metin", "[metin]") {
    // "merhaba dünya"
    auto sonuc = tara("\"merhaba dünya\"");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "merhaba dünya");
}

TEST_CASE("Tek tırnaklı metin", "[metin]") {
    auto sonuc = tara("'merhaba'");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "merhaba");
}

TEST_CASE("Yeni satır kaçış dizisi", "[metin]") {
    auto sonuc = tara("\"sat\\nr1\"");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "sat\nr1");
}

TEST_CASE("Tab kaçış dizisi", "[metin]") {
    auto sonuc = tara("\"a\\tb\"");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "a\tb");
}

TEST_CASE("Ters bölü kaçış dizisi", "[metin]") {
    auto sonuc = tara("\"a\\\\b\"");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "a\\b");
}

TEST_CASE("Çift tırnak kaçış dizisi", "[metin]") {
    auto sonuc = tara("\"o \\\"dedi\\\"\"");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[0].lexem == "o \"dedi\"");
}

TEST_CASE("\\u Unicode kaçış dizisi", "[metin]") {
    // \u00FC = ü
    auto sonuc = tara("\"\\u00FC\"");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::METIN_LIT);
    // ü = 0xC3 0xBC
    REQUIRE(sonuc.tokenler[0].lexem == "ü");
}

TEST_CASE("Kapatılmamış metin hata üretir", "[metin][hata]") {
    auto sonuc = tara("\"bitmemiş");
    REQUIRE(sonuc.tanilar.hatalarVar());
}

TEST_CASE("Satır sonu içeren metin hata üretir", "[metin][hata]") {
    auto sonuc = tara("\"satır\nsonu\"");
    REQUIRE(sonuc.tanilar.hatalarVar());
}

// ============================================================
// BÖLÜM 7: Operatörler
// ============================================================
TEST_CASE("Aritmetik operatörler", "[operatör]") {
    auto turler = tokenTurleri("+ - * / %");
    REQUIRE(turler.size() == 5);
    REQUIRE(turler[0] == TokenTuru::ARTI);
    REQUIRE(turler[1] == TokenTuru::EKSI);
    REQUIRE(turler[2] == TokenTuru::CARPIM);
    REQUIRE(turler[3] == TokenTuru::BOLUM);
    REQUIRE(turler[4] == TokenTuru::MOD);
}

TEST_CASE("Karşılaştırma operatörleri", "[operatör]") {
    auto turler = tokenTurleri("== != < <= > >=");
    REQUIRE(turler.size() == 6);
    REQUIRE(turler[0] == TokenTuru::ESIT_ESIT);
    REQUIRE(turler[1] == TokenTuru::ESIT_DEGIL);
    REQUIRE(turler[2] == TokenTuru::KUCUK);
    REQUIRE(turler[3] == TokenTuru::KUCUK_ESIT);
    REQUIRE(turler[4] == TokenTuru::BUYUK);
    REQUIRE(turler[5] == TokenTuru::BUYUK_ESIT);
}

TEST_CASE("Atama operatörü", "[operatör]") {
    auto sonuc = tara("=");
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::ESIT);
}

TEST_CASE("Atama eşit ile eşitlik karıştırılmamalı", "[operatör]") {
    auto turler = tokenTurleri("= ==");
    REQUIRE(turler[0] == TokenTuru::ESIT);
    REQUIRE(turler[1] == TokenTuru::ESIT_ESIT);
}

TEST_CASE("Mantıksal operatörler sembolik", "[operatör]") {
    auto turler = tokenTurleri("&& ||");
    REQUIRE(turler[0] == TokenTuru::VE_VE);
    REQUIRE(turler[1] == TokenTuru::VEYA_VEYA);
}

TEST_CASE("Ünlem ve eşit değil", "[operatör]") {
    auto turler = tokenTurleri("! !=");
    REQUIRE(turler[0] == TokenTuru::UNLEM);
    REQUIRE(turler[1] == TokenTuru::ESIT_DEGIL);
}

TEST_CASE("Artırma ve azaltma operatörleri", "[operatör]") {
    auto turler = tokenTurleri("++ --");
    REQUIRE(turler[0] == TokenTuru::ARTI_ARTI);
    REQUIRE(turler[1] == TokenTuru::EKSI_EKSI);
}

TEST_CASE("Bileşik atama operatörleri", "[operatör]") {
    auto turler = tokenTurleri("+= -= *= /=");
    REQUIRE(turler[0] == TokenTuru::ARTI_ESIT);
    REQUIRE(turler[1] == TokenTuru::EKSI_ESIT);
    REQUIRE(turler[2] == TokenTuru::CARPIM_ESIT);
    REQUIRE(turler[3] == TokenTuru::BOLUM_ESIT);
}

// ============================================================
// BÖLÜM 8: Noktalama
// ============================================================
TEST_CASE("Tüm noktalama işaretleri", "[noktalama]") {
    auto turler = tokenTurleri("( ) { } [ ] ; , . :");
    REQUIRE(turler.size() == 10);
    REQUIRE(turler[0] == TokenTuru::SOL_PAREN);
    REQUIRE(turler[1] == TokenTuru::SAG_PAREN);
    REQUIRE(turler[2] == TokenTuru::SOL_SUSE);
    REQUIRE(turler[3] == TokenTuru::SAG_SUSE);
    REQUIRE(turler[4] == TokenTuru::SOL_KOSELI);
    REQUIRE(turler[5] == TokenTuru::SAG_KOSELI);
    REQUIRE(turler[6] == TokenTuru::NOKTALI_VIRGUL);
    REQUIRE(turler[7] == TokenTuru::VIRGUL);
    REQUIRE(turler[8] == TokenTuru::NOKTA);
    REQUIRE(turler[9] == TokenTuru::IKI_NOKTA);
}

// ============================================================
// BÖLÜM 9: Satır/Sütun Takibi
// ============================================================
TEST_CASE("İlk token satır 1 sütun 1'de", "[konum]") {
    auto sonuc = tara("42");
    REQUIRE(sonuc.tokenler[0].satir == 1);
    REQUIRE(sonuc.tokenler[0].sutun == 1);
}

TEST_CASE("Boşluktan sonraki token doğru sütunda", "[konum]") {
    auto sonuc = tara("   42");
    REQUIRE(sonuc.tokenler[0].satir == 1);
    REQUIRE(sonuc.tokenler[0].sutun == 4);
}

TEST_CASE("İkinci satırdaki token doğru satır numarasında", "[konum]") {
    auto sonuc = tara("x\n42");
    // x satır 1, 42 satır 2
    REQUIRE(sonuc.tokenler[0].satir == 1);
    REQUIRE(sonuc.tokenler[1].satir == 2);
    REQUIRE(sonuc.tokenler[1].sutun == 1);
}

TEST_CASE("Yorum sonrası token satır numarası doğru", "[konum]") {
    auto sonuc = tara("// yorum\n// yorum2\nx");
    // x satır 3'te
    REQUIRE(sonuc.tokenler[0].satir == 3);
}

TEST_CASE("DOSYA_SONU doğru konumda", "[konum]") {
    auto sonuc = tara("x");
    auto& eof = sonuc.tokenler.back();
    REQUIRE(eof.tur == TokenTuru::DOSYA_SONU);
    REQUIRE(eof.satir == 1);
}

// ============================================================
// BÖLÜM 10: Hata Kurtarma
// ============================================================
TEST_CASE("Bilinmeyen karakter BILINMEYEN token üretir", "[hata]") {
    auto sonuc = tara("@");
    // BILINMEYEN token üretilmeli ve hata kaydedilmeli
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::BILINMEYEN);
    REQUIRE(sonuc.tanilar.hatalarVar());
}

TEST_CASE("Tekil & hata üretir", "[hata]") {
    auto sonuc = tara("&");
    REQUIRE(sonuc.tanilar.hatalarVar());
    REQUIRE(sonuc.tanilar.hataSayisi() == 1);
    const auto& tani = sonuc.tanilar.tanilar()[0];
    REQUIRE(tani.kaynak == TaniKaynagi::LEXER);
}

TEST_CASE("Tekil | hata üretir", "[hata]") {
    auto sonuc = tara("|");
    REQUIRE(sonuc.tanilar.hatalarVar());
}

TEST_CASE("Hata kurtarma — devam eder", "[hata]") {
    // @ geçersiz karakter, sonrasında 42 hâlâ taranmalı
    auto sonuc = tara("@ 42");
    // @ BILINMEYEN, 42 TAM_SAYI_LIT, DOSYA_SONU
    bool tamSayiBulundu = false;
    for (const auto& tok : sonuc.tokenler) {
        if (tok.tur == TokenTuru::TAM_SAYI_LIT) {
            tamSayiBulundu = true;
            break;
        }
    }
    REQUIRE(tamSayiBulundu);
}

TEST_CASE("Birden fazla hata toplanır", "[hata]") {
    auto sonuc = tara("@ # $");
    REQUIRE(sonuc.tanilar.hataSayisi() == 3);
}

TEST_CASE("Biçimlendirilmiş hata mesajı dosya adı içerir", "[hata]") {
    Lexer lx("@", "benim.drk");
    auto sonuc = lx.tara();
    REQUIRE(!sonuc.tanilar.tanilar().empty());
    std::string mesaj = sonuc.tanilar.tanilar()[0].bicimle();
    REQUIRE(mesaj.find("benim.drk") != std::string::npos);
}

// ============================================================
// BÖLÜM 11: Entegrasyon Testleri — Gerçek DORUK Kodu
// ============================================================
TEST_CASE("Değişken tanımı", "[entegrasyon]") {
    // değişken x = 5
    auto sonuc = tara("değişken x = 5");
    auto& tok = sonuc.tokenler;
    REQUIRE(tok[0].tur == TokenTuru::DEGISKEN);
    REQUIRE(tok[1].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(tok[1].lexem == "x");
    REQUIRE(tok[2].tur == TokenTuru::ESIT);
    REQUIRE(tok[3].tur == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(tok[3].lexem == "5");
    REQUIRE(sonuc.basarili());
}

TEST_CASE("Fonksiyon tanımı başlığı", "[entegrasyon]") {
    // fonksiyon topla(a, b) {
    auto sonuc = tara("fonksiyon topla(a, b) {");
    auto& tok = sonuc.tokenler;
    REQUIRE(tok[0].tur == TokenTuru::FONKSIYON);
    REQUIRE(tok[1].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(tok[1].lexem == "topla");
    REQUIRE(tok[2].tur == TokenTuru::SOL_PAREN);
    REQUIRE(tok[3].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(tok[3].lexem == "a");
    REQUIRE(tok[4].tur == TokenTuru::VIRGUL);
    REQUIRE(tok[5].tur == TokenTuru::TANIMLAYICI);
    REQUIRE(tok[5].lexem == "b");
    REQUIRE(tok[6].tur == TokenTuru::SAG_PAREN);
    REQUIRE(tok[7].tur == TokenTuru::SOL_SUSE);
}

TEST_CASE("For döngüsü başlığı", "[entegrasyon]") {
    // için (değişken i = 0; i < 5; i = i + 1)
    const char* kaynak =
        "için "
        "(değişken i = 0; i < 5; i = i + 1)";
    auto sonuc = tara(kaynak);
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::ICIN);
    REQUIRE(sonuc.tokenler[1].tur == TokenTuru::SOL_PAREN);
    REQUIRE(sonuc.tokenler[2].tur == TokenTuru::DEGISKEN);
    REQUIRE(sonuc.basarili());
}

TEST_CASE("Eğer-değilse ifadesi", "[entegrasyon]") {
    // eğer (x > 0) { ... } değilse { ... }
    const char* kaynak =
        "eğer (x > 0) { } "
        "değilse { }";
    auto sonuc = tara(kaynak);
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::EGER);
    // ... değilse token'ı bul
    bool degilseBulundu = false;
    for (const auto& tok : sonuc.tokenler) {
        if (tok.tur == TokenTuru::DEGILSE) { degilseBulundu = true; break; }
    }
    REQUIRE(degilseBulundu);
    REQUIRE(sonuc.basarili());
}

TEST_CASE("Faktöriyel fonksiyonu tokenları", "[entegrasyon]") {
    const char* kaynak =
        "fonksiyon faktöriyel(n) {\n"
        "    eğer (n <= 1) {\n"
        "        döndür 1\n"
        "    }\n"
        "    döndür n * faktöriyel(n - 1)\n"
        "}";

    auto sonuc = tara(kaynak);
    REQUIRE(sonuc.basarili());
    REQUIRE(!sonuc.tokenler.empty());

    // İlk token FONKSIYON olmalı
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::FONKSIYON);

    // Son anlamlı token SAG_SUSE olmalı
    size_t son = sonuc.tokenler.size() - 2; // DOSYA_SONU'ndan bir önceki
    REQUIRE(sonuc.tokenler[son].tur == TokenTuru::SAG_SUSE);
}

TEST_CASE("yazdır çağrısı", "[entegrasyon]") {
    // yazdır("merhaba dünya")
    const char* kaynak =
        "yazdır(\"merhaba dünya\")";
    auto sonuc = tara(kaynak);
    REQUIRE(sonuc.tokenler[0].tur == TokenTuru::YAZDIR);
    REQUIRE(sonuc.tokenler[1].tur == TokenTuru::SOL_PAREN);
    REQUIRE(sonuc.tokenler[2].tur == TokenTuru::METIN_LIT);
    REQUIRE(sonuc.tokenler[2].lexem == "merhaba dünya");
    REQUIRE(sonuc.tokenler[3].tur == TokenTuru::SAG_PAREN);
    REQUIRE(sonuc.basarili());
}

TEST_CASE("Karmaşık ifade: a * b + c / d", "[entegrasyon]") {
    auto turler = tokenTurleri("a * b + c / d");
    REQUIRE(turler.size() == 7);
    REQUIRE(turler[0] == TokenTuru::TANIMLAYICI); // a
    REQUIRE(turler[1] == TokenTuru::CARPIM);       // *
    REQUIRE(turler[2] == TokenTuru::TANIMLAYICI); // b
    REQUIRE(turler[3] == TokenTuru::ARTI);         // +
    REQUIRE(turler[4] == TokenTuru::TANIMLAYICI); // c
    REQUIRE(turler[5] == TokenTuru::BOLUM);        // /
    REQUIRE(turler[6] == TokenTuru::TANIMLAYICI); // d
}

TEST_CASE("Liste erişimi: liste[0]", "[entegrasyon]") {
    auto turler = tokenTurleri("liste[0]");
    REQUIRE(turler[0] == TokenTuru::TANIMLAYICI);
    REQUIRE(turler[1] == TokenTuru::SOL_KOSELI);
    REQUIRE(turler[2] == TokenTuru::TAM_SAYI_LIT);
    REQUIRE(turler[3] == TokenTuru::SAG_KOSELI);
}

TEST_CASE("Üye erişimi: nesne.alan", "[entegrasyon]") {
    auto turler = tokenTurleri("nesne.alan");
    REQUIRE(turler[0] == TokenTuru::TANIMLAYICI);
    REQUIRE(turler[1] == TokenTuru::NOKTA);
    REQUIRE(turler[2] == TokenTuru::TANIMLAYICI);
}

TEST_CASE("Yorum satırdaki tokenleri etkilemez", "[entegrasyon]") {
    const char* kaynak =
        "x = 5 // bu yorum\n"
        "y = 10";
    auto sonuc = tara(kaynak);
    REQUIRE(sonuc.basarili());
    // x = 5 y = 10 → 6 token + DOSYA_SONU = 7
    REQUIRE(sonuc.tokenler.size() == 7);
}

// ============================================================
// BÖLÜM 12: TaniListesi testleri
// ============================================================
TEST_CASE("TaniListesi birleştirme", "[tani]") {
    TaniListesi liste1, liste2;
    liste1.ekle(lexerHatasi("Hata 1", "test.drk", 1, 1, 1));
    liste2.ekle(lexerHatasi("Hata 2", "test.drk", 2, 1, 1));
    liste1.birlestir(liste2);
    REQUIRE(liste1.toplamSayi() == 2);
    REQUIRE(liste1.hataSayisi() == 2);
}

TEST_CASE("TaniListesi hata sayısı", "[tani]") {
    TaniListesi liste;
    liste.ekle(lexerHatasi("Hata", "t.drk", 1, 1, 1));
    liste.ekle(Tani(Siddet::UYARI, TaniKaynagi::LEXER,
                    "Uyarı", "t.drk", 2, 1, 1));
    REQUIRE(liste.hataSayisi() == 1);
    REQUIRE(liste.uyariSayisi() == 1);
    REQUIRE(liste.hatalarVar());
    REQUIRE(liste.uyarilarVar());
}

TEST_CASE("Tani bicimle dosya ve konum içerir", "[tani]") {
    Tani t = lexerHatasi("Test hatası", "kaynak.drk", 5, 3, 2,
                          "Bu bir ipucu.");
    std::string bic = t.bicimle();
    REQUIRE(bic.find("kaynak.drk") != std::string::npos);
    REQUIRE(bic.find("5") != std::string::npos);
    REQUIRE(bic.find("3") != std::string::npos);
    REQUIRE(bic.find("ipucu") != std::string::npos);
}

// ============================================================
// BÖLÜM 13: Token yardımcı fonksiyonları
// ============================================================
TEST_CASE("tokenTuruAdi tanınmış turler için çalışır", "[token]") {
    REQUIRE(std::string(tokenTuruAdi(TokenTuru::DEGISKEN)) == "DEGISKEN");
    REQUIRE(std::string(tokenTuruAdi(TokenTuru::DOSYA_SONU)) == "DOSYA_SONU");
    REQUIRE(std::string(tokenTuruAdi(TokenTuru::TANIMLAYICI)) == "TANIMLAYICI");
}

TEST_CASE("Token::str debug çıktısı", "[token]") {
    Token tok(TokenTuru::TAM_SAYI_LIT, "42", 3, 7);
    std::string s = tok.str();
    REQUIRE(s.find("TAM_SAYI_LIT") != std::string::npos);
    REQUIRE(s.find("42") != std::string::npos);
    REQUIRE(s.find("3") != std::string::npos);
    REQUIRE(s.find("7") != std::string::npos);
}

TEST_CASE("Token::kapaticiMi kapatıcı tokenlar için doğru", "[token]") {
    Token kapali(TokenTuru::SAG_SUSE, "}", 1, 1);
    Token normal(TokenTuru::TANIMLAYICI, "x", 1, 1);
    REQUIRE(kapali.kapaticiMi());
    REQUIRE_FALSE(normal.kapaticiMi());
}

TEST_CASE("LexerSonucu::basarili hata yoksa true", "[lexer]") {
    auto sonuc = tara("42");
    REQUIRE(sonuc.basarili());
}

TEST_CASE("LexerSonucu::basarili hata varsa false", "[lexer]") {
    auto sonuc = tara("@");
    REQUIRE_FALSE(sonuc.basarili());
}

TEST_CASE("Lexer idempotent: iki kez çağrılabilir", "[lexer]") {
    Lexer lx("42 + 5", "test.drk");
    auto s1 = lx.tara();
    auto s2 = lx.tara();
    REQUIRE(s1.tokenler.size() == s2.tokenler.size());
    REQUIRE(s1.basarili());
    REQUIRE(s2.basarili());
}

