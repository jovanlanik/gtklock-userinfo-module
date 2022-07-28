# gtklock-userinfo-module
# Copyright (c) 2022 Jovan Lanik

# Makefile

NAME := userinfo-module.so

LIBS := gtk+-3.0 gmodule-export-2.0 accountsservice
CFLAGS += -std=c11 -fPIC $(shell pkg-config --cflags $(LIBS))
LDLIBS += $(shell pkg-config --libs $(LIBS))

SRC = $(wildcard *.c) 
OBJ = $(SRC:%.c=%.o)

TRASH = $(OBJ) $(NAME)

.PHONY: all clean

all: $(NAME)

clean:
	@rm $(TRASH) | true

$(NAME): $(OBJ)
	$(LINK.c) -shared $(LDFLAGS) $(LDLIBS) $(OBJ) -o $@
