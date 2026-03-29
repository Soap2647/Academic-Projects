// ============================================================
// Interpreter.cpp — DORUK tree-walking yorumlayıcı
// ============================================================
#include "Interpreter.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace Doruk {

// ============================================================
// Kurucu
// ============================================================

Interpreter::Interpreter(CiktiCb ciktiCb, GirisCb girisCb)
    : ciktiCb_(std::move(ciktiCb))
    , girisCb_(std::move(girisCb))
{
    kureselOrtam_ = std::make_shared<Environment>();
    ortam_        = kureselOrtam_;
    sonDeger_     = BOSDeger();

    yerlesikleriKaydet();
}

// ============================================================
// calistir
// ============================================================

YorumSonucu Interpreter::calistir(Program& program) {
    tanilar_ = TaniListesi{};
    dosya_   = program.dosyaAdi;

    try {
        program.kabul(*this);
    }
    catch (const CalismaHatasi& h) {
        tanilar_.ekle(calismaHatasi(h.mesaj, dosya_, h.satir, h.sutun));
        return YorumSonucu{ false, std::move(tanilar_) };
    }

    return YorumSonucu{ !tanilar_.hatalarVar(), std::move(tanilar_) };
}

// ============================================================
// Yardımcılar
// ============================================================

void Interpreter::ciktiYaz(const std::string& s) {
    if (ciktiCb_) ciktiCb_(s);
    else          std::cout << s;
}

std::string Interpreter::girisOku() {
    if (girisCb_) return girisCb_();
    std::string satir;
    std::getline(std::cin, satir);
    return satir;
}

DegerRef Interpreter::degerlendir(Ifade& ifade) {
    sonDeger_ = BOSDeger();
    ifade.kabul(*this);
    return sonDeger_;
}

void Interpreter::calistirDeyim(Deyim& deyim) {
    deyim.kabul(*this);
}

void Interpreter::blokCalistir(BlokDeyim& blok,
                                std::shared_ptr<Environment> ortam) {
    auto oncekiOrtam = ortam_;
    ortam_ = std::move(ortam);
    try {
        for (auto& d : blok.deyimler) calistirDeyim(*d);
    }
    catch (...) {
        ortam_ = oncekiOrtam;
        throw;
    }
    ortam_ = oncekiOrtam;
}

DegerRef Interpreter::fonksiyonCagir(DorukFonksiyon& fn,
                                      std::vector<DegerRef> arglar,
                                      uint32_t satir, uint32_t sutun) {
    if (fn.parametreler.size() != arglar.size()) {
        hataFirlat("'" + fn.isim + "' " +
                   std::to_string(fn.parametreler.size()) +
                   " argüman bekliyor, " +
                   std::to_string(arglar.size()) + " verildi.",
                   satir, sutun);
    }

    if (++cagriDerinligi_ > MAX_CAGRI_DERINLIGI) {
        --cagriDerinligi_;
        hataFirlat("Maksimum çağrı derinliği aşıldı (sonsuz özyınım?).",
                   // Maksimum çağrı derinliği aşıldı (sonsuz özyınım?)
                   satir, sutun);
    }

    auto kapsamOrtam = std::make_shared<Environment>(fn.kapatma);
    for (size_t i = 0; i < fn.parametreler.size(); ++i)
        kapsamOrtam->tanimla(fn.parametreler[i], arglar[i]);

    DegerRef sonuc = BOSDeger();
    try {
        blokCalistir(*fn.govde, kapsamOrtam);
    }
    catch (DondurSinyali& s) {
        sonuc = s.deger ? s.deger : BOSDeger();
    }

    --cagriDerinligi_;
    return sonuc;
}

[[noreturn]] void Interpreter::hataFirlat(const std::string& mesaj,
                                            uint32_t satir, uint32_t sutun) {
    throw CalismaHatasi(mesaj, satir, sutun);
}

// ============================================================
// Yerleşik fonksiyonlar
// ============================================================

