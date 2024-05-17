CC = gcc -m32
CPP = cpp
SEC = "$(CTESK_HOME)/bin/modelverify.sh"
SECPP = "$(CTESK_HOME)/bin/sec-pp.sh"
INCLUDE = $(XINCLUDE)
LIBTYPE = gcc
CTESK_LIBPATH = $(CTESK_HOME)/lib/$(LIBTYPE)
LIB = $(XLIB) "-L$(CTESK_LIBPATH)" -lctesk -lpthread
CFLAGS = $(XCFLAGS) -g -std=c99 $(INCLUDE)
LDFLAGS = $(XLDFLAGS) -g -std=c99 $(LIB)

.SECONDARY: $(sec_sources:%.sec=~%.c) $(sec_sources:%.sec=~%.sei)


%.c : %.sei
	$(SEC) $< $@

~%.sei : %.sec
	$(SECPP) $< -o $@ $(CFLAGS)

%.d : %.c
	$(CC) $(CFLAGS) -MM $< > $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

~%.sed : %.sec
	$(SECPP) $(CFLAGS) -E -MM $< | sed -e "s/$(<:.sec=.o):/$(<:.sec=.o), $(<:.sec=.sei):/" > $@

.PHONY: all clean

all: $(example)

$(example): $(sec_sources:%.sec=~%.o) $(c_sources:%.c=%.o)
	$(CC) $^ -o $@ $(LDFLAGS)

-include $(sec_sources:%.sec=~%.sed) $(c_sources:%.c=%.d)

clean:
	rm -f $(example) \
	$(sec_sources:%.sec=~%.o) $(sec_sources:%.sec=~%.c) $(sec_sources:%.sec=~%.sed) $(sec_sources:%.sec=~%.sei) \
	$(c_sources:%.c=%.o) $(c_sources:%.c=%.d)
