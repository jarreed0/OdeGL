#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
//#include <string.h>
#include <math.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#include <stdio.h>
#include <stdlib.h>
//#include <curses.h> //-lncurses
//#include "conio.h"

#include <vector>
#include <string>

//loading files
#include <fstream>
#include <sstream>

constexpr int FULLSCREEN = 0x0001;
constexpr int RESIZABLE = 0x0010;
constexpr int DEBUG = 0x0100;
constexpr int SCALABLE = 0x1000;

#define PI 3.1415926535

#define RED 16711680
#define GREEN 65280
#define BLUE 255
#define WHITE 16777215
#define BLACK 0
#define YELLOW 16776960

struct coord {
 int x, y, w;
};

struct line {
 coord s, e;
};

struct coord3d {
  float x, y, z, w;
};

struct triangle {
 coord3d c[3];
};

struct matrix {
  float m[4][4] = { 0 };
};

void multiplyMatrix(coord3d &i, coord3d &o, matrix &m) {
  o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
	o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
	o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
	float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];
	if (w != 0.0f) {
		o.x /= w; o.y /= w; o.z /= w;
	}
}

bool attributes;
matrix projection;

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
Visual *vis;
void update() {
    if(attributes) {
        XGetWindowAttributes(dis, win, &wnd);
        Width=wnd.width;
        Height=wnd.height;
    }
}
};

struct mesh {
 std::vector<triangle> tris;
 bool loadObj(std::string fn) {
   std::ifstream f(fn);
   if(!f.is_open()) {
     return false;
   }
   std::vector<coord3d> verts;
   while(!f.eof()) {
     char line[128];
     f.getline(line, 128);
     std::stringstream s;
     s << line;
     char junk;
     if(line[0] == 'v') {
       coord3d tmp;
       s >> junk >> tmp.x >> tmp.y >> tmp.z;
       verts.push_back(tmp);
     }
     if(line[0] == 'f') {
       int f[3];
       s >> junk >> f[0] >> f[1] >> f[2];
       tris.push_back({verts[f[0]-1], verts[f[1]-1], verts[f[2]-1]});
     }
   }
   return true;
 }
 float fNear = 0.1f;
 float fFar = 1000.0f;
 float fFov = 90.0f;
 float fAspectRatio;
 void calcAspect(GBWindow &gb) { fAspectRatio = (float)gb.Height / (float)gb.Width; }
 float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * PI);
 void init(GBWindow &gb) {
  calcAspect(gb);
  projection.m[0][0] = fAspectRatio * fFovRad;
  projection.m[1][1] = fFovRad;
  projection.m[2][2] = fFar / (fFar - fNear);
  projection.m[3][2] = (-fFar * fNear) / (fFar - fNear);
  projection.m[2][3] = 1.0f;
  projection.m[3][3] = 0.0f;
 }
};


triangle projectTriangle(GBWindow &gb, triangle &i, triangle &o) {
  multiplyMatrix(i.c[0], o.c[0], projection);
  multiplyMatrix(i.c[1], o.c[1], projection);
  multiplyMatrix(i.c[2], o.c[2], projection);
  o.c[0].x += 1.0f; o.c[0].y += 1.0f;
	o.c[1].x += 1.0f; o.c[1].y += 1.0f;
	o.c[2].x += 1.0f; o.c[2].y += 1.0f;
	o.c[0].x *= 0.5f * (float)gb.Width;
	o.c[0].y *= 0.5f * (float)gb.Height;
	o.c[1].x *= 0.5f * (float)gb.Width;
	o.c[1].y *= 0.5f * (float)gb.Height;
	o.c[2].x *= 0.5f * (float)gb.Width;
	o.c[2].y *= 0.5f * (float)gb.Height;
  return o;
}

triangle offsetTriangle(triangle &i, triangle &o, float of) { //translate
  o.c[0].z = i.c[0].z + of;
  o.c[0].z = i.c[1].z + of;
  o.c[0].z = i.c[2].z + of;
  return o;
}

