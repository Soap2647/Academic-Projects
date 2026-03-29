// ============================================================
// AST.cpp — AST yazdırma yardımcısı (debug/test için)
// ============================================================
#include "AST.h"
#include "Token.h"

#include <sstream>
#include <string>

namespace Doruk {

// ============================================================
// ASTYazici — Ziyaretci implementasyonu (yazdırma için)
// ============================================================
class ASTYazici : public Ziyaretci {
public:
    explicit ASTYazici(std::ostringstream& ss) : ss_(ss), girinti_(0) {}

    // ── İfadeler ────────────────────────────────────────────
    void ziyaretSayi(SayiLiteralIfade& d) override {
        ss_ << d.deger;
    }
    void ziyaretOndalik(OndalikLiteralIfade& d) override {
        ss_ << d.deger;
    }
    void ziyaretMetin(MetinLiteralIfade& d) override {
        ss_ << '"' << d.deger << '"';
    }
    void ziyaretBool(BoolLiteralIfade& d) override {
        ss_ << (d.deger ? "doğru" : "yanlış");
    }
    void ziyaretBos(BosLiteralIfade&) override {
        ss_ << "boş";
    }
    void ziyaretKimlik(KimlikIfade& d) override {
        ss_ << d.isim;
    }
    void ziyaretIkili(IkiliIfade& d) override {
        ss_ << '(';
        d.sol->kabul(*this);
        ss_ << ' ' << d.op.lexem << ' ';
        d.sag->kabul(*this);
        ss_ << ')';
    }
    void ziyaretTekli(TekliIfade& d) override {
        ss_ << '(' << d.op.lexem;
        d.operand->kabul(*this);
        ss_ << ')';
    }
    void ziyaretAtama(AtamaIfade& d) override {
        ss_ << '(' << d.hedef << " = ";
        d.deger->kabul(*this);
        ss_ << ')';
    }
    void ziyaretCagri(CagriIfade& d) override {
        d.fonksiyon->kabul(*this);
        ss_ << '(';
        for (size_t i = 0; i < d.argumanlar.size(); ++i) {
            if (i) ss_ << ", ";
            d.argumanlar[i]->kabul(*this);
        }
        ss_ << ')';
    }
    void ziyaretIndis(IndisIfade& d) override {
        d.nesne->kabul(*this);
        ss_ << '[';
        d.indis->kabul(*this);
        ss_ << ']';
    }
    void ziyaretUye(UyeIfade& d) override {
        d.nesne->kabul(*this);
        ss_ << '.' << d.alan;
    }
    void ziyaretListe(ListeIfade& d) override {
        ss_ << '[';
        for (size_t i = 0; i < d.elemanlar.size(); ++i) {
            if (i) ss_ << ", ";
            d.elemanlar[i]->kabul(*this);
        }
        ss_ << ']';
    }
    void ziyaretSozluk(SozlukIfade& d) override {
        ss_ << '{';
        for (size_t i = 0; i < d.cifter.size(); ++i) {
            if (i) ss_ << ", ";
            d.cifter[i].first->kabul(*this);
            ss_ << ": ";
            d.cifter[i].second->kabul(*this);
        }
        ss_ << '}';
    }
    void ziyaretYeni(YeniIfade& d) override {
        ss_ << "yeni " << d.sinifIsmi << '(';
        for (size_t i = 0; i < d.argumanlar.size(); ++i) {
            if (i) ss_ << ", ";
            d.argumanlar[i]->kabul(*this);
        }
        ss_ << ')';
    }

