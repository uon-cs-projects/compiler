# --- OS Detection Logic ---
ifeq ($(OS),Windows_NT)
    # Settings for Windows (MinGW/CMD)
    SHELL := cmd.exe
    RM = del /Q /F
    EXE = .exe
    # We use backslashes for 'del' but forward slashes work for GCC
    CLEAN_CMD = $(RM) scanner$(EXE) *.o src\lexer\lex.yy.c src\lexer\*.o src\symbol_table\*.o src\error_handler\*.o
else
    # Settings for Linux/macOS/WSL
    RM = rm -f
    EXE =
    CLEAN_CMD = $(RM) $(TARGET) $(OBJS) $(GEN_SRC)
endif

# --- Compiler Configuration ---
CC = gcc
LEX = flex
CFLAGS = -Wall -g
INCLUDES = -Isrc/lexer -Isrc/symbol_table -Isrc/error_handler

# --- File Paths ---
SRCS = src/symbol_table/symbol_table.c src/error_handler/error_handler.c
LEX_FILE = src/lexer/lexer.l
GEN_SRC = src/lexer/lex.yy.c
OBJS = src/symbol_table/symbol_table.o src/error_handler/error_handler.o src/lexer/lex.yy.o

TARGET = scanner$(EXE)

# --- Build Rules ---
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Compile C files to Object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Generate Lexer C code
$(GEN_SRC): $(LEX_FILE)
	$(LEX) -o $(GEN_SRC) $(LEX_FILE)

# Compile the generated Lexer
src/lexer/lex.yy.o: $(GEN_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(GEN_SRC) -o src/lexer/lex.yy.o

# Clean rule using the OS-specific command
clean:
	$(CLEAN_CMD)

rebuild: clean all