triangle project(GBWindow &gb, triangle tri, float offset) {
  triangle triProjected, triTranslated;
	triTranslated = tri;
	triTranslated.c[0].z = tri.c[0].z + offset;
	triTranslated.c[1].z = tri.c[1].z + offset;
	triTranslated.c[2].z = tri.c[2].z + offset;
	multiplyMatrix(triTranslated.c[0], triProjected.c[0], projection);
	multiplyMatrix(triTranslated.c[1], triProjected.c[1], projection);
	multiplyMatrix(triTranslated.c[2], triProjected.c[2], projection);
	triProjected.c[0].x += 1.0f; triProjected.c[0].y += 1.0f;
	triProjected.c[1].x += 1.0f; triProjected.c[1].y += 1.0f;
	triProjected.c[2].x += 1.0f; triProjected.c[2].y += 1.0f;
	triProjected.c[0].x *= 0.5f * (float)gb.Width;
	triProjected.c[0].y *= 0.5f * (float)gb.Height;
	triProjected.c[1].x *= 0.5f * (float)gb.Width;
	triProjected.c[1].y *= 0.5f * (float)gb.Height;
	triProjected.c[2].x *= 0.5f * (float)gb.Width;
	triProjected.c[2].y *= 0.5f * (float)gb.Height;
  return triProjected;
}

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
    /*int update2() {
        int ch = getch();
        if(ch != ERR) return ch;
        return -1;
    }*/
    void update(GBWindow &gb) {
        //XPending(gb.dis);
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
void drawLine(GBWindow& win, coord a, coord b) {
    setForeground(win, win.draw);
    XDrawLine(win.dis, win.win, win.context, a.x, a.y, b.x, b.y);
}
void drawLine(GBWindow& win, coord3d a, coord3d b) {
    setForeground(win, win.draw);
    XDrawLine(win.dis, win.win, win.context, a.x, a.y, b.x, b.y);
}
void drawLine(GBWindow& win, line l) {
    setForeground(win, win.draw);
    XDrawLine(win.dis, win.win, win.context, l.s.x, l.s.y, l.e.x, l.e.y);
}
void drawTriangle(GBWindow &gb, triangle tr) {
  drawLine(gb, tr.c[0], tr.c[1]);
  drawLine(gb, tr.c[1], tr.c[2]);
  drawLine(gb, tr.c[2], tr.c[0]);
}
void drawMesh(GBWindow &gb, mesh m, float offset) {
 for(auto tri : m.tris) {
    triangle tri3D = project(gb, tri, offset);
    drawTriangle(gb, tri3D);
 }
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
    tmp.vis = DefaultVisual(tmp.dis, 0);
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
    XClassHint *class_hint = XAllocClassHint();
    if (class_hint) {
      class_hint->res_name = class_hint->res_class = (char *)title;
      XSetClassHint(tmp.dis, tmp.win, class_hint);
      XFree(class_hint);
    }
    /*initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);*/
}

void close(GBWindow &gb) {
	XFreeGC(gb.dis, gb.context);
	XDestroyWindow(gb.dis,gb.win);
	XCloseDisplay(gb.dis);
	exit(1);
        //endwin();
};

void redraw(GBWindow &gb) {
	XClearWindow(gb.dis, gb.win);
};


bool overlaps(GBShape a, GBShape b) {
  if ((a.x < (b.x + b.width)) && ((a.x + a.width) > b.x) &&
   (a.y < (b.y + b.height)) && ((a.y + a.height) > b.y)) {
    return true;
  }
  else {
    return false;
  }
}

bool touching(GBShape a, GBShape b) {
 return overlaps(a,b);
}
bool collision(GBShape a, GBShape b) {
 return overlaps(a,b);
}
bool collides(GBShape a, GBShape b) {
 return overlaps(a,b);
}


bool contains(GBShape a, GBShape b) {
    if (((b.x + b.width) < (a.x + a.width)) && (b.x > a.x) &&
     (b.y > a.y) && ((b.y + b.height) < (a.y+a.height))) {
        return true;
    }
    else {
        return false;
    }
}

bool contains(GBShape o, int x, int y) {
 GBShape tmp;
 tmp.x=x;tmp.y=y;tmp.width=tmp.height=1;
 return contains(o, tmp);
}

bool contains(GBShape o, coord c) {
 return contains(o, c.x, c.y);
}

bool isAbove(GBShape a, GBShape b) {
  if((a.y + a.height) < b.y) {
    return true;
  } else {
    return false;
  }
}
bool isAbove(GBShape a, coord b) {
  if((a.y + a.height) < b.y) {
    return true;
  } else {
    return false;
  }
}

bool isBelow(GBShape a, GBShape b) {
  if(a.y > (b.y + b.height)) {
    return true;
  } else {
    return false;
  }
}
bool isBelow(GBShape a, coord b) {
  if(a.y > (b.y)) {
    return true;
  } else {
    return false;
  }
}

bool isRightOf(GBShape a, GBShape b) {
  if(a.x > (b.x + b.width)) {
    return true;
  } else {
    return false;
  }
}
bool isRightOf(GBShape a, coord b) {
  if(a.x > (b.x)) {
    return true;
  } else {
    return false;
  }
}

bool isLeftOf(GBShape a, GBShape b) {
  if((a.x + a.width) < b.x) {
    return true;
  } else {
    return false;
  }
}
bool isLeftOf(GBShape a, coord b) {
  if((a.x + a.width) < b.x) {
    return true;
  } else {
    return false;
  }
}

bool outOfBoundsOf(GBShape a, GBShape b) {
  if(isAbove(a, b) || isBelow(a, b)) {
    return true;
  } else if(isRightOf(a, b) || isLeftOf(a, b)) {
    return true;
  } else {
    return false;
  }
}
bool outOfBoundsOf(GBShape a, coord b) {
  if(isAbove(a, b) || isBelow(a, b)) {
    return true;
  } else if(isRightOf(a, b) || isLeftOf(a, b)) {
    return true;
  } else {
    return false;
  }
}

int max(int a, int b) {
if(a>b) return a;
return b;
}

int min(int a, int b) {
if(a<b) return a;
return b;
}

bool onSegment(coord p, coord q, coord r) {
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y)) return true;
    return false;
}
int orientation(coord p, coord q, coord r) {
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;
    return (val > 0)? 1: 2;
}

