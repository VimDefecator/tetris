#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <functional>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "sdlctx.hh"
#include "font.hh"
#include "fontutils.hh"
#include "text.hh"

static constexpr auto NFIGURES = 7;
static constexpr auto NCOLORS = 6;

struct Figure
{
  Figure(std::initializer_list<std::string_view> input)
  {
    for(int v = 0; v < 4; v++)
      for(int y = 0; y < 4; y++)
        for(int x = 0; x < 4; x++)
          views[v][y][x] = std::data(input)[v][4*y + x] != ' ';
  }

  bool views[4][4][4] = {};
};

static const std::array<Figure, NFIGURES> g_figures
{{
  {
    "00  "
    "00  "
    "    "
    "    ", "00  "
            "00  "
            "    "
            "    ", "00  "
                    "00  "
                    "    "
                    "    ", "00  "
                            "00  "
                            "    "
                            "    "
  },
  {
    "0   "
    "0   "
    "0   "
    "0   ", "0000"
            "    "
            "    "
            "    ", "0   "
                    "0   "
                    "0   "
                    "0   ", "0000"
                            "    "
                            "    "
                            "    "
  },
  {
    "000 "
    " 0  "
    "    "
    "    ", "0   "
            "00  "
            "0   "
            "    ", " 0  "
                    "000 "
                    "    "
                    "    ", " 0  "
                            "00  "
                            " 0  "
                            "    "
  },
  {
    " 00 "
    "00  "
    "    "
    "    ", "0   "
            "00  "
            " 0  "
            "    ", " 00 "
                    "00  "
                    "    "
                    "    ", "0   "
                            "00  "
                            " 0  "
                            "    "
  },
  {
    "00  "
    " 00 "
    "    "
    "    ", " 0  "
            "00  "
            "0   "
            "    ", "00  "
                    " 00 "
                    "    "
                    "    ", " 0  "
                            "00  "
                            "0   "
                            "    "
  },
  {
    "0   "
    "000 "
    "    "
    "    ", " 0  "
            " 0  "
            "00  "
            "    ", "000 "
                    "  0 "
                    "    "
                    "    ", "00  "
                            "0   "
                            "0   "
                            "    "
  },
  {
    "  0 "
    "000 "
    "    "
    "    ", "00  "
            " 0  "
            " 0  "
            "    ", "000 "
                    "0   "
                    "    "
                    "    ", "0   "
                            "0   "
                            "00  "
                            "    "
  },

}};

class Game
{
public:
  void execute(int scale);

private:
  struct Falling
  {
    const Figure *fig;
    int col, v, y, x;
  };

  void spawn();
  void moveLeft();
  void moveRight();
  void turn();
  void skip();
  void land();
  void reduce();
  bool collides();
  const bool (*getFig(Falling &falling))[4];

  void onInit();
  void onEvent(const SDL_Event &event);
  void onLoop();
  void onRender();

private:
  Sdl::Context sdl_;
  Font font_;

  int cellSize_;

  uint8_t cell_[16][8] = {};

  Falling falling_, fallingNext_;

  int clock_ = 0;

  int score_ = 0;

  bool quit_ = false;
};

void Game::spawn()
{
  auto r = rand();

  falling_ = fallingNext_;

  fallingNext_.fig = &g_figures[r % NFIGURES];
  r /= NFIGURES;

  fallingNext_.col = 1 + r % NCOLORS;
  r /= NCOLORS;

  fallingNext_.v = r  % 4;
  r /= 4;

  fallingNext_.y = 0;
  fallingNext_.x = 2;
}

void Game::moveLeft()
{
  if(falling_.x > 0)
  {
    falling_.x--;
    if(collides())
      falling_.x++;
  }
}

void Game::moveRight()
{
  falling_.x++;
  if(collides())
    falling_.x--;
}

void Game::turn()
{
  falling_.v = (falling_.v + 1) % 4;
  if(collides())
    falling_.v = (falling_.v + 3) % 4;
}

void Game::skip()
{
  falling_.y++;

  if(collides())
  {
    clock_ = 0;
  }
  else
  {
    do falling_.y++; while(!collides());
    clock_ = 1;
  }

  falling_.y--;
}

void Game::land()
{
  auto fig = getFig(falling_);

  for(int yRel = 0; yRel < 4; yRel++)
    for(int xRel = 0; xRel < 4; xRel++)
      if(fig[yRel][xRel])
        cell_[falling_.y + yRel][falling_.x + xRel] = falling_.col;
}