void Interpreter::yerlesikleriKaydet() {

    // uzunluk(x) → tam sayı
    ortam_->tanimla("uzunluk",
        DorukDeger::yerlesikOlstur("uzunluk", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() != 1) throw CalismaHatasi("uzunluk: 1 arguman bekleniyor.");
            auto& d = *args[0];
            if (d.tur == DorukDeger::Tur::METIN)
                return DorukDeger::tamSayiOlstur(static_cast<int64_t>(d.metin.size()));
            if (d.tur == DorukDeger::Tur::LISTE && d.liste)
                return DorukDeger::tamSayiOlstur(static_cast<int64_t>(d.liste->elemanlar.size()));
            if (d.tur == DorukDeger::Tur::SOZLUK && d.sozluk)
                return DorukDeger::tamSayiOlstur(static_cast<int64_t>(d.sozluk->cifter.size()));
            throw CalismaHatasi("uzunluk: metin, liste veya sozluk bekleniyor.");
        })
    );

    // tip(x) → metin
    ortam_->tanimla("tip",
        DorukDeger::yerlesikOlstur("tip", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() != 1) throw CalismaHatasi("tip: 1 arguman bekleniyor.");
            return DorukDeger::metinOlstur(args[0]->turAdi());
        })
    );

    // tamSayi(x) → tam sayıya çevir
    ortam_->tanimla("tamSayi",
        DorukDeger::yerlesikOlstur("tamSayi", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() != 1) throw CalismaHatasi("tamSayi: 1 arguman bekleniyor.");
            auto& d = *args[0];
            switch (d.tur) {
                case DorukDeger::Tur::TAM_SAYI:  return args[0];
                case DorukDeger::Tur::ONDALIK:   return DorukDeger::tamSayiOlstur(static_cast<int64_t>(d.ondalik));
                case DorukDeger::Tur::METIN:     try { return DorukDeger::tamSayiOlstur(std::stoll(d.metin)); }
                                                 catch (...) { throw CalismaHatasi("tamSayi: gecersiz metin: '" + d.metin + "'"); }
                case DorukDeger::Tur::MANTIKSAL: return DorukDeger::tamSayiOlstur(d.mantiksal ? 1 : 0);
                default: throw CalismaHatasi("tamSayi: donusturulemez tip: " + d.turAdi());
            }
        })
    );

    // ondalik(x) → ondalık sayıya çevir
    ortam_->tanimla("ondalik",
        DorukDeger::yerlesikOlstur("ondalik", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() != 1) throw CalismaHatasi("ondalik: 1 arguman bekleniyor.");
            auto& d = *args[0];
            switch (d.tur) {
                case DorukDeger::Tur::ONDALIK:   return args[0];
                case DorukDeger::Tur::TAM_SAYI:  return DorukDeger::ondalikOlstur(static_cast<double>(d.tamSayi));
                case DorukDeger::Tur::METIN:     try { return DorukDeger::ondalikOlstur(std::stod(d.metin)); }
                                                 catch (...) { throw CalismaHatasi("ondalik: gecersiz metin: '" + d.metin + "'"); }
                case DorukDeger::Tur::MANTIKSAL: return DorukDeger::ondalikOlstur(d.mantiksal ? 1.0 : 0.0);
                default: throw CalismaHatasi("ondalik: donusturulemez tip: " + d.turAdi());
            }
        })
    );

    // metin(x) → metine çevir
    ortam_->tanimla("metin",
        DorukDeger::yerlesikOlstur("metin", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() != 1) throw CalismaHatasi("metin: 1 arguman bekleniyor.");
            return DorukDeger::metinOlstur(args[0]->metineCevir());
        })
    );

    // liste(uzunluk, baslangic?) → dizi
    ortam_->tanimla("liste",
        DorukDeger::yerlesikOlstur("liste", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.empty() || args.size() > 2)
                throw CalismaHatasi("liste: 1 veya 2 arguman bekleniyor.");
            auto& ilk = *args[0];
            if (ilk.tur != DorukDeger::Tur::TAM_SAYI)
                throw CalismaHatasi("liste: ilk arguman tam_sayi olmali.");
            int64_t uzunluk = ilk.tamSayi;
            if (uzunluk < 0) throw CalismaHatasi("liste: negatif uzunluk.");
            DegerRef baslangic = (args.size() == 2) ? args[1] : BOSDeger();
            std::vector<DegerRef> elemanlar(static_cast<size_t>(uzunluk), baslangic);
            return DorukDeger::listeOlstur(std::move(elemanlar));
        })
    );

    // ekle(liste, eleman) veya ekle(liste, indis, eleman)
    ortam_->tanimla("ekle",
        DorukDeger::yerlesikOlstur("ekle", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() < 2 || args.size() > 3)
                throw CalismaHatasi("ekle: 2 veya 3 arguman bekleniyor.");
            auto& lst = *args[0];
            if (lst.tur != DorukDeger::Tur::LISTE || !lst.liste)
                throw CalismaHatasi("ekle: ilk arguman liste olmali.");
            if (args.size() == 2) {
                lst.liste->elemanlar.push_back(args[1]);
            } else {
                auto& indisD = *args[1];
                if (indisD.tur != DorukDeger::Tur::TAM_SAYI)
                    throw CalismaHatasi("ekle: indis tam_sayi olmali.");
                int64_t idx = indisD.tamSayi;
                int64_t sz  = static_cast<int64_t>(lst.liste->elemanlar.size());
                if (idx < 0) idx += sz + 1;
                if (idx < 0 || idx > sz) throw CalismaHatasi("ekle: indis sinir disi.");
                lst.liste->elemanlar.insert(
                    lst.liste->elemanlar.begin() + idx, args[2]);
            }
            return BOSDeger();
        })
    );

    // sil(liste, indis) veya sil(sozluk, anahtar)
    ortam_->tanimla("sil",
        DorukDeger::yerlesikOlstur("sil", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() != 2) throw CalismaHatasi("sil: 2 arguman bekleniyor.");
            auto& kap = *args[0];
            if (kap.tur == DorukDeger::Tur::LISTE && kap.liste) {
                auto& idxD = *args[1];
                if (idxD.tur != DorukDeger::Tur::TAM_SAYI)
                    throw CalismaHatasi("sil(liste): indis tam_sayi olmali.");
                int64_t idx = idxD.tamSayi;
                int64_t sz  = static_cast<int64_t>(kap.liste->elemanlar.size());
                if (idx < 0) idx += sz;
                if (idx < 0 || idx >= sz) throw CalismaHatasi("sil: indis sinir disi.");
                auto silinen = kap.liste->elemanlar[static_cast<size_t>(idx)];
                kap.liste->elemanlar.erase(
                    kap.liste->elemanlar.begin() + idx);
                return silinen;
            }
            if (kap.tur == DorukDeger::Tur::SOZLUK && kap.sozluk) {
                auto& anahtarD = *args[1];
                if (anahtarD.tur != DorukDeger::Tur::METIN)
                    throw CalismaHatasi("sil(sozluk): anahtar metin olmali.");
                auto it = kap.sozluk->cifter.find(anahtarD.metin);
                if (it == kap.sozluk->cifter.end()) return BOSDeger();
                auto silinen = it->second;
                kap.sozluk->cifter.erase(it);
                auto& sira = kap.sozluk->anahtarSirasi;
                sira.erase(std::remove(sira.begin(), sira.end(), anahtarD.metin), sira.end());
                return silinen;
            }
            throw CalismaHatasi("sil: liste veya sozluk bekleniyor.");
        })
    );

    // ters(liste) → listeyi tersine çevir (yerinde)
    ortam_->tanimla("ters",
        DorukDeger::yerlesikOlstur("ters", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() != 1) throw CalismaHatasi("ters: 1 arguman bekleniyor.");
            auto& d = *args[0];
            if (d.tur != DorukDeger::Tur::LISTE || !d.liste)
                throw CalismaHatasi("ters: liste bekleniyor.");
            std::reverse(d.liste->elemanlar.begin(), d.liste->elemanlar.end());
            return args[0];
        })
    );

    // sirala(liste) → listeyi sırala (sayısal veya metinsel)
    ortam_->tanimla("sirala",
        DorukDeger::yerlesikOlstur("sirala", [](std::vector<DegerRef> args) -> DegerRef {
            if (args.size() != 1) throw CalismaHatasi("sirala: 1 arguman bekleniyor.");
            auto& d = *args[0];
            if (d.tur != DorukDeger::Tur::LISTE || !d.liste)
                throw CalismaHatasi("sirala: liste bekleniyor.");
            std::sort(d.liste->elemanlar.begin(), d.liste->elemanlar.end(),
                [](const DegerRef& a, const DegerRef& b) -> bool {
                    if (!a || !b) return false;
                    if (a->tur == DorukDeger::Tur::TAM_SAYI && b->tur == DorukDeger::Tur::TAM_SAYI)
                        return a->tamSayi < b->tamSayi;
                    if (a->tur == DorukDeger::Tur::ONDALIK && b->tur == DorukDeger::Tur::ONDALIK)
                        return a->ondalik < b->ondalik;
                    if (a->tur == DorukDeger::Tur::METIN && b->tur == DorukDeger::Tur::METIN)
                        return a->metin < b->metin;
                    return a->metineCevir() < b->metineCevir();
                });
            return args[0];
        })
    );

    // rastgele(min, maks) → tam sayı
    ortam_->tanimla("rastgele",
        DorukDeger::yerlesikOlstur("rastgele", [](std::vector<DegerRef> args) -> DegerRef {
            static bool baslatildi = false;
            if (!baslatildi) { std::srand(static_cast<unsigned>(std::time(nullptr))); baslatildi = true; }
            if (args.size() == 0) {
                return DorukDeger::ondalikOlstur(static_cast<double>(std::rand()) / RAND_MAX);
            }
            if (args.size() == 2) {
                auto& mn = *args[0]; auto& mx = *args[1];
                if (mn.tur != DorukDeger::Tur::TAM_SAYI || mx.tur != DorukDeger::Tur::TAM_SAYI)
                    throw CalismaHatasi("rastgele: tam_sayi argumanlari bekleniyor.");
                if (mn.tamSayi > mx.tamSayi) throw CalismaHatasi("rastgele: min > maks.");
                int64_t aralik = mx.tamSayi - mn.tamSayi + 1;
                return DorukDeger::tamSayiOlstur(mn.tamSayi + (std::rand() % aralik));
            }
            throw CalismaHatasi("rastgele: 0 veya 2 arguman bekleniyor.");
        })
    );

    // zamanDamgasi() → milisaniye (int64)
    ortam_->tanimla("zamanDamgasi",
        DorukDeger::yerlesikOlstur("zamanDamgasi", [](std::vector<DegerRef>) -> DegerRef {
            auto now = std::chrono::steady_clock::now().time_since_epoch();
            auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
            return DorukDeger::tamSayiOlstur(static_cast<int64_t>(ms));
        })
    );

    // oku(istem?) → metin (stdin'den satır)
    ortam_->tanimla("oku",
        DorukDeger::yerlesikOlstur("oku", [this](std::vector<DegerRef> args) -> DegerRef {
            if (!args.empty()) ciktiYaz(args[0]->metineCevir());
            return DorukDeger::metinOlstur(girisOku());
        })
    );

    // hata(mesaj) → çalışma zamanı hatası fırlat
    ortam_->tanimla("hata",
        DorukDeger::yerlesikOlstur("hata", [](std::vector<DegerRef> args) -> DegerRef {
            std::string mesaj = args.empty() ? "hata" : args[0]->metineCevir();
            throw CalismaHatasi(mesaj, 0, 0);
        })
    );
}

