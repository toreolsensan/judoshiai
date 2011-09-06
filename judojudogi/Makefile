include ../common/Makefile.inc

all: judojudogi$(SUFF)

OBJS += main.o menus.o comm.o ip.o protocol.o trees.o avl.o

CFLAGS += -DAPPLICATION="\"judojudogi\""

### Windows executable ###
ifeq ($(TGT),WIN32)
judojudogi.exe: judojudogi1.exe judojudogi-icon.res
	$(RESHACKER) -add judojudogi1.exe, judojudogi.exe, judojudogi-icon.res, ,,

judojudogi-icon.res: judojudogi-icon.rc
	$(WINDRES) judojudogi-icon.rc judojudogi-icon.res

judojudogi-icon.rc:
	echo "APPICONS ICON \"../etc/judojudogi.ico\"" >judojudogi-icon.rc

### Linux executable ###
else
judojudogi: judojudogi1
	cp judojudogi1 judojudogi

endif

judojudogi1$(SUFF): $(OBJS)
	$(LD) -g -Wl,-rpath -Wl,bin $(OBJS) -o judojudogi1$(SUFF) $(LIBS)

avl.o: ../common/avl.c
	${CC} -g $(CFLAGS) -c $< -o $@

ip.o: ../common/ip.c
	${CC} -g $(CFLAGS) -c $< -o $@

protocol.o: ../common/protocol.c
	${CC} -g $(CFLAGS) -c $< -o $@

binreloc.o: ../common/binreloc.c
	${CC} -g $(CFLAGS) -c $< -o $@

%.o: %.c
	${CC} -g $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.exe judojudogi judojudogi1 *.res *.rc *~
