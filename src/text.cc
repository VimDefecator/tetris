#include "text.hh"
#include <cctype>

void renderText(std::string_view text, TextRenderParams p)
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

void renderTextAt(std::string_view text, TextRenderParams rp, TextPositionParams pp)
{
  auto [numRows, numCols] = getNumRowsAndCols(text);
  auto resWid = (numCols + rp.skipCols) * rp.font.wid() * rp.scale;
  auto resHei = (numRows + rp.skipRows) * rp.font.hei() * rp.scale;

  switch(pp.hAlign)
  {
    case HAlign::Left: break;
    case HAlign::Right: pp.pos.x -= resWid; break;
    case HAlign::Center: pp.pos.x -= resWid / 2; break;
  }

  switch(pp.vAlign)
  {
    case VAlign::Up: break;
    case VAlign::Down: pp.pos.y -= resHei; break;
    case VAlign::Center: pp.pos.y -= resHei / 2; break;
  }

  auto wxy = rp.sdl.withBaseXY(pp.pos);

  renderText(text, rp);
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
