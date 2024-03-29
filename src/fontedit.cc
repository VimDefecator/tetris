#include "font.hh"
#include "fontutils.hh"
#include "sdlctx.hh"
#include "common/args.hh"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cctype>

namespace fs = std::filesystem;

class Edit
{
public:
  void init(const Args &args);
  void exec();
  void save();

private:
  Font font_;
  std::string filename_;
  int w_, h_, s_;
  
  bool isNew_;
};

void Edit::init(const Args &args)
{
  filename_ = args.get("file");
  
  isNew_ = !fs::exists(filename_);

  if(isNew_)
    font_.init(args.getInt("width"), args.getInt("height"));
  else
    readFontFromFile(font_, filename_);
  
  w_ = font_.wid();
  h_ = font_.hei();
  s_ = args.getIntO("scale").value_or(1);
}

void Edit::exec()
{
  Sdl::Context sdl;
  sdl.init("Font editor", w_ * 4, h_ * 4, s_);
  sdl.setColor(Sdl::WHITE);
  
  bool isEditMode = false;
  int curCh = 0, curX = 0, curY = 0;

  auto render = [&]
  {
    sdl.withColor(Sdl::BLACK)->clear();
    
    for(int x = 0; x < w_; x++)
      for(int y = 0; y < h_; y++)
        if(font_[curCh][x][y])
          sdl.pixArtPut(x, y, 4);
    
    if(isEditMode)
      sdl.withColor(Sdl::gray(128))->pixArtPut(curX, curY, 4, 0.5);
    
    sdl.present();
  };
  
  SDL_StartTextInput();

  while(sdl.wait() && sdl.event().type != SDL_QUIT)
  {
    render();

    if(!isEditMode)
    {
      if(sdl.event().type == SDL_KEYDOWN)
      {
        auto sym = sdl.event().key.keysym.sym;

        if(sym == SDLK_RETURN)
        {
          isEditMode = true;
        }
        else if(sym == SDLK_DELETE)
        {
          font_.erase(curCh);
        }
        else if(sym == SDLK_ESCAPE)
        {
          break;
        }
      }
      else if(sdl.event().type == SDL_TEXTINPUT)
      {
        curCh = sdl.event().text.text[0];
      }
    }
    else
    {
      if(sdl.event().type == SDL_KEYDOWN)
      {
        int dx = 0, dy = 0;

        switch(sdl.event().key.keysym.sym)
        {
          case SDLK_LEFT:
            dx = -1;
          break;
          case SDLK_RIGHT:
            dx = 1;
          break;
          case SDLK_UP:
            dy = -1;
          break;
          case SDLK_DOWN:
            dy = 1;
          break;
          case SDLK_RETURN:
          case SDLK_SPACE:
          case SDLK_LSHIFT:
            font_[curCh][curX][curY].flip();
          break;
          case SDLK_ESCAPE:
            isEditMode = false;
          break;
        }

        if(dx && 0 <= curX + dx && curX + dx < w_
          || dy && 0 <= curY + dy && curY + dy < h_)
        {
          curX += dx;
          curY += dy;
          if(sdl.event().key.keysym.mod & KMOD_LSHIFT)
            font_[curCh][curX][curY].flip();
        }
      }
    }
  }
  
  SDL_StopTextInput();
}

void Edit::save()
{
  writeFontToFile(font_, filename_);
}

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"f", "file"},
                         {"w", "width"},
                         {"h", "height"},
                         {"s", "scale"}}, {});

  Edit edit;
  edit.init(args);
  edit.exec();
  edit.save();
}
