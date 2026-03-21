# Lexical Analyzer (Scanner) for Compiler Construction

This project is a modular Lexical Analyzer (Scanner) developed as part of the Compiler Construction course. It identifies tokens from a source language, manages a Symbol Table for efficient identifier tracking, and implements robust lexical error handling. This lexer is the first component of a complete compiler that will be built to completion.

## 🚀 Key Features

- **Tokenization**: Utilizes Flex to convert source text into a stream of categorized tokens (Keywords, Identifiers, Operators, Literals).
- **Modular Architecture**: Organized into distinct components for the Lexer, Symbol Table, and Error Handling to ensure scalability for future Parser integration.
- **Smart Symbol Table**: Implements a "lookup-before-insert" strategy to maintain unique entries per scope, preventing redundant memory usage.
- **Advanced Error Recovery**: Detects and reports complex lexical errors, such as identifiers beginning with digits (e.g., `9variable`), without halting the scanning process.
- **Cross-Platform Build System**: A unified Makefile compatible with both Linux (WSL/Ubuntu) and Windows (MinGW/MSYS2).

## 📁 Project Structure

```
.
├── Makefile                # Cross-platform build script
├── scanner                 # Executable scanner (generated after build)
├── test_code.txt           # Sample source code for testing
└── src/
    ├── lexer/              # Flex specification and token definitions
    │   ├── lexer.l
    │   └── tokens.h
    ├── symbol_table/       # Symbol Table logic and data structures
    │   ├── symbol_table.c
    │   └── symbol_table.h
    └── error_handler/      # Custom error and warning reporting
        ├── error_handler.c
        └── error_handler.h
```

## 🛠️ Installation & Build

### Prerequisites

- GCC (MinGW-w64 for Windows or build-essential for Linux)
- Flex (Lexical Analyzer Generator)
- Make (GnuWin32 or mingw32-make)

### Prerequisites Installation

#### Windows
1. **MinGW-w64**: Download and install from [mingw-w64.org](https://www.mingw-w64.org/). This provides GCC and Make. Ensure the `bin` directory is added to your PATH.
2. Update MinGW packages: Open the MinGW terminal and run `pacman -Syu` to ensure packages are up to date.
3. **Flex**: Install via MinGW/MSYS2 package manager using `pacman -S mingw-w64-x86_64-flex`, or download the latest release from [GitHub Flex releases](https://github.com/westes/flex/releases). Extract and add the `bin` directory to your PATH.

#### Linux / WSL
Run the following commands to install the required packages:
```bash
sudo apt update
sudo apt install build-essential flex
```

### Building the Project

Navigate to the project root and run:

```bash
# On Linux / WSL
make

# On Windows (PowerShell/CMD)
mingw32-make
```
If for some reason you are running both linix and windows on the same folder always use ```bash make clean``` or ```bash mingw32-make clean``` to delete the object files and trigger a fresh build
## Running the Scanner

To scan a source file (e.g., `test_code.txt`), use:

```bash
# On Windows
./scanner.exe test_code.txt

# On Linux / WSL
./scanner test_code.txt
```

## 📊 Example Output

When running the scanner against a standard function definition, the system produces a categorized token stream followed by the final state of the Symbol Table:

**Token Stream:**

| TOKEN NAME | ID | LEXEME | LINE |
|------------|----|--------|------|
| T_DEF      | 101| def    | 1    |
| T_ID       | 301| add    | 1    |
| [ LEXICAL ERROR ] | | Invalid identifier '9i' | 2 |

**Symbol Table:**

| IDENTIFIER | CLASS | SCOPE | INITIALIZED | LINE |
|------------|-------|-------|-------------|------|
| add        | 301   | 0     | No          | 1    |
| result     | 301   | 0     | No          | 2    |

## Future Development

This lexer is the foundation for a complete compiler. Future components will include a parser, semantic analyzer, code generator, and optimizer to fully compile the source language.