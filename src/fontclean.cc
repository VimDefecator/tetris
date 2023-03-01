#include "font.hh"
#include "args.hh"
#include <fstream>
#include <algorithm>

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"f", "file"},
                         {"x", "except"}}, {}); 
  
  auto filename = std::string(args.get("file"));

  Font font;
  {
    std::ifstream in(filename);
    font.load(in);
  }
  
  auto charsToKeep = args.get("except");
  
  for(int c = 0; c < 128; ++c)
    if(charsToKeep.find(c) == std::string_view::npos)
      font.erase(c);

  {
    std::ofstream out(filename);
    font.store(out);
  }
}
