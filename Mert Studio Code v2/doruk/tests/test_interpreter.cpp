// ============================================================
// test_interpreter.cpp — Adım 4-5 testleri
// Catch2 v3 kullanır.
// ============================================================
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "Lexer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "Interpreter.h"

#include <string>
#include <sstream>

using namespace Doruk;

// ── Yardımcı fonksiyon: kaynak kodu çalıştır, çıktıyı döndür ──
struct CalismaYardimcisi {
    std::string cikti;
    std::string hata;
    bool        basarili = false;

    CalismaYardimcisi(const std::string& kaynak,
                       bool semantikKontrol = true) {
        // Lexer
        Lexer lexer(kaynak, "<test>");
        auto lexSonucu = lexer.tara();
        if (!lexSonucu.basarili()) {
            for (auto& t : lexSonucu.tanilar) hata += t.bicimle() + "\n";
            return;
        }

        // Parser
        Parser parser(std::move(lexSonucu.tokenler), "<test>");
        auto parseSonucu = parser.ayristir();
        if (!parseSonucu.basarili()) {
            for (auto& t : parseSonucu.tanilar) hata += t.bicimle() + "\n";
            return;
        }

        // Semantik
        if (semantikKontrol) {
            SemanticAnalyzer anlam("<test>");
            auto anlamS = anlam.analiz(*parseSonucu.program);
            if (!anlamS.basarili()) {
                for (auto& t : anlamS.tanilar) hata += t.bicimle() + "\n";
                return;
            }
        }

        // Yorumlayıcı
        std::string tamCikti;
        Interpreter yorumlayici(
            [&tamCikti](const std::string& s) { tamCikti += s; },
            []() -> std::string { return ""; }
        );

        auto yorumS = yorumlayici.calistir(*parseSonucu.program);
        cikti = tamCikti;
        if (!yorumS.basarili) {
            for (auto& t : yorumS.tanilar) hata += t.bicimle() + "\n";
        } else {
            basarili = true;
        }
    }
};

// ── Kısaltma makroları ────────────────────────────────────────
#define CALISTIR(kod)        CalismaYardimcisi c(kod)
#define CIKTI_ESIT(beklenen) REQUIRE(c.cikti == (beklenen))
#define BASARILI()           REQUIRE(c.basarili)
#define BASARISIZ()          REQUIRE_FALSE(c.basarili)

// ============================================================
// [literal] — Değer tipleri
// ============================================================
TEST_CASE("Sayı literali yazdır", "[literal]") {
    CALISTIR("yazdır(42)");
    BASARILI();
    CIKTI_ESIT("42\n");
}

TEST_CASE("Ondalık literali yazdır", "[literal]") {
    CALISTIR("yazdır(3.14)");
    BASARILI();
    CIKTI_ESIT("3.14\n");
}

TEST_CASE("Metin literali yazdır", "[literal]") {
    CALISTIR("yazdır(\"merhaba\")");
    BASARILI();
    CIKTI_ESIT("merhaba\n");
}

TEST_CASE("Bool doğru yazdır", "[literal]") {
    CALISTIR("yazdır(doğru)");
    BASARILI();
    // doğru
    REQUIRE_THAT(c.cikti, Catch::Matchers::ContainsSubstring("do"));
}

TEST_CASE("Boş literali yazdır", "[literal]") {
    CALISTIR("yazdır(boş)");
    BASARILI();
}

// ============================================================
// [aritmetik] — Sayısal işlemler
// ============================================================
TEST_CASE("Toplama", "[aritmetik]") {
    CALISTIR("yazdır(3 + 4)");
    BASARILI();
    CIKTI_ESIT("7\n");
}

TEST_CASE("Çıkarma", "[aritmetik]") {
    CALISTIR("yazdır(10 - 3)");
    BASARILI();
    CIKTI_ESIT("7\n");
}

TEST_CASE("Çarpma", "[aritmetik]") {
    CALISTIR("yazdır(6 * 7)");
    BASARILI();
    CIKTI_ESIT("42\n");
}

TEST_CASE("Bölme ondalık sonuç", "[aritmetik]") {
    CALISTIR("yazdır(7 / 2)");
    BASARILI();
    CIKTI_ESIT("3.5\n");
}

TEST_CASE("Mod işlemi", "[aritmetik]") {
    CALISTIR("yazdır(10 % 3)");
    BASARILI();
    CIKTI_ESIT("1\n");
}

TEST_CASE("Sıfıra bölme hatası", "[aritmetik]") {
    CalismaYardimcisi c("yazdır(1 / 0)", false);
    BASARISIZ();
}

TEST_CASE("Metin birleştirme +", "[aritmetik]") {
    CALISTIR("yazdır(\"a\" + \"b\")");
    BASARILI();
    CIKTI_ESIT("ab\n");
}

