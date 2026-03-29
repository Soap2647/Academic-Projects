// ============================================================
// AST.h — DORUK Dili Soyut Sözdizim Ağacı (AST)
//
// Tasarım: Virtual hiyerarşi + Visitor deseni.
//   Tüm AST düğümleri Ifade veya Deyim'den türer.
//   Ziyaretci sınıfı compile-time polimorfizm sağlar.
//
// Düğüm kategorileri:
//   İfadeler (Ifade) — değer üretir, sağ tarafta kullanılabilir
//   Deyimler (Deyim) — yan etki oluşturur, değer üretmez
//   Program           — kök düğüm (deyimler listesi)
//
// Bellek: Tüm sahiplik std::unique_ptr ile yönetilir.
//         Ham sahip işaretçi yok.
// ============================================================
#pragma once

#include "Token.h"

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <utility>

namespace Doruk {

// ============================================================
// İleri bildirimler — tüm düğüm tipleri
// ============================================================

// İfade düğümleri
struct SayiLiteralIfade;
struct OndalikLiteralIfade;
struct MetinLiteralIfade;
struct BoolLiteralIfade;
struct BosLiteralIfade;
struct KimlikIfade;
struct IkiliIfade;
struct TekliIfade;
struct AtamaIfade;
struct CagriIfade;
struct IndisIfade;
struct UyeIfade;
struct ListeIfade;
struct SozlukIfade;
struct YeniIfade;       // yeni SinifAdi(args)

// Deyim düğümleri
struct BlokDeyim;
struct IfadeDeyim;
struct DegiskenTanim;
struct FonksiyonTanim;
struct SinifTanim;
struct EgerDeyim;
struct IcinDeyim;
struct DonguDeyim;
struct DondurDeyim;
struct KirDeyim;
struct DevamDeyim;
struct YazdirDeyim;

// Kök
struct Program;

// ============================================================
// Ziyaretci — tüm düğüm tipleri için saf sanal metotlar
// ============================================================
class Ziyaretci {
public:
    virtual ~Ziyaretci() = default;

    // ── İfadeler ────────────────────────────────────────────
    virtual void ziyaretSayi     (SayiLiteralIfade&    d) = 0;
    virtual void ziyaretOndalik  (OndalikLiteralIfade& d) = 0;
    virtual void ziyaretMetin    (MetinLiteralIfade&   d) = 0;
    virtual void ziyaretBool     (BoolLiteralIfade&    d) = 0;
    virtual void ziyaretBos      (BosLiteralIfade&     d) = 0;
    virtual void ziyaretKimlik   (KimlikIfade&         d) = 0;
    virtual void ziyaretIkili    (IkiliIfade&          d) = 0;
    virtual void ziyaretTekli    (TekliIfade&          d) = 0;
    virtual void ziyaretAtama    (AtamaIfade&          d) = 0;
    virtual void ziyaretCagri    (CagriIfade&          d) = 0;
    virtual void ziyaretIndis    (IndisIfade&          d) = 0;
    virtual void ziyaretUye      (UyeIfade&            d) = 0;
    virtual void ziyaretListe    (ListeIfade&          d) = 0;
    virtual void ziyaretSozluk   (SozlukIfade&         d) = 0;
    virtual void ziyaretYeni     (YeniIfade&           d) = 0;

    // ── Deyimler ────────────────────────────────────────────
    virtual void ziyaretBlok          (BlokDeyim&      d) = 0;
    virtual void ziyaretIfade         (IfadeDeyim&     d) = 0;
    virtual void ziyaretDegiskenTanim (DegiskenTanim&  d) = 0;
    virtual void ziyaretFonksiyonTanim(FonksiyonTanim& d) = 0;
    virtual void ziyaretSinifTanim    (SinifTanim&     d) = 0;
    virtual void ziyaretEger          (EgerDeyim&      d) = 0;
    virtual void ziyaretIcin          (IcinDeyim&      d) = 0;
    virtual void ziyaretDongu         (DonguDeyim&     d) = 0;
    virtual void ziyaretDondur        (DondurDeyim&    d) = 0;
    virtual void ziyaretKir           (KirDeyim&       d) = 0;
    virtual void ziyaretDevam         (DevamDeyim&     d) = 0;
    virtual void ziyaretYazdir        (YazdirDeyim&    d) = 0;