void Game::reduce()
{
  int numReduced = 0;

  for(auto line = &cell_[0]; line < &cell_[16]; line++)
  {
    if(std::all_of(&**line, &**(line+1), std::identity()))
    {
      memset(line, 0, sizeof(*line));
      memmove(&cell_[1], &cell_[0], (uint8_t *)line - (uint8_t *)cell_);
      numReduced++;
    }
  }

  score_ += numReduced * numReduced;
}

bool Game::collides()
{
  auto fig = getFig(falling_);

  for(int yRel = 0; yRel < 4; yRel++)
    for(int xRel = 0; xRel < 4; xRel++)
      if(fig[yRel][xRel])
        if(auto y = falling_.y + yRel, x = falling_.x + xRel; y >= 16 || x >= 8 || cell_[y][x])
          return true;

  return false;
}

const bool (*Game::getFig(Falling &falling))[4]
{
  return falling.fig->views[falling.v];
}

void Game::onInit()
{
  sdl_.init("tetris", (8 + 1 + 4) * cellSize_, 16 * cellSize_);
  readFontFromFile(font_, "digits.font");

  srand(time(NULL));
  spawn();
  spawn();
}

void Game::onEvent(const SDL_Event &event)
{
  switch(event.type)
  {
    case SDL_KEYDOWN:
    {
      switch(event.key.keysym.sym)
      {
        case SDLK_LEFT  : moveLeft();  break;
        case SDLK_RIGHT : moveRight(); break;
        case SDLK_UP    : turn();      break;
        case SDLK_DOWN  : skip();      break;

        case 'q': quit_ = true; break;
      }
    }
    break;
    case SDL_QUIT:
    {
      quit_ = true;
    }
    break;
    default:
    {
    }
    break;
  }

}

void Game::onLoop()
{
  if(clock_ == 0)
  {
    falling_.y++;
    if(collides())
    {
      falling_.y--;
      land();
      reduce();
      spawn();
      if(collides())
        quit_ = true;
    }
  }

  clock_ = (clock_ + 1) % 10;
}

void Game::onRender()
{
  sdl_.setColor(Sdl::BLACK);
  sdl_.clear();

  auto idx2col = [](int colIdx) -> Sdl::Color
  {
    switch(colIdx)
    {
      case 1: return Sdl::RED;
      case 2: return Sdl::BLUE;
      case 3: return Sdl::GREEN;
      case 4: return Sdl::YELLOW;
      case 5: return Sdl::MAGENTA;
      case 6: return Sdl::CYAN;

      default: return Sdl::GRAY;
    }
  };

  auto renderCell = [&](int x, int y, int colIdx)
  {
    sdl_.setColor(idx2col(colIdx));
    sdl_.pixArtPut(x, y, cellSize_, 0.875);
  };

  auto renderFalling = [&](Falling &falling, int x, int y)
  {
    auto fig = getFig(falling);

    for(int yRel = 0; yRel < 4; yRel++)
      for(int xRel = 0; xRel < 4; xRel++)
        if(fig[yRel][xRel])
          renderCell(x + xRel, y + yRel, falling.col);
  };

  for(int y = 0; y < 16; y++)
    for(int x = 0; x < 8; x++)
      if(auto col = cell_[y][x])
        renderCell(x, y, col);

  for(int y = 0; y < 16; y++)
    renderCell(8, y, 0);

  renderFalling(falling_, falling_.x, falling_.y);
  renderFalling(fallingNext_, 9, 0);
  
  auto scoreStr = std::to_string(score_);
  if(scoreStr.size() < 4)
    scoreStr = std::string(4 - scoreStr.size(), ' ') + scoreStr;

  renderText(scoreStr,
             font_,
             sdl_,
             Sdl::WHITE,
             {9 * cellSize_, 14 * cellSize_},
             cellSize_ / 4);

  sdl_.present();
}

void Game::execute(int scale)
{
  cellSize_ = scale * 16;

  onInit();

  while(true)
  {
    while(sdl_.poll())
      onEvent(sdl_.event());

    onLoop();
    onRender();

    if(quit_)
    {
      std::cout << "SCORE: " << score_ << '\n';
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

int main(int argc, char **argv)
{
  Game().execute(argc > 1 ? atoi(argv[1]) : 1);
}
