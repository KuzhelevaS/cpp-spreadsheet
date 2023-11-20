# Электронная таблица
Позволяет задавать значения ячеек, ссылаться на другие ячейки и проверять циклические зависимости. Проверяет корректность содержимого и кэширует вычисленные значения.

### В проекте используются
![C++17](https://img.shields.io/badge/C++17-1F2023?style=flat-square)
![STL](https://img.shields.io/badge/STL-1F2023?style=flat-square)
![Cmake](https://img.shields.io/badge/Cmake-1F2023?style=flat-square)
![ANTLR](https://img.shields.io/badge/ANTLR-1F2023?style=flat-square)

## Минимальные требования 
Для сборки и запуска проекта необходимы:
* версия С++17 или выше,
* CMake версии 3.8 или выше,
* Библиотека [ANTLR](https://www.antlr.org/) версии 4.12.0 или выше. Файлы для скачивания можно найти [на github](https://github.com/antlr/website-antlr4/tree/gh-pages/download).

## Подготовка и компиляция
В корне проекта  (рядом с `CMakeLists.txt`) создайте папку `antlr4_runtime`. Распакуйте в нее содержимое архива `antlr4-cpp-runtime-*-source.zip`.

У вас получится следующая структура проекта:
```
spreadsheet/
├── antlr4_runtime/
│   ├── cmake/
│   ├── runtime/
│   ├── CMakeLists.txt
│   └── Остальное содержимое архива antlr4-cpp-runtime-*-source.zip.
├── build/
├── antlr-*-complete.jar
├── CMakeLists.txt
├── FindANTLR.cmake
├── Formula.g4
└── Файлы *.cpp и *.h.
```

Перейдите в папку `build` и выполните команду:
```
cmake ../
cmake --build .
```

## Использование 
С принципами работы можно ознакомиться в файле `main.cpp`, ознакомившись с тестами.

Для использования таблицы необходимо подключить заголовочный файл:
```
#include "common.h"
```

Примеры использования:
* `std::unique_ptr<SheetInterface> sheet = CreateSheet();` - создает таблицу.
* `sheet->SetCell(Position::FromString("A1"), "1");` - задает ячейке A1 числовое значение 1.
* `sheet->GetCell(Position::FromString("A1"))->GetText();` - возвращает текстовое значение ячейки.
* `sheet->GetCell(Position::FromString("A1"))->GetValue();` - возвращает числовое значение ячейки.
* `sheet->GetPrintableSize();` - возвращает размер печатаемой области.
* `sheet->PrintTexts(output);` - выводит текстовое значение ячеек в поток вывода `output`.
* `sheet->PrintValues(output);` - выводит числовое значение ячеек в поток вывода `output`.