// ============================================================
// [degisken] — Değişken tanımı ve kullanımı
// ============================================================
TEST_CASE("Değişken tanımla ve yazdır", "[degisken]") {
    CALISTIR(
        "değişken x = 5\n"   // değişken x = 5
        "yazdır(x)"
    );
    BASARILI();
    CIKTI_ESIT("5\n");
}

TEST_CASE("Değişken güncelle", "[degisken]") {
    CALISTIR(
        "değişken x = 1\n"
        "x = 99\n"
        "yazdır(x)"
    );
    BASARILI();
    CIKTI_ESIT("99\n");
}

TEST_CASE("Bileşik atama +=", "[degisken]") {
    CALISTIR(
        "değişken x = 10\n"
        "x += 5\n"
        "yazdır(x)"
    );
    BASARILI();
    CIKTI_ESIT("15\n");
}

// ============================================================
// [kontrol] — Kontrol akışı
// ============================================================
TEST_CASE("Eğer koşul doğru", "[kontrol]") {
    CALISTIR(
        "eğer (1 < 2) {\n"   // eğer
        "  yazdır(\"evet\")\n"
        "}"
    );
    BASARILI();
    CIKTI_ESIT("evet\n");
}

TEST_CASE("Eğer değilse", "[kontrol]") {
    CALISTIR(
        "eğer (1 > 2) {\n"
        "  yazdır(\"yanlis\")\n"
        "} değilse {\n"       // değilse
        "  yazdır(\"dogru\")\n"
        "}"
    );
    BASARILI();
    CIKTI_ESIT("dogru\n");
}

TEST_CASE("Döngü sayaç", "[kontrol]") {
    CALISTIR(
        "değişken i = 0\n"
        "döngü (i < 3) {\n"   // döngü
        "  yazdır(i)\n"
        "  i += 1\n"
        "}"
    );
    BASARILI();
    CIKTI_ESIT("0\n1\n2\n");
}

TEST_CASE("İçin döngüsü", "[kontrol]") {
    CALISTIR(
        "için (değişken i = 0; i < 3; i += 1) {\n"
        // için
        "  yazdır(i)\n"
        "}"
    );
    BASARILI();
    CIKTI_ESIT("0\n1\n2\n");
}

TEST_CASE("Kır döngüden çıkar", "[kontrol]") {
    CALISTIR(
        "değişken i = 0\n"
        "döngü (doğru) {\n"  // döngü (doğru)
        "  eğer (i == 2) { kır }\n"  // eğer / kır
        "  yazdır(i)\n"
        "  i += 1\n"
        "}"
    );
    BASARILI();
    CIKTI_ESIT("0\n1\n");
}

// ============================================================
// [fonksiyon] — Fonksiyon tanımı ve çağrısı
// ============================================================
TEST_CASE("Basit fonksiyon", "[fonksiyon]") {
    CALISTIR(
        "fonksiyon merhaba() {\n"
        "  yazdır(\"merhaba!\")\n"
        "}\n"
        "merhaba()"
    );
    BASARILI();
    CIKTI_ESIT("merhaba!\n");
}

TEST_CASE("Parametreli fonksiyon", "[fonksiyon]") {
    CALISTIR(
        "fonksiyon topla(a, b) {\n"
        "  döndür a + b\n"  // döndür
        "}\n"
        "yazdır(topla(3, 4))"
    );
    BASARILI();
    CIKTI_ESIT("7\n");
}

TEST_CASE("Özyinelemeli fonksiyon — faktöriyel", "[fonksiyon]") {
    CALISTIR(
        "fonksiyon faktoriyel(n) {\n"
        "  eğer (n <= 1) { döndür 1 }\n"
        "  döndür n * faktoriyel(n - 1)\n"
        "}\n"
        "yazdır(faktoriyel(6))"
    );
    BASARILI();
    CIKTI_ESIT("720\n");
}

TEST_CASE("Closure değişken yakalar", "[fonksiyon]") {
    CALISTIR(
        "değişken x = 10\n"
        "fonksiyon al() {\n"
        "  döndür x\n"
        "}\n"
        "yazdır(al())"
    );
    BASARILI();
    CIKTI_ESIT("10\n");
}

// ============================================================
// [liste] — Liste işlemleri
// ============================================================
TEST_CASE("Liste literali", "[liste]") {
    CALISTIR("yazdır([1, 2, 3])");
    BASARILI();
    CIKTI_ESIT("[1, 2, 3]\n");
}

TEST_CASE("Liste indis erişimi", "[liste]") {
    CALISTIR(
        "değişken lst = [10, 20, 30]\n"
        "yazdır(lst[1])"
    );
    BASARILI();
    CIKTI_ESIT("20\n");
}

TEST_CASE("ekle() fonksiyonu", "[liste]") {
    CALISTIR(
        "değişken lst = [1, 2]\n"
        "ekle(lst, 3)\n"
        "yazdır(uzunluk(lst))"
    );
    BASARILI();
    CIKTI_ESIT("3\n");
}

