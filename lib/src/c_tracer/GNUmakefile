TRACER_FILE_LIST = TraceConsoleWriter.c     TraceFormatter.c \
      TraceWriteController.c   TraceXMLFormatter.c \
      TraceList.c           TraceWriter.c            TSEnvironment.c \
      TraceBufferWriter.c   TraceMultiWriter.c       Tracer.c \
      TraceFileWriter.c     TraceConstrainer.c       c_tracer.c

TRACER_SRCS = $(TRACER_FILE_LIST:%.c=c_tracer/%.c)
TRACER_OBJS = $(patsubst %.c,%.o,$(TRACER_SRCS))

CTESK_SRCS += $(TRACER_SRCS)
CTESK_OBJS += $(TRACER_OBJS)

#LIB_TRACER = lib/linux/libtracer.a

#LIBS += $(LIB_TRACER)

#$(LIB_TRACER): $(TRACER_OBJS)
#	$(AR) $(LIB_TRACER) $(TRACER_OBJS)
#	ranlib $(LIB_TRACER)
