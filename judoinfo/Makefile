include ../common/Makefile.inc

all: judoweight$(SUFF)

OBJS += main.o menus.o comm.o ip.o protocol.o serial.o

CFLAGS += -DAPPLICATION="\"judoweight\""

### Windows executable ###
ifeq ($(TGT),WIN32)
judoweight.exe: judoweight1.exe judoweight-icon.res
	$(RESHACKER) -add judoweight1.exe, judoweight.exe, judoweight-icon.res, ,,

judoweight-icon.res: judoweight-icon.rc
	$(WINDRES) judoweight-icon.rc judoweight-icon.res

judoweight-icon.rc:
	echo "APPICONS ICON \"../etc/judoweight.ico\"" >judoweight-icon.rc

### Linux executable ###
else
judoweight: judoweight1
	cp judoweight1 judoweight

endif

judoweight1$(SUFF): $(OBJS)
	$(LD) -g -Wl,-rpath -Wl,bin $(OBJS) -o judoweight1$(SUFF) $(LIBS)

avl.o: ../common/avl.c
	${CC} -g $(CFLAGS) -c $< -o $@

ip.o: ../common/ip.c
	${CC} -g $(CFLAGS) -c $< -o $@

protocol.o: ../common/protocol.c
	${CC} -g $(CFLAGS) -c $< -o $@

binreloc.o: ../common/binreloc.c
	${CC} -g $(CFLAGS) -c $< -o $@

serial.o: ../common/serial.c
	${CC} -g $(CFLAGS) -c $< -o $@

%.o: %.c
	${CC} -g $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.exe judoweight judoweight1 *.res *.rc *~
