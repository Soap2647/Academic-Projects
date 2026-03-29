// ============================================================
// Parser.cpp — DORUK Dili Recursive Descent Çözümleyici
//
// DORUK dilinin tam sözdizimi grameri (BNF benzeri gösterim):
//
//   program        → deyim* EOF
//   deyim          → degiskenTanim | fonksiyonTanim | sinifTanim
//                  | egerDeyim | icinDeyim | donguDeyim
//                  | dondurDeyim | kirDeyim | devamDeyim
//                  | yazdirDeyim | blok | ifadeDeyim
//   blok           → "{" deyim* "}"
//   degiskenTanim  → "değişken" KIMLIK ( "=" ifade )?
//   fonksiyonTanim → "fonksiyon" KIMLIK "(" params ")" blok
//   sinifTanim     → "sınıf" KIMLIK ( "<" KIMLIK )? "{" sinifGovde "}"
//   egerDeyim      → "eğer" "(" ifade ")" blok ( "değilse" ( blok | egerDeyim ) )?
//   icinDeyim      → "için" "(" ( degiskenTanim | ifadeDeyim ) ifade? ";" ifade? ")" blok
//   donguDeyim     → "döngü" "(" ifade ")" blok
//   dondurDeyim    → "döndür" ifade?
//   kirDeyim       → "kır"
//   devamDeyim     → "devam"
//   yazdirDeyim    → "yazdır" "(" argumanlar ")"
//   ifadeDeyim     → ifade
//
//   ifade          → atama
//   atama          → KIMLIK ( "=" | "+=" | "-=" | "*=" | "/=" ) atama
//                  | mantıksalVeya
//   mantıksalVeya  → mantıksalVe  ( ( "||" | "veya" ) mantıksalVe  )*
//   mantıksalVe    → esitlik      ( ( "&&" | "ve"   ) esitlik      )*
//   esitlik        → karsilastirma ( ( "==" | "!="  ) karsilastirma )*
//   karsilastirma  → toplama       ( ( "<"  | "<="  | ">" | ">=" ) toplama )*
//   toplama        → carpma        ( ( "+"  | "-"   ) carpma       )*
//   carpma         → tekli         ( ( "*"  | "/"   | "%"  ) tekli )*
//   tekli          → ( "!" | "-" | "değil" ) tekli | sonEk
//   sonEk          → birincil ( "(" argümanlar ")" | "[" ifade "]" | "." KIMLIK )*
//   birincil       → TAM_SAYI | ONDALIK | METIN | "doğru" | "yanlış" | "boş"
//                  | KIMLIK | "bu" | "(" ifade ")" | "[" listeLit "]"
//                  | "{" sozlukLit "}" | "yeni" KIMLIK "(" argümanlar ")"
//                  | "oku" "(" ")"
// ============================================================
#include "Parser.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace Doruk {

// ============================================================
// Yapıcı
// ============================================================
Parser::Parser(std::vector<Token> tokenler, std::string dosya)
    : tokenler_(std::move(tokenler))
    , konum_(0)
    , dosya_(std::move(dosya))
    , panikMod_(false)
{}

// ============================================================
// Ana ayrıştırma
// ============================================================
ParserSonucu Parser::ayristir() {
    konum_    = 0;
    panikMod_ = false;
    tanilar_  = TaniListesi{};

    auto program = std::make_unique<Program>(dosya_);

    while (!bitti()) {
        // Panik modundan çıkma: kapatıcı token görülünce
        if (panikMod_) {
            senkronize();
            panikMod_ = false;
            if (bitti()) break;
        }
        try {
            auto d = deyim();
            if (d) program->deyimler.push_back(std::move(d));
        } catch (...) {
            // Asla buraya gelmemeli — hatalar tanı listesine eklenir
            panikMod_ = true;
        }
    }

    return ParserSonucu{std::move(program), std::move(tanilar_)};
}

// ============================================================
// Token Navigasyonu
// ============================================================
const Token& Parser::mevcut() const {
    return tokenler_[konum_];
}

const Token& Parser::onceki() const {
    assert(konum_ > 0);
    return tokenler_[konum_ - 1];
}

const Token& Parser::sonraki() const {
    if (konum_ + 1 < tokenler_.size())
        return tokenler_[konum_ + 1];
    return tokenler_.back();  // DOSYA_SONU
}

