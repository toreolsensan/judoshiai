include ../common/Makefile.inc

OBJDIR := $(JS_BUILD_DIR)/$(LASTPART)/$(OBJDIR)

all: $(OBJDIR) $(OBJDIR)/judoinfo$(SUFF)

$(OBJDIR):
	mkdir -p $(OBJDIR)

OBJS += $(OBJDIR)/main.o $(OBJDIR)/menus.o $(OBJDIR)/comm.o $(OBJDIR)/ip.o \
	$(OBJDIR)/protocol.o $(OBJDIR)/trees.o $(OBJDIR)/avl.o $(OBJDIR)/language.o \
	$(OBJDIR)/svg.o $(OBJDIR)/common.o

CFLAGS += -DAPPLICATION="\"judoinfo\""
CFLAGS += -DAPP_NUM=3

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

$(OBJDIR)/language.o: ../common/language.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/common.o: ../common/common.c
	${CC} $(PROF) -g $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c judoinfo.h ../common/comm.h
	${CC} -g $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/* *.res *.rc *~
