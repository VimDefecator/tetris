#include "text.hh"
#include <cctype>

void renderText(std::string_view text, RenderTextParams p)
{
  int row = p.skipLines, col = 0;

  for(auto c : text)
  {
    if(c != '\n')
    {
      auto fontElem = p.font[c];

      for(int x = 0; x < p.font.wid(); ++x)
        for(int y = 0; y < p.font.hei(); ++y)
          if(fontElem[x][y])
            p.sdl.pixArtPut(p.font.wid()*col + x,
                            p.font.hei()*row + y,
                            p.scale,
                            1. + p.pixelOverlap);
            
      col += 1;
    }
    else
    {
      row += 1;
      col = 0;
    }
  }
}