TEST_CASE("uzunluk() liste", "[liste]") {
    CALISTIR("yazdır(uzunluk([1, 2, 3]))");
    BASARILI();
    CIKTI_ESIT("3\n");
}

TEST_CASE("uzunluk() metin", "[liste]") {
    CALISTIR("yazdır(uzunluk(\"doruk\"))");
    BASARILI();
    CIKTI_ESIT("5\n");
}

// ============================================================
// [sozluk] — Sözlük işlemleri
// ============================================================
TEST_CASE("Sözlük literali erişim", "[sozluk]") {
    CALISTIR(
        "değişken s = {\"ad\": \"Doruk\", \"yas\": 1}\n"
        "yazdır(s[\"ad\"])"
    );
    BASARILI();
    CIKTI_ESIT("Doruk\n");
}

// ============================================================
// [stdlib] — Yerleşik fonksiyonlar
// ============================================================
TEST_CASE("tip() fonksiyonu", "[stdlib]") {
    CALISTIR("yazdır(tip(42))");
    BASARILI();
    // "tam_sayı" = tam_sayı
    REQUIRE_THAT(c.cikti, Catch::Matchers::ContainsSubstring("tam_say"));
}

TEST_CASE("tamSayi() dönüşümü", "[stdlib]") {
    CALISTIR("yazdır(tamSayi(\"42\"))");
    BASARILI();
    CIKTI_ESIT("42\n");
}

TEST_CASE("metin() dönüşümü", "[stdlib]") {
    CALISTIR("yazdır(metin(99))");
    BASARILI();
    CIKTI_ESIT("99\n");
}

TEST_CASE("ters() listesi", "[stdlib]") {
    CALISTIR(
        "değişken lst = [1, 2, 3]\n"
        "ters(lst)\n"
        "yazdır(lst)"
    );
    BASARILI();
    CIKTI_ESIT("[3, 2, 1]\n");
}

TEST_CASE("sirala() listesi", "[stdlib]") {
    CALISTIR(
        "değişken lst = [3, 1, 2]\n"
        "sirala(lst)\n"
        "yazdır(lst)"
    );
    BASARILI();
    CIKTI_ESIT("[1, 2, 3]\n");
}

// ============================================================
// [sinif] — Sınıf ve nesne
// ============================================================
TEST_CASE("Basit sınıf metot çağrısı", "[sinif]") {
    // Not: bu.alan = deger üye ataması parser'da henüz desteklenmiyor.
    // Sadece metot çağrısı ve nesne oluşturma test edilir.
    CALISTIR(
        "sınıf Hesap {\n"    // sınıf
        "  fonksiyon iki_kati(x) {\n"
        "    döndür x * 2\n"  // döndür
        "  }\n"
        "}\n"
        "değişken h = yeni Hesap()\n"  // yeni
        "yazdır(h.iki_kati(21))"
    );
    BASARILI();
    CIKTI_ESIT("42\n");
}

// ============================================================
// [semantik] — Semantik analizör hataları
// ============================================================
TEST_CASE("Tanımsız değişken — semantik hata", "[semantik]") {
    CalismaYardimcisi c("yazdır(yok_degisken)");
    REQUIRE_FALSE(c.basarili);
    REQUIRE_THAT(c.hata, Catch::Matchers::ContainsSubstring("yok_degisken"));
}

TEST_CASE("Döndür döngü dışında — semantik hata", "[semantik]") {
    CalismaYardimcisi c("kır");  // kır
    REQUIRE_FALSE(c.basarili);
}

// ============================================================
// [entegrasyon] — Daha kapsamlı programlar
// ============================================================
TEST_CASE("FizzBuzz 1-15", "[entegrasyon]") {
    CALISTIR(
        "için (değişken i = 1; i <= 15; i += 1) {\n"
        "  eğer (i % 15 == 0) {\n"
        "    yazdır(\"FizzBuzz\")\n"
        "  } değilse eğer (i % 3 == 0) {\n"
        "    yazdır(\"Fizz\")\n"
        "  } değilse eğer (i % 5 == 0) {\n"
        "    yazdır(\"Buzz\")\n"
        "  } değilse {\n"
        "    yazdır(i)\n"
        "  }\n"
        "}"
    );
    BASARILI();
    REQUIRE_THAT(c.cikti, Catch::Matchers::ContainsSubstring("FizzBuzz"));
    REQUIRE_THAT(c.cikti, Catch::Matchers::ContainsSubstring("Fizz"));
    REQUIRE_THAT(c.cikti, Catch::Matchers::ContainsSubstring("Buzz"));
}

TEST_CASE("Fibonacci 10. terim", "[entegrasyon]") {
    CALISTIR(
        "fonksiyon fib(n) {\n"
        "  eğer (n <= 1) { döndür n }\n"
        "  döndür fib(n-1) + fib(n-2)\n"
        "}\n"
        "yazdır(fib(10))"
    );
    BASARILI();
    CIKTI_ESIT("55\n");
}

