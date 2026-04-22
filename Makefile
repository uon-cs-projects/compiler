# --- OS Detection Logic ---
ifeq ($(OS),Windows_NT)
    EXE = .exe
else
    EXE =
endif
RM = rm -f
CLEAN_CMD = $(RM) compiler$(EXE) *.o lexer_adapter.o main.o src/lexer/lex.yy.c src/lexer/*.o src/symbol_table/*.o src/error_handler/*.o src/parser/*.o

# --- Compiler Configuration ---
CC      = gcc
LEX     = flex
CFLAGS  = -Wall -Wextra -std=gnu99 -g
INCLUDES = -Isrc -Isrc/lexer -Isrc/symbol_table -Isrc/error_handler -Isrc/parser -I.

# --- File Paths ---
SRCS = main.c \
       lexer_adapter.c \
       src/symbol_table/symbol_table.c \
       src/error_handler/error_handler.c \
       src/parser/grammar.c \
       src/parser/first_follow.c \
       src/parser/parse_table.c \
       src/parser/token_stream.c \
       src/parser/parse_tree.c \
       src/parser/parser.c

LEX_FILE = src/lexer/lexer.l
GEN_SRC = src/lexer/lex.yy.c

OBJS = main.o \
       lexer_adapter.o \
       src/symbol_table/symbol_table.o \
       src/error_handler/error_handler.o \
       src/lexer/lex.yy.o \
       src/parser/grammar.o \
       src/parser/first_follow.o \
       src/parser/parse_table.o \
       src/parser/token_stream.o \
       src/parser/parse_tree.o \
       src/parser/parser.o

TARGET  = compiler$(EXE)

# --- Build Rules ---
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Generate Lexer C code
$(GEN_SRC): $(LEX_FILE)
	$(LEX) -o $(GEN_SRC) $(LEX_FILE)

# Compile the generated Lexer
src/lexer/lex.yy.o: $(GEN_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(GEN_SRC) -o $@

# Compile standard C files to Object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(CLEAN_CMD)

rebuild: clean all

test: $(TARGET)
	./$(TARGET) test_code.txt