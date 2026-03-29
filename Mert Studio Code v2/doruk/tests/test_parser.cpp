// ============================================================
// test_parser.cpp — DORUK Parser Test Paketi
// Framework: Catch2 v3
//
// Test kategorileri:
//   [literal]      — sayı, metin, bool, boş literalleri
//   [ifade]        — ikili, tekli, öncelik testleri
//   [atama]        — atama ifadeleri
//   [degisken]     — değişken tanımları
//   [fonksiyon]    — fonksiyon tanım ve çağrıları
//   [kontrol]      — eğer, için, döngü
//   [dongu]        — kır, devam
//   [yapi]         — liste, sözlük, sınıf
//   [hata]         — hata kurtarma
//   [entegrasyon]  — gerçek DORUK kodu parçaları
// ============================================================
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "Lexer.h"
#include "Parser.h"
#include "AST.h"
#include "Token.h"
#include "Diagnostics.h"

using namespace Doruk;

// ============================================================
// Yardımcılar
// ============================================================
static ParserSonucu ayristir(const std::string& kaynak) {
    Lexer lx(kaynak, "test.drk");
    auto lSonuc = lx.tara();

    Parser parser(std::move(lSonuc.tokenler), "test.drk");
    return parser.ayristir();
}

// Belirli bir tür deyim var mı?
template<typename T>
static bool ilkDeyimTipi(const ParserSonucu& s) {
    if (!s.program || s.program->deyimler.empty()) return false;
    return dynamic_cast<T*>(s.program->deyimler[0].get()) != nullptr;
}

// İlk deyimi belirli türe dönüştür
template<typename T>
static T* ilkDeyim(ParserSonucu& s) {
    if (!s.program || s.program->deyimler.empty()) return nullptr;
    return dynamic_cast<T*>(s.program->deyimler[0].get());
}

// İlk ifade deyimindeki ifadeyi belirli türe dönüştür
template<typename T>
static T* ilkIfade(ParserSonucu& s) {
    auto* id = ilkDeyim<IfadeDeyim>(s);
    if (!id) return nullptr;
    return dynamic_cast<T*>(id->ifade.get());
}

// ============================================================
// BÖLÜM 1: Literaller
// ============================================================
TEST_CASE("Tam sayı literali ayrıştırılır", "[literal]") {
    auto s = ayristir("42");
    REQUIRE(s.basarili());
    auto* lit = ilkIfade<SayiLiteralIfade>(s);
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == 42);
}

TEST_CASE("Sıfır tam sayı", "[literal]") {
    auto s = ayristir("0");
    REQUIRE(s.basarili());
    auto* lit = ilkIfade<SayiLiteralIfade>(s);
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == 0);
}

TEST_CASE("Ondalık literali ayrıştırılır", "[literal]") {
    auto s = ayristir("3.14");
    REQUIRE(s.basarili());
    auto* lit = ilkIfade<OndalikLiteralIfade>(s);
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == Catch::Approx(3.14));
}

TEST_CASE("Metin literali ayrıştırılır", "[literal]") {
    auto s = ayristir("\"merhaba\"");
    REQUIRE(s.basarili());
    auto* lit = ilkIfade<MetinLiteralIfade>(s);
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == "merhaba");
}

TEST_CASE("doğru boolean literali", "[literal]") {
    auto s = ayristir("doğru");
    REQUIRE(s.basarili());
    auto* lit = ilkIfade<BoolLiteralIfade>(s);
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == true);
}

TEST_CASE("yanlış boolean literali", "[literal]") {
    auto s = ayristir("yanlış");
    REQUIRE(s.basarili());
    auto* lit = ilkIfade<BoolLiteralIfade>(s);
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == false);
}

TEST_CASE("boş null literali", "[literal]") {
    auto s = ayristir("boş");
    REQUIRE(s.basarili());
    auto* lit = ilkIfade<BosLiteralIfade>(s);
    REQUIRE(lit != nullptr);
}

