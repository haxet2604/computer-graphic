#include <stdio.h>
#include <math.h>

#include <iostream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "climits"

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

class Pentagon
{
public:
    Vec p0, p1, p2, p3, p4;
    Pentagon(Vec p0, Vec p1, Vec p2, Vec p3, Vec p4)
            : p0(p0), p1(p1), p2(p2), p3(p3), p4(p4) {}

    bool isValid()
    {
        Vec v01 = p1 - p0;
        Vec v12 = p2 - p1;
        Vec v23 = p3 - p2;
        Vec v34 = p4 - p3;
        Vec v40 = p0 - p4;

        if (v01.len() == 0 || v12.len() == 0 || v23.len() == 0 || v34.len() == 0 || v40.len() == 0)
            return false;

        if (v01.getcos(v12) == 1 || v12.getcos(v23) == 1 || v23.getcos(v34) == 1 || v34.getcos(v40) == 1 || v40.getcos(v01) == 1)
            return false;

        return true;
    }

    void scale(float scaleFactor)
    {
        const Vec center = (p0 + p1 + p2 + p3 + p4) * 0.2;
        p0 = (p0 - center) * scaleFactor + center;
        p1 = (p1 - center) * scaleFactor + center;
        p2 = (p2 - center) * scaleFactor + center;
        p3 = (p3 - center) * scaleFactor + center;
        p4 = (p4 - center) * scaleFactor + center;
    }

    void rotate(float angle)
    {
        const Vec center = (p0 + p1 + p2 + p3 + p4) * 0.2;
        const float cosAngle = cos(angle);
        const float sinAngle = sin(angle);

        move(-center.x, -center.y);

        p0 = Vec(p0.x * cosAngle - p0.y * sinAngle, p0.x * sinAngle + p0.y * cosAngle);
        p1 = Vec(p1.x * cosAngle - p1.y * sinAngle, p1.x * sinAngle + p1.y * cosAngle);
        p2 = Vec(p2.x * cosAngle - p2.y * sinAngle, p2.x * sinAngle + p2.y * cosAngle);
        p3 = Vec(p3.x * cosAngle - p3.y * sinAngle, p3.x * sinAngle + p3.y * cosAngle);
        p4 = Vec(p4.x * cosAngle - p4.y * sinAngle, p4.x * sinAngle + p4.y * cosAngle);

        move(center.x, center.y);
    }


    void move(float dx, float dy)
    {
        p0.x += dx;
        p0.y += dy;
        p1.x += dx;
        p1.y += dy;
        p2.x += dx;
        p2.y += dy;
        p3.x += dx;
        p3.y += dy;
        p4.x += dx;
        p4.y += dy;
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
        WhitePixel(display, screen),
        WhitePixel(display, screen));
    XSetStandardProperties(display, window, "Lab1", "lab1", None, NULL, 0, NULL);
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);
    gc = XCreateGC(display, window, 0, 0);
    XSetForeground(display, gc, BlackPixel(display, screen));
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
    bool isInsidePentagon(int x, int y, const Pentagon& pentagon)
    {
        const Vec* vertices[5] = {&pentagon.p0, &pentagon.p1, &pentagon.p2, &pentagon.p3, &pentagon.p4};
        int intersections = 0;

        for (int i = 0; i < 5; ++i)
        {
            const Vec& p1 = *vertices[i];
            const Vec& p2 = *vertices[(i + 1) % 5];

            if ((p1.y < y && p2.y >= y) || (p2.y < y && p1.y >= y))
            {
                float x_intercept = static_cast<float>(y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;

                if (x_intercept < x)
                {
                    ++intersections;
                }
            }
        }

        return (intersections % 2) != 0;
    }

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

    void drawPentagon(XInfo& xinfo, const Pentagon& pentagon)
    {
        this->drawLineBresenham(xinfo, pentagon.p1.x, pentagon.p1.y, pentagon.p2.x, pentagon.p2.y);
        this->drawLineBresenham(xinfo, pentagon.p0.x, pentagon.p0.y, pentagon.p1.x, pentagon.p1.y);
        this->drawLineBresenham(xinfo, pentagon.p2.x, pentagon.p2.y, pentagon.p3.x, pentagon.p3.y);
        this->drawLineBresenham(xinfo, pentagon.p3.x, pentagon.p3.y, pentagon.p4.x, pentagon.p4.y);
        this->drawLineBresenham(xinfo, pentagon.p4.x, pentagon.p4.y, pentagon.p0.x, pentagon.p0.y);
    }
};

