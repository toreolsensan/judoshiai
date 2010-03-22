SHIAI_VER_NUM=2.0-pre2
SHIAI_VERSION="\"$(SHIAI_VER_NUM)\""

ifeq ($(OS),Windows_NT)
SUFF=.exe
# PATH TO YOUR GTK+ INSTALLATION
DEVELDIR=c:/msys/1.0/home/hjokinen/gtk+-bundle_2.16.4-20090708_win32
else
SUFF=
DEVELDIR=
endif

PKGCONFIG=pkg-config

CFLAGS=-Wall $(shell $(PKGCONFIGPATH) $(PKGCONFIG) --cflags gtk+-2.0) \
       $(shell $(PKGCONFIGPATH) $(PKGCONFIG) --cflags cairo) \
       -DENABLE_BINRELOC -DENABLE_NLS \
       -I$(DEVELDIR)/include -I../common -DSHIAI_VERSION=$(SHIAI_VERSION)

LIBS=$(shell $(PKGCONFIGPATH) $(PKGCONFIG) --libs gtk+-2.0 gthread-2.0 cairo)

ifeq ($(OS),Windows_NT)

CFLAGS += -mms-bitfields
LIBS += -lws2_32  -mwindows

else

OBJS += binreloc.o

endif
