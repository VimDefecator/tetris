#include "font.hh"
#include "fontutils.hh"
#include "common/args.hh"
#include <fstream>
#include <algorithm>

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"i", "in"},
                         {"o", "out"},
                         {"l", "left"},
                         {"r", "right"},
                         {"u", "up"},
                         {"d", "down"}}, {}); 
  
  auto ifont = readFontFromFile(args.get("in").data());
  
  auto l = args.getIntO("left").value_or(0);
  auto r = args.getIntO("right").value_or(0);
  auto u = args.getIntO("up").value_or(0);
  auto d = args.getIntO("down").value_or(0);

  auto iw = ifont.wid();
  auto ih = ifont.hei();
  auto ow = iw + l + r;
  auto oh = ih + u + d;
  
  Font ofont;
  ofont.init(ow, oh);

  auto xShift = l;
  auto yShift = u;
  auto fromX = std::max(0, -xShift);
  auto fromY = std::max(0, -yShift);
  auto toX = std::min(iw, ow - xShift);
  auto toY = std::min(ih, oh - yShift);

  for(int c = 0; c < 128; ++c)
    for(int x = fromX; x < toX; ++x)
      for(int y = fromY; y < toY; ++y)
        ofont[c][x+xShift][y+yShift] = bool(ifont[c][x][y]);
  
  writeFontToFile(ofont, args.get("out").data());
}
