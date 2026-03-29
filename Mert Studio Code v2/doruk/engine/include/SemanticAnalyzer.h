// ============================================================
// SemanticAnalyzer.h — DORUK semantik analiz (Adım 4)
//
// Kontroller:
//   - Tanımsız değişken kullanımı
//   - Değişken çift tanımı (aynı kapsamda)
//   - Fonksiyon çağrısında argüman sayısı uyuşmazlığı
//   - kır/devam döngü dışında kullanımı
//   - döndür fonksiyon dışında kullanımı
//
// Çıktı: TaniListesi (hatalar + uyarılar)
// AST değiştirilmez — salt okuma geçişi.
// ============================================================
#pragma once

#include "AST.h"
#include "Diagnostics.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Doruk {

// ============================================================
// AnlamSonucu
// ============================================================
struct AnlamSonucu {
    TaniListesi tanilar;
    bool basarili() const { return !tanilar.hatalarVar(); }
};

// ============================================================
// SemanticAnalyzer
// ============================================================
class SemanticAnalyzer : public Ziyaretci {
public:
    explicit SemanticAnalyzer(std::string dosya = "<isimsiz>");

    AnlamSonucu analiz(Program& program);

private:
    // ── Durum ────────────────────────────────────────────────
    std::string  dosya_;
    TaniListesi  tanilar_;

    // Kapsam yığını: her katman tanımlı isimler kümesi
    std::vector<std::unordered_set<std::string>> kapsamlar_;

    // Fonksiyon parametreleri (mevcut fonksiyonun)
    struct FonkBilgi {
        std::string              isim;
        size_t                   paramSayisi;
    };
    // Bilinen fonksiyonlar (isim → param sayısı)
    std::unordered_map<std::string, size_t> fonksiyonlar_;

    int donguDerinligi_    = 0;  // kır/devam kontrolü için
    int fonksiyonDerinligi_= 0;  // döndür kontrolü için

    // ── Kapsam yönetimi ──────────────────────────────────────
    void kapsamAc();
    void kapsamKapat();
    void tanimla(const std::string& isim, uint32_t satir, uint32_t sutun);
    bool tanimliMi(const std::string& isim) const;

    // ── Hata yardımcıları ────────────────────────────────────
    void hata(uint32_t satir, uint32_t sutun, std::string mesaj);
    void uyari(uint32_t satir, uint32_t sutun, std::string mesaj);

    // ── Ziyaretci implementasyonu ─────────────────────────────

    // İfadeler
    void ziyaretSayi     (SayiLiteralIfade&)    override {}
    void ziyaretOndalik  (OndalikLiteralIfade&)  override {}
    void ziyaretMetin    (MetinLiteralIfade&)    override {}
    void ziyaretBool     (BoolLiteralIfade&)     override {}
    void ziyaretBos      (BosLiteralIfade&)      override {}
    void ziyaretKimlik   (KimlikIfade&    d)     override;
    void ziyaretIkili    (IkiliIfade&     d)     override;
    void ziyaretTekli    (TekliIfade&     d)     override;
    void ziyaretAtama    (AtamaIfade&     d)     override;
    void ziyaretCagri    (CagriIfade&     d)     override;
    void ziyaretIndis    (IndisIfade&     d)     override;
    void ziyaretUye      (UyeIfade&       d)     override;
    void ziyaretListe    (ListeIfade&     d)     override;
    void ziyaretSozluk   (SozlukIfade&   d)     override;
    void ziyaretYeni     (YeniIfade&     d)      override;

    // Deyimler
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