// ============================================================
// Program
// ============================================================

void Interpreter::ziyaretProgram(Program& d) {
    dosya_ = d.dosyaAdi;
    for (auto& deyim : d.deyimler) calistirDeyim(*deyim);
}

// ============================================================
// Literal ifadeler
// ============================================================

void Interpreter::ziyaretSayi(SayiLiteralIfade& d) {
    sonDeger_ = DorukDeger::tamSayiOlstur(d.deger);
}

void Interpreter::ziyaretOndalik(OndalikLiteralIfade& d) {
    sonDeger_ = DorukDeger::ondalikOlstur(d.deger);
}

void Interpreter::ziyaretMetin(MetinLiteralIfade& d) {
    sonDeger_ = DorukDeger::metinOlstur(d.deger);
}

void Interpreter::ziyaretBool(BoolLiteralIfade& d) {
    sonDeger_ = DorukDeger::mantiksalOlstur(d.deger);
}

void Interpreter::ziyaretBos(BosLiteralIfade&) {
    sonDeger_ = BOSDeger();
}

// ============================================================
// Kimlik (değişken okuma)
// ============================================================

void Interpreter::ziyaretKimlik(KimlikIfade& d) {
    sonDeger_ = ortam_->oku(d.isim, d.satir, d.sutun);
}