TEST_CASE("Tanımlayıcı ifade", "[literal]") {
    auto s = ayristir("x");
    REQUIRE(s.basarili());
    auto* kimlik = ilkIfade<KimlikIfade>(s);
    REQUIRE(kimlik != nullptr);
    REQUIRE(kimlik->isim == "x");
}

// ============================================================
// BÖLÜM 2: İkili İfadeler ve Öncelik
// ============================================================
TEST_CASE("Basit toplama: 1 + 2", "[ifade]") {
    auto s = ayristir("1 + 2");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::ARTI);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(bin->sol.get())->deger == 1);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(bin->sag.get())->deger == 2);
}

TEST_CASE("Çarpma soldan ilişkilendirilir: 2 * 3 * 4", "[ifade]") {
    // ((2 * 3) * 4)
    auto s = ayristir("2 * 3 * 4");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::CARPIM);
    // Sol: (2 * 3)
    auto* solBin = dynamic_cast<IkiliIfade*>(bin->sol.get());
    REQUIRE(solBin != nullptr);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(solBin->sol.get())->deger == 2);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(solBin->sag.get())->deger == 3);
    // Sağ: 4
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(bin->sag.get())->deger == 4);
}

TEST_CASE("Çarpma toplama üzerinde öncelikli: 2 + 3 * 4", "[ifade]") {
    // 2 + (3 * 4)
    auto s = ayristir("2 + 3 * 4");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::ARTI);
    // Sol: 2
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(bin->sol.get())->deger == 2);
    // Sağ: (3 * 4)
    auto* sagBin = dynamic_cast<IkiliIfade*>(bin->sag.get());
    REQUIRE(sagBin != nullptr);
    REQUIRE(sagBin->op.tur == TokenTuru::CARPIM);
}

TEST_CASE("Parantez önceliği geçersiz kılar: (2 + 3) * 4", "[ifade]") {
    // (2 + 3) * 4
    auto s = ayristir("(2 + 3) * 4");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::CARPIM);
    auto* solBin = dynamic_cast<IkiliIfade*>(bin->sol.get());
    REQUIRE(solBin != nullptr);
    REQUIRE(solBin->op.tur == TokenTuru::ARTI);
}

TEST_CASE("Karşılaştırma operatörü", "[ifade]") {
    auto s = ayristir("x < 10");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::KUCUK);
}

TEST_CASE("Eşitlik operatörü", "[ifade]") {
    auto s = ayristir("x == y");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::ESIT_ESIT);
}

TEST_CASE("Mantıksal AND: a && b", "[ifade]") {
    auto s = ayristir("a && b");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::VE_VE);
}

TEST_CASE("Mantıksal VE anahtar kelimesi: a ve b", "[ifade]") {
    auto s = ayristir("a ve b");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::VE);
}

TEST_CASE("Mantıksal OR: a || b", "[ifade]") {
    auto s = ayristir("a || b");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::VEYA_VEYA);
}

TEST_CASE("Mantıksal VEYA anahtar kelimesi: a veya b", "[ifade]") {
    auto s = ayristir("a veya b");
    REQUIRE(s.basarili());
    auto* bin = ilkIfade<IkiliIfade>(s);
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.tur == TokenTuru::VEYA);
}

TEST_CASE("Tekli eksi", "[ifade]") {
    auto s = ayristir("-x");
    REQUIRE(s.basarili());
    auto* tek = ilkIfade<TekliIfade>(s);
    REQUIRE(tek != nullptr);
    REQUIRE(tek->op.tur == TokenTuru::EKSI);
    REQUIRE(dynamic_cast<KimlikIfade*>(tek->operand.get()) != nullptr);
}

TEST_CASE("Tekli değil (unary NOT)", "[ifade]") {
    auto s = ayristir("!x");
    REQUIRE(s.basarili());
    auto* tek = ilkIfade<TekliIfade>(s);
    REQUIRE(tek != nullptr);
    REQUIRE(tek->op.tur == TokenTuru::UNLEM);
}