void fillPentagonSolid(XInfo& xinfo, Drawing drawing, const Pentagon& pentagon)
{
    int x_min = INT_MAX;
    int x_max = INT_MIN;
    int y_min = INT_MAX;
    int y_max = INT_MIN;

    const Vec* vertices[5] = {&pentagon.p0, &pentagon.p1, &pentagon.p2, &pentagon.p3, &pentagon.p4};

    for (int i = 0; i < 5; ++i)
    {
        const Vec& p1 = *vertices[i];
        const Vec& p2 = *vertices[(i + 1) % 5];

        if (p1.y < p2.y)
        {
            for (int y = p1.y; y <= p2.y; ++y)
            {
                float x = static_cast<float>(y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;

                if (x < x_min)
                    x_min = static_cast<int>(x);
                if (x > x_max)
                    x_max = static_cast<int>(x);
                if (y < y_min)
                    y_min = y;
                if (y > y_max)
                    y_max = y;
            }
        }
        else
        {
            for (int y = p2.y; y <= p1.y; ++y)
            {
                float x = static_cast<float>(y - p2.y) * (p1.x - p2.x) / (p1.y - p2.y) + p2.x;

                if (x < x_min)
                    x_min = static_cast<int>(x);
                if (x > x_max)
                    x_max = static_cast<int>(x);
                if (y < y_min)
                    y_min = y;
                if (y > y_max)
                    y_max = y;
            }
        }
    }

    for (int y = y_min; y <= y_max; ++y)
    {
        bool inside = false;
        float x_intercept = 0.0f;

        for (int x = x_min; x <= x_max; ++x)
        {
            if (drawing.isInsidePentagon(x, y, pentagon))
            {
                if (!inside)
                {
                    inside = true;
                    x_intercept = static_cast<float>(x);
                }
            }
            else
            {
                if (inside)
                {
                    inside = false;
                    drawing.drawLineBresenham(xinfo, x_intercept, y, static_cast<float>(x) - 1, y);
                }
            }
        }

        if (inside)
        {
            drawing.drawLineBresenham(xinfo, x_intercept, y, static_cast<float>(x_max), y);
        }
    }
}



void handleKeyPress(XInfo &xinfo, XEvent &event, Pentagon &pentagon)
{
    constexpr float MOVE = 10.0f;
    constexpr float SCALE = 0.9f;
    constexpr float TINY_ANGLE = 0.1f;
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
        case XK_q:
            XCloseDisplay(xinfo.display);
            exit(0);
        case XK_Left:
            pentagon.move(-MOVE, 0);
            break;
        case XK_Right:
            pentagon.move(MOVE, 0);
            break;
        case XK_Up:
            pentagon.move(0, -MOVE);
            break;
        case XK_Down:
            pentagon.move(0, MOVE);
            break;
        case XK_plus:
            pentagon.scale(SCALE);
            break;
        case XK_minus:
            pentagon.scale(1.0f/SCALE);
            break;
        case XK_r:
            pentagon.rotate(TINY_ANGLE);
            break;
        case XK_R:
            pentagon.rotate(-TINY_ANGLE);
            break;
    }
}

int main()
{
  XInfo xinfo;
  Pentagon pentagon(
          Vec(100, 100),
          Vec(200, 100),
          Vec(300, 200),
          Vec(200, 400),
          Vec(150, 400)
          );

  Drawing drawing;

  while (true)
  {
    if (XPending(xinfo.display) > 0)
    {
      XEvent event;
      XNextEvent(xinfo.display, &event);
      switch (event.type)
      {
      case KeyPress:
          handleKeyPress(xinfo, event, pentagon);
          break;
      }
    }

    XSetWindowBackground(xinfo.display, xinfo.window, WhitePixel(xinfo.display, xinfo.screen));
    XClearWindow(xinfo.display, xinfo.window);

    drawing.drawPentagon(xinfo, pentagon);
    fillPentagonSolid(xinfo, drawing,  pentagon);

    XFlush(xinfo.display);

    usleep(10000);
  }
}