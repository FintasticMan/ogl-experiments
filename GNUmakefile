.PHONY: all clean run debug

MKDIR := mkdir -p
DEBUGGER ?= lldb
SRCDIR ?= src
OBJDIR ?= obj
DEPDIR ?= dep
INCDIR ?= inc
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(subst $(SRCDIR)/,$(OBJDIR)/,$(SRCS:.c=.o))
DEPS := $(subst $(SRCDIR)/,$(DEPDIR)/,$(SRCS:.c=.d))
BIN ?= main

ifeq ($(CC),cc)
CC := clang
endif

CPPFLAGS := -I$(INCDIR) $(CPPFLAGS)
CFLAGS := -Wall -Wextra -Wpedantic -pipe -std=c17 \
          $(CFLAGS)
LDFLAGS := -fuse-ld=lld -lm $(LDFLAGS)

LIBS := glfw3

ifeq ($(strip $(STATIC)),1)
CPPFLAGS += $(shell pkg-config --static --cflags-only-I $(LIBS))
CFLAGS += $(shell pkg-config --static --cflags-only-other $(LIBS))
LDFLAGS += $(shell pkg-config --static --libs $(LIBS)) -static
else
CPPFLAGS += $(shell pkg-config --cflags-only-I $(LIBS))
CFLAGS += $(shell pkg-config --cflags-only-other $(LIBS))
LDFLAGS += $(shell pkg-config --libs $(LIBS))
endif

DEBUGFLAGS ?= -g -glldb
SANFLAGS ?= -fsanitize=undefined,address
OPTIFLAGS ?= -flto=thin -O2

ifeq ($(strip $(DEBUG)),1)
CFLAGS += $(DEBUGFLAGS)
endif
ifeq ($(strip $(SANITIZE)),1)
CFLAGS += $(SANFLAGS)
endif
ifeq ($(strip $(OPTI)),1)
CFLAGS += $(OPTIFLAGS)
endif

all: $(BIN)

$(DEPDIR)/%.d: $(SRCDIR)/%.c
	@$(MKDIR) "$(DEPDIR)"
	@$(CC) $(CPPFLAGS) -MM "$<" | sed 's,\($*\)\.o[ :]*,$(OBJDIR)/\1.o: ,g' > "$@"

include $(DEPS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(MKDIR) "$(OBJDIR)"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

$(BIN): $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ -o "$@"

clean:
	$(RM) $(OBJDIR)/* $(DEPDIR)/* $(BIN)

run: $(BIN)
	@./$(BIN) $(ARGS)

debug: $(BIN)
	@$(DEBUGGER) ./$(BIN)