// ============================================================
// Tekli operatör
// ============================================================

void Interpreter::ziyaretTekli(TekliIfade& d) {
    auto operand = degerlendir(*d.operand);
    auto tur = d.op.tur;

    if (tur == TokenTuru::EKSI) {
        if (operand->tur == DorukDeger::Tur::TAM_SAYI)
            sonDeger_ = DorukDeger::tamSayiOlstur(-operand->tamSayi);
        else if (operand->tur == DorukDeger::Tur::ONDALIK)
            sonDeger_ = DorukDeger::ondalikOlstur(-operand->ondalik);
        else
            hataFirlat("'-' operatoru sadece sayilar icin gecerli.",
                        d.op.satir, d.op.sutun);
    }
    else if (tur == TokenTuru::UNLEM || tur == TokenTuru::DEGIL) {
        sonDeger_ = DorukDeger::mantiksalOlstur(!operand->gercegiDeger());
    }
    else {
        hataFirlat("Bilinmeyen tekli operator: " + d.op.lexem,
                    d.op.satir, d.op.sutun);
    }
}

// ============================================================
// İkili operatör
// ============================================================

void Interpreter::ziyaretIkili(IkiliIfade& d) {
    // Kısa devre değerlendirme
    auto tur = d.op.tur;
    if (tur == TokenTuru::VE_VE || tur == TokenTuru::VE) {
        auto sol = degerlendir(*d.sol);
        if (!sol->gercegiDeger()) { sonDeger_ = sol; return; }
        sonDeger_ = degerlendir(*d.sag);
        return;
    }
    if (tur == TokenTuru::VEYA_VEYA || tur == TokenTuru::VEYA) {
        auto sol = degerlendir(*d.sol);
        if (sol->gercegiDeger()) { sonDeger_ = sol; return; }
        sonDeger_ = degerlendir(*d.sag);
        return;
    }

    auto sol = degerlendir(*d.sol);
    auto sag = degerlendir(*d.sag);
    auto sl = sol.get();
    auto sr = sag.get();

    switch (tur) {
    // Aritmetik
    case TokenTuru::ARTI: {
        if (sl->tur == DorukDeger::Tur::METIN || sr->tur == DorukDeger::Tur::METIN) {
            sonDeger_ = DorukDeger::metinOlstur(sl->metineCevir() + sr->metineCevir());
        } else if (sl->tur == DorukDeger::Tur::TAM_SAYI && sr->tur == DorukDeger::Tur::TAM_SAYI) {
            sonDeger_ = DorukDeger::tamSayiOlstur(sl->tamSayi + sr->tamSayi);
        } else {
            double a = sl->tur == DorukDeger::Tur::TAM_SAYI ? (double)sl->tamSayi : sl->ondalik;
            double b = sr->tur == DorukDeger::Tur::TAM_SAYI ? (double)sr->tamSayi : sr->ondalik;
            sonDeger_ = DorukDeger::ondalikOlstur(a + b);
        }
        break;
    }
    case TokenTuru::EKSI: {
        if (sl->tur == DorukDeger::Tur::TAM_SAYI && sr->tur == DorukDeger::Tur::TAM_SAYI)
            sonDeger_ = DorukDeger::tamSayiOlstur(sl->tamSayi - sr->tamSayi);
        else {
            double a = sl->tur == DorukDeger::Tur::TAM_SAYI ? (double)sl->tamSayi : sl->ondalik;
            double b = sr->tur == DorukDeger::Tur::TAM_SAYI ? (double)sr->tamSayi : sr->ondalik;
            sonDeger_ = DorukDeger::ondalikOlstur(a - b);
        }
        break;
    }
    case TokenTuru::CARPIM: {
        if (sl->tur == DorukDeger::Tur::TAM_SAYI && sr->tur == DorukDeger::Tur::TAM_SAYI)
            sonDeger_ = DorukDeger::tamSayiOlstur(sl->tamSayi * sr->tamSayi);
        else {
            double a = sl->tur == DorukDeger::Tur::TAM_SAYI ? (double)sl->tamSayi : sl->ondalik;
            double b = sr->tur == DorukDeger::Tur::TAM_SAYI ? (double)sr->tamSayi : sr->ondalik;
            sonDeger_ = DorukDeger::ondalikOlstur(a * b);
        }
        break;
    }
    case TokenTuru::BOLUM: {
        // Her iki taraf da TAM_SAYI ise → tam sayi bolmesi (5/2=2)
        // En az biri ONDALIK ise → kesirli bolme (5.0/2=2.5)
        if (sl->tur == DorukDeger::Tur::TAM_SAYI && sr->tur == DorukDeger::Tur::TAM_SAYI) {
            if (sr->tamSayi == 0) hataFirlat("Sifira bolme.", d.op.satir, d.op.sutun);
            sonDeger_ = DorukDeger::tamSayiOlstur(sl->tamSayi / sr->tamSayi);
        } else {
            double b = sr->tur == DorukDeger::Tur::TAM_SAYI ? (double)sr->tamSayi : sr->ondalik;
            if (b == 0.0) hataFirlat("Sifira bolme.", d.op.satir, d.op.sutun);
            double a = sl->tur == DorukDeger::Tur::TAM_SAYI ? (double)sl->tamSayi : sl->ondalik;
            sonDeger_ = DorukDeger::ondalikOlstur(a / b);
        }
        break;
    }
    case TokenTuru::MOD: {
        // ONDALIK olsa bile kesirli kismi yoksa (15.0, 2.0) tam sayi kabul et
        auto tamMi = [](const DorukDeger* d) -> bool {
            if (d->tur == DorukDeger::Tur::TAM_SAYI) return true;
            if (d->tur == DorukDeger::Tur::ONDALIK)
                return d->ondalik == std::floor(d->ondalik);
            return false;
        };
        auto tamDeger = [](const DorukDeger* d) -> long long {
            return d->tur == DorukDeger::Tur::TAM_SAYI
                ? d->tamSayi
                : static_cast<long long>(d->ondalik);
        };
        if (!tamMi(sl) || !tamMi(sr))
            hataFirlat("'%' sadece tam sayilar icin gecerli.", d.op.satir, d.op.sutun);
        long long modSol = tamDeger(sl);
        long long modSag = tamDeger(sr);
        if (modSag == 0) hataFirlat("Sifira bolme (mod).", d.op.satir, d.op.sutun);
        sonDeger_ = DorukDeger::tamSayiOlstur(modSol % modSag);
        break;
    }
    // Karşılaştırma
    case TokenTuru::ESIT_ESIT:
        sonDeger_ = DorukDeger::mantiksalOlstur(sl->esittir(*sr));
        break;
    case TokenTuru::ESIT_DEGIL:
        sonDeger_ = DorukDeger::mantiksalOlstur(!sl->esittir(*sr));
        break;
    case TokenTuru::KUCUK: {
        if (sl->tur == DorukDeger::Tur::TAM_SAYI && sr->tur == DorukDeger::Tur::TAM_SAYI)
            sonDeger_ = DorukDeger::mantiksalOlstur(sl->tamSayi < sr->tamSayi);
        else if (sl->tur == DorukDeger::Tur::METIN && sr->tur == DorukDeger::Tur::METIN)
            sonDeger_ = DorukDeger::mantiksalOlstur(sl->metin < sr->metin);
        else {
            double a = sl->tur == DorukDeger::Tur::TAM_SAYI ? (double)sl->tamSayi : sl->ondalik;
            double b = sr->tur == DorukDeger::Tur::TAM_SAYI ? (double)sr->tamSayi : sr->ondalik;
            sonDeger_ = DorukDeger::mantiksalOlstur(a < b);
        }
        break;
    }
    case TokenTuru::KUCUK_ESIT: {
        if (sl->tur == DorukDeger::Tur::TAM_SAYI && sr->tur == DorukDeger::Tur::TAM_SAYI)
            sonDeger_ = DorukDeger::mantiksalOlstur(sl->tamSayi <= sr->tamSayi);
        else {
            double a = sl->tur == DorukDeger::Tur::TAM_SAYI ? (double)sl->tamSayi : sl->ondalik;
            double b = sr->tur == DorukDeger::Tur::TAM_SAYI ? (double)sr->tamSayi : sr->ondalik;
            sonDeger_ = DorukDeger::mantiksalOlstur(a <= b);
        }
        break;
    }
    case TokenTuru::BUYUK: {
        if (sl->tur == DorukDeger::Tur::TAM_SAYI && sr->tur == DorukDeger::Tur::TAM_SAYI)
            sonDeger_ = DorukDeger::mantiksalOlstur(sl->tamSayi > sr->tamSayi);
        else {
            double a = sl->tur == DorukDeger::Tur::TAM_SAYI ? (double)sl->tamSayi : sl->ondalik;
            double b = sr->tur == DorukDeger::Tur::TAM_SAYI ? (double)sr->tamSayi : sr->ondalik;
            sonDeger_ = DorukDeger::mantiksalOlstur(a > b);
        }
        break;
    }
    case TokenTuru::BUYUK_ESIT: {
        if (sl->tur == DorukDeger::Tur::TAM_SAYI && sr->tur == DorukDeger::Tur::TAM_SAYI)
            sonDeger_ = DorukDeger::mantiksalOlstur(sl->tamSayi >= sr->tamSayi);
        else {
            double a = sl->tur == DorukDeger::Tur::TAM_SAYI ? (double)sl->tamSayi : sl->ondalik;
            double b = sr->tur == DorukDeger::Tur::TAM_SAYI ? (double)sr->tamSayi : sr->ondalik;
            sonDeger_ = DorukDeger::mantiksalOlstur(a >= b);
        }
        break;
    }
    default:
        hataFirlat("Bilinmeyen ikili operator: " + d.op.lexem,
                    d.op.satir, d.op.sutun);
    }
}

