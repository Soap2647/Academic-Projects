# ============================================================
# CMakeLists.txt — DORUK Programlama Dili
# Adım 1-6: doruk_engine + doruk_cli + testler
#
# Kullanım:
#   mkdir build && cd build
#   cmake .. -DCMAKE_BUILD_TYPE=Release
#   cmake --build .
#   ctest --output-on-failure
# ============================================================
cmake_minimum_required(VERSION 3.20)
project(doruk
    VERSION     1.0.0
    DESCRIPTION "DORUK - Turkce anahtar kelimeli betik dili"
    LANGUAGES   CXX
)

# ── C++17 zorunlu ────────────────────────────────────────────
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ── Derleme bayrakları ────────────────────────────────────────
if(MSVC)
    add_compile_options(
        /W4
        /WX
        /utf-8
        /wd4996    # deprecated POSIX isimleri
        /wd4100    # unreferenced formal parameter (visitor default impl)
    )
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# ── Sürüm bilgisi başlığı ──────────────────────────────────
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/include/Surum.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/Surum.h"
    @ONLY
)

# ============================================================
# doruk_engine — Statik Kütüphane
# Adım 1: Lexer (Token + Diagnostics + Lexer)
# Adım 2: AST
# Adım 3: Parser
# Adım 4: SemanticAnalyzer
# Adım 5: Value + Environment + Interpreter
# ============================================================
add_library(doruk_engine STATIC
    # Adım 1 — Lexer
    engine/src/Token.cpp
    engine/src/Diagnostics.cpp
    engine/src/Lexer.cpp
    # Adım 2 — AST
    engine/src/AST.cpp
    # Adım 3 — Parser
    engine/src/Parser.cpp
    # Adım 4 — Semantik Analizör
    engine/src/SemanticAnalyzer.cpp
    # Adım 5 — Yorumlayıcı
    engine/src/Value.cpp
    engine/src/Environment.cpp
    engine/src/Interpreter.cpp
)

target_include_directories(doruk_engine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/include"
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

target_compile_options(doruk_engine PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# ============================================================
# doruk_cli — CLI çalıştırıcı (Adım 6)
# ============================================================
add_executable(doruk cli/main.cpp)
target_link_libraries(doruk PRIVATE doruk_engine)
target_compile_options(doruk PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# Derleme sonrası doruk.exe'yi proje köküne kopyala (kolaylık için)
add_custom_command(TARGET doruk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:doruk>
        "${CMAKE_CURRENT_SOURCE_DIR}/doruk$<$<PLATFORM_ID:Windows>:.exe>"
    COMMENT "doruk executable -> proje kokune kopyalandi"
)

# ============================================================
# Catch2 — FetchContent
# ============================================================
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
include(Catch)

# ============================================================
# doruk_ide — Adım 7-12 (Qt6 Masaüstü Uygulaması) — opsiyonel
# ============================================================
find_package(Qt6 COMPONENTS Widgets Core Gui QUIET)
if(Qt6_FOUND)
    add_subdirectory(ide)
    message(STATUS "Qt6 bulundu — doruk_ide hedefi etkin")
else()
    message(STATUS "Qt6 bulunamadı — doruk_ide atlandı (sadece CLI derlenir)")
endif()

# ============================================================
# Testler
# ============================================================
enable_testing()

# ── Lexer testleri (Adım 1) ──────────────────────────────────
add_executable(test_lexer tests/test_lexer.cpp)
target_link_libraries(test_lexer PRIVATE doruk_engine Catch2::Catch2WithMain)
target_compile_options(test_lexer PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
catch_discover_tests(test_lexer PROPERTIES TIMEOUT 30)

# ── Parser testleri (Adım 3) ─────────────────────────────────
add_executable(test_parser tests/test_parser.cpp)
target_link_libraries(test_parser PRIVATE doruk_engine Catch2::Catch2WithMain)
target_compile_options(test_parser PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
catch_discover_tests(test_parser PROPERTIES TIMEOUT 30)

# ── Yorumlayıcı testleri (Adım 5) ───────────────────────────
add_executable(test_interpreter tests/test_interpreter.cpp)
target_link_libraries(test_interpreter PRIVATE doruk_engine Catch2::Catch2WithMain)
target_compile_options(test_interpreter PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8 /source-charset:utf-8 /execution-charset:utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
catch_discover_tests(test_interpreter PROPERTIES TIMEOUT 60)

# ============================================================
# Özet
# ============================================================
message(STATUS "")
message(STATUS "=== DORUK v${PROJECT_VERSION} ===")
message(STATUS "  C++           : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build         : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Derleyici     : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "")
message(STATUS "Hedefler:")
message(STATUS "  doruk_engine    -> statik kutuphane")
message(STATUS "  doruk           -> CLI calistirici (doruk.exe)")
message(STATUS "  doruk_ide       -> Qt6 IDE Masaustu Uygulamasi")
message(STATUS "  test_lexer      -> Adim 1 testleri")
message(STATUS "  test_parser     -> Adim 2-3 testleri")
message(STATUS "  test_interpreter-> Adim 4-5 testleri")
message(STATUS "")
message(STATUS "  ctest --output-on-failure")
message(STATUS "")
