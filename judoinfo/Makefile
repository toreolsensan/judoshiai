include ../common/Makefile.inc

all: $(OBJDIR)/judoinfo$(SUFF)

OBJS += $(OBJDIR)/main.o $(OBJDIR)/menus.o $(OBJDIR)/comm.o $(OBJDIR)/ip.o \
	$(OBJDIR)/protocol.o $(OBJDIR)/trees.o $(OBJDIR)/avl.o

CFLAGS += -DAPPLICATION="\"judoinfo\""

### Windows executable ###
ifeq ($(TGT),WIN32)
OBJS += judoinfo-icon.res

judoinfo-icon.res: judoinfo-icon.rc
	$(WINDRES) -O coff judoinfo-icon.rc judoinfo-icon.res

judoinfo-icon.rc:
	echo "APPICONS ICON \"../etc/judoinfo.ico\"" >judoinfo-icon.rc

### Linux executable ###
else
endif

$(OBJDIR)/judoinfo$(SUFF): $(OBJS)
	$(LD) -g -Wl,-rpath -Wl,bin $(OBJS) -o $(OBJDIR)/judoinfo$(SUFF) $(LIBS)

$(OBJDIR)/avl.o: ../common/avl.c
	${CC} -g $(CFLAGS) -c $< -o $@

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
