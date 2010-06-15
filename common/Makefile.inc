SHIAI_VER_NUM=2.1-alpha1
SHIAI_VERSION="\"$(SHIAI_VER_NUM)\""

CC=gcc

ifeq ($(OS),Windows_NT)
SUFF=.exe
# PATHS TO YOUR GTK+ INSTALLATION AND SOUND LIBRARY
DEVELDIR=c:/msys/1.0/home/Administrator/gtk
SOUNDDIR=c:\Program\ Files/FMOD\ SoundSystem/FMOD\ Programmers\ API\ Win32
else
SUFF=
DEVELDIR=
SOUNDDIR=/opt/fmodapi43004linux/api
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
