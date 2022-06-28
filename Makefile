.PHONY: clean

CC := gcc

ifeq ($(OS),Windows_NT)
    BIN := clox-basic
else
    BIN := clox-basic
endif

CFLAGS += -Wall -Wextra

DEBUG:=0
# Release mode and flags
ifeq ($(DEBUG),1)
	CFLAGS += -g3 -fsanitize=address,undefined
else
	CFLAGS += -O3
endif

# Third Party Libraries
# The '/usr/local/*' is present to 
# ensure compatibiltiy across distributions
LDFLAGS += -L /usr/local/lib -lreadline
CFLAGS += -I /usr/local/include

# Static or dynamic linking
STATICBIN=0
ifeq ($(STATICBIN),1)
	LDFLAGS += -static
endif

OBJ = \
	src/clox.o \
	src/token.o \
	src/scanner.o \
	src/utility.o

# Track header file dependency changes
DEP = $(OBJ:.o=.d)
-include $(DEP)

all: $(BIN)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MD -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f $(BIN) $(DEP) $(OBJ)