bool intersects(coord p1, coord q1, coord p2, coord q2) {
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);
    if (o1 != o2 && o3 != o4) return true;
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;
    return false;
}

bool intersects(line a, line b) {
 return intersects(a.s,a.e,b.s,b.e);
}

bool intersects(line a, GBShape b) {
 line l;
 l.s.x=b.x;l.s.y=b.y;
 l.e.x=b.x+b.width;l.e.y=b.y;
 if(intersects(a,l)) return 1;
 l.s.x=b.x;l.s.y=b.y;
 l.e.x=b.x;l.e.y=b.y+b.height;
 if(intersects(a,l)) return 1;
 l.s.x=b.x;l.s.y=b.y+b.height;
 l.e.x=b.x+b.width;l.e.y=b.y+b.height;
 if(intersects(a,l)) return 1;
 l.s.x=b.x+b.width;l.s.y=b.y;
 l.e.x=b.x+b.width;l.e.y=b.y+b.height;
 if(intersects(a,l)) return 1;
 return 0;
}

struct GBSprite {
  XImage *img;
  uint8_t *imageData;
  /*void load(GBWindow &gb, int w, int h) {
    static XShmSegmentInfo shminfo;
	  XImage *imgTmp = XShmCreateImage(gb.dis, gb.vis, 24, ZPixmap, 0, &shminfo, w, h); 
  	shminfo.shmid = shmget(IPC_PRIVATE, imgTmp->bytes_per_line * imgTmp->height,IPC_CREAT|0777);
    void ** data = (void**) &imageData;
	  shminfo.shmaddr = imgTmp->data = *data = shmat(shminfo.shmid, 0, 0);
	  shminfo.readOnly = False;
  	XShmAttach(gb.dis, &shminfo);
    img = imgTmp;
  }*/
};

struct GBObject2d {
  GBShape dest, src;
  GBSprite spr;
};

void drawSprite(GBWindow &gb, GBObject2d &obj) {
  int i, j;
  static double time = 0;
  unsigned char *p=obj.spr.imageData;
  double ttime = sin(time)*256;
  for(i=0; i<obj.dest.width; i++) {
    for(j=0; j<obj.dest.height; j++) {
      *p++=i%256; // blue
      *p++=j%256; // green
      *p++=ttime; // red
      p++;
    }
  }
  time+=0.06;
  if (time >= M_PI) time = 0;
  XPutImage(gb.dis, gb.win, gb.context, obj.spr.img, 0, 0, 0, 0, obj.src.width, obj.src.height);
}