    // ── Kök ─────────────────────────────────────────────────
    virtual void ziyaretProgram (Program& d) = 0;
};

// ============================================================
// Tür kısaltmaları
// ============================================================
using IfadePtr = std::unique_ptr<struct Ifade>;
using DeyimPtr = std::unique_ptr<struct Deyim>;

// ============================================================
// İFADE TABANI
// ============================================================
struct Ifade {
    uint32_t satir;
    uint32_t sutun;

    Ifade(uint32_t satir = 0, uint32_t sutun = 0)
        : satir(satir), sutun(sutun) {}

    virtual ~Ifade() = default;
    virtual void kabul(Ziyaretci& z) = 0;
};

// ============================================================
// DEYİM TABANI
// ============================================================
struct Deyim {
    uint32_t satir;
    uint32_t sutun;

    Deyim(uint32_t satir = 0, uint32_t sutun = 0)
        : satir(satir), sutun(sutun) {}

    virtual ~Deyim() = default;
    virtual void kabul(Ziyaretci& z) = 0;
};

// ============================================================
// İFADE DÜĞÜMLERİ
// ============================================================

// Tam sayı literali: 42, 0, 0xFF
struct SayiLiteralIfade : Ifade {
    int64_t deger;

    SayiLiteralIfade(int64_t deger, uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun), deger(deger) {}

    void kabul(Ziyaretci& z) override { z.ziyaretSayi(*this); }
};

// Ondalık literali: 3.14, 2.718e-5
struct OndalikLiteralIfade : Ifade {
    double deger;

    OndalikLiteralIfade(double deger, uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun), deger(deger) {}

    void kabul(Ziyaretci& z) override { z.ziyaretOndalik(*this); }
};

// Metin literali: "merhaba dünya"
struct MetinLiteralIfade : Ifade {
    std::string deger;

    MetinLiteralIfade(std::string deger, uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun), deger(std::move(deger)) {}

    void kabul(Ziyaretci& z) override { z.ziyaretMetin(*this); }
};

// Boolean literali: doğru / yanlış
struct BoolLiteralIfade : Ifade {
    bool deger;

    BoolLiteralIfade(bool deger, uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun), deger(deger) {}

    void kabul(Ziyaretci& z) override { z.ziyaretBool(*this); }
};

// Boş (null) literali: boş
struct BosLiteralIfade : Ifade {
    BosLiteralIfade(uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun) {}

    void kabul(Ziyaretci& z) override { z.ziyaretBos(*this); }
};

// Tanımlayıcı: x, sayac
struct KimlikIfade : Ifade {
    std::string isim;

    KimlikIfade(std::string isim, uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun), isim(std::move(isim)) {}

    void kabul(Ziyaretci& z) override { z.ziyaretKimlik(*this); }
};

// İkili ifade: a + b, x == y, i < 10, a && b
struct IkiliIfade : Ifade {
    IfadePtr sol;
    Token    op;
    IfadePtr sag;

    IkiliIfade(IfadePtr sol, Token op, IfadePtr sag)
        : Ifade(sol->satir, sol->sutun)
        , sol(std::move(sol))
        , op(std::move(op))
        , sag(std::move(sag))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretIkili(*this); }
};

// Tekli (önek) ifade: !x, -y, değil z
struct TekliIfade : Ifade {
    Token    op;
    IfadePtr operand;

    TekliIfade(Token op, IfadePtr operand)
        : Ifade(op.satir, op.sutun)
        , op(std::move(op))
        , operand(std::move(operand))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretTekli(*this); }
};

// Atama: x = ifade, x += 1 vb.
// Hedef: basit değişken ismi (karmaşık atama ileriki adımlarda)
struct AtamaIfade : Ifade {
    std::string hedef;   // sol taraf değişken ismi
    Token       op;      // =, +=, -=, *=, /=
    IfadePtr    deger;   // sağ taraf ifadesi

    AtamaIfade(std::string hedef, Token op, IfadePtr deger)
        : Ifade(op.satir, op.sutun)
        , hedef(std::move(hedef))
        , op(std::move(op))
        , deger(std::move(deger))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretAtama(*this); }
};

