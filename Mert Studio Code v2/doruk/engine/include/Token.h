// ============================================================
// Token.h — DORUK Dili Token Tanımları
// Yazar: DORUK Geliştirme Ekibi
// Açıklama: Lexer tarafından üretilen token türlerini ve
//           Token yapısını tanımlar. Her token; türü, ham
//           metni (lexem) ve kaynak konumunu taşır.
// ============================================================
#pragma once

#include <string>
#include <cstdint>

namespace Doruk {

// ============================================================
// TokenTuru — Tüm olası token türlerini listeler
// ============================================================
enum class TokenTuru {
    // ── Literaller ──────────────────────────────────────────
    TAM_SAYI_LIT,      // 42, 100, 0
    ONDALIK_LIT,       // 3.14, 0.5, 2.718
    METIN_LIT,         // "merhaba dünya"

    // ── Anahtar Kelimeler ────────────────────────────────────
    DEGISKEN,          // değişken  → değişken tanımı
    EGER,              // eğer      → koşul ifadesi
    DEGILSE,           // değilse   → else dalı
    DONGU,             // döngü     → while döngüsü
    ICIN,              // için      → for döngüsü
    FONKSIYON,         // fonksiyon → fonksiyon tanımı
    DONDUR,            // döndür    → return ifadesi
    YAZDIR,            // yazdır    → stdout'a yaz
    OKU,               // oku       → stdin'den oku
    DOGRU,             // doğru     → boolean true
    YANLIS,            // yanlış    → boolean false
    BOS,               // boş       → null/nil
    VE,                // ve        → mantıksal AND (keyword)
    VEYA,              // veya      → mantıksal OR (keyword)
    DEGIL,             // değil     → mantıksal NOT (keyword)
    KIR,               // kır       → break
    DEVAM,             // devam     → continue
    SINIF,             // sınıf     → class tanımı
    YENI,              // yeni      → nesne örnekleme
    BU,                // bu        → this pointer

    // ── Tanımlayıcı ─────────────────────────────────────────
    TANIMLAYICI,       // değişken/fonksiyon ismi

    // ── Aritmetik Operatörler ────────────────────────────────
    ARTI,              // +
    EKSI,              // -
    CARPIM,            // *
    BOLUM,             // /
    MOD,               // %

    // ── Atama ───────────────────────────────────────────────
    ESIT,              // =

    // ── Karşılaştırma Operatörleri ───────────────────────────
    ESIT_ESIT,         // ==
    ESIT_DEGIL,        // !=
    KUCUK,             // <
    KUCUK_ESIT,        // <=
    BUYUK,             // >
    BUYUK_ESIT,        // >=

    // ── Mantıksal Operatörler (sembolik) ────────────────────
    VE_VE,             // &&
    VEYA_VEYA,         // ||
    UNLEM,             // !  (değil operatörünün sembolik formu)

    // ── Noktalama İşaretleri ─────────────────────────────────
    SOL_PAREN,         // (
    SAG_PAREN,         // )
    SOL_SUSE,          // {
    SAG_SUSE,          // }
    SOL_KOSELI,        // [
    SAG_KOSELI,        // ]
    NOKTALI_VIRGUL,    // ;
    VIRGUL,            // ,
    NOKTA,             // .
    IKI_NOKTA,         // :

    // ── Artırma/Azaltma ─────────────────────────────────────
    ARTI_ARTI,         // ++
    EKSI_EKSI,         // --
    ARTI_ESIT,         // +=
    EKSI_ESIT,         // -=
    CARPIM_ESIT,       // *=
    BOLUM_ESIT,        // /=

    // ── Özel ────────────────────────────────────────────────
    DOSYA_SONU,        // EOF — kaynak dosyanın sonu
    BILINMEYEN,        // Tanınmayan karakter (hata kurtarma için)
};

// ============================================================
// Token — Tek bir token birimi
// ============================================================
struct Token {
    TokenTuru   tur;       // token türü
    std::string lexem;     // kaynak metindeki ham gösterim
    uint32_t    satir;     // 1-tabanlı satır numarası
    uint32_t    sutun;     // 1-tabanlı sütun numarası (byte offset)

    Token(TokenTuru tur, std::string lexem, uint32_t satir, uint32_t sutun)
        : tur(tur)
        , lexem(std::move(lexem))
        , satir(satir)
        , sutun(sutun)
    {}

    // Varsayılan yapıcı (yalnızca DOSYA_SONU için)
    Token() : tur(TokenTuru::DOSYA_SONU), satir(0), sutun(0) {}

    // Token'ın "kapatıcı" olup olmadığını döndürür
    // (panik-modlu hata kurtarmada kullanılır)
    bool kapaticiMi() const;

    // Debug çıktısı için insan okunabilir gösterim
    std::string str() const;
};

// ============================================================
// Yardımcı Fonksiyonlar
// ============================================================

// Token türünün Türkçe okunabilir adını döndürür
const char* tokenTuruAdi(TokenTuru tur) noexcept;

// Token türünün kaynak kodda nasıl göründüğünü döndürür
// Örn: ARTI → "'+'"
const char* tokenTuruGoruntu(TokenTuru tur) noexcept;

} // namespace Doruk

