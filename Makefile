# --- OS Detection Logic ---
ifeq ($(OS),Windows_NT)
    SHELL := cmd.exe
    RM = del /Q /F
    EXE = .exe
    CLEAN_CMD = $(RM) scanner$(EXE) *.o src\lexer\lex.yy.c src\lexer\*.o src\symbol_table\*.o src\error_handler\*.o src\parser\*.o
else
    RM = rm -f
    EXE =
    CLEAN_CMD = $(RM) $(TARGET) $(OBJS) $(GEN_SRC)
endif

# --- Compiler Configuration ---
CC = gcc
LEX = flex
CFLAGS = -Wall -g
INCLUDES = -Isrc/lexer -Isrc/symbol_table -Isrc/error_handler -Isrc/parser

# --- File Paths ---
SRCS = src/symbol_table/symbol_table.c \
       src/error_handler/error_handler.c \
       src/parser/grammar.c \
       src/parser/first_follow.c \
       src/parser/parse_table.c \
       src/parser/token_stream.c \
       src/parser/parse_tree.c \
       src/parser/parser.c
LEX_FILE = src/lexer/lexer.l
GEN_SRC = src/lexer/lex.yy.c
OBJS = src/symbol_table/symbol_table.o \
       src/error_handler/error_handler.o \
       src/lexer/lex.yy.o \
       src/parser/grammar.o \
       src/parser/first_follow.o \
       src/parser/parse_table.o \
       src/parser/token_stream.o \
       src/parser/parse_tree.o \
       src/parser/parser.o
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