TEST_CASE("Türkçe değil anahtar kelimesiyle tekli NOT", "[ifade]") {
    auto s = ayristir("değil x");
    REQUIRE(s.basarili());
    auto* tek = ilkIfade<TekliIfade>(s);
    REQUIRE(tek != nullptr);
    REQUIRE(tek->op.tur == TokenTuru::DEGIL);
}

TEST_CASE("Zincirleme karşılaştırma: a < b değil", "[ifade]") {
    auto s = ayristir("a < b");
    REQUIRE(s.basarili());
}

// ============================================================
// BÖLÜM 3: Atama İfadeleri
// ============================================================
TEST_CASE("Basit atama: x = 5", "[atama]") {
    auto s = ayristir("x = 5");
    REQUIRE(s.basarili());
    auto* ata = ilkIfade<AtamaIfade>(s);
    REQUIRE(ata != nullptr);
    REQUIRE(ata->hedef == "x");
    REQUIRE(ata->op.tur == TokenTuru::ESIT);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(ata->deger.get())->deger == 5);
}

TEST_CASE("Bileşik atama açılır: x += 1 → x = x + 1", "[atama]") {
    auto s = ayristir("x += 1");
    REQUIRE(s.basarili());
    auto* ata = ilkIfade<AtamaIfade>(s);
    REQUIRE(ata != nullptr);
    REQUIRE(ata->hedef == "x");
    // Sağ taraf: x + 1 (ikili ifade)
    auto* sagBin = dynamic_cast<IkiliIfade*>(ata->deger.get());
    REQUIRE(sagBin != nullptr);
    REQUIRE(sagBin->op.tur == TokenTuru::ARTI);
}

TEST_CASE("Çıkarmalı bileşik atama: x -= 2", "[atama]") {
    auto s = ayristir("x -= 2");
    REQUIRE(s.basarili());
    auto* ata = ilkIfade<AtamaIfade>(s);
    REQUIRE(ata != nullptr);
    auto* sagBin = dynamic_cast<IkiliIfade*>(ata->deger.get());
    REQUIRE(sagBin != nullptr);
    REQUIRE(sagBin->op.tur == TokenTuru::EKSI);
}

// ============================================================
// BÖLÜM 4: Değişken Tanımları
// ============================================================
TEST_CASE("Değişken tanımı başlangıç değeri ile", "[degisken]") {
    auto s = ayristir("değişken x = 5");
    REQUIRE(s.basarili());
    auto* d = ilkDeyim<DegiskenTanim>(s);
    REQUIRE(d != nullptr);
    REQUIRE(d->isim == "x");
    REQUIRE(d->baslangicDegeri != nullptr);
    auto* lit = dynamic_cast<SayiLiteralIfade*>(d->baslangicDegeri.get());
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == 5);
}

TEST_CASE("Değişken tanımı başlangıç değeri olmadan", "[degisken]") {
    auto s = ayristir("değişken sayac");
    REQUIRE(s.basarili());
    auto* d = ilkDeyim<DegiskenTanim>(s);
    REQUIRE(d != nullptr);
    REQUIRE(d->isim == "sayac");
    REQUIRE(d->baslangicDegeri == nullptr);
}

TEST_CASE("Değişken tanımı metin değeri", "[degisken]") {
    auto s = ayristir("değişken mesaj = \"merhaba\"");
    REQUIRE(s.basarili());
    auto* d = ilkDeyim<DegiskenTanim>(s);
    REQUIRE(d != nullptr);
    REQUIRE(d->isim == "mesaj");
    auto* lit = dynamic_cast<MetinLiteralIfade*>(d->baslangicDegeri.get());
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == "merhaba");
}

