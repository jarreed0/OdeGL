#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define PI 3.1415926535

//! FULLSCREEN FLAG
constexpr int FULLSCREEN = 0x0001;
//! RESIZABLE FLAG
constexpr int RESIZABLE = 0x0010;
//! DEBUG FLAG
constexpr int DEBUG = 0x0100;
//! SCALABLE FLAG
constexpr int SCALABLE = 0x1000;

//! COLOR RED
#define RED 16711680
//! COLOR GREEN
#define GREEN 65280
//! COLOR BLUE
#define BLUE 255
//! COLOR WHITE
#define WHITE 16777215
//! COLOR BLACK
#define BLACK 0
//! COLOR YELLOW
#define YELLOW 16776960

unsigned long RGB(int r, int g, int b);
void RGB(unsigned long &rgb, int r, int g, int b);
//! Color Struct
struct ODEColor {
  //! rgb represents the color
  unsigned long rgb;
  //! Set the color with the r, g, b
  void set(int r, int g, int b) { RGB(rgb, r, g, b); }
  ODEColor &operator=(const ODEColor &c) {
    rgb = c.rgb;
    return *this;
  }
  bool operator==(const ODEColor &c) { return (rgb == c.rgb); }
  bool operator!=(const ODEColor &c) { return (rgb != c.rgb); }
  bool operator>(const ODEColor &c) { return (rgb > c.rgb); }
  bool operator<(const ODEColor &c) { return (rgb < c.rgb); }
  bool operator>=(const ODEColor &c) { return (rgb >= c.rgb); }
  bool operator<=(const ODEColor &c) { return (rgb <= c.rgb); }
  bool operator+(const ODEColor &c) { return (rgb + c.rgb); }
  bool operator-(const ODEColor &c) { return (rgb - c.rgb); }
  bool operator*(const ODEColor &c) { return (rgb * c.rgb); }
  bool operator/(const ODEColor &c) { return (rgb / c.rgb); }
  bool operator%(const ODEColor &c) { return (rgb % c.rgb); }
  ODEColor &operator=(const int i) {
    rgb = i;
    return *this;
  }
  bool operator==(const int i) { return (rgb == i); }
  bool operator!=(const int i) { return (rgb != i); }
  bool operator>(const int i) { return (rgb > i); }
  bool operator<(const int i) { return (rgb < i); }
  bool operator>=(const int i) { return (rgb >= i); }
  bool operator<=(const int i) { return (rgb <= i); }
  bool operator+(const int i) { return (rgb + i); }
  bool operator-(const int i) { return (rgb - i); }
  bool operator*(const int i) { return (rgb * i); }
  bool operator/(const int i) { return (rgb / i); }
  bool operator%(const int i) { return (rgb % i); }
};
//! Adjust all r,g,bs in a color with an offset
ODEColor shade(ODEColor color, int s) {
  ODEColor shaded;
  long int c = color.rgb;
  double r = c / (pow(256, 2));
  double g = (c / 256) % 256;
  double b = c % 256;
  RGB(shaded.rgb, r, g, b);

  return shaded;
}
//! Generate color with r,g,b
unsigned long RGB(int r, int g, int b) { return b + (g << 8) + (r << 16); }
//! Generate and set a color with r,g,b
void RGB(unsigned long &rgb, int r, int g, int b) { rgb = RGB(r, g, b); }

bool attributes;

//! Window
struct ODEWindow {
  Display *dis;
  int screen;
  Window win;
  GC context;
  ODEColor bkg, draw, font, wire, face;
  //! Width and Height of window
  int Width, Height;
  XWindowAttributes wnd;
  Visual *vis;
  //! Update Window with attributes
  void update() {
    if (attributes) {
      XGetWindowAttributes(dis, win, &wnd);
      Width = wnd.width;
      Height = wnd.height;
    }
  }
};

//! Struct for string a 4x4 matrix
struct matrix {
  float m[4][4] = {0};
};

//! 3D Coordinate
struct coord3d {
  float x, y, z;
};

