# gtklock-userinfo-module
# Copyright (c) 2022 Jovan Lanik

# Makefile

NAME := userinfo-module.so

PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INSTALL = install

LIBS := gtk+-3.0 gmodule-export-2.0 accountsservice
CFLAGS += -std=c11 -fPIC $(shell pkg-config --cflags $(LIBS))
LDLIBS += $(shell pkg-config --libs $(LIBS))

SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

TRASH = $(OBJ) $(NAME)

.PHONY: all clean install uninstall

all: $(NAME)

clean:
	@rm $(TRASH) | true

install:
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)/gtklock
	$(INSTALL) $(NAME) $(DESTDIR)$(LIBDIR)/gtklock/$(NAME)

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/gtklock/$(NAME)

$(NAME): $(OBJ)
	$(LINK.c) -shared $^ $(LDLIBS) -o $@
