## Parameters

# compiler related
CC       = gcc
CFLAGS   = -std=c99 -g
CXX      = clang++
CXXFLAGS = -std=c++11 -g

MAKE     = make
DOXYGEN  = doxygen

# prefix, src Directory
PREFIX    = ./build

SRC_DIR   = ./src
OBJ_DIR   = ./obj
BIN_DIR   = ./bin
DOC_DIR   = ./doc
# if we modify $SRC_DIR and $DOC_DIR, we should also change Doxyfile setting

EXE       = sudoku_solver
OBJS      = main.o sudoku_solver.o
SRCS      = $(patsubst %.o,%.cpp,$(OBJS))

EXE_PATH  = $(addprefix $(BIN_DIR)/, $(EXE))
OBJS_PATH = $(addprefix $(OBJ_DIR)/, $(OBJS))
SRCS_PATH = $(addprefix $(SRC_DIR)/, $(SRCS))

# platform issue

UNAME = $(shell uname)
ifeq ($(UNAME), FreeBSD)
    MAKE = gmake
endif

## Rules

all: $(EXE_PATH)

clean: 
	rm -rf $(EXE_PATH) $(OBJS_PATH) $(BIN_DIR) $(OBJ_DIR)

install:
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/doc
	-cp    $(BIN_DIR)/*    $(PREFIX)/bin/
	-cp    $(OBJ_DIR)/*.a  $(PREFIX)/lib/
	-cp    $(OBJ_DIR)/*.so $(PREFIX)/lib/
	# -cp -r $(DOC_DIR)/*    $(PREFIX)/doc/

doc: $(DOC_DIR)

$(EXE_PATH): $(OBJS_PATH) | $(BIN_DIR)
	$(CXX) -o $@ $(CXXFLAGS) $^

$(OBJS_PATH): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) -o $@ $(CXXFLAGS) -c $<

# make directory
$(PREFIX) $(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

$(DOC_DIR):
	$(DOXYGEN) Doxyfile

.PHONY: all clean install doc