    // ── Deyimler ────────────────────────────────────────────
    void ziyaretBlok(BlokDeyim& d) override {
        satirBas();
        ss_ << "{\n";
        ++girinti_;
        for (auto& deyim : d.deyimler) {
            deyim->kabul(*this);
        }
        --girinti_;
        satirBas();
        ss_ << "}\n";
    }
    void ziyaretIfade(IfadeDeyim& d) override {
        satirBas();
        d.ifade->kabul(*this);
        ss_ << '\n';
    }
    void ziyaretDegiskenTanim(DegiskenTanim& d) override {
        satirBas();
        ss_ << "değişken " << d.isim;
        if (d.baslangicDegeri) {
            ss_ << " = ";
            d.baslangicDegeri->kabul(*this);
        }
        ss_ << '\n';
    }
    void ziyaretFonksiyonTanim(FonksiyonTanim& d) override {
        satirBas();
        ss_ << "fonksiyon " << d.isim << '(';
        for (size_t i = 0; i < d.parametreler.size(); ++i) {
            if (i) ss_ << ", ";
            ss_ << d.parametreler[i].isim;
        }
        ss_ << ") ";
        if (d.govde) d.govde->kabul(*this);
    }
    void ziyaretSinifTanim(SinifTanim& d) override {
        satirBas();
        ss_ << "sınıf " << d.isim;
        if (!d.ebeveyn.empty()) ss_ << " < " << d.ebeveyn;
        ss_ << " {\n";
        ++girinti_;
        for (auto& alan : d.alanlar) {
            alan->kabul(*this);
        }
        for (auto& metod : d.metodlar) {
            metod->kabul(*this);
        }
        --girinti_;
        satirBas();
        ss_ << "}\n";
    }
    void ziyaretEger(EgerDeyim& d) override {
        satirBas();
        ss_ << "eğer (";
        d.kosul->kabul(*this);
        ss_ << ") ";
        if (d.dogruKol) d.dogruKol->kabul(*this);
        if (d.yanlisKol) {
            satirBas();
            ss_ << "değilse ";
            d.yanlisKol->kabul(*this);
        }
    }
    void ziyaretIcin(IcinDeyim& d) override {
        satirBas();
        ss_ << "için (";
        if (d.baslangic) d.baslangic->kabul(*this);
        ss_ << "; ";
        if (d.kosul) d.kosul->kabul(*this);
        ss_ << "; ";
        if (d.arttir) d.arttir->kabul(*this);
        ss_ << ") ";
        if (d.govde) d.govde->kabul(*this);
    }
    void ziyaretDongu(DonguDeyim& d) override {
        satirBas();
        ss_ << "döngü (";
        d.kosul->kabul(*this);
        ss_ << ") ";
        if (d.govde) d.govde->kabul(*this);
    }
    void ziyaretDondur(DondurDeyim& d) override {
        satirBas();
        ss_ << "döndür";
        if (d.deger) {
            ss_ << ' ';
            d.deger->kabul(*this);
        }
        ss_ << '\n';
    }
    void ziyaretKir(KirDeyim&) override {
        satirBas();
        ss_ << "kır\n";
    }
    void ziyaretDevam(DevamDeyim&) override {
        satirBas();
        ss_ << "devam\n";
    }
    void ziyaretYazdir(YazdirDeyim& d) override {
        satirBas();
        ss_ << "yazdır(";
        for (size_t i = 0; i < d.argumanlar.size(); ++i) {
            if (i) ss_ << ", ";
            d.argumanlar[i]->kabul(*this);
        }
        ss_ << ")\n";
    }
    void ziyaretProgram(Program& d) override {
        for (auto& deyim : d.deyimler) {
            deyim->kabul(*this);
        }
    }

private:
    std::ostringstream& ss_;
    int                 girinti_;

    void satirBas() {
        for (int i = 0; i < girinti_ * 4; ++i) ss_ << ' ';
    }
};

// ============================================================
// astYazdir — dışa açık fonksiyon
// ============================================================
std::string astYazdir(const Program& program) {
    std::ostringstream ss;
    ASTYazici yazici(ss);

    // const_cast: Ziyaretci non-const referans alıyor (değiştirmez ama)
    const_cast<Program&>(program).kabul(yazici);

    return ss.str();
}

} // namespace Doruk


