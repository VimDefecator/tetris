#include "font.hh"
#include <fstream>

template<class Path>
inline Font readFontFromFile(Path&& path)
{
  Font font;
  {
    std::ifstream in(path);
    font.load(in);
  }
  return font;
}

template<class Path>
inline void readFontFromFile(Font &font, Path&& path)
{
  std::ifstream in(path);
  font.load(in);
}

template<class Path>
inline void writeFontToFile(const Font &font, Path&& path)
{
  std::ofstream out(path, std::ios::trunc);
  font.store(out);
}
