include ../common/Makefile.inc

all: $(OBJDIR)/judojudogi$(SUFF)

OBJS += $(OBJDIR)/main.o $(OBJDIR)/menus.o $(OBJDIR)/comm.o $(OBJDIR)/ip.o \
	$(OBJDIR)/protocol.o $(OBJDIR)/trees.o $(OBJDIR)/avl.o

CFLAGS += -DAPPLICATION="\"judojudogi\""

### Windows executable ###
ifeq ($(TGT),WIN32)
OBJS += judojudogi-icon.res

judojudogi-icon.res: judojudogi-icon.rc
	$(WINDRES) -O coff judojudogi-icon.rc judojudogi-icon.res

judojudogi-icon.rc:
	echo "APPICONS ICON \"../etc/judojudogi.ico\"" >judojudogi-icon.rc

### Linux executable ###
else
endif

$(OBJDIR)/judojudogi$(SUFF): $(OBJS)
	$(LD) -g -Wl,-rpath -Wl,bin $(OBJS) -o $(OBJDIR)/judojudogi$(SUFF) $(LIBS)

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
