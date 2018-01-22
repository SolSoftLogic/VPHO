CC     = gcc
CP     = g++
AR     = ar
CFLAGS = -c -O3 -fno-strict-aliasing -Wall -D_GNU_SOURCE -DAVC -I../common/include -I../enc/include -I../enc/src -I../dec/include -I../colorconvert/include
VPATH = ../common/src:../colorconvert/src:../dec/src:../enc/src:../../codecs
COMMONFILES = deblock.o dpb.o fmo.o mb_access.o reflist.o
COLORCONVERTFILES =  ccrgb12toyuv420.o ccyuv422toyuv420.o ccrgb16toyuv420.o cczoomrotation12.o \
	ccrgb24torgb16.o cczoomrotation16.o ccrgb24toyuv420.o cczoomrotation24.o \
	ccyuv420semiplnrtoyuv420plnr.o cczoomrotation32.o ccyuv420toyuv420semi.o cczoomrotationbase.o \
	ccyuv420toyuv422.o cpvvideoblend.o
ENCFILES = avcenc_api.o init.o sad.o intra_est.o sad_halfpel.o motion_comp.o motion_est.o \
	bitstream_io.o pvavcencoder.o block.o pvavcencoder_factory.o slice.o \
	findhalfpel.o rate_control.o vlc_encode.o header.o residual.o
DECFILES = avc_bitstream.o pred_intra.o sliced.o avcdec_api.o headerd.o vlc.o itrans.o pred_inter.o residuald.o

all: avc.a codecstest

.c.o:
	$(CC) $(CFLAGS) $<
.cpp.o:
	$(CP) $(CFLAGS) $<

avc.a:	$(COMMONFILES) $(COLORCONVERTFILES) $(ENCFILES) $(DECFILES) avc.o
	$(AR) r avc.a $(COMMONFILES) $(COLORCONVERTFILES) $(ENCFILES) $(DECFILES) avc.o
codecstest: avc.a codecstest.o Makefile
	$(CP) -o codecstest codecstest.o avc.a -lm
 
clean:
	rm -f *.o avc.a codecstest

