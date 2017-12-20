SHELL ?= /bin/bash


LIBNAME = libmlbuf
LIB_VER_MAJOR = 0
LIB_VER_MINOR = 0
LIB_VER_PATCH = 1
LIB_VER = $(LIB_VER_MAJOR).$(LIB_VER_MINOR).$(LIB_VER_PATCH)


CFLAGS ?= -g
CCLD   ?= $(CC)

LIBPCRE_CFLAGS  ?= $(shell pkg-config libpcre --cflags)
LIBPCRE_LDFLAGS ?= $(shell pkg-config libpcre --libs-only-L --libs-only-other)
LIBPCRE_LDLIBS  ?= $(shell pkg-config libpcre --libs-only-l)

CFLAGS  += -D_GNU_SOURCE -Wall -fPIC $(LIBPCRE_CFLAGS)
LDFLAGS += -shared $(LIBPCRE_LDFLAGS)
LDLIBS  += $(LIBPCRE_LDLIBS)


SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)


all: $(LIBNAME).so $(LIBNAME).a

$(LIBNAME).so: $(OBJS)
	$(CCLD) $(LDFLAGS) -Wl,-soname,$(LIBNAME).so.$(LIB_VER_MAJOR) -o $(LIBNAME).so.$(LIB_VER) $(OBJS) $(LDLIBS)
	ln -s $(LIBNAME).so.$(LIB_VER) $(LIBNAME).so.$(LIB_VER_MAJOR).$(LIB_VER_MINOR)
	ln -s $(LIBNAME).so.$(LIB_VER) $(LIBNAME).so.$(LIB_VER_MAJOR)
	ln -s $(LIBNAME).so.$(LIB_VER) $(LIBNAME).so

$(LIBNAME).a: $(OBJS)
	$(AR) rcs $(LIBNAME).a $(OBJS)

$(OBJS): %.o: %.c

test: $(LIBNAME).so
	$(MAKE) -C tests

clean:
	$(RM) -f *.o $(LIBNAME).a $(LIBNAME).so*
	$(MAKE) -C tests clean

.PHONY: all test clean