// ============================================================
// Atama
// ============================================================

void Interpreter::ziyaretAtama(AtamaIfade& d) {
    auto deger = degerlendir(*d.deger);
    ortam_->guncelle(d.hedef, deger, d.satir, d.sutun);
    sonDeger_ = deger;
}

// ============================================================
// Fonksiyon çağrısı
// ============================================================

void Interpreter::ziyaretCagri(CagriIfade& d) {
    auto fn = degerlendir(*d.fonksiyon);

    std::vector<DegerRef> arglar;
    arglar.reserve(d.argumanlar.size());
    for (auto& arg : d.argumanlar)
        arglar.push_back(degerlendir(*arg));

    if (fn->tur == DorukDeger::Tur::FONKSIYON && fn->fonksiyon) {
        sonDeger_ = fonksiyonCagir(*fn->fonksiyon, std::move(arglar),
                                    d.satir, d.sutun);
        return;
    }
    if (fn->tur == DorukDeger::Tur::YERLESIK && fn->yerlesik) {
        try {
            sonDeger_ = fn->yerlesik->fn(std::move(arglar));
        } catch (const CalismaHatasi& h) {
            hataFirlat(h.mesaj, h.satir ? h.satir : d.satir,
                                h.sutun ? h.sutun : d.sutun);
        }
        return;
    }
    if (fn->tur == DorukDeger::Tur::SINIF && fn->sinif) {
        // Sınıfı nesne olarak örnekle
        auto nesne = std::make_shared<DorukNesne>();
        nesne->sinif = fn->sinif;

        // Ebeveyn alanlarını miras al
        auto s = fn->sinif;
        while (s->ebeveynSinif) {
            s = s->ebeveynSinif;
        }

        auto ref = DorukDeger::nesneOlstur(nesne);

        // baslangic metodu varsa çağır
        auto it = fn->sinif->metodlar.find("baslangic");
        if (it != fn->sinif->metodlar.end()) {
            auto& metod = it->second;
            auto kapsamOrtam = std::make_shared<Environment>(metod.kapatma);
            kapsamOrtam->tanimla("bu", ref);
            for (size_t i = 0; i < metod.parametreler.size() && i < arglar.size(); ++i)
                kapsamOrtam->tanimla(metod.parametreler[i], arglar[i]);
            try {
                blokCalistir(*metod.govde, kapsamOrtam);
            } catch (DondurSinyali&) {}
        }

        sonDeger_ = ref;
        return;
    }

    hataFirlat("Cagrilabilir degil: " + fn->turAdi(), d.satir, d.sutun);
}

