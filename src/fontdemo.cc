#include "sdlctx.hh"
#include "font.hh"
#include "fontutils.hh"
#include "text.hh"
#include "args.hh"
#include <algorithm>

namespace
{
  std::pair<size_t, size_t> getNumLinesAndMaxSize(std::string_view str)
  {
    size_t numLines = 0, maxSize = 0;

    auto head = std::string_view(),
         tail = str;
    
    while(!tail.empty())
    {
      numLines += 1;

      auto nlpos = tail.find('\n');
      head = tail.substr(0, nlpos);
      tail = nlpos != std::string_view::npos
           ? tail.substr(nlpos + 1)
           : std::string_view();

      if(head.size() > maxSize)
        maxSize = head.size();
    }
    
    return {numLines, maxSize};
  }
}

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
  
  auto [numRows, numCols] = getNumLinesAndMaxSize(text);
  
  auto font = readFontFromFile(file.data());

  Sdl::Context sdl;
  sdl.init(text.data(), scale * font.wid() * numCols, scale * font.hei() * numRows);
  sdl.setColor(Sdl::BLACK);
  sdl.clear();
  
  renderText(sdl, text, font, scale);
  sdl.present();
  
  while(sdl.wait(), sdl.event().type != SDL_QUIT);
}
