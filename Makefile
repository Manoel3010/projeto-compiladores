# Ferramentas. Podem ser sobrescritas na linha de comando, por exemplo:
#   make FLEX=win_flex BISON=win_bison
FLEX  ?= flex
BISON ?= bison
CXX   ?= g++

CXXFLAGS ?= -Wall -Wno-unused-function

# Sufixo do executavel: .exe no Windows, vazio no Linux.
# O cmd.exe define OS=Windows_NT, mas o shell de login do MSYS2 nao repassa
# essa variavel, entao o uname e consultado como segunda tentativa.
ifeq ($(OS),Windows_NT)
EXE = .exe
else
UNAME_S := $(shell uname -s)
ifneq (,$(findstring MINGW,$(UNAME_S))$(findstring MSYS,$(UNAME_S))$(findstring CYGWIN,$(UNAME_S)))
EXE = .exe
else
EXE =
endif
endif

SRC   = projeto/src
BUILD = build

ALVO     = linguagem$(EXE)
INCLUDES = -I$(BUILD) -I$(SRC)

OBJS = $(BUILD)/parser.tab.o $(BUILD)/lex.yy.o $(BUILD)/main.o \
       $(BUILD)/value.o $(BUILD)/ast.o $(BUILD)/symtab.o $(BUILD)/interp.o

.PHONY: all clean

all: $(ALVO)

$(ALVO): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# O bison gera parser.tab.c, parser.tab.h e o relatorio parser.output em build/.
$(BUILD)/parser.tab.c $(BUILD)/parser.tab.h: $(SRC)/parser.y | $(BUILD)
	$(BISON) -d -v -o $(BUILD)/parser.tab.c $(SRC)/parser.y

$(BUILD)/lex.yy.c: $(SRC)/lexer.l $(BUILD)/parser.tab.h | $(BUILD)
	$(FLEX) -o $(BUILD)/lex.yy.c $(SRC)/lexer.l

$(BUILD)/parser.tab.o: $(BUILD)/parser.tab.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $(BUILD)/parser.tab.c

$(BUILD)/lex.yy.o: $(BUILD)/lex.yy.c $(BUILD)/parser.tab.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $(BUILD)/lex.yy.c

$(BUILD)/main.o: $(SRC)/main.cpp $(BUILD)/parser.tab.h $(SRC)/ast.h $(SRC)/interp.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $(SRC)/main.cpp

$(BUILD)/value.o: $(SRC)/value.cpp $(SRC)/value.h | $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC)/value.cpp

$(BUILD)/ast.o: $(SRC)/ast.cpp $(SRC)/ast.h $(SRC)/value.h | $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC)/ast.cpp

$(BUILD)/symtab.o: $(SRC)/symtab.cpp $(SRC)/symtab.h $(SRC)/value.h | $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC)/symtab.cpp

$(BUILD)/interp.o: $(SRC)/interp.cpp $(SRC)/interp.h $(SRC)/ast.h $(SRC)/symtab.h | $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $(SRC)/interp.cpp

$(BUILD):
	mkdir $(BUILD)

clean:
	rm -rf $(BUILD) $(ALVO)
