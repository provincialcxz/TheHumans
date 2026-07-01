cmake_minimum_required(VERSION 3.16)

project(Lyudishki VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/forms)

add_compile_options(-Wall -Wextra)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Gui Core Svg)

set(DOMAIN_SOURCES
    src/domain/Person.h
    src/domain/Group.h
    src/domain/SocialAccount.h
    src/domain/Email.h
    src/domain/PersonProfile.h
    src/domain/PersonEvent.h
    src/domain/PhoneNumber.h
    src/domain/PersonNote.h
    src/domain/PersonFile.h
)

set(DATA_SOURCES
    src/data/db/DatabaseManager.h
    src/data/db/DatabaseManager.cpp
    src/data/db/MigrationManager.h
    src/data/db/MigrationManager.cpp
    src/data/repositories/IPersonRepository.h
    src/data/repositories/IGroupRepository.h
    src/data/repositories/SqlitePersonRepository.h
    src/data/repositories/SqlitePersonRepository.cpp
    src/data/repositories/SqliteGroupRepository.h
    src/data/repositories/SqliteGroupRepository.cpp
)

set(SERVICE_SOURCES
    src/services/PeopleService.h
    src/services/PeopleService.cpp
    src/services/GroupService.h
    src/services/GroupService.cpp
    src/services/SearchService.h
    src/services/SearchService.cpp
    src/services/ReminderService.h
    src/services/ReminderService.cpp
    src/services/CalendarExportService.h
    src/services/CalendarExportService.cpp
    src/services/DataService.h
    src/services/DataService.cpp
)

set(UI_SOURCES
    src/ui/windows/MainWindow.h
    src/ui/windows/MainWindow.cpp
    src/ui/models/PersonListModel.h
    src/ui/models/PersonListModel.cpp
    src/ui/models/PersonSortFilterProxy.h
    src/ui/models/PersonSortFilterProxy.cpp
    src/ui/delegates/PersonRowDelegate.h
    src/ui/delegates/PersonRowDelegate.cpp
    src/ui/views/PersonListView.h
    src/ui/views/PersonListView.cpp
    src/ui/views/PersonDetailView.h
    src/ui/views/PersonDetailView.cpp
    src/ui/views/EmptyStateView.h
    src/ui/views/EmptyStateView.cpp
    src/ui/views/PersonEditView.h
    src/ui/views/PersonEditView.cpp
    src/ui/widgets/SocialIcon.h
    src/ui/widgets/SocialIcon.cpp
    src/ui/widgets/PersonGraphView.h
    src/ui/widgets/PersonGraphView.cpp
    src/ui/widgets/PersonGraphNode.h
    src/ui/widgets/PersonGraphNode.cpp
    src/ui/dialogs/PersonGraphDialog.h
    src/ui/dialogs/PersonGraphDialog.cpp
    src/ui/dialogs/PersonStatsDialog.h
    src/ui/dialogs/PersonStatsDialog.cpp
)

set(FORM_FILES
    forms/main_window.ui
    forms/group_empty.ui
    forms/person_list.ui
    forms/person_row.ui
    forms/person_detail.ui
    forms/person_edit.ui
    forms/event_edit.ui
    forms/settings.ui
)

set(APP_SOURCES
    src/app/AppContext.h
    src/app/AppContext.cpp
)

qt_add_executable(Lyudishki
    MANUAL_FINALIZATION
    src/main.cpp
    ${APP_SOURCES}
    ${DOMAIN_SOURCES}
    ${DATA_SOURCES}
    ${SERVICE_SOURCES}
    ${UI_SOURCES}
    ${FORM_FILES}
    resources/resources.qrc
)

target_include_directories(Lyudishki PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(Lyudishki PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Gui
    Qt6::Core
    Qt6::Svg
)

set(MACOSX_BUNDLE_ICON_FILE Lyudishki.icns)
set(APP_ICON_MACOS ${CMAKE_SOURCE_DIR}/resources/icons/Lyudishki.icns)
set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
target_sources(Lyudishki PRIVATE ${APP_ICON_MACOS})

# Windows icon
if(WIN32)
    target_sources(Lyudishki PRIVATE ${CMAKE_SOURCE_DIR}/resources/icons/Lyudishki.rc)
endif()

set_target_properties(Lyudishki PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE Lyudishki.icns
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Lyudishki
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(Lyudishki)

# --- Tests ---
enable_testing()

qt_add_executable(LyudishkiTests
    tests/main_test.cpp
    tests/TestGroupRepository.cpp
    tests/TestPersonRepository.cpp
    tests/TestServices.cpp
    tests/TestPersonSortFilterProxy.cpp
    ${DOMAIN_SOURCES}
    ${DATA_SOURCES}
    ${SERVICE_SOURCES}
    src/ui/models/PersonListModel.cpp
    src/ui/models/PersonSortFilterProxy.cpp
    resources/resources.qrc
)

target_include_directories(LyudishkiTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(LyudishkiTests PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Gui
    Qt6::Core
    Qt6::Test
)

find_package(Qt6 REQUIRED COMPONENTS Test)

add_test(NAME LyudishkiTests COMMAND LyudishkiTests)
