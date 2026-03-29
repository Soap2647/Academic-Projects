// ============================================================
// Lexer.h — DORUK Dili Sözdizimsel Tarayıcı (Tokenizer)
// Açıklama: Ham UTF-8 kaynak metnini token akışına dönüştürür.
//           Her token; türü, ham metni, satır ve sütun bilgisi
//           içerir. Türkçe anahtar kelimeler ve UTF-8 karakterler
//           (ğ, ş, ı, ö, ü, ç vb.) tam olarak desteklenir.
//
// Kullanım:
//   Lexer lx(kaynakMetin, "dosya.drk");
//   auto [tokenler, tanilar] = lx.tara();
//   if (!tanilar.hatalarVar()) { /* devam et */ }
// ============================================================
#pragma once

#include "Token.h"
#include "Diagnostics.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>   // std::pair

namespace Doruk {

// ============================================================
// LexerSonucu — Tarama sonucunu tutan yapı
// ============================================================
struct LexerSonucu {
    std::vector<Token> tokenler;   // üretilen token akışı
    TaniListesi        tanilar;    // hata/uyarı listesi

    bool basarili() const { return !tanilar.hatalarVar(); }
};

// ============================================================
// Lexer — Ana tokenizer sınıfı
// ============================================================
class Lexer {
public:
    // kaynak : UTF-8 kodlu kaynak metin
    // dosya  : hata mesajlarında gösterilecek dosya adı
    explicit Lexer(std::string kaynak, std::string dosya = "<isimsiz>");

    // Lexer kopyalanamaz, taşınabilir
    Lexer(const Lexer&)            = delete;
    Lexer& operator=(const Lexer&) = delete;
    Lexer(Lexer&&)                 = default;
    Lexer& operator=(Lexer&&)      = default;

    // Ana tarama fonksiyonu
    // Kaynağın tamamını tarayarak LexerSonucu döndürür.
    // Her çağrıda yeniden başlar (idempotent).
    LexerSonucu tara();

    // Kaynak metnin belirli bir satırını döndürür (hata gösterimi için)
    std::string satirGetir(uint32_t satirNo) const;

private:
    // ── Kaynak Durumu ────────────────────────────────────────
    std::string  kaynak_;        // UTF-8 kaynak metin
    std::string  dosya_;         // dosya adı
    size_t       konum_;         // mevcut byte konumu
    uint32_t     satir_;         // mevcut satır (1-tabanlı)
    uint32_t     sutun_;         // mevcut sütun (1-tabanlı, byte-offset)
    size_t       satirBaslangic_; // mevcut satırın byte başlangıcı

    // ── Çıktı Tamponları ─────────────────────────────────────
    std::vector<Token> tokenler_;
    TaniListesi        tanilar_;

    // ── Durumu Sıfırlama ─────────────────────────────────────
    void sifirla();

    // ── İlerleme ve Bakış ────────────────────────────────────

    // Dosyanın sonuna gelindi mi?
    bool bitti() const noexcept;

    // Mevcut byte'ı döndürür (ilerlemez)
    char mevcutKar() const noexcept;

    // Bir sonraki byte'ı döndürür (ilerlemez)
    char sonrakiKar() const noexcept;

    // İki sonraki byte'ı döndürür (ilerlemez)
    char sonrakiSonrakiKar() const noexcept;

    // Mevcut byte'ı tüketir; satır/sütun günceller; tüketilen char'ı döndürür
    char ilerle();

    // Mevcut byte beklenenle eşleşiyorsa tüket ve true döndür, değilse false
    bool eslestir(char beklenen);

    // ── UTF-8 Yardımcıları ────────────────────────────────────

    // Mevcut konumdan bir UTF-8 kod noktası okur, konumu ilerletir
    // Hatalı UTF-8 dizisiyse U+FFFD döndürür
    uint32_t utf8OkuIlerle();

    // Mevcut konumdaki UTF-8 kod noktasını ilerletmeden okur
    uint32_t utf8MevcutBak() const;

    // UTF-8 başlangıç byte'ı mı? (çok-byte'lı karakter)
    static bool utf8CokByteMi(unsigned char b) noexcept;

    // Verilen kod noktası bir tanımlayıcı başlangıcı olabilir mi?
    // ASCII: [A-Za-z_], Türkçe ve Latin genişletilmiş: U+00C0+
    static bool kimlikBaslangiciMi(uint32_t cp) noexcept;

    // Verilen kod noktası bir tanımlayıcı devamı olabilir mi?
    // Başlangıç koşulları + ASCII rakamlar
    static bool kimlikDevamMi(uint32_t cp) noexcept;

    // ── Boşluk ve Yorum Atlama ────────────────────────────────

    // Boşluk, tab, satır sonu karakterlerini atla
    void bosluklarAtla();

    // // ile başlayan satır yorumunu atla
    void satirYorumAtla();

    // /* ... */ bloğunu atla; iç içe yorumlar desteklenmez
    void blokYorumAtla();

    // ── Token Üreticiler ─────────────────────────────────────

    // Bir sonraki token'ı üret
    Token sonrakiToken();

    // Metin literal'ini tara: "içerik"
    // Çıkış karakterleri: \n \t \r \\ \" \' \0
    Token metinTara(uint32_t basSatir, uint32_t basSutun);

    // Sayı literal'ini tara: tam sayı veya ondalık
    Token sayiTara(uint32_t basSatir, uint32_t basSutun);

    // Tanımlayıcı ya da anahtar kelime tara
    Token kimlikTara(uint32_t basSatir, uint32_t basSutun);

    // ── Tanı Yardımcıları ────────────────────────────────────

    void hataEkle(uint32_t satir,
                  uint32_t sutun,
                  uint32_t uzunluk,
                  std::string mesaj,
                  std::string ipucu = "");

    // ── Anahtar Kelime Tablosu ───────────────────────────────
    // UTF-8 string → TokenTuru eşlemesi
    // Statik, program ömrü boyunca yaşar
    static const std::unordered_map<std::string, TokenTuru>& anahtarKelimeler();
};

} // namespace Doruk