bool Parser::bitti() const {
    return mevcut().tur == TokenTuru::DOSYA_SONU;
}

Token Parser::ilerle() {
    Token t = tokenler_[konum_];
    if (!bitti()) ++konum_;
    return t;
}

bool Parser::kontrol(TokenTuru tur) const {
    return mevcut().tur == tur;
}

bool Parser::eslestir(TokenTuru tur) {
    if (kontrol(tur)) {
        ilerle();
        return true;
    }
    return false;
}

const Token& Parser::tuket(TokenTuru beklenen, const std::string& hataMsg) {
    if (kontrol(beklenen)) {
        ilerle();
        return onceki();
    }
    hataEkle(mevcut(), hataMsg);
    panikMod_ = true;
    return mevcut();  // hata kurtarma: sahte dönüş
}

// ============================================================
// Hata Yönetimi
// ============================================================
void Parser::hataEkle(const Token& konum,
                      std::string  mesaj,
                      std::string  ipucu) {
    tanilar_.ekle(parserHatasi(std::move(mesaj),
                               dosya_,
                               konum.satir,
                               konum.sutun,
                               static_cast<uint32_t>(konum.lexem.size()),
                               std::move(ipucu)));
}

// senkronize — panik modundan çıkış için kapatıcı token ara
void Parser::senkronize() {
    while (!bitti()) {
        // Önceki token noktalı virgülse — yeni bir deyim başlıyor
        if (onceki().tur == TokenTuru::NOKTALI_VIRGUL) return;

        // Mevcut token bir deyim başlangıcıysa — duraksama noktası
        switch (mevcut().tur) {
            case TokenTuru::FONKSIYON:
            case TokenTuru::DEGISKEN:
            case TokenTuru::SINIF:
            case TokenTuru::EGER:
            case TokenTuru::ICIN:
            case TokenTuru::DONGU:
            case TokenTuru::DONDUR:
            case TokenTuru::YAZDIR:
            case TokenTuru::SAG_SUSE:
                return;
            default:
                break;
        }
        ilerle();
    }
}

// ============================================================
// Yardımcı: sayı dönüşümleri
// ============================================================
int64_t Parser::metniTamSayiyaCevir(const std::string& s) {
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        // Hex
        return static_cast<int64_t>(std::stoll(s, nullptr, 16));
    }
    return static_cast<int64_t>(std::stoll(s, nullptr, 10));
}

double Parser::metniOndaligaCevir(const std::string& s) {
    return std::stod(s);
}

// ============================================================
// DEYİM AYRIŞTIRICILAR
// ============================================================
DeyimPtr Parser::deyim() {
    const Token& tok = mevcut();

    switch (tok.tur) {
        case TokenTuru::DEGISKEN:   return degiskenTanim();
        case TokenTuru::FONKSIYON:  return fonksiyonTanim();
        case TokenTuru::SINIF:      return sinifTanim();
        case TokenTuru::EGER:       return egerDeyim();
        case TokenTuru::ICIN:       return icinDeyim();
        case TokenTuru::DONGU:      return donguDeyim();
        case TokenTuru::DONDUR:     return dondurDeyim();
        case TokenTuru::KIR:        return kirDeyim();
        case TokenTuru::DEVAM:      return devamDeyim();
        case TokenTuru::YAZDIR:     return yazdirDeyim();
        case TokenTuru::SOL_SUSE:   return blok();
        default:                    return ifadeDeyim();
    }
}

// ── Blok ──────────────────────────────────────────────────
std::unique_ptr<BlokDeyim> Parser::blok() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    tuket(TokenTuru::SOL_SUSE,
          "Blok '{' ile başlamalı.");  // Blok '{' ile başlamalı.

    std::vector<DeyimPtr> deyimler;

    while (!bitti() && !kontrol(TokenTuru::SAG_SUSE)) {
        if (panikMod_) {
            senkronize();
            panikMod_ = false;
            if (bitti() || kontrol(TokenTuru::SAG_SUSE)) break;
        }
        auto d = deyim();
        if (d) deyimler.push_back(std::move(d));
    }

    tuket(TokenTuru::SAG_SUSE,
          "Blok '}' ile kapatılmalı.");  // Blok '}' ile kapatılmalı.

    return std::make_unique<BlokDeyim>(std::move(deyimler), sat, sut);
}

