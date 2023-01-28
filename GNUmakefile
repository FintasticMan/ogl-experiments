.PHONY: all clean run debug

PROJECT := ai-learns-to-drive
VERSION := 0.1.0

CC := clang
DEBUGGER := lldb
MKDIR := mkdir -p
SRCDIR := src
LIBDIR := lib
BUILDDIR := build
INCDIRS := inc
SRCS := $(shell find $(SRCDIR) $(LIBDIR) -type f -name '*.c')
OBJS := $(SRCS:%=$(BUILDDIR)/%.o)
DEPS := $(SRCS:%=$(BUILDDIR)/%.d)
BIN := $(BUILDDIR)/$(PROJECT)-$(VERSION)

CPPFLAGS := $(foreach INCDIR,$(INCDIRS),-I $(INCDIR)) $(CPPFLAGS)
CFLAGS := -pipe $(CFLAGS)
LDFLAGS := -fuse-ld=mold -lm $(LDFLAGS)

LIBS := glfw3

ifneq ($(strip $(LIBS)),)
ifeq ($(strip $(STATIC)),1)
CPPFLAGS += $(shell pkg-config --static --cflags-only-I $(LIBS))
CFLAGS += $(shell pkg-config --static --cflags-only-other $(LIBS))
LDFLAGS += $(shell pkg-config --static --libs $(LIBS))
else
CPPFLAGS += $(shell pkg-config --cflags-only-I $(LIBS))
CFLAGS += $(shell pkg-config --cflags-only-other $(LIBS))
LDFLAGS += $(shell pkg-config --libs $(LIBS))
endif
endif

OWNFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -Werror
DEBUGFLAGS ?= -g -glldb
SANFLAGS ?= -fsanitize=undefined,address
OPTIFLAGS ?= -O2
LTOFLAGS ?= -flto
STATICFLAGS ?= -static

ifeq ($(strip $(DEBUG)),1)
CFLAGS += $(DEBUGFLAGS)
endif
ifeq ($(strip $(SANITIZE)),1)
CFLAGS += $(SANFLAGS)
endif
ifeq ($(strip $(OPTI)),1)
CFLAGS += $(OPTIFLAGS)
endif
ifeq ($(strip $(LTO)),1)
CFLAGS += $(LTOFLAGS)
endif
ifeq ($(strip $(STATIC)),1)
LDFLAGS += $(STATICFLAGS)
endif

all: $(BIN)

$(BUILDDIR)/%.c.d: %.c
	@$(MKDIR) $(@D)
	@$(CC) $(CPPFLAGS) -MM $< | sed 's,$(*F)\.o[: ]*,$(BUILDDIR)/$<.o $(BUILDDIR)/$<.d: ,g' > $@

include $(DEPS)

$(BUILDDIR)/$(SRCDIR)/%.c.o: $(SRCDIR)/%.c
	@$(MKDIR) $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OWNFLAGS) -c $< -o $@

$(BUILDDIR)/$(LIBDIR)/%.c.o: $(LIBDIR)/%.c
	@$(MKDIR) $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS)
	@$(MKDIR) $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OWNFLAGS) $(LDFLAGS) $^ -o $@

clean:
	$(RM) -r $(BUILDDIR)

run: $(BIN)
	./$(BIN) $(ARGS)

debug: $(BIN)
	@$(DEBUGGER) ./$(BIN)
