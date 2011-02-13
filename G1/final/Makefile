CC=gcc
CFLAGS=-g -std=c99 -pedantic -Wall -Wextra
SOURCES := $(wildcard *.c)
LIBS = $(SOURCES:.c=.o)
TARGET=a.out
DEPDIR=.deps
MAKEDEPEND = gcc -M $(CPPFLAGS) -o $(DEPDIR)/$*.d $<

all: $(DEPDIR) $(TARGET)

run: all
	./$(TARGET)

$(TARGET): $(LIBS)
	$(CC) $(CFLAGS) $(LIBS) -o $@

%.o: %.c
	@$(MAKEDEPEND)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(LIBS) $(DEPDIR) $(TARGET)

$(DEPDIR):
	@mkdir $(DEPDIR)

-include $(SOURCES:%.c=$(DEPDIR)/%.d)
