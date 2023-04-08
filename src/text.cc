#include "text.hh"
#include <cctype>

void renderText(std::string_view text, RenderTextParams p)
{
  int row = p.skipRows, col = p.skipCols;

  for(auto c : text)
  {
    if(c != '\n')
    {
      auto fontElem = p.font[c];

      for(int x = 0; x < p.font.wid(); ++x)
        for(int y = 0; y < p.font.hei(); ++y)
          if(fontElem[x][y])
            p.sdl.pixArtPut(p.font.wid()*col + x,
                            p.font.hei()*row + y,
                            p.scale,
                            1. + p.pixelOverlap);
            
      col += 1;
    }
    else
    {
      row += 1;
      col = p.skipCols;
    }
  }
}

void renderTextAt(std::string_view text, RenderTextParams p, Sdl::XY pos, bool center/* = false*/)
{
  if(center)
  {
    auto [numRows, numCols] = getNumRowsAndCols(text);

    auto resWid = (numCols + p.skipCols) * p.font.wid() * p.scale;
    auto resHei = (numRows + p.skipRows) * p.font.hei() * p.scale;

    pos -= Sdl::XY{int(resWid / 2), int(resHei / 2)};
  }

  auto wxy = p.sdl.withBaseXY(pos);

  renderText(text, p);
}

std::pair<size_t, size_t> getNumRowsAndCols(std::string_view str)
{
  size_t numRows = 0, numCols = 0;

  auto head = std::string_view(),
       tail = str;
  
  while(!tail.empty())
  {
    numRows += 1;

    auto nlpos = tail.find('\n');
    head = tail.substr(0, nlpos);
    tail = nlpos != std::string_view::npos
         ? tail.substr(nlpos + 1)
         : std::string_view();

    if(head.size() > numCols)
      numCols = head.size();
  }
  
  return {numRows, numCols};
}
