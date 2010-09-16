include ../common/Makefile.inc

all: judoinfo$(SUFF)

OBJS += main.o menus.o comm.o ip.o protocol.o trees.o avl.o

CFLAGS += -DAPPLICATION="\"judoinfo\""

### Windows executable ###
ifeq ($(TGT),WIN32)
judoinfo.exe: judoinfo1.exe judoinfo-icon.res
	$(RESHACKER) -add judoinfo1.exe, judoinfo.exe, judoinfo-icon.res, ,,

judoinfo-icon.res: judoinfo-icon.rc
	$(WINDRES) judoinfo-icon.rc judoinfo-icon.res

judoinfo-icon.rc:
	echo "APPICONS ICON \"../etc/judoinfo.ico\"" >judoinfo-icon.rc

### Linux executable ###
else
judoinfo: judoinfo1
	cp judoinfo1 judoinfo

endif

judoinfo1$(SUFF): $(OBJS)
	$(LD) -g -Wl,-rpath -Wl,bin $(OBJS) -o judoinfo1$(SUFF) $(LIBS)

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
	rm -f *.o *.exe judoinfo judoinfo1 *.res *.rc *~
