# DSL CodeGen

[![CI](https://github.com/gordon0056/DSLCodeGen/actions/workflows/ci.yml/badge.svg)](https://github.com/gordon0056/DSLCodeGen/actions/workflows/ci.yml)

Read in: [English](#-english) | [Русский](#-русский)

---

## English

CLI tool for transforming Simulink-like XML models into deterministic C code (`nwocg_run.c`).
The tool parses blocks and connections, builds a dependency graph, computes a topological order, and generates a C runtime with `struct`, `init()`, `step()`, and external port descriptors.

### Supported Blocks

* `Inport`
* `Outport`
* `Sum`
* `Gain`
* `UnitDelay`

---

### Dependencies

* **C++17 compiler**: GCC 9+, MSVC 2019+, Clang 10+
* **CMake**: 3.16+
* **pugixml**: v1.14 (fetched automatically via `FetchContent`)

---

### Build

#### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### Windows (MSVC)

```cmd
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

#### Build with tests

Tests are built automatically if `tests/CMakeLists.txt` exists.

---

### Quick Start

Generate C code from XML model:

```bash
./build/Release/dsl-codegen model.xml > nwocg_run.c
```

Compile generated code:

```bash
gcc -Wall -Wextra -c nwocg_run.c -o nwocg_run.o
```

---

### Example Input XML

```xml
<System>
  <Block Name="In1" SID="1" BlockType="Inport"/>
  <Block Name="Sum1" SID="2" BlockType="Sum" Inputs="+-"/>
  <Block Name="Kp" SID="3" BlockType="Gain">
    <P Name="Gain">1.5</P>
  </Block>
  <Block Name="Out1" SID="4" BlockType="Outport"/>

  <Line><P Name="Src">1#out:1</P><P Name="Dst">2#in:1</P></Line>
  <Line><P Name="Src">2#out:1</P><P Name="Dst">3#in:1</P></Line>
  <Line><P Name="Src">3#out:1</P><P Name="Dst">4#in:1</P></Line>
</System>
```

---

### Output Format

Generated C file contains four sections:

1. **Includes**

```
#include "nwocg_run.h"
#include <stddef.h>
#include <math.h>
```

2. **Static struct**

```
static struct {
    double sig1;
    double sig2;
} nwocg;
```

All signals are stored as fields.
`UnitDelay` fields are placed before their first reader.

3. **Init & Step functions**

* `void nwocg_generated_init()` - resets UnitDelay states
* `void nwocg_generated_step()` - executes blocks in topological order + UnitDelay flush pass

4. **External Ports**

```
ext_ports[]
nwocg_generated_ext_ports
nwocg_generated_ext_ports_size
```

---

### Error Handling

| Return Code | Meaning                                                 |
| ----------- | ------------------------------------------------------- |
| `0`         | Success                                                 |
| `1`         | XML parsing, graph, scheduler, or code generation error |

Errors are printed to `stderr`:

```
[ERROR] <Module>: <message>. SID=<N>
```

[⬆ Back to top](#dsl-codegen)

---

## Русский

CLI-утилита для преобразования Simulink-подобных XML-моделей в детерминированный C-код (`nwocg_run.c`).
Инструмент парсит блоки и соединения, строит граф зависимостей, вычисляет топологический порядок и генерирует runtime с `struct`, `init()`, `step()` и описателями внешних портов.

### Поддерживаемые блоки

* `Inport`
* `Outport`
* `Sum`
* `Gain`
* `UnitDelay`

---

### Зависимости

* **Компилятор C++17**: GCC 9+, MSVC 2019+, Clang 10+
* **CMake**: 3.16+
* **pugixml**: v1.14 (подтягивается автоматически через `FetchContent`)

---

### Сборка

#### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

#### Windows (MSVC)

```cmd
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

#### Сборка с тестами

Тесты собираются автоматически при наличии `tests/CMakeLists.txt`.

---

### Быстрый старт

Генерация C-кода из XML:

```bash
./build/Release/dsl-codegen model.xml > nwocg_run.c
```

Компиляция сгенерированного файла:

```bash
gcc -Wall -Wextra -c nwocg_run.c -o nwocg_run.o
```

---

### Пример входного XML

```xml
<System>
  <Block Name="In1" SID="1" BlockType="Inport"/>
  <Block Name="Sum1" SID="2" BlockType="Sum" Inputs="+-"/>
  <Block Name="Kp" SID="3" BlockType="Gain">
    <P Name="Gain">1.5</P>
  </Block>
  <Block Name="Out1" SID="4" BlockType="Outport"/>

  <Line><P Name="Src">1#out:1</P><P Name="Dst">2#in:1</P></Line>
  <Line><P Name="Src">2#out:1</P><P Name="Dst">3#in:1</P></Line>
  <Line><P Name="Src">3#out:1</P><P Name="Dst">4#in:1</P></Line>
</System>
```

---

### Формат выходного файла

Сгенерированный C-файл содержит 4 секции:

1. **Includes**
2. **Static struct** — все сигналы как поля структуры
3. **Функции Init & Step**

* `nwocg_generated_init()` - обнуление UnitDelay
* `nwocg_generated_step()` - вычисления + flush pass

4. **External Ports**

* `ext_ports[]`
* `nwocg_generated_ext_ports`
* `nwocg_generated_ext_ports_size`

---

### Обработка ошибок

| Код возврата | Значение                                               |
| ------------ | ------------------------------------------------------ |
| `0`          | Успех                                                  |
| `1`          | Ошибка парсинга XML, графа, планировщика или генерации |

Ошибки выводятся в `stderr`:

```
[ERROR] <Модуль>: <сообщение>. SID=<N>
```

[⬆ Наверх](#dsl-codegen)
