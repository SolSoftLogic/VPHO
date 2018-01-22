CC     = gcc
CP     = g++
AR     = ar
CFLAGS = -c -O3 -fno-strict-aliasing -Wall -Wno-parentheses -D_GNU_SOURCE
OBJFILES = aes.o audiocodecs.o consolevp.o g711.o ielements.o md5c.o \
			netutil.o mixingaudio.o sip.o udpstream.o util.o vpstack.o \
			avifile.o audioutil.o resample.o unix/portability.o
HFILES = aes.h audiocodecs.h gsm.h ielements.h md5.h netutil.h \
			portability.h rtfdictionary.h mixingaudio.h sip.h udpstream.h \
			vpaudio.h vprotocol.h vpstack.h vpstackpriv.h \
			avifile.h audioutil.h unix/uportability.h unix/audio.h
all: consolevposs consolevpalsa

.c.o:
	$(CC) $(CFLAGS) $< -o $@
.cpp.o:
	$(CP) $(CFLAGS) $< -o $@

consolevposs:	$(OBJFILES) unix/audiooss.o $(HFILES)
		$(CP) -o consolevposs $(OBJFILES) unix/audiooss.o ../gsmlib/gsmlib.a -lpthread

consolevpalsa:	$(OBJFILES) unix/audioalsa.o unix/alsa.o $(HFILES)
		$(CP) -o consolevpalsa $(OBJFILES) unix/audioalsa.o unix/alsa.o ../gsmlib/gsmlib.a -lpthread -lasound

clean:
	rm -f *.o consolevposs consolevpalsa unix/*.o

