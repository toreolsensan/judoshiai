SHIAI_VER_NUM=2.2-alpha12
SHIAI_VERSION="\"$(SHIAI_VER_NUM)\""

OBJS=

### Target executable type ###
ifeq ($(TARGETOS),WIN32)
TGT=WIN32
else 
ifeq ($(OS),Windows_NT)
TGT=WIN32
else
TGT=LINUX
endif
endif

####################################
# Default directories and programs #
####################################

### Cross compilation (host=Linux, target=Windows) ###
ifeq ($(TARGETOS),WIN32)
CC=i586-mingw32msvc-gcc
LD=i586-mingw32msvc-gcc
SUFF=.exe
DEVELDIR=/opt/win32/gtk
SOUNDDIR=/opt/win32/fmod
RESHACKER=wine "C:\\bin\\ResHacker.exe"
WINDRES=wine "C:\\MinGW\\bin\\windres.exe"
INNOSETUP=wine "C:\\Program Files\\Inno Setup 5\\ISCC.exe"

### Windows ###
else 
ifeq ($(OS),Windows_NT)
CC=gcc
LD=gcc
SUFF=.exe
DEVELDIR=c:/msys/1.0/home/hjokinen/gtk+-bundle_2.20
SOUNDDIR=c:/Program\ Files/FMOD\ SoundSystem/FMOD\ Programmers\ API\ Win32/api
RESHACKER=ResHacker.exe
WINDRES=windres.exe
INNOSETUP="C:\\Program Files\\Inno Setup 5\\ISCC.exe"

### Linux ###
else
CC=gcc
LD=gcc
SUFF=
DEVELDIR=
SOUNDDIR=/opt/fmodapi43004linux/api
RESHACKER=
WINDRES=
INNOSETUP=

endif
endif

###################
# CFLAGS and LIBS #
###################

### Cross compilation ###
ifeq ($(TARGETOS),WIN32)
CFLAGS=-Wall -g \
-I/opt/win32/gtk/include/gtk-2.0 \
-I/opt/win32/gtk/lib/gtk-2.0/include \
-I/opt/win32/gtk/include/atk-1.0 \
-I/opt/win32/gtk/include/cairo \
-I/opt/win32/gtk/include/pango-1.0 \
-I/opt/win32/gtk/include/glib-2.0 \
-I/opt/win32/gtk/lib/glib-2.0/include \
-I/opt/win32/gtk/include/freetype2 \
-I/opt/win32/gtk/include \
-I/opt/win32/gtk/include/libpng14   \
-I/opt/win32/gtk/include/cairo \
-I/opt/win32/gtk/include/freetype2 \
-I/opt/win32/gtk/include \
-I/opt/win32/gtk/include/libpng14   \
-I/opt/win32/w32api/include \
-I../common -mms-bitfields \
-DSHIAI_VERSION=$(SHIAI_VERSION) \
-DENABLE_NLS

LIBS=-L/opt/win32/gtk/lib \
-lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgio-2.0 -lgdk_pixbuf-2.0 \
-lpangowin32-1.0 -lgdi32 -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 \
-lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl -lcairo \
-L/opt/win32/w32api/lib -lws2_32 -mwindows

### Native compilations ###
else
PKGCONFIG=pkg-config

CFLAGS=-Wall $(shell $(PKGCONFIGPATH) $(PKGCONFIG) --cflags gtk+-2.0) \
       $(shell $(PKGCONFIGPATH) $(PKGCONFIG) --cflags cairo) \
       -DENABLE_BINRELOC -DENABLE_NLS \
       -I$(DEVELDIR)/include -I../common -DSHIAI_VERSION=$(SHIAI_VERSION)

LIBS=$(shell $(PKGCONFIGPATH) $(PKGCONFIG) --libs gtk+-2.0 gthread-2.0 cairo)

## Windows ##
ifeq ($(OS),Windows_NT)
CFLAGS += -mms-bitfields
LIBS += -lws2_32  -mwindows

## Linux ##
else
OBJS += binreloc.o

endif

endif
