#include "sdlctx.hh"
#include <cmath>

namespace
{
  Sdl::Color fixLum(Sdl::Color c)
  {
    static const double RW = 0.299, GW = 0.587, BW = 0.114;

    double r = c.r, g = c.g, b = c.b;
    double lumIn = r*r*RW + g*g*GW + b*b*BW;
    double lumOut = 255.*255.*BW;
    double fixFactor = sqrt(lumOut / lumIn);

    return {Uint8(std::min(255., r*fixFactor)),
            Uint8(std::min(255., g*fixFactor)),
            Uint8(std::min(255., b*fixFactor))};
  }
}

const Sdl::Color Sdl::BLACK   = {  0,   0,   0},
                 Sdl::WHITE   = {255, 255, 255},
                 Sdl::GRAY    = fixLum(Sdl::Color{255, 255, 255}),
                 Sdl::RED     = fixLum(Sdl::Color{255,   0,   0}),
                 Sdl::GREEN   = fixLum(Sdl::Color{  0, 255,   0}),
                 Sdl::BLUE    = fixLum(Sdl::Color{  0,   0, 255}),
                 Sdl::YELLOW  = fixLum(Sdl::Color{255, 255,   0}),
                 Sdl::MAGENTA = fixLum(Sdl::Color{255,   0, 255}),
                 Sdl::CYAN    = fixLum(Sdl::Color{  0, 255, 255});

Sdl::Color Sdl::gray(Uint8 bri)
{
  return {bri, bri, bri};
}

void Sdl::Context::init(const char *title, int w, int h)
{
  SDL_Init(SDL_INIT_EVERYTHING);

  window_ = SDL_CreateWindow(title,
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             w,
                             h,
                             SDL_WINDOW_OPENGL);

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  
  initialized_ = true;
}

Sdl::Context::~Context()
{
  if(initialized_)
    SDL_Quit();
}

Sdl::Color Sdl::Context::getColor()
{
  Uint8 r, g, b, a;
  SDL_GetRenderDrawColor(renderer_, &r, &g, &b, &a);
  return {r, g, b};
}

void Sdl::Context::setColor(Color color)
{
  SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, 0xff);
}

void Sdl::Context::clear()
{
  SDL_RenderClear(renderer_);
}

void Sdl::Context::fillRect(int x, int y, int w, int h)
{
  SDL_RenderFillRect(renderer_,
    &(const SDL_Rect &)SDL_Rect{baseX_ + x, baseY_ + y, w, h});
}

void Sdl::Context::pixArtPut(int x, int y, int step, float frac)
{
  int size = frac * step;

  fillRect(x * step + (step - size) / 2,
           y * step + (step - size) / 2,
           size,
           size);
}

void Sdl::Context::present()
{
  SDL_RenderPresent(renderer_);
}

bool Sdl::Context::poll()
{
  return SDL_PollEvent(&event_);
}

bool Sdl::Context::wait()
{
  return SDL_WaitEvent(&event_);
}