// ============================================================
// İndis erişimi
// ============================================================

void Interpreter::ziyaretIndis(IndisIfade& d) {
    auto nesne = degerlendir(*d.nesne);
    auto indis = degerlendir(*d.indis);

    if (nesne->tur == DorukDeger::Tur::LISTE && nesne->liste) {
        if (indis->tur != DorukDeger::Tur::TAM_SAYI)
            hataFirlat("Liste indisi tam_sayi olmali.", d.satir, d.sutun);
        int64_t idx = indis->tamSayi;
        int64_t sz  = static_cast<int64_t>(nesne->liste->elemanlar.size());
        if (idx < 0) idx += sz;
        if (idx < 0 || idx >= sz) hataFirlat("Liste indisi sinir disi.", d.satir, d.sutun);
        sonDeger_ = nesne->liste->elemanlar[static_cast<size_t>(idx)];
        return;
    }
    if (nesne->tur == DorukDeger::Tur::SOZLUK && nesne->sozluk) {
        if (indis->tur != DorukDeger::Tur::METIN)
            hataFirlat("Sozluk anahtari metin olmali.", d.satir, d.sutun);
        auto it = nesne->sozluk->cifter.find(indis->metin);
        sonDeger_ = (it != nesne->sozluk->cifter.end()) ? it->second : BOSDeger();
        return;
    }
    if (nesne->tur == DorukDeger::Tur::METIN) {
        if (indis->tur != DorukDeger::Tur::TAM_SAYI)
            hataFirlat("Metin indisi tam_sayi olmali.", d.satir, d.sutun);
        int64_t idx = indis->tamSayi;
        int64_t sz  = static_cast<int64_t>(nesne->metin.size());
        if (idx < 0) idx += sz;
        if (idx < 0 || idx >= sz) hataFirlat("Metin indisi sinir disi.", d.satir, d.sutun);
        sonDeger_ = DorukDeger::metinOlstur(
            std::string(1, nesne->metin[static_cast<size_t>(idx)]));
        return;
    }

    hataFirlat("Indisleme desteklenmiyor: " + nesne->turAdi(), d.satir, d.sutun);
}

