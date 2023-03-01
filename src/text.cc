#include "text.hh"

void renderText(std::string_view text,
                Font &font,
                Sdl::Context &sdl,
                Sdl::Color color,
                Sdl::XY baseXY,
                int scale)
{
  auto wcl = sdl.withColor(color);
  auto wxy = sdl.withBaseXY(baseXY);

  for(int i = 0; i < text.size(); ++i)
    for(int x = 0; x < font.width(); ++x)
      for(int y = 0; y < font.height(); ++y)
        if(font[text[i]][x][y])
          sdl.pixArtPut(font.width()*i + x, y, scale);
}
