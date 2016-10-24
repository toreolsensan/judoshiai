#ifndef _WIDGETS_H_
#define _WIDGETS_H_

/* Error */
typedef enum gui_error_type
{
    gui_error_None = 0,
    gui_error_Render = 1,
    gui_error_Preskin = 2,
    gui_error_Postskin = 3,
    gui_error_Event = 4,
} gui_error_type;

enum {
	GUI_ERROR_IGNORE = 0,
	GUI_ERROR_PROPOGATE,
	GUI_ERROR_FATAL
};

/* Widget */

struct gui_widget;

typedef int (*gui_event_handler)(struct gui_widget*, const SDL_Event);
typedef int (*gui_render_handler)(struct gui_widget*, SDL_Surface *);
typedef void (*gui_destroy_handler)(struct gui_widget*);
typedef int (*gui_error_handler)(gui_error_type, struct gui_widget *,
				   const SDL_Event *,
				   SDL_Surface *, int errorc);

typedef enum gui_widget_flags
{
    GUI_WIDGETFLAG_INVISIBLE     = 0x01,
    GUI_WIDGETFLAG_DISABLED      = 0x02,
    GUI_WIDGETFLAG_CANBEFOCUSED  = 0x04,
} gui_widget_flags;

enum {
	GUI_WIDGET_TYPE_BASIC = 0,
	GUI_WIDGET_TYPE_BUTTON,
	GUI_WIDGET_TYPE_ENTRY,
};

#define GUI_WIDGET_BASE_MEMBERS			\
	int			widget_id;	\
	int			widget_type;	\
	int			widget_size;	\
    						\
	void			*parent;	\
	void			*next;		\
	void			*first_child;	\
						\
	SDL_Rect                region;		\
						\
	gui_event_handler	event_cb;	\
	gui_render_handler	render_cb;	\
	gui_render_handler	skin_cb;	\
	gui_render_handler	decor_cb;	\
	gui_destroy_handler	destroy_cb;	\
						\
	void                    *custom_user_data;	\
						\
	int                  flags;

typedef struct gui_widget {
	GUI_WIDGET_BASE_MEMBERS
} gui_widget;

gui_widget *gui_widget_new(int type, int id, int w_size,
			   gui_event_handler evntcb,
			   gui_render_handler rndrcb,
			   gui_destroy_handler dstrycb,
			   int flags);
void gui_widget_destroy(gui_widget *widget);
void gui_widget_add(gui_widget *parent, gui_widget *child);
void gui_widget_insert_after(gui_widget *first, gui_widget *second);
int gui_widget_insert_after_update_pos(gui_widget *first, gui_widget *second);
int gui_widget_remove(gui_widget *child);
int gui_widget_render(gui_widget *widget, SDL_Surface *renderer, gui_error_handler eh);
int gui_widget_event(gui_widget *widget, const SDL_Event ev, gui_error_handler eh);

static inline void gui_widget_set_skinner(gui_widget *widget, gui_render_handler cb)
{
	if (widget != 0) {
		widget->skin_cb = cb;
	}
}

static inline void gui_widget_set_decorator(gui_widget *widget, gui_render_handler cb)
{
	if (widget != 0) {
		widget->decor_cb = cb;
	}
}

static inline void gui_widget_get_pos_absolute(gui_widget *widget, int *ax, int *ay)
{
	if (widget != 0) {
		*ax = widget->region.x;
		*ay = widget->region.y;
	}
}

static inline void gui_widget_get_pos(gui_widget *widget, int *rx, int *ry)
{
	gui_widget *parent = 0;
	if (widget != 0) {
		if (rx != 0)
			*rx = widget->region.x;
		if (ry != 0)
			*ry = widget->region.y;
		parent = widget->parent;
	}
	if (parent) {
		if (rx)
			*rx -= parent->region.x;
		if (ry)
			*ry -= parent->region.y;
	}
}

static inline void gui_widget_set_pos(gui_widget *widget, int x, int y)
{
	int oldx, oldy;
	if (widget != 0) {
		gui_widget_get_pos(widget, &oldx, &oldy);
		widget->region.x += (x - oldx);
		widget->region.y += (y - oldy);
	}
}

static inline void gui_widget_get_dimensions(gui_widget *widget, int* w, int *h)
{
	if (widget != 0) {
		if (w != 0)
			*w = widget->region.w;
		if (h != 0)
			*h = widget->region.h;
	}
}

static inline void gui_widget_set_dimensions(gui_widget *widget, int w, int h)
{
	if (widget != 0) {
		if (w > 0)
			widget->region.w = w;
		if (h > 0)
			widget->region.h = h;
	}
}

static inline void gui_widget_set_visible(gui_widget *widget, SDL_bool visibility)
{
	if (widget != 0) {
		if (visibility == SDL_TRUE)
			widget->flags &= ~GUI_WIDGETFLAG_INVISIBLE;
		else
			widget->flags |= GUI_WIDGETFLAG_INVISIBLE;
	}
}

static inline void gui_widget_set_disabled(gui_widget *widget, SDL_bool disabled)
{
	if (widget != 0) {
		if (disabled == SDL_TRUE)
			widget->flags |= GUI_WIDGETFLAG_DISABLED;
		else
			widget->flags &= ~GUI_WIDGETFLAG_DISABLED;
	}
}


#endif
