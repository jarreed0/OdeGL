#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdlib.h>

constexpr int FULLSCREEN = 0x0001;
constexpr int RESIZABLE = 0x0010;
constexpr int DEBUG = 0x0100;
constexpr int SCALABLE = 0x1000;

#define RED 16711680
#define GREEN 65280
#define BLUE 255
#define WHITE 16777215
#define BLACK 0
#define YELLOW 16776960

struct coord {
 int x, y;
};

bool attributes;

unsigned long RGB(int r, int g, int b);
void RGB(unsigned long &rgb, int r, int g, int b);
struct GBColor {
    unsigned long rgb;
    void set(int r, int g, int b) {
        RGB(rgb, r, g, b);
    }
    GBColor& operator =(const GBColor& c) {
        rgb = c.rgb;
        return *this;
    }
    bool operator ==(const GBColor& c) {return (rgb==c.rgb);}
    bool operator !=(const GBColor& c) {return (rgb!=c.rgb);}
    bool operator >(const GBColor& c) {return (rgb>c.rgb);}
    bool operator <(const GBColor& c) {return (rgb<c.rgb);}
    bool operator >=(const GBColor& c) {return (rgb>=c.rgb);}
    bool operator <=(const GBColor& c) {return (rgb<=c.rgb);}
    bool operator +(const GBColor& c) {return (rgb+c.rgb);}
    bool operator -(const GBColor& c) {return (rgb-c.rgb);}
    bool operator *(const GBColor& c) {return (rgb*c.rgb);}
    bool operator /(const GBColor& c) {return (rgb/c.rgb);}
    bool operator %(const GBColor& c) {return (rgb%c.rgb);}
    GBColor& operator =(const int i) {
        rgb = i;
        return *this;
    }
    bool operator ==(const int i) {return (rgb==i);}
    bool operator !=(const int i) {return (rgb!=i);}
    bool operator >(const int i) {return (rgb>i);}
    bool operator <(const int i) {return (rgb<i);}
    bool operator >=(const int i) {return (rgb>=i);}
    bool operator <=(const int i) {return (rgb<=i);}
    bool operator +(const int i) {return (rgb+i);}
    bool operator -(const int i) {return (rgb-i);}
    bool operator *(const int i) {return (rgb*i);}
    bool operator /(const int i) {return (rgb/i);}
    bool operator %(const int i) {return (rgb%i);}
};
unsigned long RGB(int r, int g, int b) {
    return b + (g<<8) + (r<<16);
}
void RGB(unsigned long &rgb, int r, int g, int b) {
    rgb = RGB(r,g,b);
}

struct GBWindow {
Display *dis;
int screen;
Window win;
GC context;
GBColor bkg, draw, font;
int Width, Height;
XWindowAttributes wnd;
void update() {
    if(attributes) {
        XGetWindowAttributes(dis, win, &wnd);
        Width=wnd.width;
        Height=wnd.height;
    }
}
};

void closes(GBWindow &gb);
void redraw(GBWindow &gb);

struct keyboard {
    XEvent event;
	KeySym key;
	char text[255];
    int mouseX, mouseY;
    void mouseUpdate() {
        mouseX=event.xbutton.x;
        mouseY=event.xbutton.y;
    }
    bool exposed() {
        if(event.type==Expose && event.xexpose.count==0) {
            return true;
        }
        return 0;
    }
    bool pressed() {
        if(event.type==KeyPress &&
		    XLookupString(&event.xkey,text,255,&key,0)==1) {
                return true;
        }
        return 0;
    }
    char down() {
        return text[0];
    }
    bool down(char c) {
        if(text[0]==c) return true;
        return 0;
    }
    bool mouseDown() {
        if(event.type==ButtonPress) {
            mouseUpdate();
            return 1;
        }
        return 0;
    }
    void getMouse(int &x, int &y) {
        mouseUpdate();
        x=mouseX; y=mouseY;
    }
    void update(GBWindow &gb) {
        XNextEvent(gb.dis, &event);
        mouseUpdate();
        if (exposed()) {
			redraw(gb);
		}
    }
} keys;

void setDrawColor(GBWindow &gb, unsigned long color) {
    gb.draw = color;
}
void setFontColor(GBWindow &gb, unsigned long color) {
    gb.font = color;
}
void setBackground(GBWindow &gb, unsigned long color) {
    gb.bkg = color;
}
void setDrawColor(GBWindow &gb, int r, int g, int b) {
    GBColor color; color.set(r,g,b);
    gb.draw = color;
}
void setFontColor(GBWindow &gb, int r, int g, int b) {
    GBColor color; color.set(r,g,b);
    gb.font = color;
}
void setBackground(GBWindow &gb, int r, int g, int b) {
    GBColor color; color.set(r,g,b);
    gb.bkg = color;
}
void setDrawColor(GBWindow &gb, GBColor color) {
    gb.draw = color;
}
void setFontColor(GBWindow &gb, GBColor color) {
    gb.font = color;
}
void setBackground(GBWindow &gb, GBColor color) {
    gb.bkg = color;
}
void setForeground(GBWindow &gb, GBColor color) {
    XSetForeground(gb.dis, gb.context, color.rgb);
}

