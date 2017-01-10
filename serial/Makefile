PKGCONFIG=pkg-config

ifndef JS_BUILD_DIR
  JS_BUILD_DIR=/home/$(USER)/js-build
endif
LASTPART := $(shell basename $(CURDIR))

### WIN32 #############################
ifeq ($(TARGETOS),WIN32)

NEWCC := $(shell command -v i686-w64-mingw32-gcc 2> /dev/null)
ifdef NEWCC
  CC=i686-w64-mingw32-gcc
  LD=i686-w64-mingw32-gcc
else
  CC=i586-mingw32msvc-gcc
  LD=i586-mingw32msvc-gcc
endif

SUFF=.exe
ZIP=.zip
DEVELDIR=/opt/win32/gtk
RESHACKER=wine "C:\\bin\\ResHacker.exe"
WINDRES=wine "/opt/win32/MinGW/bin/windres.exe"
INNOSETUP=wine "/opt/win32/Inno Setup 5/ISCC.exe"
OBJDIR=obj-win32
RELEASEDIR=$(JS_BUILD_DIR)/release-win32
STATIC =
#-all-static
#-static-libgcc -Wl,-Bstatic


CFLAGS = $(WARNINGS) -g \
-I$(DEVELDIR)/include/glib-2.0 \
-I$(DEVELDIR)/lib/glib-2.0/include \
-I$(DEVELDIR)/include \
-I/opt/win32/w32api/include \
-I../common -mms-bitfields \
-Wno-deprecated-declarations

#$(DEVELDIR)/lib/glib-2.0.lib 
#-lglib-2.0
#-L$(DEVELDIR)/lib -L$(DEVELDIR)/bin 
#-lgthread-2.0 
#$(DEVELDIR)/lib/libglib-2.0.dll.a \
#$(DEVELDIR)/lib/libgthread-2.0.dll.a \

LIBS= -L$(DEVELDIR)/bin -L$(DEVELDIR)/lib -lglib-2.0 -lgthread-2.0 \
-L/opt/win32/w32api/lib -lws2_32 \
-mwindows

DLLS = $(DEVELDIR)/bin/libglib-2.0-0.dll $(DEVELDIR)/bin/libiconv-2.dll \
       $(DEVELDIR)/bin/libintl-8.dll $(DEVELDIR)/bin/pthreadGC2.dll

### Linux ##############################
else

CC=gcc
LD=gcc
SUFF=
ZIP=
DEVELDIR=
RESHACKER=
WINDRES=
INNOSETUP=
OBJDIR=obj-linux
RELEASEDIR=$(JS_BUILD_DIR)/release-linux

PKGCONFIG=pkg-config

CFLAGS = $(WARNINGS) \
       $(shell $(PKGCONFIGPATH) $(PKGCONFIG) --cflags glib-2.0) \
       -I$(DEVELDIR)/include -I../common

LIBS=$(shell $(PKGCONFIGPATH) $(PKGCONFIG) --libs gthread-2.0 glib-2.0) -ldl -lm

endif
#########################################

EXE = websock-serial$(SUFF)
APP = websock-serial$(ZIP)

OBJS = $(OBJDIR)/main.o $(OBJDIR)/serial.o $(OBJDIR)/comm.o \
       $(OBJDIR)/websock-protocol.o $(OBJDIR)/cJSON.o \
       $(OBJDIR)/websock.o $(OBJDIR)/httpp.o $(OBJDIR)/sha1.o \
       $(OBJDIR)/avl.o

OBJDIR := $(JS_BUILD_DIR)/$(LASTPART)/$(OBJDIR)

CFLAGS += -I../3rdparty/httpp -I../3rdparty/sqlite3 -I../3rdparty/cJSON

../etc/$(APP): $(OBJDIR)/$(EXE)
ifeq ($(ZIP),.zip)
	zip -j ../etc/$(APP) $(OBJDIR)/$(EXE) $(DLLS)
else
	cp $(OBJDIR)/$(EXE) ../etc/$(APP)
endif

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/$(EXE): $(OBJDIR) $(OBJS)
	$(LD) $(STATIC) -g $(OBJS) -o $(OBJDIR)/$(EXE) $(LIBS)
ifeq ($(TARGETOS),WIN32)
	cp $(DLLS) $(OBJDIR)
endif

$(OBJDIR)/%.o: %.c
	${CC} -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/avl.o: ../common/avl.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/websock-protocol.o: ../common/websock-protocol.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/websock.o: ../judoshiai/websock.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/httpp.o: ../3rdparty/httpp/httpp.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/cJSON.o: ../3rdparty/cJSON/cJSON.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/sha1.o: ../common/sha1.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o
