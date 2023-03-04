#include "text.hh"
#include <cctype>

void renderText(std::string_view text,
                bool uppercase,
                Font &font,
                Sdl::Context &sdl,
                Sdl::Color color,
                Sdl::XY baseXY,
                int scale)
{
  auto wcl = sdl.withColor(color);
  auto wxy = sdl.withBaseXY(baseXY);

  int row = 0, col = 0;

  for(auto c : text)
  {
    if(c != '\n')
    {
      auto fontElem = font[uppercase ? toupper(c) : int(c)];

      for(int x = 0; x < font.wid(); ++x)
        for(int y = 0; y < font.hei(); ++y)
          if(fontElem[x][y])
            sdl.pixArtPut(font.wid()*col + x, font.hei()*row + y, scale);
            
      col += 1;
    }
    else
    {
      row += 1;
      col = 0;
    }
  }
}
