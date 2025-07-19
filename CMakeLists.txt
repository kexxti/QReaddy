cmake_minimum_required(VERSION 3.19)
project(Practice_Pdf_Manager LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Sql)

# Попытка найти Poppler
find_package(Poppler COMPONENTS Qt6)

# Если Poppler не найден, указываем пути вручную
if(NOT Poppler_FOUND)
    message(WARNING "Poppler-Qt6 not found via find_package, setting paths manually")
    find_library(POPPLER_QT6_LIBRARY NAMES poppler-qt6 PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
    find_path(POPPLER_QT6_INCLUDE_DIR poppler-qt6.h PATHS /usr/include/poppler/qt6)
    if(POPPLER_QT6_LIBRARY AND POPPLER_QT6_INCLUDE_DIR)
        add_library(Poppler::Qt6 INTERFACE IMPORTED)
        set_target_properties(Poppler::Qt6 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${POPPLER_QT6_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${POPPLER_QT6_LIBRARY}"
        )
    else()
        message(FATAL_ERROR "Could not find Poppler-Qt6 library or headers")
    endif()
endif()


qt_standard_project_setup()

qt_add_executable(Practice_Pdf_Manager
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    pdfviewer.h
    pdfviewer.cpp
    progressbar_delegate.h progressbar_delegate.cpp
    pdfcoverdelegate.h pdfcoverdelegate.cpp
)

target_link_libraries(Practice_Pdf_Manager
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql     
        Poppler::Qt6
)

include(GNUInstallDirs)

install(TARGETS Practice_Pdf_Manager
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Practice_Pdf_Manager
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
