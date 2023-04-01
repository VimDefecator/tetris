#pragma once

#include <SDL.h>
#include <tuple>
#include "utils.hh"

namespace Sdl
{
  using Color = std::tuple<Uint8, Uint8, Uint8>;
  struct XY
  {
    int x, y;

    XY operator+(XY other) { return {x + other.x, y + other.y}; }
    XY operator-(XY other) { return {x - other.x, y - other.y}; }

    XY &operator+=(XY other) { x += other.x; y += other.y; return *this; }
    XY &operator-=(XY other) { x -= other.x; y -= other.y; return *this; }
  };
  
  extern const Color BLACK, GRAY, WHITE, RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN;
  
  Color gray(Uint8 bri);

  class Context
  {
  public:
    Context() = default;
    ~Context();
    
    void init(const char *title, int w, int h);
    
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

  private:
    SDL_Window *window_ = nullptr; 
    SDL_Renderer *renderer_ = nullptr;

    SDL_Event event_;
    
    Color color_;

    bool initialized_ = false;
    
    int baseX_ = 0, baseY_ = 0;
  };
}
