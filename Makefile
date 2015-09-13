#
# gHermes Makefile (Linux)
#
CC=gcc
LINK=gcc 
OPTIONS=-g
INCLUDES=`pkg-config --cflags --libs gtk+-2.0`\
         -I. -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include\
         -I/usr/include/gdk-pixbuf-2.0\
         -I/usr/include/atk-1.0 -I/usr/include/cairo\
         -I/usr/include/pango-1.0 -I/usr/include/glib-2.0\
         -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1\
         -I/usr/include/freetype2 -I/usr/include/libpng12 

LIBS=-L. -lDttSP -lpthread -lusb-1.0\
     -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0\
     -lm -lpangocairo-1.0 -lgio-2.0 -lcairo -lpango-1.0 -lfreetype -lz\
     -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lfftw3f

COMPILE=$(CC) $(INCLUDES)

PROGRAM=bin/gHermes

SOURCES= agc.c audio.c band.c command.c\
         display.c filter.c frequency.c main.c meter.c\
         meter_update.c mode.c ozy_buffers.c ozy.c ozy_ringbuffer.c\
         preamp.c property.c receiver.c\
         setup.c display_setup.c hpsdr_setup.c xvtr_setup.c alex_setup.c\
         spectrum_buffers.c spectrum.c spectrum_update.c util.c vfo.c volume.c\
         transmit.c\
         audiostream.c\
         subrx.c\
         metis.c\
         twotone.c keyer.c keyer_setup.c\
         alex_rx_test.c alex_tx_test.c test.c expression.c hamlibserv.c\
         mic_meter.c zoom.c

HEADERS= agc.h alex_rx_test.h alex_setup.h alex_tx_test.h audio.h audiostream.h band.h\
         bandstack.h command.h twotone.h keyer.h keyer_setup.h display.h display_setup.h\
         dttsp.h filter.h frequency.h hpsdr_setup.h main.h meter.h meter_update.h\
         metis.h mode.h ozy_buffers.h ozy.h ozy_ringbuffer.h preamp.h property.h receiver.h\
         screensize.h setup.h spectrum_buffers.h spectrum.h\
         spectrum_update.h subrx.h test.h transmit.h util.h version.h vfo.h volume.h\
         xvtr.h xvtr_setup.h expression.h hamlibserv.h\
         mic_meter.h zoom.h

OBJS= agc.o audio.o band.o command.o\
      display.o filter.o frequency.o main.o meter.o\
      meter_update.o mode.o ozy_buffers.o ozy.o ozy_ringbuffer.o\
      preamp.o property.o receiver.o\
      setup.o display_setup.o hpsdr_setup.o xvtr_setup.o alex_setup.o\
      spectrum_buffers.o spectrum.o spectrum_update.o util.o vfo.o volume.o\
      transmit.o\
      audiostream.o\
      subrx.o\
      metis.o\
      twotone.o keyer.o keyer_setup.o\
      alex_rx_test.o alex_tx_test.o test.o expression.o hamlibserv.o\
      mic_meter.o zoom.o


all: $(PROGRAM) $(HEADERS) $(SOURCES)

bin/gHermes: $(OBJS)
	$(LINK) -o bin/gHermes $(OBJS) $(DTTSP) $(FFTW) $(LIBS)

.c.o:
	$(COMPILE) $(OPTIONS) -c -o $@ $<


clean:
	-rm -f *.o
	-rm -f bin/gHermes