//! Multiply two matrices with input, ouput, multiply
void MultiplyMatrixVector(coord3d &i, coord3d &o, matrix &m) {
  o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
  o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
  o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
  float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

  if (w != 0.0f) {
    o.x /= w;
    o.y /= w;
    o.z /= w;
  }
}

//! Triangle, has 3 3d coordinates
struct triangle {
  coord3d c[3];
};

template <typename Out>
void split(const std::string &s, char delim, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) {
    *result++ = item;
  }
}

//! Mesh, stores 3d faces and coordinates
struct mesh {
  //! If the mesh only has one face, set to 1
  bool plane = 0;
  std::vector<triangle> tris;
  //! If enabled, uses meshes colors; if 0, uses defualt colors
  bool enable_colors = 0;
  ODEColor wire, face;
  //! Set wireframe and face colors, enables colors
  void setColors(int w, int f) {
    enable_colors = 1;
    wire = w;
    face = f;
  }
  //! If the object should be drawn to the screen
  bool visible = 1;

  //! Load in an .obj
  bool loadObj(std::string fn) {
    std::ifstream in(fn);
    if (!in.is_open()) return false;
    std::string str;
    std::vector<coord3d> verts;
    std::vector<std::string> file;
    while (getline(in, str)) {
      if (str.size() > 0) {
        file.push_back(str);
      }
    }
    for (int i = 0; i < file.size(); i++) {
      std::vector<std::string> r;
      split(file[i], ' ', back_inserter(r));
      if (r[0] == "v") {
        coord3d v;
        v.x = stoi(r[1]);
        v.y = stoi(r[2]);
        v.z = stoi(r[3]);
        verts.push_back(v);
      }
      if (r[0] == "f") {
        int f[3];
        f[0] = stoi(r[1]);
        f[1] = stoi(r[2]);
        f[2] = stoi(r[3]);
        tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1]});
      }
    }
    return true;
  }
};

