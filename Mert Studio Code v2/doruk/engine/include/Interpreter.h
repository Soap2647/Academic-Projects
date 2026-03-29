// ============================================================
// Interpreter.h — DORUK tree-walking yorumlayıcı (Adım 5)
//
// Kontrol akışı: C++ exception tabanlı sinyaller
//   DondurSinyali, KirSinyali, DevamSinyali
//
// Stdlib: uzunluk, tip, tamSayi, ondalik, metin,
//         liste, ekle, sil, ters, sirala, rastgele,
//         zamanDamgasi, oku, hata
//
// IDE entegrasyonu:
//   ciktiCallback_ — stdout yerine; IDE çıktı paneline yazar
//   girisCb_       — stdin yerine; IDE'den satır okur
// ============================================================
#pragma once

#include "AST.h"
#include "Value.h"
#include "Environment.h"
#include "Diagnostics.h"

#include <string>
#include <memory>
#include <functional>

namespace Doruk {

// ============================================================
// Kontrol akışı sinyalleri (exception tabanlı)
// ============================================================
struct DondurSinyali { DegerRef deger; };
struct KirSinyali    {};
struct DevamSinyali  {};

// ============================================================
// YorumSonucu
// ============================================================
struct YorumSonucu {
    bool        basarili = true;
    TaniListesi tanilar;
};

// ============================================================
// Interpreter
// ============================================================
class Interpreter : public Ziyaretci {
public:
    // ciktiCb  : yazdır çıktısı (varsayılan: stdout)
    // girisCb  : oku girişi    (varsayılan: stdin)
    using CiktiCb = std::function<void(const std::string&)>;
    using GirisCb = std::function<std::string()>;

    explicit Interpreter(CiktiCb ciktiCb = nullptr,
                         GirisCb girisCb = nullptr);

    // Programı çalıştır
    YorumSonucu calistir(Program& program);

    // Son değer (REPL için)
    DegerRef sonDeger() const { return sonDeger_; }

private:
    // ── Durum ────────────────────────────────────────────────
    std::shared_ptr<Environment> ortam_;       // mevcut kapsam
    std::shared_ptr<Environment> kureselOrtam_; // küresel kapsam
    DegerRef                     sonDeger_;    // son değerlendirilen
    TaniListesi                  tanilar_;
    std::string                  dosya_;

    CiktiCb ciktiCb_;
    GirisCb girisCb_;

    int cagriDerinligi_ = 0;
    static constexpr int MAX_CAGRI_DERINLIGI = 1000;

    // ── Yardımcılar ──────────────────────────────────────────
    void ciktiYaz(const std::string& s);
    std::string girisOku();

    // İfadeyi değerlendir
    DegerRef degerlendir(Ifade& ifade);

    // Deyimi çalıştır
    void calistirDeyim(Deyim& deyim);

    // Blok çalıştır (verilen ortamda)
    void blokCalistir(BlokDeyim& blok,
                      std::shared_ptr<Environment> ortam);

    // Fonksiyon çağrısı
    DegerRef fonksiyonCagir(DorukFonksiyon& fn,
                             std::vector<DegerRef> arglar,
                             uint32_t satir, uint32_t sutun);

    // Hata fırlat (CalismaHatasi)
    [[noreturn]] void hataFirlat(const std::string& mesaj,
                                  uint32_t satir, uint32_t sutun);

    // Yerleşik fonksiyon kaydı
    void yerlesikleriKaydet();

    // ── Ziyaretci: ifadeler ──────────────────────────────────
    void ziyaretSayi     (SayiLiteralIfade&    d) override;
    void ziyaretOndalik  (OndalikLiteralIfade& d) override;
    void ziyaretMetin    (MetinLiteralIfade&   d) override;
    void ziyaretBool     (BoolLiteralIfade&    d) override;
    void ziyaretBos      (BosLiteralIfade&     d) override;
    void ziyaretKimlik   (KimlikIfade&         d) override;
    void ziyaretIkili    (IkiliIfade&          d) override;
    void ziyaretTekli    (TekliIfade&          d) override;
    void ziyaretAtama    (AtamaIfade&          d) override;
    void ziyaretCagri    (CagriIfade&          d) override;
    void ziyaretIndis    (IndisIfade&          d) override;
    void ziyaretUye      (UyeIfade&            d) override;
    void ziyaretListe    (ListeIfade&          d) override;
    void ziyaretSozluk   (SozlukIfade&         d) override;
    void ziyaretYeni     (YeniIfade&           d) override;

    // ── Ziyaretci: deyimler ──────────────────────────────────
    void ziyaretBlok          (BlokDeyim&      d) override;
    void ziyaretIfade         (IfadeDeyim&     d) override;
    void ziyaretDegiskenTanim (DegiskenTanim&  d) override;
    void ziyaretFonksiyonTanim(FonksiyonTanim& d) override;
    void ziyaretSinifTanim    (SinifTanim&     d) override;
    void ziyaretEger          (EgerDeyim&      d) override;
    void ziyaretIcin          (IcinDeyim&      d) override;
    void ziyaretDongu         (DonguDeyim&     d) override;
    void ziyaretDondur        (DondurDeyim&    d) override;
    void ziyaretKir           (KirDeyim&       d) override;
    void ziyaretDevam         (DevamDeyim&     d) override;
    void ziyaretYazdir        (YazdirDeyim&    d) override;
    void ziyaretProgram       (Program&        d) override;
};

} // namespace Doruk

