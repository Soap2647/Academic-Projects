// ============================================================
// SemanticAnalyzer.cpp — semantik analiz implementasyonu
// ============================================================
#include "SemanticAnalyzer.h"

namespace Doruk {

// ============================================================
// Kurucu + analiz
// ============================================================

SemanticAnalyzer::SemanticAnalyzer(std::string dosya)
    : dosya_(std::move(dosya))
{}

AnlamSonucu SemanticAnalyzer::analiz(Program& program) {
    tanilar_ = TaniListesi{};
    kapsamlar_.clear();
    fonksiyonlar_.clear();
    donguDerinligi_     = 0;
    fonksiyonDerinligi_ = 0;

    kapsamAc(); // küresel kapsam
    program.kabul(*this);
    kapsamKapat();

    return AnlamSonucu{ std::move(tanilar_) };
}

// ============================================================
// Kapsam yönetimi
// ============================================================

void SemanticAnalyzer::kapsamAc() {
    kapsamlar_.push_back({});
}

void SemanticAnalyzer::kapsamKapat() {
    if (!kapsamlar_.empty()) kapsamlar_.pop_back();
}

void SemanticAnalyzer::tanimla(const std::string& isim,
                                uint32_t satir, uint32_t sutun) {
    if (kapsamlar_.empty()) return;
    auto& kapsam = kapsamlar_.back();
    if (kapsam.count(isim)) {
        uyari(satir, sutun,
              "'" + isim + "' bu kapsamda zaten tanımlı.");
              // zaten tanımlı
    }
    kapsam.insert(isim);
}

bool SemanticAnalyzer::tanimliMi(const std::string& isim) const {
    // İç kapsamdan dışa doğru ara
    for (int i = static_cast<int>(kapsamlar_.size()) - 1; i >= 0; --i) {
        if (kapsamlar_[i].count(isim)) return true;
    }
    // Yerleşik fonksiyonlar
    static const std::unordered_set<std::string> yerlesikler = {
        "uzunluk", "tip", "tamSayi", "ondalik", "metin",
        "liste", "ekle", "sil", "ters", "sirala",
        "rastgele", "zamanDamgasi", "oku", "hata"
    };
    return yerlesikler.count(isim) > 0;
}

// ============================================================
// Hata yardımcıları
// ============================================================

void SemanticAnalyzer::hata(uint32_t satir, uint32_t sutun,
                              std::string mesaj) {
    tanilar_.ekle(anlamHatasi(std::move(mesaj), dosya_, satir, sutun));
}

void SemanticAnalyzer::uyari(uint32_t satir, uint32_t sutun,
                               std::string mesaj) {
    tanilar_.ekle(anlamUyarisi(dosya_, satir, sutun, std::move(mesaj)));
}

// ============================================================
// Program
// ============================================================

void SemanticAnalyzer::ziyaretProgram(Program& d) {
    // Önce tüm fonksiyon isimlerini kaydet (forward declaration desteği)
    for (auto& deyim : d.deyimler) {
        if (auto* fn = dynamic_cast<FonksiyonTanim*>(deyim.get())) {
            fonksiyonlar_[fn->isim] = fn->parametreler.size();
            tanimla(fn->isim, fn->satir, fn->sutun);
        }
        if (auto* sn = dynamic_cast<SinifTanim*>(deyim.get())) {
            tanimla(sn->isim, sn->satir, sn->sutun);
        }
    }
    for (auto& deyim : d.deyimler) deyim->kabul(*this);
}

// ============================================================
// Blok
// ============================================================

void SemanticAnalyzer::ziyaretBlok(BlokDeyim& d) {
    kapsamAc();
    for (auto& deyim : d.deyimler) deyim->kabul(*this);
    kapsamKapat();
}

// ============================================================
// İfade deyimi
// ============================================================

void SemanticAnalyzer::ziyaretIfade(IfadeDeyim& d) {
    d.ifade->kabul(*this);
}

// ============================================================
// Değişken tanımı
// ============================================================

void SemanticAnalyzer::ziyaretDegiskenTanim(DegiskenTanim& d) {
    if (d.baslangicDegeri) d.baslangicDegeri->kabul(*this);
    tanimla(d.isim, d.satir, d.sutun);
}

// ============================================================
// Fonksiyon tanımı
// ============================================================

void SemanticAnalyzer::ziyaretFonksiyonTanim(FonksiyonTanim& d) {
    // İsim zaten Program'da kaydedildi
    kapsamAc();
    ++fonksiyonDerinligi_;

    for (auto& p : d.parametreler) tanimla(p.isim, p.satir, p.sutun);

    if (d.govde) d.govde->kabul(*this);

    --fonksiyonDerinligi_;
    kapsamKapat();
}

// ============================================================
// Sınıf tanımı
// ============================================================

void SemanticAnalyzer::ziyaretSinifTanim(SinifTanim& d) {
    // Ebeveyn kontrolü
    if (!d.ebeveyn.empty() && !tanimliMi(d.ebeveyn)) {
        hata(d.satir, d.sutun,
             "Bilinmeyen ebeveyn sınıf: '" + d.ebeveyn + "'");
             // Bilinmeyen ebeveyn sınıf
    }

    kapsamAc();
    ++fonksiyonDerinligi_;
    tanimla("bu", d.satir, d.sutun); // 'bu' anahtar kelimesi

    for (auto& alan : d.alanlar) alan->kabul(*this);
    for (auto& metod : d.metodlar) metod->kabul(*this);

    --fonksiyonDerinligi_;
    kapsamKapat();
}

// ============================================================
// Eğer deyimi
// ============================================================

void SemanticAnalyzer::ziyaretEger(EgerDeyim& d) {
    d.kosul->kabul(*this);
    if (d.dogruKol)  d.dogruKol->kabul(*this);
    if (d.yanlisKol) d.yanlisKol->kabul(*this);
}

// ============================================================
// İçin döngüsü
// ============================================================

void SemanticAnalyzer::ziyaretIcin(IcinDeyim& d) {
    kapsamAc();
    ++donguDerinligi_;

    if (d.baslangic) d.baslangic->kabul(*this);
    if (d.kosul)     d.kosul->kabul(*this);
    if (d.arttir)    d.arttir->kabul(*this);
    if (d.govde)     d.govde->kabul(*this);

    --donguDerinligi_;
    kapsamKapat();
}

// ============================================================
// Döngü (while)
// ============================================================

void SemanticAnalyzer::ziyaretDongu(DonguDeyim& d) {
    ++donguDerinligi_;
    d.kosul->kabul(*this);
    if (d.govde) d.govde->kabul(*this);
    --donguDerinligi_;
}

// ============================================================
// Döndür
// ============================================================

void SemanticAnalyzer::ziyaretDondur(DondurDeyim& d) {
    if (fonksiyonDerinligi_ == 0) {
        hata(d.satir, d.sutun,
             "'döndür' fonksiyon dışında kullanılamaz.");
             // 'döndür' fonksiyon dışında kullanılamaz.
    }
    if (d.deger) d.deger->kabul(*this);
}

// ============================================================
// Kır / Devam
// ============================================================

void SemanticAnalyzer::ziyaretKir(KirDeyim& d) {
    if (donguDerinligi_ == 0) {
        hata(d.satir, d.sutun,
             "'kır' döngü dışında kullanılamaz.");
             // 'kır' döngü dışında kullanılamaz.
    }
}

void SemanticAnalyzer::ziyaretDevam(DevamDeyim& d) {
    if (donguDerinligi_ == 0) {
        hata(d.satir, d.sutun,
             "'devam' döngü dışında kullanılamaz.");
             // 'devam' döngü dışında kullanılamaz.
    }
}

// ============================================================
// Yazdır
// ============================================================

void SemanticAnalyzer::ziyaretYazdir(YazdirDeyim& d) {
    for (auto& arg : d.argumanlar) arg->kabul(*this);
}

// ============================================================
// İfadeler
// ============================================================

void SemanticAnalyzer::ziyaretKimlik(KimlikIfade& d) {
    if (!tanimliMi(d.isim)) {
        hata(d.satir, d.sutun,
             "Tanımsız değişken: '" + d.isim + "'");
             // Tanımsız değişken
    }
}

void SemanticAnalyzer::ziyaretIkili(IkiliIfade& d) {
    d.sol->kabul(*this);
    d.sag->kabul(*this);
}

void SemanticAnalyzer::ziyaretTekli(TekliIfade& d) {
    d.operand->kabul(*this);
}

void SemanticAnalyzer::ziyaretAtama(AtamaIfade& d) {
    if (!tanimliMi(d.hedef)) {
        hata(d.satir, d.sutun,
             "Tanımsız değişken: '" + d.hedef + "'");
    }
    d.deger->kabul(*this);
}

void SemanticAnalyzer::ziyaretCagri(CagriIfade& d) {
    d.fonksiyon->kabul(*this);

    // Bilinen kullanıcı fonksiyonları için argüman sayısı kontrolü
    if (auto* kimlik = dynamic_cast<KimlikIfade*>(d.fonksiyon.get())) {
        auto it = fonksiyonlar_.find(kimlik->isim);
        if (it != fonksiyonlar_.end()) {
            size_t beklenen = it->second;
            size_t verilen  = d.argumanlar.size();
            if (beklenen != verilen) {
                hata(d.satir, d.sutun,
                     "'" + kimlik->isim + "' " +
                     std::to_string(beklenen) + " argüman bekliyor, " +
                     // argüman
                     std::to_string(verilen) + " verildi.");
            }
        }
    }

    for (auto& arg : d.argumanlar) arg->kabul(*this);
}

void SemanticAnalyzer::ziyaretIndis(IndisIfade& d) {
    d.nesne->kabul(*this);
    d.indis->kabul(*this);
}

void SemanticAnalyzer::ziyaretUye(UyeIfade& d) {
    d.nesne->kabul(*this);
}

void SemanticAnalyzer::ziyaretListe(ListeIfade& d) {
    for (auto& e : d.elemanlar) e->kabul(*this);
}

void SemanticAnalyzer::ziyaretSozluk(SozlukIfade& d) {
    for (auto& cift : d.cifter) {
        cift.first->kabul(*this);
        cift.second->kabul(*this);
    }
}

void SemanticAnalyzer::ziyaretYeni(YeniIfade& d) {
    if (!tanimliMi(d.sinifIsmi)) {
        hata(d.satir, d.sutun,
             "Bilinmeyen sınıf: '" + d.sinifIsmi + "'");
             // Bilinmeyen sınıf
    }
    for (auto& arg : d.argumanlar) arg->kabul(*this);
}

} // namespace Doruk


