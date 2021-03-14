CC=gcc
CFLAGS=-I ../include

BUILDDIR=../build
INCLUDEDIR=../include

TARGET = wish

_OBJ = scanner.o
OBJ = $(patsubst %,$(BUILDDIR)/%,$(_OBJ))

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJ) $(BUILDDIR)
	$(CC) -g -o $@ wish.c $(CFLAGS) $(OBJ)

$(BUILDDIR)/%.o: %.c $(BUILDDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILDDIR):
	mkdir $@

scanner.c: scanner.l
	flex -o scanner.c --header-file=$(INCLUDEDIR)/scanner_generated.h scanner.l

.PHONY: clean
clean:
	rm -f scanner.c
	rm -f $(INCLUDEDIR)/scanner_generated.h
	rm -f $(BUILDDIR)/*

.PHONY: rebuild
rebuild: clean all

.PHONY: run
run: $(BUILDDIR)/$(TARGET)
	$(BUILDDIR)/wish