// ── Değişken Tanımı ────────────────────────────────────────
DeyimPtr Parser::degiskenTanim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    ilerle();  // 'değişken' tüket

    if (!kontrol(TokenTuru::TANIMLAYICI)) {
        hataEkle(mevcut(),
                 "'değişken' sonrasında değişken ismi bekleniyor.",
                 "Örnek: değişken x = 5");
        panikMod_ = true;
        return nullptr;
    }

    std::string isim = ilerle().lexem;  // tanımlayıcı tüket

    IfadePtr baslangic;
    if (eslestir(TokenTuru::ESIT)) {
        baslangic = ifade();
    }

    // Noktalı virgül isteğe bağlı
    eslestir(TokenTuru::NOKTALI_VIRGUL);

    return std::make_unique<DegiskenTanim>(
        std::move(isim), std::move(baslangic), sat, sut);
}

// ── Fonksiyon Tanımı ──────────────────────────────────────
DeyimPtr Parser::fonksiyonTanim(bool /*sinifMetodu*/) {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    ilerle();  // 'fonksiyon' tüket

    if (!kontrol(TokenTuru::TANIMLAYICI)) {
        hataEkle(mevcut(),
                 "'fonksiyon' sonrasında fonksiyon ismi bekleniyor.",
                 "Örnek: fonksiyon topla(a, b) { ... }");
        panikMod_ = true;
        return nullptr;
    }

    std::string isim = ilerle().lexem;  // fonksiyon ismi

    tuket(TokenTuru::SOL_PAREN,
          "Fonksiyon parametre listesi '(' ile başlamalı.");

    // Parametre listesi
    std::vector<Parametre> params;
    if (!kontrol(TokenTuru::SAG_PAREN)) {
        do {
            if (!kontrol(TokenTuru::TANIMLAYICI)) {
                hataEkle(mevcut(), "Parametre ismi bekleniyor.");
                panikMod_ = true;
                break;
            }
            Parametre p;
            p.satir = mevcut().satir;
            p.sutun = mevcut().sutun;
            p.isim  = ilerle().lexem;
            params.push_back(std::move(p));
        } while (eslestir(TokenTuru::VIRGUL));
    }

    tuket(TokenTuru::SAG_PAREN,
          "Parametre listesi ')' ile kapatılmalı.");

    if (!kontrol(TokenTuru::SOL_SUSE)) {
        hataEkle(mevcut(), "Fonksiyon gövdesi '{' ile başlamalı.");
        panikMod_ = true;
        return nullptr;
    }

    auto govde = blok();
    if (!govde) return nullptr;

    return std::make_unique<FonksiyonTanim>(
        std::move(isim), std::move(params), std::move(govde), sat, sut);
}

// ── Sınıf Tanımı ──────────────────────────────────────────
DeyimPtr Parser::sinifTanim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    ilerle();  // 'sınıf' tüket

    if (!kontrol(TokenTuru::TANIMLAYICI)) {
        hataEkle(mevcut(), "Sınıf ismi bekleniyor.");
        panikMod_ = true;
        return nullptr;
    }

    std::string isim    = ilerle().lexem;
    std::string ebeveyn = "";

    // Kalıtım: sınıf Alt < Üst
    if (eslestir(TokenTuru::KUCUK)) {
        if (!kontrol(TokenTuru::TANIMLAYICI)) {
            hataEkle(mevcut(), "Ebeveyn sınıf ismi bekleniyor.");
            panikMod_ = true;
            return nullptr;
        }
        ebeveyn = ilerle().lexem;
    }

    tuket(TokenTuru::SOL_SUSE, "Sınıf gövdesi '{' ile başlamalı.");

    auto sinif = std::make_unique<SinifTanim>(
        std::move(isim), std::move(ebeveyn), sat, sut);

    // Sınıf gövdesi: alan ve metot tanımları
    while (!bitti() && !kontrol(TokenTuru::SAG_SUSE)) {
        if (panikMod_) {
            senkronize();
            panikMod_ = false;
            if (bitti() || kontrol(TokenTuru::SAG_SUSE)) break;
        }
        if (kontrol(TokenTuru::FONKSIYON)) {
            auto metodD = fonksiyonTanim(true);
            if (metodD) {
                // Statik downcast — fonksiyonTanim her zaman FonksiyonTanim üretir
                auto* f = dynamic_cast<FonksiyonTanim*>(metodD.get());
                if (f) {
                    metodD.release();
                    sinif->metodlar.push_back(
                        std::unique_ptr<FonksiyonTanim>(f));
                }
            }
        } else if (kontrol(TokenTuru::DEGISKEN)) {
            auto alanD = degiskenTanim();
            if (alanD) {
                auto* v = dynamic_cast<DegiskenTanim*>(alanD.get());
                if (v) {
                    alanD.release();
                    sinif->alanlar.push_back(
                        std::unique_ptr<DegiskenTanim>(v));
                }
            }
        } else {
            hataEkle(mevcut(),
                     "Sınıf gövdesinde yalnızca alan ve metot tanımları olabilir.");
            panikMod_ = true;
        }
    }

    tuket(TokenTuru::SAG_SUSE, "Sınıf gövdesi '}' ile kapatılmalı.");
    return sinif;
}

