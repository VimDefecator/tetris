#include "text.hh"
#include <cctype>

void renderText(Sdl::Context &sdl,
                std::string_view text,
                Font &font,
                int scale,
                float pixelOverlap/* = 0.*/,
                int skipLines/* = 0*/)
{
  int row = skipLines, col = 0;

  for(auto c : text)
  {
    if(c != '\n')
    {
      auto fontElem = font[c];

      for(int x = 0; x < font.wid(); ++x)
        for(int y = 0; y < font.hei(); ++y)
          if(fontElem[x][y])
            sdl.pixArtPut(font.wid()*col + x, font.hei()*row + y, scale, 1. + pixelOverlap);
            
      col += 1;
    }
    else
    {
      row += 1;
      col = 0;
    }
  }
}