//! Create a plane with width and height
mesh createPlane(float w, float h) {
  mesh tmp;
  tmp.plane = 1;
  tmp.tris = {
      {0.0f, 0.0f, 0.0f, 0.0f, h, 0.0f, w, h, 0.0f},
      {0.0f, 0.0f, 0.0f, w, h, 0.0f, w, 0.0f, 0.0f},
  };
  return tmp;
}
//! Create plane with width, height, x and y location
mesh createPlane(float w, float h, float x, float y) {
  mesh tmp;
  tmp.tris = {
      {0.0f + x, 0.0f + y, 0.0f, 0.0f + x, h + y, 0.0f, w + x, h + y, 0.0f},
      {0.0f + x, 0.0f + y, 0.0f, w + x, h + y, 0.0f, w + x, 0.0f + y, 0.0f},
  };
  return tmp;
}
//! Create a 3d rectangle with width, height, and z location
mesh createRect(float w, float h, float z) {
  mesh tmp;
  tmp.tris = {

      // SOUTH
      {0.0f, 0.0f, 0.0f, 0.0f, h, 0.0f, w, h, 0.0f},
      {0.0f, 0.0f, 0.0f, w, h, 0.0f, w, 0.0f, 0.0f},

      // EAST
      {w, 0.0f, 0.0f, w, h, 0.0f, w, h, z},
      {w, 0.0f, 0.0f, w, h, z, w, 0.0f, z},

      // NORTH
      {w, 0.0f, z, w, h, z, 0.0f, h, z},
      {w, 0.0f, z, 0.0f, h, z, 0.0f, 0.0f, z},

      // WEST
      {0.0f, 0.0f, z, 0.0f, h, z, 0.0f, h, 0.0f},
      {0.0f, 0.0f, z, 0.0f, h, 0.0f, 0.0f, 0.0f, 0.0f},

      // TOP
      {0.0f, h, 0.0f, 0.0f, h, z, w, h, z},
      {0.0f, h, 0.0f, w, h, z, w, h, 0.0f},

      // BOTTOM
      {w, 0.0f, z, 0.0f, 0.0f, z, 0.0f, 0.0f, 0.0f},
      {w, 0.0f, z, 0.0f, 0.0f, 0.0f, w, 0.0f, 0.0f},

  };
  return tmp;
}
//! Create a 3d cube with size
mesh createCube(float s) { return createRect(s, s, 0); }
//! Create a 3d traingle (flat) with size
mesh createTriangle(float s) {
  mesh tmp;
  tmp.tris = {
      {0.0f, 0.0f, 0.0f, -s, s, 0.0f, s, s, 0.0f},
  };
  return tmp;
}
//! Shift the mesh on the x axis
void shiftMeshOnX(mesh &m, float x) {
  for (int i = 0; i < m.tris.size(); i++) {
    for (int k = 0; k < 3; k++) {
      m.tris[i].c[k].x += x;
    }
  }
}
//! Shift the mesh on the y axis
void shiftMeshOnY(mesh &m, float y) {
  for (int i = 0; i < m.tris.size(); i++) {
    for (int k = 0; k < 3; k++) {
      m.tris[i].c[k].y += y;
    }
  }
}
//! Shift the mesh on the z axis
void shiftMeshOnZ(mesh &m, float z) {
  for (int i = 0; i < m.tris.size(); i++) {
    for (int k = 0; k < 3; k++) {
      m.tris[i].c[k].z += z;
    }
  }
}
//! Set default foreground color with window and color
void setForeground(ODEWindow &ode, ODEColor color) {
  XSetForeground(ode.dis, ode.context, color.rgb);
}
//! Set default drawing color with window and color
void setDrawColor(ODEWindow &ode, unsigned long color) { ode.draw = color; }
//! Set default wireframe drawing color with window and color
void setWireColor(ODEWindow &ode, unsigned long color) { ode.wire = color; }
//! Set default face drawing color with window and color
void setFaceColor(ODEWindow &ode, unsigned long color) { ode.face = color; }
//! Draw and fill in a 2d rect with window, x and y location, width and height
void drawFilledRect(ODEWindow &ode, int x, int y, int w, int h) {
  setForeground(ode, ode.draw);
  XFillRectangle(ode.dis, ode.win, ode.context, x, y, w, h);
}
//! Set font color with window, r,g,b
void setFontColor(ODEWindow &ode, int r, int g, int b) {
  ODEColor color;
  color.set(r, g, b);
  ode.font = color;
}
//! Set font color with window and color
void setFontColor(ODEWindow &ode, unsigned long color) { ode.font = color; }
//! Set font color with window and color
void setFontColor(ODEWindow &ode, ODEColor color) { ode.font = color; }
//! Draw a 2d line with window, start and end points
void drawLine(ODEWindow &win, coord3d a, coord3d b) {
  setForeground(win, win.draw);
  XDrawLine(win.dis, win.win, win.context, a.x, a.y, b.x, b.y);
}
//! Draw a 2d triangle with window and triangle
void drawTriangle(ODEWindow &ode, triangle tr) {
  drawLine(ode, tr.c[0], tr.c[1]);
  drawLine(ode, tr.c[1], tr.c[2]);
  drawLine(ode, tr.c[2], tr.c[0]);
}
//! Draw a filled in 2d traingle with window and triangle
void fillTriangle(ODEWindow &ode, triangle tr) {
  setForeground(ode, ode.draw);
  XPoint p[3];
  XPoint t;
  t.x = tr.c[0].x;
  t.y = tr.c[0].y;
  p[0] = t;
  t.x = tr.c[1].x;
  t.y = tr.c[1].y;
  p[1] = t;
  t.x = tr.c[2].x;
  t.y = tr.c[2].y;
  p[2] = t;
  XFillPolygon(ode.dis, ode.win, ode.context, p, 3, Convex, CoordModeOrigin);
}
//! Fills in the screen
void redraw(ODEWindow &ode) { XClearWindow(ode.dis, ode.win); };
//! Set defualt draw color with window, r,g,b
void setDrawColor(ODEWindow &ode, int r, int g, int b) {
  ODEColor color;
  color.set(r, g, b);
  ode.draw = color;
}
//! Set defualt draw color with window and color
void setDrawColor(ODEWindow &ode, ODEColor color) { ode.draw = color; }
//! Fills in window with set background color, pass in window
void drawBackground(ODEWindow &ode) {
  ODEColor o = ode.draw;
  setDrawColor(ode, ode.bkg);
  drawFilledRect(ode, 0, 0, ode.Width, ode.Height);
  setDrawColor(ode, o);
}
//! Camera coordinate
coord3d camera;
//! Matrix used for calculating 3d projections
matrix matProj;