// ── Eğer Deyimi ────────────────────────────────────────────
DeyimPtr Parser::egerDeyim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    ilerle();  // 'eğer' tüket

    tuket(TokenTuru::SOL_PAREN,
          "'eğer' sonrasında '(' bekleniyor.");

    auto kosul = ifade();

    tuket(TokenTuru::SAG_PAREN,
          "Koşul ifadesi ')' ile kapatılmalı.");

    if (!kontrol(TokenTuru::SOL_SUSE)) {
        hataEkle(mevcut(),
                 "'eğer' gövdesi '{' ile başlamalı.");
        panikMod_ = true;
        return nullptr;
    }

    auto dogruKol = blok();

    DeyimPtr yanlisKol;
    if (eslestir(TokenTuru::DEGILSE)) {
        // değilse eğer → zincirleme
        if (kontrol(TokenTuru::EGER)) {
            yanlisKol = egerDeyim();
        } else {
            yanlisKol = blok();
        }
    }

    return std::make_unique<EgerDeyim>(
        std::move(kosul), std::move(dogruKol),
        std::move(yanlisKol), sat, sut);
}

// ── İçin Döngüsü ──────────────────────────────────────────
DeyimPtr Parser::icinDeyim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    ilerle();  // 'için' tüket

    tuket(TokenTuru::SOL_PAREN,
          "'için' sonrasında '(' bekleniyor.");

    // Başlangıç: değişken tanımı veya ifade
    DeyimPtr baslangic;
    if (kontrol(TokenTuru::DEGISKEN)) {
        baslangic = degiskenTanim();
    } else if (!kontrol(TokenTuru::NOKTALI_VIRGUL)) {
        baslangic = ifadeDeyim();
    } else {
        // Boş başlangıç
        ilerle();  // ';' tüket
    }

    // Koşul
    IfadePtr kosul;
    if (!kontrol(TokenTuru::NOKTALI_VIRGUL)) {
        kosul = ifade();
    }
    tuket(TokenTuru::NOKTALI_VIRGUL,
          "Döngü koşulu sonrasında ';' bekleniyor.");

    // Arttırma
    IfadePtr arttir;
    if (!kontrol(TokenTuru::SAG_PAREN)) {
        arttir = ifade();
    }

    tuket(TokenTuru::SAG_PAREN,
          "Döngü başlığı ')' ile kapatılmalı.");

    if (!kontrol(TokenTuru::SOL_SUSE)) {
        hataEkle(mevcut(),
                 "Döngü gövdesi '{' ile başlamalı.");
        panikMod_ = true;
        return nullptr;
    }

    auto govde = blok();

    return std::make_unique<IcinDeyim>(
        std::move(baslangic), std::move(kosul),
        std::move(arttir), std::move(govde), sat, sut);
}

// ── Döngü (while) ─────────────────────────────────────────
DeyimPtr Parser::donguDeyim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    ilerle();  // 'döngü' tüket

    tuket(TokenTuru::SOL_PAREN,
          "'döngü' sonrasında '(' bekleniyor.");

    auto kosul = ifade();

    tuket(TokenTuru::SAG_PAREN,
          "Döngü koşulu ')' ile kapatılmalı.");

    if (!kontrol(TokenTuru::SOL_SUSE)) {
        hataEkle(mevcut(),
                 "Döngü gövdesi '{' ile başlamalı.");
        panikMod_ = true;
        return nullptr;
    }

    auto govde = blok();

    return std::make_unique<DonguDeyim>(
        std::move(kosul), std::move(govde), sat, sut);
}

