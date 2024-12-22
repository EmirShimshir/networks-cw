CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinc

SRCDIR = src
INCDIR = inc
OBJDIR = obj
BINDIR = bin

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
TARGET = $(BINDIR)/server

all: $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $@

$(OBJDIR):
	mkdir -p $@

clean:
	$(RM) -r $(OBJDIR) $(BINDIR) $(TARGET)

.PHONY: all clean
