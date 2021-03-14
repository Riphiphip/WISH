CC=gcc
CFLAGS=-I ../include

BUILDDIR=../build
INCLUDEDIR=../include

TARGET = wish

_OBJ = scanner.o
OBJ = $(patsubst %,$(BUILDDIR)/%,$(_OBJ))

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJ) $(BUILDDIR)
	$(CC) -o $@ wish.c $(CFLAGS) $(OBJ)

$(BUILDDIR)/%.o: %.c $(BUILDDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILDDIR):
	mkdir $@

scanner.c: scanner.l
	flex -o scanner.c --header-file=$(INCLUDEDIR)/scanner.h scanner.l

clean:
	rm -f scanner.c
	rm -f $(INCLUDEDIR)/scanner.h
	rm -f $(BUILDDIR)/*
