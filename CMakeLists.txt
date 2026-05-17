cmake_minimum_required(VERSION 3.21)

project(ClipFlowQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTORCC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Gui Svg)

qt_standard_project_setup()

qt_add_executable(ClipFlowQt WIN32
    resources/windows/app.rc
    resources/qt/resources.qrc
    src/config.h
    src/configmanager.h
    src/configmanager.cpp
    src/configdialog.h
    src/configdialog.cpp
    src/droplabel.h
    src/droplabel.cpp
    src/image_preview_dialog.h
    src/image_preview_dialog.cpp
    src/globalhotkey.h
    src/globalhotkey.cpp
    src/region_capture_overlay.h
    src/region_capture_overlay.cpp
    src/markdown_utils.h
    src/markdown_utils.cpp
    src/image_to_markdown_runner.h
    src/image_to_markdown_runner.cpp
    src/image_to_translation_runner.h
    src/image_to_translation_runner.cpp
    src/app_style.h
    src/app_style.cpp
    src/rounded_menu.h
    src/rounded_menu.cpp
    src/openai_chat_client.h
    src/openai_chat_client.cpp
    src/imagetab.h
    src/imagetab.cpp
    src/translationtab.h
    src/translationtab.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/main.cpp
)

target_link_libraries(ClipFlowQt PRIVATE Qt6::Widgets Qt6::Network Qt6::Gui Qt6::Svg)

set_target_properties(ClipFlowQt PROPERTIES OUTPUT_NAME ClipFlow)
