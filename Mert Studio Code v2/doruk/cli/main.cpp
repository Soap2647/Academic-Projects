// ============================================================
// main.cpp — DORUK CLI runner (Adım 6)
//
// Kullanım:
//   doruk dosya.drk          — dosya çalıştır
//   doruk -                   — stdin'den oku (pipe)
//   doruk --version | -v      — sürüm göster
//   doruk --yardim | --help   — yardım göster
//
// Çıkış kodları:
//   0  — başarı
//   1  — sözdizim / semantik hata
//   2  — çalışma zamanı hatası
//   3  — dosya bulunamadı
// ============================================================
#include "Lexer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "Interpreter.h"
#include "Surum.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

// ── Yardımcılar ──────────────────────────────────────────────

static void surumGoster() {
    std::cout << "DORUK " << DORUK_SURUM << "\n"
              << "T\xc3\xbcrk\xc3\xa7" "e anahtar kelimeli betik dili\n"
              // Türkçe anahtar kelimeli betik dili
              << "https://github.com/merty/doruk\n";
}

static void yardimGoster(const char* prog) {
    std::cout
        << "Kullan\xc4\xb1m: " << prog << " [SECENEK] [DOSYA]\n\n"
        // Kullanım
        << "Secenekler:\n"
        << "  -v, --version    S\xc3\xbcr\xc3\xbcm bilgisi g\xc3\xb6ster\n"
        // Sürüm bilgisi göster
        << "  --help           Bu yard\xc4\xb1m metnini g\xc3\xb6ster\n"
        // Bu yardım metnini göster
        << "  --yalniz-lex     Sadece tokenle\xc5\x9ftir, yazdır\n"
        // Sadece tokenleştir, yazdır
        << "  --yalniz-parse   Sadece ayr\xc4\xb1\xc5\x9ft\xc4\xb1r, AST yazdır\n"
        // Sadece ayrıştır, AST yazdır
        << "  --yalniz-anlam   Semantik analiz yap, \xc3\xa7" "al\xc4\xb1\xc5\x9ft\xc4\xb1rma\n"
        // Semantik analiz yap, çalıştırma
        << "\n"
        << "Dosya: .drk uzantil\xc4\xb1 DORUK kayna\xc4\x9f\xc4\xb1 veya '-' (stdin)\n";
        // .drk uzantılı DORUK kaynağı veya '-' (stdin)
}

static std::string dosyaOku(const std::string& yol, bool& basarili) {
    if (yol == "-") {
        // stdin'den oku
        std::ostringstream oss;
        oss << std::cin.rdbuf();
        basarili = true;
        return oss.str();
    }
    std::ifstream dosya(yol, std::ios::binary);
    if (!dosya) {
        basarili = false;
        return {};
    }
    std::ostringstream oss;
    oss << dosya.rdbuf();
    basarili = true;
    return oss.str();
}

static void tanilariYazdir(const Doruk::TaniListesi& tanilar) {
    for (const auto& t : tanilar.hepsi()) {
        std::cerr << t.bicimle() << "\n";
    }
}

// ── Ana giriş noktası ─────────────────────────────────────────

int main(int argc, char* argv[]) {
    if (argc < 2) {
        yardimGoster(argv[0]);
        return 0;
    }

    bool yalnizLex    = false;
    bool yalnizParse  = false;
    bool yalnizAnlam  = false;
    std::string dosyaYolu;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--version") { surumGoster(); return 0; }
        if (arg == "--help" || arg == "--yardim") { yardimGoster(argv[0]); return 0; }
        if (arg == "--yalniz-lex")    { yalnizLex   = true; continue; }
        if (arg == "--yalniz-parse")  { yalnizParse = true; continue; }
        if (arg == "--yalniz-anlam")  { yalnizAnlam = true; continue; }
        if (arg[0] == '-' && arg != "-") {
            std::cerr << "Bilinmeyen secenek: " << arg << "\n";
            return 1;
        }
        dosyaYolu = arg;
    }

    if (dosyaYolu.empty()) {
        std::cerr << "Hata: dosya belirtilmedi.\n";
        yardimGoster(argv[0]);
        return 1;
    }

    // ── Kaynak dosyayı oku ──────────────────────────────────
    bool okuBasarili = false;
    std::string kaynak = dosyaOku(dosyaYolu, okuBasarili);
    if (!okuBasarili) {
        std::cerr << "Hata: '" << dosyaYolu << "' a\xc3\xa7\xc4\xb1lamad\xc4\xb1.\n";
        // açılamadı
        return 3;
    }

    std::string dosyaAdi = (dosyaYolu == "-") ? "<stdin>" : dosyaYolu;

    // ── Aşama 1: Lexer ──────────────────────────────────────
    Doruk::Lexer lexer(kaynak, dosyaAdi);
    auto lexSonucu = lexer.tara();

    if (!lexSonucu.basarili()) {
        tanilariYazdir(lexSonucu.tanilar);
        return 1;
    }

    if (yalnizLex) {
        for (const auto& tok : lexSonucu.tokenler) {
            std::cout << tok.satir << ":" << tok.sutun
                      << "\t" << Doruk::tokenTuruAdi(tok.tur)
                      << "\t'" << tok.lexem << "'\n";
        }
        return 0;
    }

    // ── Aşama 2: Parser ─────────────────────────────────────
    Doruk::Parser parser(std::move(lexSonucu.tokenler), dosyaAdi);
    auto parseSonucu = parser.ayristir();

    if (!parseSonucu.basarili()) {
        tanilariYazdir(parseSonucu.tanilar);
        return 1;
    }

    if (yalnizParse) {
        std::cout << Doruk::astYazdir(*parseSonucu.program);
        return 0;
    }

    // ── Aşama 3: Semantik analiz ─────────────────────────────
    Doruk::SemanticAnalyzer anlam(dosyaAdi);
    auto anlamSonucu = anlam.analiz(*parseSonucu.program);

    if (!anlamSonucu.basarili()) {
        tanilariYazdir(anlamSonucu.tanilar);
        return 1;
    }
    // Uyarıları yine de yazdır
    if (anlamSonucu.tanilar.taniSayisi() > 0) {
        tanilariYazdir(anlamSonucu.tanilar);
    }

    if (yalnizAnlam) {
        std::cout << "Semantik analiz ba\xc5\x9f" "ar\xc4\xb1l\xc4\xb1.\n";
        // başarılı
        return 0;
    }

    // ── Aşama 4: Yorumlayıcı ────────────────────────────────
    Doruk::Interpreter yorumlayici(
        // ciktiCb — stdout'a yaz
        [](const std::string& s) { std::cout << s; std::cout.flush(); },
        // girisCb — stdin'den oku
        []() -> std::string {
            std::string satir;
            std::getline(std::cin, satir);
            return satir;
        }
    );

    auto yorumSonucu = yorumlayici.calistir(*parseSonucu.program);

    if (!yorumSonucu.basarili) {
        tanilariYazdir(yorumSonucu.tanilar);
        return 2;
    }

    return 0;
}
