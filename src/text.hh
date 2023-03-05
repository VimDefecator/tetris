#pragma once

#include <string>
#include "sdlctx.hh"
#include "font.hh"

void renderText(Sdl::Context &sdl,
                std::string_view text,
                Font &font,
                int scale,
                int skipLines = 0);
