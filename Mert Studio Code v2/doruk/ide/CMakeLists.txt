# ============================================================
# CMakeLists.txt — DORUK IDE
# ============================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)

file(GLOB_RECURSE IDE_SOURCES "src/*.cpp")
file(GLOB_RECURSE IDE_HEADERS "include/*.h")

if(WIN32)
    add_executable(doruk_ide WIN32 ${IDE_SOURCES} ${IDE_HEADERS})
else()
    add_executable(doruk_ide ${IDE_SOURCES} ${IDE_HEADERS})
endif()

target_include_directories(doruk_ide
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(doruk_ide
    PRIVATE
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
        doruk_engine
)

target_compile_options(doruk_ide PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# Windeployqt for Windows distribution
if(WIN32)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET doruk_ide POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:doruk_ide>
                "${CMAKE_SOURCE_DIR}/doruk_ide.exe"
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-compiler-runtime
                --no-opengl-sw
                "${CMAKE_SOURCE_DIR}/doruk_ide.exe"
            COMMENT "doruk_ide executable -> proje kokune kopyalandi ve windeployqt calistirildi"
        )
    endif()
endif()