// ── Döndür ────────────────────────────────────────────────
DeyimPtr Parser::dondurDeyim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    ilerle();  // 'döndür' tüket

    // İfade yoksa (satır sonu veya '}') → döndür boş
    IfadePtr deger;
    if (!bitti() &&
        !kontrol(TokenTuru::NOKTALI_VIRGUL) &&
        !kontrol(TokenTuru::SAG_SUSE)) {
        deger = ifade();
    }

    eslestir(TokenTuru::NOKTALI_VIRGUL);

    return std::make_unique<DondurDeyim>(std::move(deger), sat, sut);
}

// ── Kır ───────────────────────────────────────────────────
DeyimPtr Parser::kirDeyim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;
    ilerle();  // 'kır' tüket
    eslestir(TokenTuru::NOKTALI_VIRGUL);
    return std::make_unique<KirDeyim>(sat, sut);
}

// ── Devam ─────────────────────────────────────────────────
DeyimPtr Parser::devamDeyim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;
    ilerle();  // 'devam' tüket
    eslestir(TokenTuru::NOKTALI_VIRGUL);
    return std::make_unique<DevamDeyim>(sat, sut);
}

// ── Yazdır ────────────────────────────────────────────────
DeyimPtr Parser::yazdirDeyim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    ilerle();  // 'yazdır' tüket

    tuket(TokenTuru::SOL_PAREN,
          "'yazdır' sonrasında '(' bekleniyor.");

    auto args = argumanListesi();

    tuket(TokenTuru::SAG_PAREN,
          "'yazdır' argüman listesi ')' ile kapatılmalı.");

    eslestir(TokenTuru::NOKTALI_VIRGUL);

    return std::make_unique<YazdirDeyim>(std::move(args), sat, sut);
}

// ── İfade Deyimi ──────────────────────────────────────────
DeyimPtr Parser::ifadeDeyim() {
    uint32_t sat = mevcut().satir;
    uint32_t sut = mevcut().sutun;

    auto expr = ifade();
    eslestir(TokenTuru::NOKTALI_VIRGUL);  // isteğe bağlı

    return std::make_unique<IfadeDeyim>(std::move(expr), sat, sut);
}

// ============================================================
// İFADE AYRIŞTIRICILAR — Öncelik sırası düşükten yükseğe
// ============================================================

IfadePtr Parser::ifade() {
    return atama();
}

// ── Atama ─────────────────────────────────────────────────
IfadePtr Parser::atama() {
    // Eğer KIMLIK = / += / -= / *= / /= ise atama
    if (kontrol(TokenTuru::TANIMLAYICI)) {
        // Bir sonraki tokena bak
        size_t kaydetKonum = konum_;
        Token  isimTok     = ilerle();

        TokenTuru atamaTuru = mevcut().tur;
        bool atamaMi = (atamaTuru == TokenTuru::ESIT        ||
                        atamaTuru == TokenTuru::ARTI_ESIT   ||
                        atamaTuru == TokenTuru::EKSI_ESIT   ||
                        atamaTuru == TokenTuru::CARPIM_ESIT ||
                        atamaTuru == TokenTuru::BOLUM_ESIT);

        if (atamaMi) {
            Token opTok = ilerle();  // atama operatörünü tüket
            auto  sag   = atama();  // sağ taraf (sağdan-sola ilişkilendirme)

            // Bileşik atamayı aç: x += y → x = x + y
            if (opTok.tur != TokenTuru::ESIT) {
                TokenTuru binOp;
                switch (opTok.tur) {
                    case TokenTuru::ARTI_ESIT:   binOp = TokenTuru::ARTI;   break;
                    case TokenTuru::EKSI_ESIT:   binOp = TokenTuru::EKSI;   break;
                    case TokenTuru::CARPIM_ESIT: binOp = TokenTuru::CARPIM; break;
                    case TokenTuru::BOLUM_ESIT:  binOp = TokenTuru::BOLUM;  break;
                    default:                     binOp = TokenTuru::ARTI;   break;
                }
                // x += y → x = x + y
                auto sol = std::make_unique<KimlikIfade>(
                    isimTok.lexem, isimTok.satir, isimTok.sutun);
                Token binTok(binOp, opTok.lexem, opTok.satir, opTok.sutun);
                auto ikili = std::make_unique<IkiliIfade>(
                    std::move(sol), binTok, std::move(sag));
                Token eqTok(TokenTuru::ESIT, "=", opTok.satir, opTok.sutun);
                return std::make_unique<AtamaIfade>(
                    isimTok.lexem, eqTok, std::move(ikili));
            }

            return std::make_unique<AtamaIfade>(
                isimTok.lexem, opTok, std::move(sag));
        }

        // Atama değil — konumu geri al ve normal ifade olarak devam et
        konum_ = kaydetKonum;
    }

    return mantiksalVeya();
}