TEST_CASE("Değişken tanımı boolean değer", "[degisken]") {
    auto s = ayristir("değişken aktif = doğru");
    REQUIRE(s.basarili());
    auto* d = ilkDeyim<DegiskenTanim>(s);
    REQUIRE(d != nullptr);
    auto* lit = dynamic_cast<BoolLiteralIfade*>(d->baslangicDegeri.get());
    REQUIRE(lit != nullptr);
    REQUIRE(lit->deger == true);
}

// ============================================================
// BÖLÜM 5: Fonksiyon Tanımı ve Çağrısı
// ============================================================
TEST_CASE("Parametresiz fonksiyon tanımı", "[fonksiyon]") {
    auto s = ayristir("fonksiyon selam() { }");
    REQUIRE(s.basarili());
    auto* f = ilkDeyim<FonksiyonTanim>(s);
    REQUIRE(f != nullptr);
    REQUIRE(f->isim == "selam");
    REQUIRE(f->parametreler.empty());
    REQUIRE(f->govde != nullptr);
}

TEST_CASE("Parametreli fonksiyon tanımı", "[fonksiyon]") {
    auto s = ayristir("fonksiyon topla(a, b) { }");
    REQUIRE(s.basarili());
    auto* f = ilkDeyim<FonksiyonTanim>(s);
    REQUIRE(f != nullptr);
    REQUIRE(f->isim == "topla");
    REQUIRE(f->parametreler.size() == 2);
    REQUIRE(f->parametreler[0].isim == "a");
    REQUIRE(f->parametreler[1].isim == "b");
}

TEST_CASE("Fonksiyon çağrısı argümansız", "[fonksiyon]") {
    auto s = ayristir("selam()");
    REQUIRE(s.basarili());
    auto* cagri = ilkIfade<CagriIfade>(s);
    REQUIRE(cagri != nullptr);
    REQUIRE(cagri->argumanlar.empty());
    auto* kimlik = dynamic_cast<KimlikIfade*>(cagri->fonksiyon.get());
    REQUIRE(kimlik != nullptr);
    REQUIRE(kimlik->isim == "selam");
}

TEST_CASE("Fonksiyon çağrısı argümanlı", "[fonksiyon]") {
    auto s = ayristir("topla(3, 5)");
    REQUIRE(s.basarili());
    auto* cagri = ilkIfade<CagriIfade>(s);
    REQUIRE(cagri != nullptr);
    REQUIRE(cagri->argumanlar.size() == 2);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(cagri->argumanlar[0].get())->deger == 3);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(cagri->argumanlar[1].get())->deger == 5);
}

TEST_CASE("Döndür deyimi değer ile", "[fonksiyon]") {
    auto s = ayristir("fonksiyon f() { döndür 42 }");
    REQUIRE(s.basarili());
    auto* f = ilkDeyim<FonksiyonTanim>(s);
    REQUIRE(f != nullptr);
    REQUIRE(f->govde->deyimler.size() == 1);
    auto* ret = dynamic_cast<DondurDeyim*>(f->govde->deyimler[0].get());
    REQUIRE(ret != nullptr);
    REQUIRE(ret->deger != nullptr);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(ret->deger.get())->deger == 42);
}

TEST_CASE("Döndür deyimi değersiz", "[fonksiyon]") {
    auto s = ayristir("fonksiyon f() { döndür }");
    REQUIRE(s.basarili());
    auto* f = ilkDeyim<FonksiyonTanim>(s);
    REQUIRE(f != nullptr);
    auto* ret = dynamic_cast<DondurDeyim*>(f->govde->deyimler[0].get());
    REQUIRE(ret != nullptr);
    REQUIRE(ret->deger == nullptr);
}