// Fonksiyon/metot çağrısı: isim(arg1, arg2)
struct CagriIfade : Ifade {
    IfadePtr              fonksiyon;    // çağrılan ifade
    std::vector<IfadePtr> argumanlar;   // argüman listesi

    CagriIfade(IfadePtr fonksiyon,
               std::vector<IfadePtr> argumanlar,
               uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun)
        , fonksiyon(std::move(fonksiyon))
        , argumanlar(std::move(argumanlar))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretCagri(*this); }
};

// İndis erişimi: liste[i], sozluk["anahtar"]
struct IndisIfade : Ifade {
    IfadePtr nesne;
    IfadePtr indis;

    IndisIfade(IfadePtr nesne, IfadePtr indis,
               uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun)
        , nesne(std::move(nesne))
        , indis(std::move(indis))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretIndis(*this); }
};

// Üye erişimi: nesne.alan, bu.isim
struct UyeIfade : Ifade {
    IfadePtr    nesne;
    std::string alan;

    UyeIfade(IfadePtr nesne, std::string alan,
             uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun)
        , nesne(std::move(nesne))
        , alan(std::move(alan))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretUye(*this); }
};

// Liste literali: [1, 2, 3]
struct ListeIfade : Ifade {
    std::vector<IfadePtr> elemanlar;

    ListeIfade(std::vector<IfadePtr> elemanlar,
               uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun)
        , elemanlar(std::move(elemanlar))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretListe(*this); }
};

// Sözlük literali: {"a": 1, "b": 2}
struct SozlukIfade : Ifade {
    using Cift = std::pair<IfadePtr, IfadePtr>;
    std::vector<Cift> cifter;  // (anahtar, deger) çiftleri

    SozlukIfade(std::vector<Cift> cifter,
                uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun)
        , cifter(std::move(cifter))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretSozluk(*this); }
};

// Nesne örnekleme: yeni SinifAdi(args)
struct YeniIfade : Ifade {
    std::string           sinifIsmi;
    std::vector<IfadePtr> argumanlar;

    YeniIfade(std::string sinifIsmi,
              std::vector<IfadePtr> argumanlar,
              uint32_t satir, uint32_t sutun)
        : Ifade(satir, sutun)
        , sinifIsmi(std::move(sinifIsmi))
        , argumanlar(std::move(argumanlar))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretYeni(*this); }
};

// ============================================================
// DEYİM DÜĞÜMLERİ
// ============================================================

// Blok: { deyim1; deyim2; ... }
struct BlokDeyim : Deyim {
    std::vector<DeyimPtr> deyimler;

    BlokDeyim(std::vector<DeyimPtr> deyimler,
              uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , deyimler(std::move(deyimler))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretBlok(*this); }
};

// İfade deyimi: expr; (değeri kullanılmayan ifade)
struct IfadeDeyim : Deyim {
    IfadePtr ifade;

    IfadeDeyim(IfadePtr ifade, uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , ifade(std::move(ifade))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretIfade(*this); }
};

// Değişken tanımı: değişken x = ifade
struct DegiskenTanim : Deyim {
    std::string isim;
    IfadePtr    baslangicDegeri;  // nullptr = boş ile başlatılır

    DegiskenTanim(std::string isim, IfadePtr baslangic,
                  uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , isim(std::move(isim))
        , baslangicDegeri(std::move(baslangic))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretDegiskenTanim(*this); }
};

// ── Fonksiyon parametresi ────────────────────────────────────
struct Parametre {
    std::string isim;
    uint32_t    satir = 0;
    uint32_t    sutun = 0;
};

// Fonksiyon tanımı: fonksiyon isim(p1, p2) { govde }
struct FonksiyonTanim : Deyim {
    std::string                    isim;
    std::vector<Parametre>         parametreler;
    std::unique_ptr<BlokDeyim>     govde;

    FonksiyonTanim(std::string isim,
                   std::vector<Parametre> parametreler,
                   std::unique_ptr<BlokDeyim> govde,
                   uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , isim(std::move(isim))
        , parametreler(std::move(parametreler))
        , govde(std::move(govde))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretFonksiyonTanim(*this); }
};