// ── Mantıksal VEYA ────────────────────────────────────────
IfadePtr Parser::mantiksalVeya() {
    auto sol = mantiksalVe();

    while (kontrol(TokenTuru::VEYA_VEYA) || kontrol(TokenTuru::VEYA)) {
        Token op = ilerle();
        auto  sag = mantiksalVe();
        sol = std::make_unique<IkiliIfade>(
            std::move(sol), op, std::move(sag));
    }

    return sol;
}

// ── Mantıksal VE ──────────────────────────────────────────
IfadePtr Parser::mantiksalVe() {
    auto sol = esitlik();  // mantiksalVe

    while (kontrol(TokenTuru::VE_VE) || kontrol(TokenTuru::VE)) {
        Token op = ilerle();
        auto  sag = esitlik();
        sol = std::make_unique<IkiliIfade>(
            std::move(sol), op, std::move(sag));
    }

    return sol;
}

// ── Eşitlik ───────────────────────────────────────────────
IfadePtr Parser::esitlik() {
    auto sol = karsilastirma();

    while (kontrol(TokenTuru::ESIT_ESIT) || kontrol(TokenTuru::ESIT_DEGIL)) {
        Token op  = ilerle();
        auto  sag = karsilastirma();
        sol = std::make_unique<IkiliIfade>(
            std::move(sol), op, std::move(sag));
    }

    return sol;
}

// ── Karşılaştırma ─────────────────────────────────────────
IfadePtr Parser::karsilastirma() {
    auto sol = toplama();

    while (kontrol(TokenTuru::KUCUK)      ||
           kontrol(TokenTuru::KUCUK_ESIT) ||
           kontrol(TokenTuru::BUYUK)      ||
           kontrol(TokenTuru::BUYUK_ESIT)) {
        Token op  = ilerle();
        auto  sag = toplama();
        sol = std::make_unique<IkiliIfade>(
            std::move(sol), op, std::move(sag));
    }

    return sol;
}

// ── Toplama / Çıkarma ─────────────────────────────────────
IfadePtr Parser::toplama() {
    auto sol = carpma();

    while (kontrol(TokenTuru::ARTI) || kontrol(TokenTuru::EKSI)) {
        Token op  = ilerle();
        auto  sag = carpma();
        sol = std::make_unique<IkiliIfade>(
            std::move(sol), op, std::move(sag));
    }

    return sol;
}

// ── Çarpma / Bölme / Mod ──────────────────────────────────
IfadePtr Parser::carpma() {
    auto sol = tekli();

    while (kontrol(TokenTuru::CARPIM) ||
           kontrol(TokenTuru::BOLUM)  ||
           kontrol(TokenTuru::MOD)) {
        Token op  = ilerle();
        auto  sag = tekli();
        sol = std::make_unique<IkiliIfade>(
            std::move(sol), op, std::move(sag));
    }

    return sol;
}

// ── Tekli (önek) ──────────────────────────────────────────
IfadePtr Parser::tekli() {
    if (kontrol(TokenTuru::UNLEM)  ||
        kontrol(TokenTuru::EKSI)   ||
        kontrol(TokenTuru::DEGIL)) {
        Token op      = ilerle();
        auto  operand = tekli();
        return std::make_unique<TekliIfade>(op, std::move(operand));
    }

    return sonEk();
}