//! Draw a mesh with window and mesh
void drawMesh(ODEWindow ode, mesh m) {
  if (m.visible) {
    for (auto tri : m.tris) {
      triangle triProjected, triTranslated;

      // Offset into the screen
      triTranslated = tri;
      triTranslated.c[0].z = tri.c[0].z + 3.0f;
      triTranslated.c[1].z = tri.c[1].z + 3.0f;
      triTranslated.c[2].z = tri.c[2].z + 3.0f;

      coord3d normal, line1, line2;
      line1.x = triTranslated.c[1].x - triTranslated.c[0].x;
      line1.y = triTranslated.c[1].y - triTranslated.c[0].y;
      line1.z = triTranslated.c[1].z - triTranslated.c[0].z;

      line2.x = triTranslated.c[2].x - triTranslated.c[0].x;
      line2.y = triTranslated.c[2].y - triTranslated.c[0].y;
      line2.z = triTranslated.c[2].z - triTranslated.c[0].z;

      normal.x = line1.y * line2.z - line1.z * line2.y;
      normal.y = line1.z * line2.x - line1.x * line2.z;
      normal.z = line1.x * line2.y - line1.y * line2.x;
      float l = sqrtf(normal.x * normal.x + normal.y * normal.y +
                      normal.z * normal.z);
      normal.x /= l;
      normal.y /= l;
      normal.z /= l;
      float inview = (normal.x * (triTranslated.c[0].x - camera.x)) +
                     (normal.y * (triTranslated.c[0].y - camera.y)) +
                     (normal.z * (triTranslated.c[0].z - camera.z));

      if (inview < 0.0f) {
        coord3d light = {0.0f, 0.0f, -1.0f};
        float l =
            sqrtf(light.x * light.x + light.y * light.y + light.z * light.z);
        light.x /= l;
        light.y /= l;
        light.z /= l;
        float dp = normal.x * light.x + normal.y * light.y + normal.z * light.z;
        ODEColor c;

        // Project triangles from 3D --> 2D
        MultiplyMatrixVector(triTranslated.c[0], triProjected.c[0], matProj);
        MultiplyMatrixVector(triTranslated.c[1], triProjected.c[1], matProj);
        MultiplyMatrixVector(triTranslated.c[2], triProjected.c[2], matProj);
        // Scale into view
        triProjected.c[0].x += 1.0f;
        triProjected.c[0].y += 1.0f;
        triProjected.c[1].x += 1.0f;
        triProjected.c[1].y += 1.0f;
        triProjected.c[2].x += 1.0f;
        triProjected.c[2].y += 1.0f;
        triProjected.c[0].x *= 0.5f * (float)ode.Width;
        triProjected.c[0].y *= 0.5f * (float)ode.Height;
        triProjected.c[1].x *= 0.5f * (float)ode.Width;
        triProjected.c[1].y *= 0.5f * (float)ode.Height;
        triProjected.c[2].x *= 0.5f * (float)ode.Width;
        triProjected.c[2].y *= 0.5f * (float)ode.Height;

        if (m.enable_colors) {
          c = shade(m.face, dp);
        } else {
          c = shade(ode.face, dp);
        }
        // Rasterize triangle
        setDrawColor(ode, c);
        fillTriangle(ode, triProjected);
        setDrawColor(ode, ode.wire);
        if (m.enable_colors) setDrawColor(ode, m.wire);
        drawTriangle(ode, triProjected);
      }
    }
  }
}
//! Set background color with window and color
void setBackground(ODEWindow &ode, unsigned long color) { ode.bkg = color; }