// ============================================================
// BÖLÜM 6: Kontrol Akışı
// ============================================================
TEST_CASE("Eğer deyimi tek kol", "[kontrol]") {
    auto s = ayristir("eğer (x > 0) { }");
    REQUIRE(s.basarili());
    auto* eger = ilkDeyim<EgerDeyim>(s);
    REQUIRE(eger != nullptr);
    REQUIRE(eger->kosul != nullptr);
    REQUIRE(eger->dogruKol != nullptr);
    REQUIRE(eger->yanlisKol == nullptr);
}

TEST_CASE("Eğer-değilse deyimi", "[kontrol]") {
    auto s = ayristir("eğer (x > 0) { } değilse { }");
    REQUIRE(s.basarili());
    auto* eger = ilkDeyim<EgerDeyim>(s);
    REQUIRE(eger != nullptr);
    REQUIRE(eger->yanlisKol != nullptr);
    REQUIRE(dynamic_cast<BlokDeyim*>(eger->yanlisKol.get()) != nullptr);
}

TEST_CASE("Zincirleme eğer-değilse eğer", "[kontrol]") {
    auto s = ayristir(
        "eğer (x > 0) { } "
        "değilse eğer (x < 0) { } "
        "değilse { }");
    REQUIRE(s.basarili());
    auto* eger = ilkDeyim<EgerDeyim>(s);
    REQUIRE(eger != nullptr);
    // İkinci kol da bir EgerDeyim olmalı
    auto* ikinci = dynamic_cast<EgerDeyim*>(eger->yanlisKol.get());
    REQUIRE(ikinci != nullptr);
    // Üçüncü kol BlokDeyim
    REQUIRE(dynamic_cast<BlokDeyim*>(ikinci->yanlisKol.get()) != nullptr);
}

TEST_CASE("Döngü (while) deyimi", "[dongu]") {
    auto s = ayristir("döngü (x < 10) { }");
    REQUIRE(s.basarili());
    auto* dongu = ilkDeyim<DonguDeyim>(s);
    REQUIRE(dongu != nullptr);
    REQUIRE(dongu->kosul != nullptr);
    REQUIRE(dongu->govde != nullptr);
}

TEST_CASE("İçin döngüsü tam sözdizimi", "[kontrol]") {
    auto s = ayristir(
        "için (değişken i = 0; i < 5; i = i + 1) { }");
    REQUIRE(s.basarili());
    auto* icin = ilkDeyim<IcinDeyim>(s);
    REQUIRE(icin != nullptr);
    REQUIRE(icin->baslangic != nullptr);
    REQUIRE(icin->kosul != nullptr);
    REQUIRE(icin->arttir != nullptr);
    REQUIRE(icin->govde != nullptr);
}

TEST_CASE("Kır deyimi", "[dongu]") {
    auto s = ayristir("döngü (doğru) { kır }");
    REQUIRE(s.basarili());
    auto* dongu = ilkDeyim<DonguDeyim>(s);
    REQUIRE(dongu != nullptr);
    REQUIRE(dongu->govde->deyimler.size() == 1);
    REQUIRE(dynamic_cast<KirDeyim*>(dongu->govde->deyimler[0].get()) != nullptr);
}

TEST_CASE("Devam deyimi", "[dongu]") {
    auto s = ayristir("döngü (doğru) { devam }");
    REQUIRE(s.basarili());
    auto* dongu = ilkDeyim<DonguDeyim>(s);
    REQUIRE(dongu != nullptr);
    REQUIRE(dynamic_cast<DevamDeyim*>(dongu->govde->deyimler[0].get()) != nullptr);
}

// ============================================================
// BÖLÜM 7: Liste ve Sözlük
// ============================================================
TEST_CASE("Boş liste literali", "[yapi]") {
    auto s = ayristir("[]");
    REQUIRE(s.basarili());
    auto* liste = ilkIfade<ListeIfade>(s);
    REQUIRE(liste != nullptr);
    REQUIRE(liste->elemanlar.empty());
}

