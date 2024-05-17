TS_C_FILE_LIST = \
		convex.c  dfsm.c dserialization.c  engine.c\
		generated.c ndfsm.c register.c serialization.c\
		system.c timemark.c ts.c coverage.c cpp_iface.c

TS_SRCS = $(TS_C_FILE_LIST:%.c=ts/%.c) 

TS_OBJS = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(TS_SRCS)))

CTESK_SRCS += $(TS_SRCS)
CTESK_OBJS += $(TS_OBJS)

#LIB_TS = lib/linux/libts.a

#LIBS += $(LIB_TS)

#$(LIB_TS): $(TS_OBJS)
#	$(AR) $(LIB_TS) $(TS_OBJS)
#	ranlib $(LIB_TS)
