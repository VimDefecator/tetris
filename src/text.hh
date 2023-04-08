#pragma once

#include <string>
#include "sdlctx.hh"
#include "font.hh"

struct RenderTextParams
{
  Sdl::Context &sdl;
  Font &font;
  int scale = 1;
  int skipCols = 0;
  int skipRows = 0;
  float pixelOverlap = 0.;
};

void renderText(std::string_view text, RenderTextParams p);
void renderTextAt(std::string_view text, RenderTextParams p, Sdl::XY pos, bool center = false);

std::pair<size_t, size_t> getNumRowsAndCols(std::string_view str);