TEST_CASE("Üç elemanlı liste literali", "[yapi]") {
    auto s = ayristir("[1, 2, 3]");
    REQUIRE(s.basarili());
    auto* liste = ilkIfade<ListeIfade>(s);
    REQUIRE(liste != nullptr);
    REQUIRE(liste->elemanlar.size() == 3);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(liste->elemanlar[0].get())->deger == 1);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(liste->elemanlar[1].get())->deger == 2);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(liste->elemanlar[2].get())->deger == 3);
}

TEST_CASE("Sözlük literali", "[yapi]") {
    auto s = ayristir("{\"a\": 1, \"b\": 2}");
    REQUIRE(s.basarili());
    auto* sozluk = ilkIfade<SozlukIfade>(s);
    REQUIRE(sozluk != nullptr);
    REQUIRE(sozluk->cifter.size() == 2);
}

TEST_CASE("İndis erişimi", "[yapi]") {
    auto s = ayristir("liste[0]");
    REQUIRE(s.basarili());
    auto* indis = ilkIfade<IndisIfade>(s);
    REQUIRE(indis != nullptr);
    REQUIRE(dynamic_cast<KimlikIfade*>(indis->nesne.get()) != nullptr);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(indis->indis.get())->deger == 0);
}

TEST_CASE("Üye erişimi", "[yapi]") {
    auto s = ayristir("nesne.alan");
    REQUIRE(s.basarili());
    auto* uye = ilkIfade<UyeIfade>(s);
    REQUIRE(uye != nullptr);
    REQUIRE(uye->alan == "alan");
}

TEST_CASE("Zincirleme üye erişimi", "[yapi]") {
    auto s = ayristir("a.b.c");
    REQUIRE(s.basarili());
    auto* uye = ilkIfade<UyeIfade>(s);
    REQUIRE(uye != nullptr);
    REQUIRE(uye->alan == "c");
    auto* uyeIc = dynamic_cast<UyeIfade*>(uye->nesne.get());
    REQUIRE(uyeIc != nullptr);
    REQUIRE(uyeIc->alan == "b");
}

// ============================================================
// BÖLÜM 8: Yazdır
// ============================================================
TEST_CASE("yazdır tek argüman", "[yazdir]") {
    auto s = ayristir("yazdır(42)");
    REQUIRE(s.basarili());
    auto* yd = ilkDeyim<YazdirDeyim>(s);
    REQUIRE(yd != nullptr);
    REQUIRE(yd->argumanlar.size() == 1);
    REQUIRE(dynamic_cast<SayiLiteralIfade*>(yd->argumanlar[0].get())->deger == 42);
}

TEST_CASE("yazdır metin argüman", "[yazdir]") {
    auto s = ayristir("yazdır(\"merhaba\")");
    REQUIRE(s.basarili());
    auto* yd = ilkDeyim<YazdirDeyim>(s);
    REQUIRE(yd != nullptr);
    REQUIRE(yd->argumanlar.size() == 1);
    auto* met = dynamic_cast<MetinLiteralIfade*>(yd->argumanlar[0].get());
    REQUIRE(met != nullptr);
    REQUIRE(met->deger == "merhaba");
}

// ============================================================
// BÖLÜM 9: Hata Kurtarma
// ============================================================
TEST_CASE("Parser hata üretir ve kurtarır", "[hata]") {
    // Kapatılmamış parantez
    auto s = ayristir("x = (5 + 3");
    REQUIRE(s.tanilar.hatalarVar());
    // Ama program nesnesi hâlâ var
    REQUIRE(s.program != nullptr);
}

TEST_CASE("Değişken ismi eksikse hata", "[hata]") {
    auto s = ayristir("değişken = 5");
    REQUIRE(s.tanilar.hatalarVar());
}

TEST_CASE("Blok kapatılmamışsa hata", "[hata]") {
    auto s = ayristir("fonksiyon f() {");
    REQUIRE(s.tanilar.hatalarVar());
}

