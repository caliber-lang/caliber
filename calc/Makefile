CC = cc
CFLAGS = -Wall -Wextra -std=c11 -O2
CPPFLAGS = -Isrc

TARGET = calc

SOURCES = \
	src/ast.c \
	src/caliber--.c \
	src/caliber--parse.c \
	src/codegen.c \
	src/lexer.c \
	src/main.c \
	src/parser.c \
	src/typetab.c \
	src/utils.c \
	src/x86_64.c

OBJECTS = \
	src/ast.o \
	src/caliber--.o \
	src/caliber--parse.o \
	src/codegen.o \
	src/lexer.o \
	src/main.o \
	src/parser.o \
	src/typetab.o \
	src/utils.o \
	src/x86_64.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

src/ast.o: src/ast.c src/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/ast.c -o src/ast.o

src/caliber--.o: src/caliber--.c src/caliber--.h src/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/caliber--.c -o src/caliber--.o

src/caliber--parse.o: src/caliber--parse.c src/caliber--parse.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/caliber--parse.c -o src/caliber--parse.o

src/codegen.o: src/codegen.c src/codegen.h src/ast.h src/typetab.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/codegen.c -o src/codegen.o

src/lexer.o: src/lexer.c src/lexer.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/lexer.c -o src/lexer.o

src/main.o: src/main.c src/parser.h src/caliber--.h src/caliber--parse.h src/x86_64.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/main.c -o src/main.o

src/parser.o: src/parser.c src/parser.h src/lexer.h src/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/parser.c -o src/parser.o

src/typetab.o: src/typetab.c src/typetab.h src/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/typetab.c -o src/typetab.o

src/utils.o: src/utils.c src/utils.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/utils.c -o src/utils.o

src/x86_64.o: src/x86_64.c src/x86_64.h src/caliber--parse.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/x86_64.c -o src/x86_64.o

clean:
	rm -f $(OBJECTS) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
