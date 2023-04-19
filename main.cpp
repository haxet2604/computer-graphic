#include <stdio.h>
#include <math.h>

#include <iostream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define abs(x) ((x) < 0 ? -(x) : (x))
#define max(a, b) (a) > (b) ? a : b

class Vec
{
public:
  float x, y;
  Vec() {}
  Vec(float x, float y) : x(x), y(y) {}

  Vec operator+(Vec const &v2) const
  {
    return Vec(x + v2.x, y + v2.y);
  }
  Vec operator-(Vec const &v2) const
  {
    return Vec(x - v2.x, y - v2.y);
  }
  Vec operator*(float const k) const
  {
    return Vec(x * k, y * k);
  }
  float dot(Vec const &v2) const
  {
    return x * v2.x + y * v2.y;
  }
  float lensq() const
  {
    return dot(*this);
  }
  float len() const
  {
    return sqrt(lensq());
  }
  float getcos(Vec const &v2) const
  {
    return dot(v2) / sqrt(lensq() * v2.lensq());
  }
};

class Line
{
public:
  Vec p0, p1;
  Line() {}
  Line(Vec p0_, Vec p1_) : p0(p0_), p1(p1_) {}

  void move(Vec v)
  {
    p0 = p0 + v;
    p1 = p1 + v;
  }

  void print()
  {
    std::cout << "(" << p0.x << "; " << p0.y << ")(" << p1.x << "; " << p1.y << ")";
  }

  void scale(float v)
  {
    const Vec middle = (p1 + p0) * 0.5;
    p0 = (p0 - middle) * v + middle;
    p1 = (p1 - middle) * v + middle;
  }

  void rotateTinyAngle(float angle)
  {
    const Vec middle = (p1 + p0) * 0.5;
    Vec v0 = p0 - middle;
    Vec v1 = p1 - middle;
    const float _sin = sin(angle / 180 * M_PI);
    const float _cos = cos(angle / 180 * M_PI);
    Vec temp_v = v0;
    v0.x = temp_v.x * _cos - temp_v.y * _sin;
    v0.y = temp_v.x * _sin + temp_v.y * _cos;
    p0 = v0 + middle;
    temp_v = v1;
    v1.x = temp_v.x * _cos - temp_v.y * _sin;
    v1.y = temp_v.x * _sin + temp_v.y * _cos;
    p1 = v1 + middle;
  }

  void rotate(float angle)
  {
    const Vec middle = (p1 + p0) * 0.5;
    Vec v0 = p0 - middle;
    Vec v1 = p1 - middle;
    const float _cos = cos(angle);
    const float _sin = sin(angle);
    Vec temp_v = v0;
    v0.x = temp_v.x * _cos - temp_v.y * _sin;
    v0.y = temp_v.x * _sin + temp_v.y * _cos;
    p0 = v0 + middle;
    temp_v = v1;
    v1.x = temp_v.x * _cos - temp_v.y * _sin;
    v1.y = temp_v.x * _sin + temp_v.y * _cos;
    p1 = v1 + middle;
  }
};

class XInfo
{
public:
  Display *display;
  int screen;
  Window window;
  GC gc;
  int x11_fd;
  fd_set in_fds;
  XInfo()
  {
    display = XOpenDisplay((char *)0);
    screen = DefaultScreen(display);
    window = XCreateSimpleWindow(
        display,
        DefaultRootWindow(display),
        0, 0, 400, 600, 5,
        BlackPixel(display, screen),
        WhitePixel(display, screen));
    XSetStandardProperties(display, window, "Lab1", "lab1", None, NULL, 0, NULL);
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);
    gc = XCreateGC(display, window, 0, 0);
    XSetBackground(display, gc, BlackPixel(display, screen));
    XSetForeground(display, gc, WhitePixel(display, screen));
    XMapRaised(display, window);
    x11_fd = ConnectionNumber(display);
  }
  ~XInfo()
  {
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
  }
};

class Drawing
{
public:
  void drawLineBresenham(XInfo &xinfo, int x0, int y0, int x1, int y1)
  {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
      XDrawPoint(xinfo.display, xinfo.window, xinfo.gc, x0, y0);
      if (x0 == x1 && y0 == y1)
        break;

      int e2 = 2 * err;
      if (e2 > -dy)
      {
        err -= dy;
        x0 += sx;
      }
      if (e2 < dx)
      {
        err += dx;
        y0 += sy;
      }
    }
  }
};

void handleKeyPress(XInfo &xinfo, XEvent &event, Line &line)
{
  constexpr float MOVE = 10.0f;
  constexpr float SCALE = 0.9f;
  constexpr float TINY_ANGLE = 1.0f;
  constexpr float ANGLE = 1.0f;
  constexpr int BufferSize = 100;

  KeySym key;
  char text[BufferSize];

  int i = XLookupString((XKeyEvent *)&event, text, BufferSize, &key, nullptr);

  if (i == 1 && text[0] == 'q')
  {
    XCloseDisplay(xinfo.display);
    exit(0);
  }

  switch (key)
  {
  case XK_Up:
    line.move(Vec(0.0f, -MOVE));
    break;
  case XK_Down:
    line.move(Vec(0.0f, MOVE));
    break;
  case XK_Left:
    line.move(Vec(-MOVE, 0.0f));
    break;
  case XK_Right:
    line.move(Vec(MOVE, 0.0f));
    break;
  case XK_equal:
    line.scale(SCALE);
    break;
  case XK_minus:
    line.scale(1.0f / SCALE);
    break;
  case XK_period:
    line.rotateTinyAngle(TINY_ANGLE);
    break;
  case XK_comma:
    line.rotateTinyAngle(-TINY_ANGLE);
    break;
  case XK_q:
    XCloseDisplay(xinfo.display);
    exit(0);
    break;
  case XK_r:
    line.rotate(-ANGLE);
    break;
  case XK_R:
    line.rotate(ANGLE);
    break;
  }
}

int main()
{
  XInfo xinfo;
  Line lineOne(Vec(100, 100), Vec(200, 200));
  Line lineTwo(Vec(200, 100), Vec(150, 100));
  Drawing drawing;
  int lineNumber = 0;

  while (true)
  {
    if (XPending(xinfo.display) > 0)
    {
      XEvent event;
      XNextEvent(xinfo.display, &event);
      switch (event.type)
      {
      case KeyPress:
        handleKeyPress(xinfo, event, lineNumber == 1 ? lineOne : lineTwo);
        if (event.xkey.keycode == XKeysymToKeycode(xinfo.display, XK_space))
        {
          lineNumber = 1 - lineNumber;
        }
        break;
      }
    }

    XSetWindowBackground(xinfo.display, xinfo.window, BlackPixel(xinfo.display, xinfo.screen));
    XClearWindow(xinfo.display, xinfo.window);

    drawing.drawLineBresenham(xinfo, lineOne.p0.x, lineOne.p0.y, lineOne.p1.x, lineOne.p1.y);
    drawing.drawLineBresenham(xinfo, lineTwo.p0.x, lineTwo.p0.y, lineTwo.p1.x, lineTwo.p1.y);

    XFlush(xinfo.display);

    usleep(10000);
  }
}