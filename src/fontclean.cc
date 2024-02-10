#include "font.hh"
#include "fontutils.hh"
#include "common/args.hh"
#include <fstream>
#include <algorithm>

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"f", "file"},
                         {"x", "except"}}, {}); 
  
  auto filename = args.get("file").data();

  auto font = readFontFromFile(filename);
  
  auto charsToKeep = args.get("except");
  
  for(int c = 0; c < 128; ++c)
    if(charsToKeep.find(c) == std::string_view::npos)
      font.erase(c);

  writeFontToFile(font, filename);
}