// ============================================================
// Üye erişimi
// ============================================================

void Interpreter::ziyaretUye(UyeIfade& d) {
    auto nesne = degerlendir(*d.nesne);

    if (nesne->tur == DorukDeger::Tur::NESNE && nesne->nesne) {
        auto it = nesne->nesne->alanlar.find(d.alan);
        if (it != nesne->nesne->alanlar.end()) {
            sonDeger_ = it->second;
            return;
        }
        // Metot ara
        auto s = nesne->nesne->sinif;
        while (s) {
            auto mit = s->metodlar.find(d.alan);
            if (mit != s->metodlar.end()) {
                // Bağlı metot: closure'a 'bu'yu ekle
                DorukFonksiyon bagliMetot = mit->second;
                auto kapatma = std::make_shared<Environment>(bagliMetot.kapatma);
                kapatma->tanimla("bu", nesne);
                bagliMetot.kapatma = kapatma;
                sonDeger_ = DorukDeger::fonksiyonOlstur(std::move(bagliMetot));
                return;
            }
            s = s->ebeveynSinif;
        }
        sonDeger_ = BOSDeger();
        return;
    }

    // Liste/Sözlük için yerleşik özellikler
    if (nesne->tur == DorukDeger::Tur::LISTE && nesne->liste) {
        if (d.alan == "uzunluk") {
            sonDeger_ = DorukDeger::tamSayiOlstur(
                static_cast<int64_t>(nesne->liste->elemanlar.size()));
            return;
        }
    }
    if (nesne->tur == DorukDeger::Tur::METIN) {
        if (d.alan == "uzunluk") {
            sonDeger_ = DorukDeger::tamSayiOlstur(
                static_cast<int64_t>(nesne->metin.size()));
            return;
        }
    }

    hataFirlat("'" + nesne->turAdi() + "' uzerinde '" + d.alan + "' alani yok.",
                d.satir, d.sutun);
}

// ============================================================
// Liste literali
// ============================================================

void Interpreter::ziyaretListe(ListeIfade& d) {
    std::vector<DegerRef> elemanlar;
    elemanlar.reserve(d.elemanlar.size());
    for (auto& e : d.elemanlar)
        elemanlar.push_back(degerlendir(*e));
    sonDeger_ = DorukDeger::listeOlstur(std::move(elemanlar));
}

// ============================================================
// Sözlük literali
// ============================================================

void Interpreter::ziyaretSozluk(SozlukIfade& d) {
    auto soz = DorukDeger::sozlukOlstur();
    for (auto& cift : d.cifter) {
        auto anahtar = degerlendir(*cift.first);
        if (anahtar->tur != DorukDeger::Tur::METIN)
            hataFirlat("Sozluk anahtari metin olmali.", d.satir, d.sutun);
        auto deger = degerlendir(*cift.second);
        const auto& k = anahtar->metin;
        if (!soz->sozluk->cifter.count(k))
            soz->sozluk->anahtarSirasi.push_back(k);
        soz->sozluk->cifter[k] = deger;
    }
    sonDeger_ = soz;
}

// ============================================================
// Yeni (sınıf örnekleme — CagriIfade'de de ele alındı)
// ============================================================

void Interpreter::ziyaretYeni(YeniIfade& d) {
    auto sinifDeger = ortam_->oku(d.sinifIsmi, d.satir, d.sutun);
    if (sinifDeger->tur != DorukDeger::Tur::SINIF || !sinifDeger->sinif)
        hataFirlat("'" + d.sinifIsmi + "' bir sinif degil.", d.satir, d.sutun);

    std::vector<DegerRef> arglar;
    for (auto& arg : d.argumanlar) arglar.push_back(degerlendir(*arg));

    auto nesne = std::make_shared<DorukNesne>();
    nesne->sinif = sinifDeger->sinif;
    auto ref = DorukDeger::nesneOlstur(nesne);

    auto it = sinifDeger->sinif->metodlar.find("baslangic");
    if (it != sinifDeger->sinif->metodlar.end()) {
        auto& metod = it->second;
        auto kapsamOrtam = std::make_shared<Environment>(metod.kapatma);
        kapsamOrtam->tanimla("bu", ref);
        for (size_t i = 0; i < metod.parametreler.size() && i < arglar.size(); ++i)
            kapsamOrtam->tanimla(metod.parametreler[i], arglar[i]);
        try { blokCalistir(*metod.govde, kapsamOrtam); }
        catch (DondurSinyali&) {}
    }

    sonDeger_ = ref;
}

// ============================================================
// Blok deyim
// ============================================================

void Interpreter::ziyaretBlok(BlokDeyim& d) {
    blokCalistir(d, ortam_->cocukOlustur());
}

// ============================================================
// İfade deyimi
// ============================================================

void Interpreter::ziyaretIfade(IfadeDeyim& d) {
    sonDeger_ = degerlendir(*d.ifade);
}

// ============================================================
// Değişken tanımı
// ============================================================

