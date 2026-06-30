# Lyudishki

## Требования

- **Qt 6** (6.5+), модули: Widgets, Sql, Gui, Core, Svg
- **CMake** 3.16+
- **C++17** совместимый компилятор (GCC 9+, Clang 10+, MSVC 2019+)
- SQLite (встроен в Qt)

## Сборка и запуск

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
cmake --build .

# macOS
open Lyudishki.app

# Linux / Windows
./Lyudishki
```

### Запуск тестов

```bash
cd build
ctest
# или напрямую:
./LyudishkiTests
```

## Структура проекта

```
Lyudishki/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── app/                  # composition root, DI
│   ├── domain/               # Person, Group, SocialAccount, Email, PersonEvent, PersonProfile
│   ├── data/
│   │   ├── db/               # DatabaseManager, MigrationManager
│   │   └── repositories/     # I*Repository + Sqlite*Repository
│   ├── services/             # PeopleService, GroupService, SearchService,
│   │                         #   ReminderService, CalendarExportService
│   └── ui/
│       ├── windows/          # MainWindow
│       ├── views/            # EmptyStateView, PersonListView, PersonDetailView, PersonEditView
│       ├── models/           # PersonListModel, PersonSortFilterProxy
│       └── delegates/        # PersonRowDelegate
├── forms/                    # все .ui файлы
│   ├── main_window.ui
│   ├── group_empty.ui
│   ├── person_list.ui
│   ├── person_row.ui
│   ├── person_detail.ui
│   ├── person_edit.ui
│   ├── event_edit.ui
│   └── settings.ui
├── resources/
│   ├── resources.qrc
│   ├── styles/mrrobot.qss
│   ├── fonts/                # JetBrains Mono
│   └── icons/
└── tests/                    # Qt Test (репозитории, сервисы)
```

## Хранилище данных

SQLite, один файл в `QStandardPaths::AppDataLocation`. Схема создаётся автоматически при первом запуске через систему миграций.

