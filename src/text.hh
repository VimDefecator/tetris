#pragma once

#include <string>
#include "sdlctx.hh"
#include "font.hh"

struct RenderTextParams
{
  Sdl::Context &sdl;
  Font &font;
  int scale = 1;
  int skipLines = 0;
  float pixelOverlap = 0.;
};

void renderText(std::string_view text, RenderTextParams p);
