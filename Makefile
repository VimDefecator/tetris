CXX := clang++
CXXFLAGS := -std=c++20 `sdl2-config --cflags`
CPPFLAGS := -MMD -MP
LDFLAGS := -lpthread `sdl2-config --libs`

ALLDIRS := common
ALLUNITS := tetris fontedit fontpad fontclean fontdemo sdlctx font text common/args

TARGETS := tetris fontedit fontpad fontclean fontdemo
UNITS_tetris := tetris sdlctx font text common/args
UNITS_fontedit := fontedit font sdlctx common/args
UNITS_fontpad := fontpad font common/args
UNITS_fontclean := fontclean font common/args
UNITS_fontdemo := fontdemo sdlctx font text common/args

include Makefile.template
