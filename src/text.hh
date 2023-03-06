#pragma once

#include <string>
#include "sdlctx.hh"
#include "font.hh"

void renderText(Sdl::Context &sdl,
                std::string_view text,
                Font &font,
                int scale,
                float pixelOverlap = 0.,
                int skipLines = 0);