TEST_CASE("Birden fazla hata toplanır", "[hata]") {
    // İki ayrı sözdizim hatası
    auto s = ayristir("değişken = 5\n"
                      "değişken = 10");
    REQUIRE(s.tanilar.hataSayisi() >= 1);
}

// ============================================================
// BÖLÜM 10: Entegrasyon Testleri
// ============================================================
TEST_CASE("Faktöriyel fonksiyonu tam ayrıştırma", "[entegrasyon]") {
    const char* kaynak =
        "fonksiyon faktöriyel(n) {\n"
        "    eğer (n <= 1) {\n"
        "        döndür 1\n"
        "    }\n"
        "    döndür n * faktöriyel(n - 1)\n"
        "}";

    auto s = ayristir(kaynak);
    REQUIRE(s.basarili());
    REQUIRE(s.program->deyimler.size() == 1);

    auto* f = dynamic_cast<FonksiyonTanim*>(s.program->deyimler[0].get());
    REQUIRE(f != nullptr);
    REQUIRE(f->isim == "faktöriyel");
    REQUIRE(f->parametreler.size() == 1);
    REQUIRE(f->parametreler[0].isim == "n");
    // Gövdede 2 deyim: eğer ve döndür
    REQUIRE(f->govde->deyimler.size() == 2);
}

TEST_CASE("Tam DORUK programı ayrıştırılır", "[entegrasyon]") {
    const char* kaynak =
        "fonksiyon faktöriyel(n) {\n"
        "    eğer (n <= 1) {\n"
        "        döndür 1\n"
        "    }\n"
        "    döndür n * faktöriyel(n - 1)\n"
        "}\n"
        "değişken sonuç = faktöriyel(10)\n"
        "yazdır(\"10! = \", sonuç)";

    auto s = ayristir(kaynak);
    REQUIRE(s.basarili());
    // 3 üst-düzey deyim: fonksiyon, değişken, yazdır
    REQUIRE(s.program->deyimler.size() == 3);
}

TEST_CASE("astYazdir çıktı üretir", "[entegrasyon]") {
    auto s = ayristir("değişken x = 42");
    REQUIRE(s.basarili());

    std::string cikti = astYazdir(*s.program);
    REQUIRE(!cikti.empty());
    // "x" ve "42" çıktıda olmalı
    REQUIRE(cikti.find("x") != std::string::npos);
    REQUIRE(cikti.find("42") != std::string::npos);
}

TEST_CASE("Kaynak konumu takibi doğru", "[entegrasyon]") {
    auto s = ayristir("değişken x = 5\n"
                      "değişken y = 10");
    REQUIRE(s.basarili());
    REQUIRE(s.program->deyimler.size() == 2);

    auto* d1 = dynamic_cast<DegiskenTanim*>(s.program->deyimler[0].get());
    auto* d2 = dynamic_cast<DegiskenTanim*>(s.program->deyimler[1].get());
    REQUIRE(d1 != nullptr);
    REQUIRE(d2 != nullptr);
    // İkinci değişken 2. satırda olmalı
    REQUIRE(d2->satir == 2);
}

TEST_CASE("yeni nesne örnekleme", "[yapi]") {
    auto s = ayristir("yeni Kisi(\"Ali\", 25)");
    REQUIRE(s.basarili());
    auto* yeni = ilkIfade<YeniIfade>(s);
    REQUIRE(yeni != nullptr);
    REQUIRE(yeni->sinifIsmi == "Kisi");
    REQUIRE(yeni->argumanlar.size() == 2);
}

TEST_CASE("oku() ifadesi ayrıştırılır", "[entegrasyon]") {
    auto s = ayristir("değişken giris = oku()");
    REQUIRE(s.basarili());
    auto* d = ilkDeyim<DegiskenTanim>(s);
    REQUIRE(d != nullptr);
    auto* cagri = dynamic_cast<CagriIfade*>(d->baslangicDegeri.get());
    REQUIRE(cagri != nullptr);
}

