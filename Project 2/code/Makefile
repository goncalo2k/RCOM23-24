# -*- Makefile -*-
# Makefile to build the project

# Parameters
CC = gcc

CFLAGS = -Wall -g

SRC = src/
INCLUDE = include/
BIN = bin/

# Targets
.PHONY: all
all: $(BIN)/main

$(BIN)/main: main.c $(SRC)/*.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE) -lrt

.PHONY: clean
clean:
	rm -f $(BIN)/main
	rm -f download.*

.PHONY: run1
run1: $(BIN)/main
	./$(BIN)/main "ftp://ftp.up.pt/pub/kodi/timestamp.txt"

.PHONY: run2
run2: $(BIN)/main
	./$(BIN)/main "ftp://ftp.up.pt/pub/kodi/robots.txt"

.PHONY: run3
run3: $(BIN)/main
	./$(BIN)/main "ftp://ftp.up.pt/pub/kodi/sparklefeeds/sparkleupdate_nightly.xml"