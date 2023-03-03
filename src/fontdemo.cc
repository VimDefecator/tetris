#include "sdlctx.hh"
#include "font.hh"
#include "fontutils.hh"
#include "text.hh"
#include "args.hh"

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"f", "file"},
                         {"t", "text"},
                         {"s", "scale"}}, {});

  auto file = args.get("file");
  auto text = args.get("text");
  auto scale = args.getIntO("scale").value_or(1);
  
  auto font = readFontFromFile(file.data());

  Sdl::Context sdl;
  sdl.init(text.data(), scale * font.wid() * text.size(), scale * font.hei());
  sdl.setColor(Sdl::BLACK);
  sdl.clear();
  
  renderText(text, font, sdl, Sdl::WHITE, {0, 0}, scale);
  sdl.present();
  
  while(sdl.wait(), sdl.event().type != SDL_QUIT);
}
