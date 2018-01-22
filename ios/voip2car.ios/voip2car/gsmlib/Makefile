CC     = gcc
AR     = ar
CFLAGS = -c -O3 -Wall -D_GNU_SOURCE -DSASR -DWAV49

all: gsmlib.a

.c.o:
	$(CC) $(CFLAGS) $<

gsmlib.a:	add.o code.o decode.o gsm_create.o gsm_decode.o gsm_destroy.o \
			gsm_encode.o gsm_option.o gsm_print.o long_term.o lpc.o \
			preprocess.o rpe.o short_term.o table.o \
			config.h gsm.h private.h proto.h unproto.h
		$(AR) r gsmlib.a add.o code.o decode.o gsm_create.o gsm_decode.o gsm_destroy.o \
			gsm_encode.o gsm_option.o gsm_print.o long_term.o lpc.o \
			preprocess.o rpe.o short_term.o table.o
 
clean:
	rm -f *.o gsmlib.a

