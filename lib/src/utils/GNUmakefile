UTILS_FILE_LIST = assertion.c network.c

UTILS_SRCS = $(UTILS_FILE_LIST:%.c=utils/%.c)
UTILS_OBJS = $(patsubst %.c,%.o,$(UTILS_SRCS))

CTESK_SRCS += $(UTILS_SRCS)
CTESK_OBJS += $(UTILS_OBJS)

#LIB_UTILS = lib/linux/libutils.a
#
#LIBS += $(LIB_UTILS)
#
#$(LIB_UTILS): $(UTILS_OBJS)
#	$(AR) $(LIB_UTILS) $(UTILS_OBJS)
#	ranlib $(LIB_UTILS)