void Interpreter::ziyaretDegiskenTanim(DegiskenTanim& d) {
    DegerRef baslangic = d.baslangicDegeri
        ? degerlendir(*d.baslangicDegeri)
        : BOSDeger();
    ortam_->tanimla(d.isim, baslangic);
    sonDeger_ = baslangic;
}

// ============================================================
// Fonksiyon tanımı
// ============================================================

void Interpreter::ziyaretFonksiyonTanim(FonksiyonTanim& d) {
    DorukFonksiyon fn;
    fn.isim    = d.isim;
    fn.govde   = d.govde.get();
    fn.kapatma = ortam_; // closure

    for (auto& p : d.parametreler)
        fn.parametreler.push_back(p.isim);

    auto ref = DorukDeger::fonksiyonOlstur(std::move(fn));
    ortam_->tanimla(d.isim, ref);
    sonDeger_ = ref;
}

// ============================================================
// Sınıf tanımı
// ============================================================

void Interpreter::ziyaretSinifTanim(SinifTanim& d) {
    auto sinif = std::make_shared<DorukSinif>();
    sinif->isim    = d.isim;
    sinif->ebeveyn = d.ebeveyn;

    // Ebeveyn sınıfı bul
    if (!d.ebeveyn.empty()) {
        try {
            auto ebeveynRef = ortam_->oku(d.ebeveyn, d.satir, d.sutun);
            if (ebeveynRef->tur == DorukDeger::Tur::SINIF && ebeveynRef->sinif) {
                sinif->ebeveynSinif = ebeveynRef->sinif;
                // Ebeveyn metodlarını miras al
                for (auto& [isim, metod] : ebeveynRef->sinif->metodlar)
                    sinif->metodlar[isim] = metod;
            }
        } catch (...) {
            hataFirlat("Bilinmeyen ebeveyn sinif: '" + d.ebeveyn + "'",
                        d.satir, d.sutun);
        }
    }

    // Metodları kaydet
    for (auto& metodAst : d.metodlar) {
        DorukFonksiyon fn;
        fn.isim    = metodAst->isim;
        fn.govde   = metodAst->govde.get();
        fn.kapatma = ortam_;
        for (auto& p : metodAst->parametreler)
            fn.parametreler.push_back(p.isim);
        sinif->metodlar[fn.isim] = std::move(fn);
    }

    auto ref = DorukDeger::sinifOlstur(sinif);
    ortam_->tanimla(d.isim, ref);
    sonDeger_ = ref;
}

// ============================================================
// Eğer deyimi
// ============================================================

void Interpreter::ziyaretEger(EgerDeyim& d) {
    auto kosul = degerlendir(*d.kosul);
    if (kosul->gercegiDeger()) {
        if (d.dogruKol) d.dogruKol->kabul(*this);
    } else if (d.yanlisKol) {
        d.yanlisKol->kabul(*this);
    }
}

// ============================================================
// İçin döngüsü
// ============================================================

void Interpreter::ziyaretIcin(IcinDeyim& d) {
    auto donguOrtam = ortam_->cocukOlustur();
    auto oncekiOrtam = ortam_;
    ortam_ = donguOrtam;

    try {
        if (d.baslangic) calistirDeyim(*d.baslangic);

        while (true) {
            if (d.kosul) {
                auto kosul = degerlendir(*d.kosul);
                if (!kosul->gercegiDeger()) break;
            }

            if (d.govde) {
                try {
                    blokCalistir(*d.govde, ortam_->cocukOlustur());
                } catch (KirSinyali&) {
                    break;
                } catch (DevamSinyali&) {
                    // devam
                }
            }

            if (d.arttir) degerlendir(*d.arttir);
        }
    } catch (...) {
        ortam_ = oncekiOrtam;
        throw;
    }

    ortam_ = oncekiOrtam;
}

// ============================================================
// Döngü (while)
// ============================================================

void Interpreter::ziyaretDongu(DonguDeyim& d) {
    while (true) {
        auto kosul = degerlendir(*d.kosul);
        if (!kosul->gercegiDeger()) break;

        if (d.govde) {
            try {
                blokCalistir(*d.govde, ortam_->cocukOlustur());
            } catch (KirSinyali&) {
                break;
            } catch (DevamSinyali&) {
                continue;
            }
        }
    }
}

// ============================================================
// Döndür
// ============================================================

void Interpreter::ziyaretDondur(DondurDeyim& d) {
    DegerRef deger = d.deger ? degerlendir(*d.deger) : BOSDeger();
    throw DondurSinyali{ deger };
}

// ============================================================
// Kır / Devam
// ============================================================

void Interpreter::ziyaretKir(KirDeyim&) {
    throw KirSinyali{};
}

void Interpreter::ziyaretDevam(DevamDeyim&) {
    throw DevamSinyali{};
}

// ============================================================
// Yazdır
// ============================================================

void Interpreter::ziyaretYazdir(YazdirDeyim& d) {
    for (size_t i = 0; i < d.argumanlar.size(); ++i) {
        if (i) ciktiYaz(" ");
        auto val = degerlendir(*d.argumanlar[i]);
        ciktiYaz(val->metineCevir());
    }
    ciktiYaz("\n");
    sonDeger_ = BOSDeger();
}

} // namespace Doruk


