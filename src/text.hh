#pragma once

#include <string>
#include "sdlctx.hh"
#include "font.hh"

enum class HAlign { Left, Right, Center };
enum class VAlign { Up, Down, Center };

struct TextRenderParams
{
  Sdl::Context &sdl;
  Font &font;
  int scale = 1;
  int skipCols = 0;
  int skipRows = 0;
  float pixelOverlap = 0.;
};

struct TextPositionParams
{
  Sdl::XY pos = {0, 0};
  HAlign hAlign = HAlign::Left;
  VAlign vAlign = VAlign::Up;
};

void renderText(std::string_view text, TextRenderParams p);
void renderTextAt(std::string_view text, TextRenderParams rp, TextPositionParams pp);

std::pair<size_t, size_t> getNumRowsAndCols(std::string_view str);
