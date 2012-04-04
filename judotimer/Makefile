include ../common/Makefile.inc

all: $(OBJDIR)/judotimer$(SUFF)

OBJS += $(OBJDIR)/main.o $(OBJDIR)/menus.o $(OBJDIR)/stopwatch.o $(OBJDIR)/comm.o \
	$(OBJDIR)/ip.o $(OBJDIR)/protocol.o $(OBJDIR)/show-ad.o $(OBJDIR)/sound.o \
	 $(OBJDIR)/video.o

CFLAGS += -DAPPLICATION="\"judotimer\""
CFLAGS += -I$(SOUNDDIR)/inc
LIBS += -L$(SOUNDDIR)/lib -lfmodex -lvlc

### Windows executable ###
ifeq ($(TGT),WIN32)
RPATH =
OBJS += judotimer-icon.res

judotimer-icon.res: judotimer-icon.rc
	$(WINDRES) -O coff judotimer-icon.rc judotimer-icon.res

judotimer-icon.rc:
	echo "APPICONS ICON \"../etc/judotimer.ico\"" >judotimer-icon.rc

### Linux executable ###
else
RPATH = -Wl,-rpath,/usr/lib/judoshiai/lib
endif

$(OBJDIR)/judotimer$(SUFF): $(OBJS)
	${LD} -g $(RPATH) $(OBJS) -o $(OBJDIR)/judotimer$(SUFF) $(LIBS)

$(OBJDIR)/ip.o: ../common/ip.c
	${CC} -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/protocol.o: ../common/protocol.c
	${CC} -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/binreloc.o: ../common/binreloc.c
	${CC} -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	${CC} -g $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/* *.res *.rc *~