// ── Son-ek (postfix): çağrı, indis, üye ──────────────────
IfadePtr Parser::sonEk() {
    auto expr = birincil();

    while (true) {
        if (kontrol(TokenTuru::SOL_PAREN)) {
            // Fonksiyon çağrısı
            uint32_t sat = mevcut().satir;
            uint32_t sut = mevcut().sutun;
            ilerle();  // '(' tüket
            auto args = argumanListesi();
            tuket(TokenTuru::SAG_PAREN,
                  "Fonksiyon argüman listesi ')' ile kapatılmalı.");
            expr = std::make_unique<CagriIfade>(
                std::move(expr), std::move(args), sat, sut);

        } else if (kontrol(TokenTuru::SOL_KOSELI)) {
            // İndis erişimi
            uint32_t sat = mevcut().satir;
            uint32_t sut = mevcut().sutun;
            ilerle();  // '[' tüket
            auto idx = ifade();
            tuket(TokenTuru::SAG_KOSELI,
                  "Endeks ifadesi ']' ile kapatılmalı.");
            expr = std::make_unique<IndisIfade>(
                std::move(expr), std::move(idx), sat, sut);

        } else if (kontrol(TokenTuru::NOKTA)) {
            // Üye erişimi
            uint32_t sat = mevcut().satir;
            uint32_t sut = mevcut().sutun;
            ilerle();  // '.' tüket
            if (!kontrol(TokenTuru::TANIMLAYICI)) {
                hataEkle(mevcut(),
                         "'.' sonrasında üye ismi bekleniyor.");
                break;
            }
            std::string alan = ilerle().lexem;
            expr = std::make_unique<UyeIfade>(
                std::move(expr), std::move(alan), sat, sut);

        } else {
            break;
        }
    }

    return expr;
}

// ── Birincil ifade ────────────────────────────────────────
IfadePtr Parser::birincil() {
    const Token& tok = mevcut();

    // ── Tam sayı literali ──────────────────────────────────
    if (tok.tur == TokenTuru::TAM_SAYI_LIT) {
        ilerle();
        int64_t deger = 0;
        try { deger = metniTamSayiyaCevir(tok.lexem); }
        catch (...) {
            hataEkle(tok,
                     "Geçersiz tam sayı literali: '" + tok.lexem + "'");
        }
        return std::make_unique<SayiLiteralIfade>(deger, tok.satir, tok.sutun);
    }

    // ── Ondalık literali ───────────────────────────────────
    if (tok.tur == TokenTuru::ONDALIK_LIT) {
        ilerle();
        double deger = 0.0;
        try { deger = metniOndaligaCevir(tok.lexem); }
        catch (...) {
            hataEkle(tok,
                     "Geçersiz ondalik literali: '" + tok.lexem + "'");
        }
        return std::make_unique<OndalikLiteralIfade>(deger, tok.satir, tok.sutun);
    }

    // ── Metin literali ─────────────────────────────────────
    if (tok.tur == TokenTuru::METIN_LIT) {
        ilerle();
        return std::make_unique<MetinLiteralIfade>(
            tok.lexem, tok.satir, tok.sutun);
    }

    // ── Boolean literalleri ────────────────────────────────
    if (tok.tur == TokenTuru::DOGRU) {
        ilerle();
        return std::make_unique<BoolLiteralIfade>(true, tok.satir, tok.sutun);
    }
    if (tok.tur == TokenTuru::YANLIS) {
        ilerle();
        return std::make_unique<BoolLiteralIfade>(false, tok.satir, tok.sutun);
    }

    // ── Boş literali ───────────────────────────────────────
    if (tok.tur == TokenTuru::BOS) {
        ilerle();
        return std::make_unique<BosLiteralIfade>(tok.satir, tok.sutun);
    }

    // ── bu (this) ──────────────────────────────────────────
    if (tok.tur == TokenTuru::BU) {
        ilerle();
        return std::make_unique<KimlikIfade>("bu", tok.satir, tok.sutun);
    }

    // ── Tanımlayıcı ───────────────────────────────────────
    if (tok.tur == TokenTuru::TANIMLAYICI) {
        ilerle();
        return std::make_unique<KimlikIfade>(
            tok.lexem, tok.satir, tok.sutun);
    }

    // ── Gruplandırma: (ifade) ──────────────────────────────
    if (tok.tur == TokenTuru::SOL_PAREN) {
        ilerle();  // '(' tüket
        auto expr = ifade();
        tuket(TokenTuru::SAG_PAREN,
              "Gruplandırılmış ifade ')' ile kapatılmalı.");
        return expr;
    }

    // ── Liste literali: [1, 2, 3] ──────────────────────────
    if (tok.tur == TokenTuru::SOL_KOSELI) {
        ilerle();
        return listeLiteral(tok.satir, tok.sutun);
    }

    // ── Sözlük literali: {"a": 1} ─────────────────────────
    if (tok.tur == TokenTuru::SOL_SUSE) {
        ilerle();
        return sozlukLiteral(tok.satir, tok.sutun);
    }

    // ── yeni Sinif(args) ──────────────────────────────────
    if (tok.tur == TokenTuru::YENI) {
        ilerle();  // 'yeni' tüket
        if (!kontrol(TokenTuru::TANIMLAYICI)) {
            hataEkle(mevcut(), "'yeni' sonrasında sınıf ismi bekleniyor.");
            panikMod_ = true;
            return std::make_unique<BosLiteralIfade>(tok.satir, tok.sutun);
        }
        std::string sinifIsmi = ilerle().lexem;
        tuket(TokenTuru::SOL_PAREN, "'yeni' sonrasında '(' bekleniyor.");
        auto args = argumanListesi();
        tuket(TokenTuru::SAG_PAREN, "Argüman listesi ')' ile kapatılmalı.");
        return std::make_unique<YeniIfade>(
            std::move(sinifIsmi), std::move(args), tok.satir, tok.sutun);
    }

    // ── oku() ─────────────────────────────────────────────
    if (tok.tur == TokenTuru::OKU) {
        ilerle();  // 'oku' tüket
        tuket(TokenTuru::SOL_PAREN, "'oku' sonrasında '(' bekleniyor.");
        tuket(TokenTuru::SAG_PAREN, "'oku()' ')' ile kapatılmalı.");
        // oku() standart kütüphane fonksiyon çağrısı olarak temsil edilir
        auto kimlik = std::make_unique<KimlikIfade>("oku", tok.satir, tok.sutun);
        std::vector<IfadePtr> bos;
        return std::make_unique<CagriIfade>(
            std::move(kimlik), std::move(bos), tok.satir, tok.sutun);
    }

    // ── Bilinmeyen token ──────────────────────────────────
    std::ostringstream msg;
    msg << "Beklenmedik token: " << tokenTuruGoruntu(tok.tur);
    if (!tok.lexem.empty()) msg << " '" << tok.lexem << "'";
    hataEkle(tok, msg.str());
    panikMod_ = true;

    // Hata kurtarma: sahte değer döndür
    ilerle();
    return std::make_unique<BosLiteralIfade>(tok.satir, tok.sutun);
}

