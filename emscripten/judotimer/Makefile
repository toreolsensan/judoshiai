all: ../../etc/timer.js ../../etc/timer.html

SRCS = main.c stopwatch.c flags.c menu.c ../../common/avl.c comm.c ../common/font.c \
  ../gui/widget.c ../gui/button.c ../gui/entry.c ../gui/menu-util.c

#  ../gui/SDL_UI_button.c ../gui/SDL_UI_element.c ../gui/SDL_UI_image.c \
#  ../gui/SDL_UI_sdlhelper.c ../gui/SDL_UI_slider.c ../gui/SDL_UI_text.c \
#  ../gui/SDL_UI_ttf_bridge.c ../gui/SDL_UI_widget.c

OBJS = $(SRCS:.c=.bc)

# CFLAGS += -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s SDL2_IMAGE_FORMATS='["png"]'
# CFLAGS += -s USE_PTHREADS=1
# CFLAGS += -s USE_SDL_NET=2
CFLAGS += -I. -I../common -I../../judotimer -I../../common -I../gui
CFLAGS += -DJUDOTIMER=1 -O2 -g4 -s NO_EXIT_RUNTIME=1 -s ASSERTIONS=1
#CFLAGS += -s EXPORTED_FUNCTIONS="['_setscreensize']"

../../etc/timer.js: timer.js
	cp timer.js* ../../etc/
	cp timer.js* ~/js-build/release-linux/judoshiai/etc/

../../etc/timer.html: timer.html
	cp timer.html ../../etc/

%.bc: %.c
	emcc $(CFLAGS) -c $< -o $@

timer.js: $(OBJS) Makefile
	emcc $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) ../../etc/timer.html timer.js* ../../etc/timer.js*
