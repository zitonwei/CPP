CC ?= cc
CSTD := -std=c11
WARN := -Wall -Wextra -pedantic
OPT := -O3

CPPFLAGS :=
CFLAGS := $(CSTD) $(WARN) $(OPT)
LDFLAGS :=
LDLIBS :=

SRC := matrix.c matmul.c benchmark.c
OBJ := $(SRC:.c=.o)
BIN := benchmark

USE_OPENMP ?= 0
USE_OPENBLAS ?= 0

ifeq ($(USE_OPENMP),1)
	CFLAGS += -fopenmp
	LDFLAGS += -fopenmp
endif

ifeq ($(USE_OPENBLAS),1)
	CPPFLAGS += -DUSE_OPENBLAS
	LDLIBS += -lopenblas
endif

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $@

%.o: %.c matrix.h matmul.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJ) $(BIN)
