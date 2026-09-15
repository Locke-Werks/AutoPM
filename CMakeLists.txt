cmake_minimum_required(VERSION 3.21)
project(AutoPM VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /EHsc)
endif()

# ── core ─────────────────────────────────────────────────────────────────
# Knows about definitions and records and nothing about windows. Following
# ProjectMan: one core, several faces.

add_library(autopm_core STATIC
    src/core/blockfile.cpp
    src/core/coach.cpp
    src/core/definition.cpp
    src/core/gitlog.cpp
    src/core/record.cpp
    src/core/workspace.cpp
)
target_include_directories(autopm_core PUBLIC src)

# ── gui ──────────────────────────────────────────────────────────────────

# ── mcp ──────────────────────────────────────────────────────────────────
# The second face on the same core, following ProjectMan. No Qt: a server that
# gets spawned once per session should not need a toolkit beside it on disk.

add_executable(autopm_mcp
    src/mcp/json.cpp
    src/mcp/main.cpp
)
target_link_libraries(autopm_mcp PRIVATE autopm_core)
set_target_properties(autopm_mcp PROPERTIES OUTPUT_NAME "autopm-mcp")
if(WIN32)
    target_link_libraries(autopm_mcp PRIVATE shell32 ole32)
endif()

add_custom_command(TARGET autopm_mcp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/definitions" "$<TARGET_FILE_DIR:autopm_mcp>/definitions"
    COMMENT "Staging definitions next to autopm-mcp.exe"
)

# ── gui ──────────────────────────────────────────────────────────────────

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(autopm WIN32
    src/gui/main.cpp
    src/gui/theme.cpp
    src/gui/accentdialog.cpp
    src/gui/chrome.cpp
    src/gui/helppanel.cpp
    src/gui/fieldeditors.cpp
    src/gui/tableeditor.cpp
    src/gui/screenview.cpp
    src/gui/boardview.cpp
    src/gui/planningview.cpp
    src/gui/planviews.cpp
    src/gui/dashboard.cpp
    src/gui/guided.cpp
    src/gui/mainwindow.cpp
    src/gui/autopm.rc
)
target_link_libraries(autopm PRIVATE autopm_core Qt6::Widgets)
if(WIN32)
    # DwmSetWindowAttribute, for the dark title bar.
    target_link_libraries(autopm PRIVATE dwmapi)
endif()
set_target_properties(autopm PROPERTIES OUTPUT_NAME "AutoPM")

# The definition files and the icon travel with the executable, so a build
# directory runs exactly like an install.
add_custom_command(TARGET autopm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/definitions" "$<TARGET_FILE_DIR:autopm>/definitions"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:autopm>/assets"
    COMMENT "Staging definitions and assets next to AutoPM.exe"
)
