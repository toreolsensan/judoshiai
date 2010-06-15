
all: judotimer

OBJS = main.o menus.o stopwatch.o comm.o ip.o protocol.o show-ad.o sound.o

include ../common/Makefile.inc

CFLAGS += -DAPPLICATION="\"judotimer\""
CFLAGS += -I$(SOUNDDIR)/inc
LIBS += -L$(SOUNDDIR)/lib -lfmodex

ifeq ($(OS),Windows_NT)

RPATH =

judotimer: judotimer1 judotimer-icon.res
	ResHacker.exe -add judotimer1.exe, judotimer.exe, judotimer-icon.res, ,,

judotimer-icon.res: judotimer-icon.rc
	windres.exe judotimer-icon.rc judotimer-icon.res

judotimer-icon.rc:
	echo "APPICONS ICON \"../etc/judotimer.ico\"" >judotimer-icon.rc

else

RPATH = -Wl,-rpath,/usr/lib/judoshiai/lib

judotimer: judotimer1
	cp judotimer1 judotimer

endif

judotimer1: $(OBJS)
	${CC} -g $(RPATH) $(OBJS) -o judotimer1 $(LIBS)

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
