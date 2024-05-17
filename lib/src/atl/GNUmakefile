# clone.c excluded from build. use clone_Object() instead of clone()

ATL_C_FILE_LIST = \
           array.c arraylist.c bigint.c char.c complex.c double.c float.c \
           hash.c hashmap.c int2ptrhash.c integer.c intset.c list.c long.c \
           map.c multiset.c set.c short.c string.c  stringbuffer.c object.c \
           unit.c void_ast.c

ATL_SRCS = $(ATL_C_FILE_LIST:%.c=atl/%.c) 

ATL_OBJS = $(patsubst %.cpp,%.o, $(patsubst %.c,%.o,$(ATL_SRCS)))

CTESK_SRCS += $(ATL_SRCS)
CTESK_OBJS += $(ATL_OBJS)
