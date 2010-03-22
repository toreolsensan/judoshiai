
all: judoshiai

OBJS = main.o competitors.o sheets.o set-one-judoka.o sqlite3.o \
       popup-menu.o util.o db.o categories.o log.o matches.o \
       names.o comm.o ip.o protocol.o set-categories.o category-graph.o \
       completion.o another-shiai.o avl.o httpp.o httpd.o import.o set-password.o \
       trees.o match-graph.o search.o menus.o menuacts.o \
       sql-dialog.o drawing.o print.o print-texts.o db-competitors.o db-categories.o \
       db-matches.o db-table.o write-results.o match-data.o

#PROF=-pg
PROF=

include ../common/Makefile.inc

CFLAGS += -DAPPLICATION="\"judoshiai\""

ifeq ($(OS),Windows_NT)

judoshiai: judoshiai1 judoshiai-icon.res
	ResHacker.exe -add judoshiai1.exe, judoshiai.exe, judoshiai-icon.res, ,,

judoshiai-icon.res: judoshiai-icon.rc
	windres.exe judoshiai-icon.rc judoshiai-icon.res

judoshiai-icon.rc: ../etc/judoshiai.ico
	echo "APPICONS ICON \"../etc/judoshiai.ico\"" >judoshiai-icon.rc

else

judoshiai: judoshiai1
	cp judoshiai1 judoshiai

endif

judoshiai1: $(OBJS)
	gcc $(PROF) -g -Wl,-rpath -Wl,bin $(OBJS) -o judoshiai1 $(LIBS)

avl.o: ../common/avl.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

ip.o: ../common/ip.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

protocol.o: ../common/protocol.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

binreloc.o: ../common/binreloc.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

%.o: %.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.exe judoshiai judoshiai1 *.res *.rc *~

