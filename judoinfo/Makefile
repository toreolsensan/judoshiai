
all: judoinfo

OBJS = main.o menus.o comm.o ip.o protocol.o trees.o avl.o

include ../common/Makefile.inc

CFLAGS += -DAPPLICATION="\"judoinfo\""

ifeq ($(OS),Windows_NT)

judoinfo: judoinfo1 judoinfo-icon.res
	../win/bin/ResHacker.exe -add judoinfo1.exe, judoinfo.exe, judoinfo-icon.res, ,,

judoinfo-icon.res: judoinfo-icon.rc
	windres.exe judoinfo-icon.rc judoinfo-icon.res

judoinfo-icon.rc:
	echo "APPICONS ICON \"../etc/judoinfo.ico\"" >judoinfo-icon.rc

else

judoinfo: judoinfo1
	cp judoinfo1 judoinfo

endif

judoinfo1: $(OBJS)
	gcc -g -Wl,-rpath -Wl,bin $(OBJS) -o judoinfo1 $(LIBS)

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
