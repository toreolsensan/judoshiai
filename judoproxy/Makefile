include ../common/Makefile.inc

VIDEO=no

LIBS += -L/usr/local/lib -lxml2

ifeq ($(VIDEO),yes)
LIBS += -lavutil -lavformat -lavcodec -lz -lavutil -lm \
-lswscale -llzma -lfdk-aac -lmp3lame -lx264 -lavresample
endif

OBJDIR := $(JS_BUILD_DIR)/$(LASTPART)/$(OBJDIR)

all: $(OBJDIR) $(OBJDIR)/judoproxy$(SUFF)

$(OBJDIR):
	mkdir -p $(OBJDIR)

OBJS += $(OBJDIR)/main.o $(OBJDIR)/menus.o $(OBJDIR)/comm.o $(OBJDIR)/ip.o \
	$(OBJDIR)/protocol.o $(OBJDIR)/language.o $(OBJDIR)/html.o

ifeq ($(VIDEO),yes)
OBJS += $(OBJDIR)/camera-video.o
endif

CFLAGS += -DAPPLICATION="\"judoproxy\""
CFLAGS += -DAPP_NUM=6
CFLAGS += -I/usr/include/libxml2

ifeq ($(VIDEO),yes)
CFLAGS += -DVIDEO
endif

### Windows executable ###
ifeq ($(TGT),WIN32)
OBJS += judoproxy-icon.res

judoproxy-icon.res: judoproxy-icon.rc
	$(WINDRES) -O coff judoproxy-icon.rc judoproxy-icon.res

judoproxy-icon.rc:
	echo "APPICONS ICON \"../etc/judoproxy.ico\"" >judoproxy-icon.rc

### Linux executable ###
else
endif

$(OBJDIR)/judoproxy$(SUFF): $(OBJS) Makefile
	$(LD) -g -Wl,-rpath -Wl,bin $(OBJS) -o $(OBJDIR)/judoproxy$(SUFF) $(LIBS)

$(OBJDIR)/avl.o: ../common/avl.c
	${CC} -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/ip.o: ../common/ip.c
	${CC} -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/protocol.o: ../common/protocol.c
	${CC} -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/binreloc.o: ../common/binreloc.c
	${CC} -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/language.o: ../common/language.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	${CC} -g $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/* *.res *.rc *~
