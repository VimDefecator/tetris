#include "sdlctx.hh"

const Sdl::Color Sdl::BLACK   = {  0,   0,   0},
                 Sdl::GRAY    = { 85,  85,  85},
                 Sdl::WHITE   = {255, 255, 255},
                 Sdl::RED     = {255,   0,   0},
                 Sdl::GREEN   = {  0, 255,   0},
                 Sdl::BLUE    = {  0,   0, 255},
                 Sdl::YELLOW  = {127, 127,   0},
                 Sdl::MAGENTA = {127,   0, 127},
                 Sdl::CYAN    = {  0, 127, 127};

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
  color_ = color;
  auto [r, g, b] = color;
  SDL_SetRenderDrawColor(renderer_, r, g, b, 0xff);
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