//! Scene, stores meshes and other data
struct ODEScene {
  //! Scene id
  int id;
  //! Vector of all objects in a scene
  std::vector<mesh> objects;
  //! Scenes plane
  mesh plane;
  //! If plane should be drawn
  bool show_plane = 1;
};
//! Add mesh to scene, with scene and mesh, returns mesh id
int addToScene(ODEScene &scene, mesh m) {
  scene.objects.push_back(m);
  int id = scene.objects.size() - 1;
  return id;
}
//! Add mesh to scene, with scene and .obj file, returns mesh id
int addToScene(ODEScene &scene, std::string file) {
  mesh tmp;
  bool test = tmp.loadObj(file);
  int id = -1;
  if (test) {
    scene.objects.push_back(tmp);
    id = scene.objects.size() - 1;
  }
  return test;
}
//! Create a cube into a scene with scene and cube size, returns mesh id
int addCubeToScene(ODEScene &scene, int s) {
  scene.objects.push_back(createCube(s));
  int id = scene.objects.size() - 1;
  return id;
}
//! Create a 3d rectangle into a scene with scene and width/height and z
//! location, returns mesh id
int addRectToScene(ODEScene &scene, int w, int h, int z) {
  scene.objects.push_back(createRect(w, h, z));
  int id = scene.objects.size() - 1;
  return id;
}
//! Render the scene with window and scene
void renderScene(ODEWindow ode, ODEScene s) {
  if (s.show_plane) drawMesh(ode, s.plane);
  for (int i = 0; i < s.objects.size(); i++) {
    drawMesh(ode, s.objects[i]);
  }
}
//! Init the application, with window, window title, window width and height,
//! and flags
void initODE(ODEWindow &tmp, const char *title, int width, int height,
             const int flags) {
  attributes = 0;
  if (flags & FULLSCREEN) {
    title = "Fullscreen";
  }
  if (flags & DEBUG) {
    title = "Debug Mode";
  }
  tmp.Width = width;
  tmp.Height = height;
  tmp.dis = XOpenDisplay((char *)0);
  tmp.vis = DefaultVisual(tmp.dis, 0);
  tmp.screen = DefaultScreen(tmp.dis);
  setBackground(tmp, WHITE);
  setDrawColor(tmp, BLACK);
  setFontColor(tmp, BLACK);
  tmp.win = XCreateSimpleWindow(tmp.dis, DefaultRootWindow(tmp.dis), 0, 0,
                                width, height, 5, tmp.draw.rgb, tmp.bkg.rgb);
  XSetStandardProperties(tmp.dis, tmp.win, title, title, None, NULL, 0, NULL);
  XSelectInput(tmp.dis, tmp.win, ExposureMask | ButtonPressMask | KeyPressMask);
  tmp.context = XCreateGC(tmp.dis, tmp.win, 0, 0);
  XClearWindow(tmp.dis, tmp.win);
  XMapRaised(tmp.dis, tmp.win);
  if (flags & RESIZABLE) {
    attributes = 1;
  } else {
    XSizeHints *hints = XAllocSizeHints();
    hints->flags = PMinSize | PMaxSize;
    hints->min_width = hints->max_width = tmp.Width;
    hints->min_height = hints->max_height = tmp.Height;
    XSetWMNormalHints(tmp.dis, tmp.win, hints);
    XSetWMSizeHints(tmp.dis, tmp.win, hints, PMinSize | PMaxSize);
  }
  XClassHint *class_hint = XAllocClassHint();
  if (class_hint) {
    class_hint->res_name = class_hint->res_class = (char *)title;
    XSetClassHint(tmp.dis, tmp.win, class_hint);
    XFree(class_hint);
  }
  float fNear = 0.1f;
  float fFar = 1000.0f;
  float fFov = 90.0f;
  float fAspectRatio = (float)tmp.Height / (float)tmp.Width;
  float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

  matProj.m[0][0] = fAspectRatio * fFovRad;
  matProj.m[1][1] = fFovRad;
  matProj.m[2][2] = fFar / (fFar - fNear);
  matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
  matProj.m[2][3] = 1.0f;
  matProj.m[3][3] = 0.0f;
}
//! Call to properly close the window
void close(ODEWindow &ode) {
  XFreeGC(ode.dis, ode.context);
  XDestroyWindow(ode.dis, ode.win);
  XCloseDisplay(ode.dis);
  exit(1);
};
