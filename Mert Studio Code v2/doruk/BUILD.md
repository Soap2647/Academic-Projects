# DORUK — Derleme Talimatları

## Gereksinimler

| Araç | Minimum Sürüm |
|------|---------------|
| CMake | 3.20 |
| MSVC (Visual Studio 2022) | 19.30+ |
| veya GCC | 10+ |
| veya Clang | 12+ |
| İnternet bağlantısı | Catch2 indirilmesi için |

## Hızlı Başlangıç (Windows — Visual Studio 2022)

```powershell
# Visual Studio Developer PowerShell'i açın, ardından:
cd "D:\Mevcut Projeler\Mert Studio Code Nihai Hali\doruk"

mkdir build
cd build

# Visual Studio 2022 (x64)
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug

# Derle
cmake --build . --config Debug

# Testleri çalıştır
ctest -C Debug --output-on-failure
```

## MSYS2 / MinGW ile

```bash
cd /d/Mevcut\ Projeler/Mert\ Studio\ Code\ Nihai\ Hali/doruk
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build .
ctest --output-on-failure
```

## Tek test çalıştırma (verbose)

```bash
./Debug/test_lexer.exe --reporter console -v
# veya belirli bir tag:
./Debug/test_lexer.exe "[anahtar]" --reporter console
```

## Proje yapısı

```
doruk/
├── engine/
│   ├── include/
│   │   ├── Token.h          — Token türleri ve yapısı
│   │   ├── Diagnostics.h    — Hata/uyarı sistemi
│   │   └── Lexer.h          — Tokenizer arayüzü
│   └── src/
│       ├── Token.cpp
│       ├── Diagnostics.cpp
│       └── Lexer.cpp
├── tests/
│   └── test_lexer.cpp       — 60+ Catch2 test vakası
├── CMakeLists.txt
└── BUILD.md
```

## Adım Adım Geliştirme

Tüm adımlar tamamlandıkça proje büyüyecek:

- [x] **Adım 1** — Lexer + test paketi
- [ ] **Adım 2** — AST düğüm tanımları
- [ ] **Adım 3** — Parser + hata kurtarma
- [ ] **Adım 4** — Semantik Analizör
- [ ] **Adım 5** — Yorumlayıcı + standart kütüphane
- [ ] **Adım 6** — CLI çalıştırıcı (doruk.exe)
- [ ] **Adım 7-12** — Qt6 IDE
