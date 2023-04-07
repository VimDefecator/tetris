#include "sdlctx.hh"
#include "font.hh"
#include "fontutils.hh"
#include "text.hh"
#include "args.hh"
#include <algorithm>

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"f", "file"},
                         {"t", "text"},
                         {"s", "scale"}}, {{"u", "uppercase"}});

  auto file = args.get("file");
  auto scale = args.getIntO("scale").value_or(1);

  auto textArg = args.getO("text");
  auto text = textArg
            ? std::string(*textArg)
            : std::string(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
  
  if(args.is("uppercase"))
    for(auto &c : text)
      c = toupper(c);
  
  auto [numRows, numCols] = getNumRowsAndCols(text);
  
  auto font = readFontFromFile(file.data());

  Sdl::Context sdl;
  sdl.init(text.data(), scale * font.wid() * numCols, scale * font.hei() * numRows);
  sdl.setColor(Sdl::BLACK);
  sdl.clear();
  
  sdl.setColor(Sdl::WHITE);
  renderText(text, {.sdl = sdl, .font = font, .scale = scale});
  sdl.present();
  
  while(sdl.wait(), sdl.event().type != SDL_QUIT);
}