void drawLine(GBWindow& win, int ax, int ay, int bx, int by) {
    setForeground(win, win.draw);
    XDrawLine(win.dis, win.win, win.context, ax, ay, bx, by);
}
void write(GBWindow& win, int x, int y, const char* text) {
    setForeground(win, win.font);
    XDrawString(win.dis, win.win, win.context, x, y, text, strlen(text));
}
void drawPoint(GBWindow& win, int x, int y) {
    setForeground(win, win.draw);
    XDrawPoint(win.dis, win.win, win.context, x, y);
}

struct GBShape {
    int x, y;
    int width, height;
};

void drawRect(GBWindow &gb, int x, int y, int w, int h) {
    setForeground(gb, gb.draw);
    XDrawRectangle(gb.dis, gb.win, gb.context, x, y, w, h);
}
void drawFilledRect(GBWindow &gb, int x, int y, int w, int h) {
    setForeground(gb, gb.draw);
    XFillRectangle(gb.dis, gb.win, gb.context, x, y, w, h);
}
void drawRect(GBWindow &gb, GBShape r) {
    drawRect(gb, r.x, r.y, r.width, r.height);
}
void drawFilledRect(GBWindow &gb, GBShape r) {
    drawFilledRect(gb, r.x, r.y, r.width, r.height);
}
void drawOval(GBWindow &gb, int x, int y, int w, int h) {
    setForeground(gb, gb.draw);
    XDrawArc(gb.dis, gb.win, gb.context, x, y, w, h, 0, 360*64);
}
void drawFilledOval(GBWindow &gb, int x, int y, int w, int h) {
    setForeground(gb, gb.draw);
    XFillArc(gb.dis, gb.win, gb.context, x, y, w, h, 0, 360*64);
}
void drawCircle(GBWindow &gb, int x, int y, int d) {
    drawOval(gb, x, y, d, d);
}
void drawFilledCircle(GBWindow &gb, int x, int y, int d) {
    drawFilledOval(gb, x, y, d, d);
}
void drawCircle(GBWindow &gb, GBShape s) {
    drawCircle(gb, s.x, s.y, s.width);
}
void drawFilledCircle(GBWindow &gb, GBShape s) {
    drawFilledCircle(gb, s.x, s.y, s.width);
}
void drawOval(GBWindow &gb, GBShape s) {
    drawOval(gb, s.x, s.y, s.width, s.height);
}
void drawFilledOval(GBWindow &gb, GBShape s) {
    drawFilledOval(gb, s.x, s.y, s.width, s.height);
}
void drawBackground(GBWindow &gb) {
    GBColor o = gb.draw;
    setDrawColor(gb, gb.bkg);
    drawFilledRect(gb, 0, 0, gb.Width, gb.Height);
    setDrawColor(gb, o);
}

void initGB(GBWindow &tmp, const char* title, int width, int height, const int flags) {
    attributes=0;
    if(flags & FULLSCREEN) {
        title="Fullscreen";
    }
    if(flags & DEBUG) {
        title="Debug Mode";
    }
    tmp.Width=width;
    tmp.Height=height;
	tmp.dis=XOpenDisplay((char *)0);
   	tmp.screen=DefaultScreen(tmp.dis);
    setBackground(tmp, WHITE);
	setDrawColor(tmp, BLACK);
    setFontColor(tmp, BLACK);
   	tmp.win=XCreateSimpleWindow(tmp.dis,DefaultRootWindow(tmp.dis),0,0,	
		width, height, 5, tmp.draw.rgb, tmp.bkg.rgb);
	XSetStandardProperties(tmp.dis,tmp.win,title,title,None,NULL,0,NULL);
	XSelectInput(tmp.dis, tmp.win, ExposureMask|ButtonPressMask|KeyPressMask);
        tmp.context=XCreateGC(tmp.dis, tmp.win, 0,0);        
	XClearWindow(tmp.dis, tmp.win);
	XMapRaised(tmp.dis, tmp.win);
    if(flags & RESIZABLE) {
        attributes=1;
    } else {
        XSizeHints *hints = XAllocSizeHints();
        hints->flags = PMinSize|PMaxSize;
        hints->min_width = hints->max_width = tmp.Width;
        hints->min_height = hints->max_height = tmp.Height;
        XSetWMNormalHints(tmp.dis, tmp.win, hints);
        XSetWMSizeHints(tmp.dis, tmp.win, hints, PMinSize|PMaxSize);
    }
}

void close(GBWindow &gb) {
	XFreeGC(gb.dis, gb.context);
	XDestroyWindow(gb.dis,gb.win);
	XCloseDisplay(gb.dis);	
	exit(1);			
};

void redraw(GBWindow &gb) {
	XClearWindow(gb.dis, gb.win);
};

