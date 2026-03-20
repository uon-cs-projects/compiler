CC = gcc
LEX = flex
CFLAGS = -Wall -g

# 1. Define where the headers are
INCLUDES = -Isrc/lexer -Isrc/symbol_table -Isrc/error_handler

# 2. Define source files with their full paths
SRCS = src/symbol_table/symbol_table.c src/error_handler/error_handler.c
LEX_FILE = src/lexer/lexer.l
GEN_SRC = src/lexer/lex.yy.c

# 3. Object files
OBJS = src/symbol_table/symbol_table.o src/error_handler/error_handler.o src/lexer/lex.yy.o

TARGET = scanner

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Use $(INCLUDES) so GCC finds headers in subfolders
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Lexer specific rules
$(GEN_SRC): $(LEX_FILE)
	$(LEX) -o $(GEN_SRC) $(LEX_FILE)

src/lexer/lex.yy.o: $(GEN_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(GEN_SRC) -o src/lexer/lex.yy.o

clean:
	rm -f $(TARGET) $(OBJS) $(GEN_SRC)