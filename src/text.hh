#pragma once

#include <string>
#include "sdlctx.hh"
#include "font.hh"

void renderText(std::string_view text,
                Font &font,
                Sdl::Context &sdl,
                Sdl::Color color,
                Sdl::XY baseXY,
                int scale);