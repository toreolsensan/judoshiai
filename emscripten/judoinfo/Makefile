all: ../../etc/info.js ../../etc/info.html

SRCS    = main.c comm.c ../../judoinfo/trees.c ../../common/avl.c ../common/font.c \
          menu.c ../gui/widget.c ../gui/button.c ../gui/entry.c ../gui/menu-util.c \
          svg.c ../../common/common.c ../../3rdparty/cJSON/cJSON.c

OBJS = $(SRCS:.c=.bc)

CFLAGS  = -I. -I../common -I../../judoinfo -I../../common -I../../3rdparty/cJSON -I../gui -O3
CFLAGS += -s ASSERTIONS=1 -O2 -g4 -s NO_EXIT_RUNTIME=1
#CFLAGS += -s EXPORTED_FUNCTIONS="['_comm_sock_rec']"

../../etc/info.js: info.js
	cp info.js* ../../etc/
	cp info.js* ~/js-build/release-linux/judoshiai/etc/
	cp info.js* ~/js-build/release-win32/judoshiai/etc/

../../etc/info.html: info.html
	cp info.html ../../etc/
	cp info.html ~/js-build/release-linux/judoshiai/etc/
	cp info.html ~/js-build/release-win32/judoshiai/etc/

%.bc: %.c
	emcc $(CFLAGS) -c $< -o $@

info.js: $(OBJS) Makefile
	emcc $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) ../../etc/info.html info.js* ../../etc/info.js*
