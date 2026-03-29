// ============================================================
// Parser.h — DORUK Dili Sözdizim Çözümleyici (Recursive Descent)
//
// Girdi : Lexer tarafından üretilen token akışı
// Çıktı : Program AST düğümü (unique_ptr)
//         + Tani listesi (hatalar/uyarılar)
//
// Hata kurtarma:
//   Panik modu — hata tespit edildiğinde parser bir
//   "senkronizasyon noktası"na (Token::kapaticiMi()) kadar
//   token'ları atar ve sonraki deyimi ayrıştırmayı dener.
//   Bu sayede tek geçişte birden fazla hata raporlanır.
//
// Operatör önceliği (düşükten yükseğe):
//   atama → veya → ve → eşitlik → karşılaştırma →
//   toplama → çarpma → tekli → son-ek → birincil
// ============================================================
#pragma once

#include "Token.h"
#include "AST.h"
#include "Diagnostics.h"

#include <vector>
#include <memory>
#include <string>

namespace Doruk {

// ============================================================
// ParserSonucu — ayrıştırma sonucunu taşır
// ============================================================
struct ParserSonucu {
    std::unique_ptr<Program> program;  // nullptr = kritik başarısızlık
    TaniListesi              tanilar;  // hata/uyarı listesi

    bool basarili() const { return !tanilar.hatalarVar(); }
};

// ============================================================
// Parser — ana ayrıştırıcı sınıf
// ============================================================
class Parser {
public:
    // tokenler : Lexer'dan gelen token akışı (DOSYA_SONU dahil)
    // dosya    : hata mesajlarında kullanılacak dosya adı
    explicit Parser(std::vector<Token> tokenler,
                    std::string         dosya = "<isimsiz>");

    // Kopyalama yok
    Parser(const Parser&)            = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&)                 = default;
    Parser& operator=(Parser&&)      = default;

    // Ana ayrıştırma fonksiyonu — tüm kaynak dosyayı ayrıştırır
    ParserSonucu ayristir();

private:
    // ── Durum ───────────────────────────────────────────────
    std::vector<Token> tokenler_;   // token akışı
    size_t             konum_;      // mevcut token indeksi
    std::string        dosya_;      // dosya adı (hata mesajları için)
    TaniListesi        tanilar_;    // biriken tanılar
    bool               panikMod_;  // hata kurtarma modu mu?

    // ── Token Navigasyonu ────────────────────────────────────

    // Mevcut token'ı döndürür (tüketmez)
    const Token& mevcut() const;

    // Bir önceki token'ı döndürür
    const Token& onceki() const;

    // Sonraki token'a bak (tüketmez)
    const Token& sonraki() const;

    // Mevcut token belirtilen türdeyse tüket, değilse hata ver
    const Token& tuket(TokenTuru beklenen, const std::string& hataMsg);

    // Mevcut token belirtilen türdeyse tüket + true döndür, değilse false
    bool eslestir(TokenTuru tur);

    // Dosyanın sonuna mı geldik?
    bool bitti() const;

    // Mevcut token'ı tüket ve döndür
    Token ilerle();

    // Mevcut token'ın türü belirtilen tür mü?
    bool kontrol(TokenTuru tur) const;

    // ── Hata Yönetimi ────────────────────────────────────────

    // Hata ekle ve sahte token döndür (kurtarma için)
    void hataEkle(const Token& konum,
                  std::string  mesaj,
                  std::string  ipucu = "");

    // Panik moduna geç: kapatıcı token'a kadar ilerle
    void senkronize();

    // ── Deyim Ayrıştırıcılar ─────────────────────────────────

    // Genel deyim giriş noktası
    DeyimPtr deyim();

    // Blok: { deyim... }
    std::unique_ptr<BlokDeyim> blok();

    // değişken x = ifade
    DeyimPtr degiskenTanim();

    // fonksiyon isim(params) { govde }
    DeyimPtr fonksiyonTanim(bool sinifMetodu = false);

    // sınıf İsim { ... }
    DeyimPtr sinifTanim();

    // eğer (koşul) { ... } değilse { ... }
    DeyimPtr egerDeyim();

    // için (init; koşul; arttır) { ... }
    DeyimPtr icinDeyim();

    // döngü (koşul) { ... }
    DeyimPtr donguDeyim();

    // döndür [ifade]
    DeyimPtr dondurDeyim();

    // kır
    DeyimPtr kirDeyim();

    // devam
    DeyimPtr devamDeyim();

    // yazdır(args)
    DeyimPtr yazdirDeyim();

    // Genel ifade deyimi
    DeyimPtr ifadeDeyim();

    // ── İfade Ayrıştırıcılar (Öncelik Sırası) ────────────────

    // Giriş noktası
    IfadePtr ifade();

    // Atama (en düşük öncelik): x = expr, x += 1
    IfadePtr atama();

    // Mantıksal VEYA: || veya
    IfadePtr mantiksalVeya();

    // Mantıksal VE: && ve
    IfadePtr mantiksalVe();

    // Eşitlik: == !=
    IfadePtr esitlik();

    // Karşılaştırma: < <= > >=
    IfadePtr karsilastirma();

    // Toplama/çıkarma: + -
    IfadePtr toplama();

    // Çarpma/bölme/mod: * / %
    IfadePtr carpma();

    // Tekli önek: ! - değil
    IfadePtr tekli();

    // Son-ek (postfix): fonksiyon çağrısı, indis, üye erişimi
    IfadePtr sonEk();

    // Birincil ifade: literal, tanımlayıcı, (expr), liste, sözlük
    IfadePtr birincil();

    // ── Yardımcı Ayrıştırıcılar ──────────────────────────────

    // Argüman listesi: (arg1, arg2, ...)
    std::vector<IfadePtr> argumanListesi();

    // Liste literali: [elem1, elem2, ...]
    IfadePtr listeLiteral(uint32_t satir, uint32_t sutun);

    // Sözlük literali: {k1: v1, k2: v2, ...}
    IfadePtr sozlukLiteral(uint32_t satir, uint32_t sutun);

    // Sayı string'ini int64_t'ye çevir
    static int64_t metniTamSayiyaCevir(const std::string& s);

    // Sayı string'ini double'a çevir
    static double metniOndaligaCevir(const std::string& s);
};

} // namespace Doruk

