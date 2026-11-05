CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2
ROOT := ../..
SOURCES := test_core.c \
           $(ROOT)/src/utils/lvgl_aic_touch_state.c \
           $(ROOT)/src/utils/lvgl_aic_display_state.c
INCLUDES := -I$(ROOT)/include -I$(ROOT)/src/utils

.PHONY: all clean test

all: test_core

test_core: $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $@

test: test_core
	./test_core

clean:
	rm -f test_core
