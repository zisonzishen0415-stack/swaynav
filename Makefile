CC = gcc
CFLAGS = -Wall -Wextra -O2 $(shell pkg-config --cflags gtk+-3.0 gtk-layer-shell-0 json-c)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0 gtk-layer-shell-0 json-c) -lm

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SRCS = $(SRCDIR)/main.c \
       $(SRCDIR)/overlay.c \
       $(SRCDIR)/grid.c \
       $(SRCDIR)/keybind.c \
       $(SRCDIR)/commands.c \
       $(SRCDIR)/mouse.c \
       $(SRCDIR)/swayipc.c \
       $(SRCDIR)/config.c \
       $(SRCDIR)/history.c \
       $(SRCDIR)/recorder.c \
       $(SRCDIR)/util.c

OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/swaynav

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/swaynav
	mkdir -p /usr/local/share/swaynav
	install -m 644 data/config.example /usr/local/share/swaynav/config.example

uninstall:
	rm -f /usr/local/bin/swaynav
	rm -f /usr/local/share/swaynav/config.example