// Sınıf tanımı: sınıf İsim { metotlar... }
struct SinifTanim : Deyim {
    std::string                                  isim;
    std::string                                  ebeveyn;   // boş = kalıtım yok
    std::vector<std::unique_ptr<FonksiyonTanim>> metodlar;
    std::vector<std::unique_ptr<DegiskenTanim>>  alanlar;

    SinifTanim(std::string isim, std::string ebeveyn,
               uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , isim(std::move(isim))
        , ebeveyn(std::move(ebeveyn))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretSinifTanim(*this); }
};

// Eğer deyimi: eğer (koşul) { doğru } değilse { yanlış }
struct EgerDeyim : Deyim {
    IfadePtr                   kosul;
    std::unique_ptr<BlokDeyim> dogruKol;
    DeyimPtr                   yanlisKol;  // nullptr, BlokDeyim veya EgerDeyim

    EgerDeyim(IfadePtr kosul,
              std::unique_ptr<BlokDeyim> dogruKol,
              DeyimPtr yanlisKol,
              uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , kosul(std::move(kosul))
        , dogruKol(std::move(dogruKol))
        , yanlisKol(std::move(yanlisKol))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretEger(*this); }
};

// İçin döngüsü: için (init; koşul; arttir) { govde }
struct IcinDeyim : Deyim {
    DeyimPtr                   baslangic;  // değişken tanımı veya ifade deyimi
    IfadePtr                   kosul;      // nullptr = sonsuz döngü
    IfadePtr                   arttir;     // nullptr = yok
    std::unique_ptr<BlokDeyim> govde;

    IcinDeyim(DeyimPtr baslangic,
              IfadePtr kosul,
              IfadePtr arttir,
              std::unique_ptr<BlokDeyim> govde,
              uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , baslangic(std::move(baslangic))
        , kosul(std::move(kosul))
        , arttir(std::move(arttir))
        , govde(std::move(govde))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretIcin(*this); }
};

// Döngü (while): döngü (koşul) { govde }
struct DonguDeyim : Deyim {
    IfadePtr                   kosul;
    std::unique_ptr<BlokDeyim> govde;

    DonguDeyim(IfadePtr kosul,
               std::unique_ptr<BlokDeyim> govde,
               uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , kosul(std::move(kosul))
        , govde(std::move(govde))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretDongu(*this); }
};

// Döndür: döndür ifade
struct DondurDeyim : Deyim {
    IfadePtr deger;  // nullptr = değer döndürmeden çık

    DondurDeyim(IfadePtr deger, uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , deger(std::move(deger))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretDondur(*this); }
};

// Kır (break): kır
struct KirDeyim : Deyim {
    KirDeyim(uint32_t satir, uint32_t sutun) : Deyim(satir, sutun) {}
    void kabul(Ziyaretci& z) override { z.ziyaretKir(*this); }
};

// Devam (continue): devam
struct DevamDeyim : Deyim {
    DevamDeyim(uint32_t satir, uint32_t sutun) : Deyim(satir, sutun) {}
    void kabul(Ziyaretci& z) override { z.ziyaretDevam(*this); }
};

// Yazdır: yazdır(arg1, arg2, ...)
struct YazdirDeyim : Deyim {
    std::vector<IfadePtr> argumanlar;

    YazdirDeyim(std::vector<IfadePtr> argumanlar,
                uint32_t satir, uint32_t sutun)
        : Deyim(satir, sutun)
        , argumanlar(std::move(argumanlar))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretYazdir(*this); }
};

// ============================================================
// Program — kök düğüm
// ============================================================
struct Program : Deyim {
    std::vector<DeyimPtr> deyimler;
    std::string           dosyaAdi;

    explicit Program(std::string dosyaAdi = "<isimsiz>")
        : Deyim(1, 1)
        , dosyaAdi(std::move(dosyaAdi))
    {}

    void kabul(Ziyaretci& z) override { z.ziyaretProgram(*this); }
};

// ============================================================
// AST yazdırma yardımcısı (debug/test için)
// ============================================================
std::string astYazdir(const Program& program);

} // namespace Doruk

