SHELL         ?= /bin/bash
CCLD          ?= $(CC)
LN            ?= ln

CFLAGS        ?= -g
CFLAGS        += -D_GNU_SOURCE -Wall -fPIC

LDFLAGS       ?=
LDFLAGS       += -shared

PCRE_LDLIBS   ?= -lpcre
LDLIBS        ?=
LDLIBS        += $(PCRE_LDLIBS)

libname       := libmlbuf
lib_ver_cur   := 1
lib_ver_rev   := 0
lib_ver_age   := 0
lib_ver       := $(lib_ver_cur).$(lib_ver_rev).$(lib_ver_age)
srcs          := $(wildcard *.c)
objs          := $(srcs:.c=.o)

all: $(libname).so $(libname).a

$(libname).so: $(objs)
	$(CCLD) $(LDFLAGS) -Wl,-soname,$(libname).so.$(lib_ver_cur) -o $(libname).so.$(lib_ver) $(objs) $(LDLIBS)
	$(LN) -s $(libname).so.$(lib_ver) $(libname).so.$(lib_ver_cur).$(lib_ver_rev)
	$(LN) -s $(libname).so.$(lib_ver) $(libname).so.$(lib_ver_cur)
	$(LN) -s $(libname).so.$(lib_ver) $(libname).so

$(libname).a: $(objs)
	$(AR) rcs $(libname).a $(objs)

$(objs): %.o: %.c

test: $(libname).so
	$(MAKE) -C tests

clean:
	$(RM) -f *.o $(libname).a $(libname).so*
	$(MAKE) -C tests clean

.PHONY: all test clean
