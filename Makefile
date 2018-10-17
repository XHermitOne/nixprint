# Makefile for NixPrint project
#

# Basic stuff
SHELL = /bin/sh

top_srcdir = .
srcdir = .
prefix = /usr
exec_prefix = ${prefix}
bindir = $(exec_prefix)/bin
infodir = $(prefix)/info
libdir = $(prefix)/lib
mandir = $(prefix)/man/man1
includedir = $(prefix)/include

#CC = gcc
CC = g++
DEFS = -DHAVE_CONFIG_H
#CFLAGS = -g -O2 -Wall
CFLAGS =
#Флаг -g необходим для отладки!!!
#CPPFLAGS = -g
CPPFLAGS = 
LDFLAGS = 
LIBS = -lm 
BASELIBS = -lm 
THREAD_LIBS = -lpthread
X11_INC = 
X11_LIB = 
WX_CPPFLAGS=$(shell wx-config --cppflags)
WX_LIBS=$(shell wx-config --libs)
CAIRO_LIBS = $(shell pkg-config --cflags --libs cairo)

# Directories

TOPSRCDIR = .
TOPOBJDIR = .
SRCDIR    = .
INCLUDEWXDIR = $(includedir)/wx-2.8
CAIRO_INCLUDEDIR = $(includedir)/cairo

MODULE    = none

#CPPFLAGS += $(WX_CPPFLAGS)
#CPPFLAGS += -I$(INCLUDEWXDIR)
CPPFLAGS += -I$(CAIRO_INCLUDEDIR)
LDFLAGS += $(CAIRO_LIBS)

nixprint: main.o version.o tools.o config.o blocks.o lines.o epson.o texts.o parser.o printer.o log.o convert.o paper.o lprint.o
	$(CC) -o nixprint ./obj/main.o ./obj/version.o ./obj/tools.o ./obj/config.o ./obj/blocks.o ./obj/lines.o ./obj/epson.o ./obj/texts.o ./obj/parser.o ./obj/printer.o ./obj/log.o ./obj/convert.o ./obj/paper.o ./obj/lprint.o $(LDFLAGS)

test: test.o tools.o config.o blocks.o lines.o epson.o texts.o parser.o printer.o log.o convert.o paper.o
	$(CC) -o test ./obj/test.o ./obj/tools.o ./obj/config.o ./obj/blocks.o ./obj/lines.o ./obj/epson.o ./obj/texts.o ./obj/parser.o ./obj/printer.o ./obj/log.o ./obj/convert.o ./obj/paper.o $(LDFLAGS)

main.o: ./src/main.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/main.c
	mv main.o ./obj/main.o

test.o: ./src/test.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/test.c
	mv test.o ./obj/test.o

debug.o: ./src/debug.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/debug.c
	mv debug.o ./obj/debug.o

version.o: ./src/version.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/version.c
	mv version.o ./obj/version.o

config.o: ./src/config.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/config.c
	mv config.o ./obj/config.o

blocks.o: ./src/blocks.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/blocks.c
	mv blocks.o ./obj/blocks.o

lines.o: ./src/lines.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/lines.c
	mv lines.o ./obj/lines.o

log.o: ./src/log.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/log.c
	mv log.o ./obj/log.o

loadfont.o: ./src/loadfont.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/loadfont.c
	mv loadfont.o ./obj/loadfont.o

tools.o: ./src/tools.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/tools.c
	mv tools.o ./obj/tools.o

lprint.o: ./src/lprint.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/lprint.c
	mv lprint.o ./obj/lprint.o

convert.o: ./src/convert.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/convert.c
	mv convert.o ./obj/convert.o

printer.o: ./src/printer.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/printer.c
	mv printer.o ./obj/printer.o

preview.o: ./src/preview.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/preview.c
	mv preview.o ./obj/preview.o

epson.o: ./src/epson.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/epson.c
	mv epson.o ./obj/epson.o

sam_prop.o: ./src/sam_prop.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/sam_prop.c
	mv sam_prop.o ./obj/sam_prop.o

texts.o: ./src/texts.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/texts.c
	mv texts.o ./obj/texts.o

winapi.o: ./src/winapi.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/winapi.c
	mv winapi.o ./obj/winapi.o

parser.o: ./src/parser.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/parser.c
	mv parser.o ./obj/parser.o

paper.o: ./src/paper.c
	$(CC) -c  $(CFLAGS) $(CPPFLAGS) ./src/paper.c
	mv paper.o ./obj/paper.o

clean:
	rm -f ./src/*.o ./obj/*.o ./*.o nixprint test

