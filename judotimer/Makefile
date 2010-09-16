include ../common/Makefile.inc

all: judotimer$(SUFF)

OBJS += main.o menus.o stopwatch.o comm.o ip.o protocol.o show-ad.o sound.o

CFLAGS += -DAPPLICATION="\"judotimer\""
CFLAGS += -I$(SOUNDDIR)/inc
LIBS += -L$(SOUNDDIR)/lib -lfmodex

### Windows executable ###
ifeq ($(TGT),WIN32)
RPATH =

judotimer.exe: judotimer1.exe judotimer-icon.res
	$(RESHACKER) -add judotimer1.exe, judotimer.exe, judotimer-icon.res, ,,

judotimer-icon.res: judotimer-icon.rc
	$(WINDRES) judotimer-icon.rc judotimer-icon.res

judotimer-icon.rc:
	echo "APPICONS ICON \"../etc/judotimer.ico\"" >judotimer-icon.rc

### Linux executable ###
else
RPATH = -Wl,-rpath,/usr/lib/judoshiai/lib

judotimer: judotimer1
	cp judotimer1 judotimer

endif

judotimer1$(SUFF): $(OBJS)
	${LD} -g $(RPATH) $(OBJS) -o judotimer1$(SUFF) $(LIBS)

ip.o: ../common/ip.c
	${CC} -g $(CFLAGS) -c $< -o $@

protocol.o: ../common/protocol.c
	${CC} -g $(CFLAGS) -c $< -o $@

binreloc.o: ../common/binreloc.c
	${CC} -g $(CFLAGS) -c $< -o $@

%.o: %.c
	${CC} -g $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.exe judotimer judotimer1 *.res *.rc *~
