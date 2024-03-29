#pragma once

#include <SDL.h>
#include <tuple>
#include "utils.hh"

namespace Sdl
{
  struct Color
  {
    Uint8 r, g, b;

    Color operator/(int d) const
    {
      auto tmp = *this;
      tmp /= d;
      return tmp;
    }

    Color &operator/=(int d)
    {
      r /= d;
      g /= d;
      b /= d;
      return *this;
    }
  };
  struct XY
  {
    int x, y;

    XY operator+(XY other) const
    {
      auto tmp = *this;
      tmp += other;
      return tmp;
    }

    XY operator-(XY other) const
    {
      auto tmp = *this;
      tmp -= other;
      return tmp;
    }

    XY &operator+=(XY other)
    {
      x += other.x;
      y += other.y;
      return *this;
    }

    XY &operator-=(XY other)
    {
      x -= other.x;
      y -= other.y;
      return *this;
    }
  };
  
  extern const Color BLACK, WHITE, GRAY, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN;
  
  inline Color gray     (Uint8 bri) { return {bri, bri, bri}; }
  inline Color red      (Uint8 bri) { return {bri,   0,   0}; }
  inline Color green    (Uint8 bri) { return {  0, bri,   0}; }
  inline Color blue     (Uint8 bri) { return {  0,   0, bri}; }
  inline Color yellow   (Uint8 bri) { return {bri, bri,   0}; }
  inline Color magenta  (Uint8 bri) { return {bri,   0, bri}; }
  inline Color cyan     (Uint8 bri) { return {  0, bri, bri}; }

  class Context
  {
  public:
    Context() = default;
    ~Context();
    
    void init(const char *title, int w, int h, int scale);
    
    Color getColor();
    void setColor(Color color);

    WithSetTmp<Context, Color> withColor(Color color)
    {
      return WithSetTmp<Context, Color>(this, color, &Context::getColor, &Context::setColor);
    }
    
    XY getBaseXY() { return {baseX_, baseY_}; }
    void setBaseXY(XY xy) { baseX_ = xy.x, baseY_ = xy.y; }
    
    WithSetTmp<Context, XY> withBaseXY(XY xy)
    {
      return WithSetTmp<Context, XY>(this, xy, &Context::getBaseXY, &Context::setBaseXY);
    }

    void clear();
    void fillRect(int x, int y, int w, int h);
    void pixArtPut(int x, int y, int step, float frac = 1.);

    void present();
    
    bool poll();
    bool wait();

    const SDL_Event &event() const { return event_; }
    
    SDL_Window *window() { return window_; }
    SDL_Renderer *renderer() { return renderer_; }

    int wid() const { return wid_; }
    int hei() const { return hei_; }

  private:
    SDL_Window *window_ = nullptr; 
    SDL_Renderer *renderer_ = nullptr;

    SDL_Event event_;

    bool initialized_ = false;
    
    int baseX_ = 0, baseY_ = 0;

    int wid_, hei_;
  };
}
