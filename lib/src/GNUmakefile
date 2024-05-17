MODULES = utils c_tracer atl ts

include $(MODULES:%=%/GNUmakefile)

DEBUG 		= -g -ggdb -O0 -fno-inline -D_GLIBCXX_DEBUG
RELEASE 	= -O
INCLUDE		= -I.
CC		= gcc
CXX 	= g++
LD		= g++
AR		= ar r

CFLAGS 		= $(INCLUDE) $(DEBUG) $(RELEASE) -DATL_CLONE_DISABLE -DUSE_FOPEN64
CFLAGS 		= $(INCLUDE) $(DEBUG) $(RELEASE) -DATL_CLONE_DISABLE -DUSE_FOPEN64
LDFLAGS		= $(DEBUG)

LIBCTESK	= libctesk.a

RM		= rm -rf

C_DEPENDS 	= $(patsubst %.c,%.d,$(filter %.c,$(CTESK_SRCS)))

all: $(LIBCTESK)

$(LIBCTESK): $(CTESK_OBJS)
	$(AR) $@ $(CTESK_OBJS)
	ranlib $@

-include $(C_DEPENDS)

$(C_DEPENDS):%.d:
	@set -e; $(CC) -MM $(CFLAGS) $*.c > $@
	@echo "$*.d $(*D)/`cat $@`" > $@

clean:
	$(RM) $(CTESK_OBJS) $(C_DEPENDS)
	$(RM) $(LIBCTESK)

.SUFFIXES: .o .c 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@