// ============================================================
// Yardımcı: argüman listesi
// ============================================================
std::vector<IfadePtr> Parser::argumanListesi() {
    std::vector<IfadePtr> args;

    if (!kontrol(TokenTuru::SAG_PAREN) && !bitti()) {
        do {
            if (args.size() >= 255) {
                hataEkle(mevcut(),
                         "Fonksiyon argüman sayısı 255'i geçemez.");
            }
            args.push_back(ifade());
        } while (eslestir(TokenTuru::VIRGUL));
    }

    return args;
}

// ============================================================
// Liste literali: [elem1, elem2, ...]
// ============================================================
IfadePtr Parser::listeLiteral(uint32_t satir, uint32_t sutun) {
    std::vector<IfadePtr> elemanlar;

    if (!kontrol(TokenTuru::SAG_KOSELI) && !bitti()) {
        do {
            elemanlar.push_back(ifade());
        } while (eslestir(TokenTuru::VIRGUL));
    }

    tuket(TokenTuru::SAG_KOSELI,
          "Liste literali ']' ile kapatılmalı.");

    return std::make_unique<ListeIfade>(
        std::move(elemanlar), satir, sutun);
}

// ============================================================
// Sözlük literali: {anahtar: deger, ...}
// ============================================================
IfadePtr Parser::sozlukLiteral(uint32_t satir, uint32_t sutun) {
    std::vector<SozlukIfade::Cift> cifter;

    if (!kontrol(TokenTuru::SAG_SUSE) && !bitti()) {
        do {
            auto anahtar = ifade();
            tuket(TokenTuru::IKI_NOKTA,
                  "Sözlük anahtar-değer arasında ':' bekleniyor.");
            auto deger = ifade();
            cifter.push_back({std::move(anahtar), std::move(deger)});
        } while (eslestir(TokenTuru::VIRGUL));
    }

    tuket(TokenTuru::SAG_SUSE,
          "Sözlük literali '}' ile kapatılmalı.");

    return std::make_unique<SozlukIfade>(std::move(cifter), satir, sutun);
}

} // namespace Doruk


