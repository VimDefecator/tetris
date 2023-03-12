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
#include "args.hh"

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
  void execute(int scale, std::string_view currentName, bool help);

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

  void init(int scale);
  void showHelp();
  void handleEvent(const SDL_Event &event);
  void loop();
  void render();
  void renderPause();
  void showScoreboard(std::string_view currentName);

private:
  Sdl::Context sdl_;
  Font font_;

  int scale_, cellSize_;

  uint8_t cell_[16][8] = {};

  Falling falling_, fallingNext_;

  int clock_ = 0;

  int score_ = 0;

  bool pause_ = false;
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
      memset(&cell_[0], 0, sizeof(cell_[0]));
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

void Game::init(int scale)
{
  scale_ = scale;
  cellSize_ = scale * 16;

  sdl_.init("tetris", (8 + 1 + 4) * cellSize_, 16 * cellSize_);
  readFontFromFile(font_, "68.font");

  srand(time(NULL));
  spawn();
  spawn();
}

void Game::showHelp()
{
  auto wxy = sdl_.withBaseXY({scale_ * 4, scale_ * 4});
  auto wcl = sdl_.withColor(Sdl::WHITE);

  renderText(sdl_, "LEFT,RIGHT|MOVE\n"
                   "UP        |ROTATE\n"
                   "DOWN      |SKIP\n"
                   "SPACE     |PAUSE\n"
                   "Q,ESC     |QUIT\n", font_, scale_ * 2);
  
  sdl_.present();

  while(sdl_.wait(), sdl_.event().type != SDL_KEYDOWN && sdl_.event().type != SDL_QUIT);
  
  quit_ = sdl_.event().type == SDL_QUIT;
}

void Game::handleEvent(const SDL_Event &event)
{
  switch(event.type)
  {
    case SDL_KEYDOWN:
      switch(event.key.keysym.sym)
      {
        case SDLK_LEFT:
          if(!pause_)
            moveLeft();
        break;

        case SDLK_RIGHT:
          if(!pause_)
            moveRight();
        break;

        case SDLK_UP:
          if(!pause_)
            turn();
        break;

        case SDLK_DOWN:
          if(!pause_)
            skip();
        break;
        
        case SDLK_SPACE:
          if((pause_ = !pause_))
          {
            renderPause();

            while(pause_ && !quit_)
            {
              sdl_.wait();
              handleEvent(sdl_.event());
            }
          }
        break;

        case 'q':
        case SDLK_ESCAPE:
          quit_ = true;
        break;
      }
    break;

    case SDL_QUIT:
      quit_ = true;
    break;

    default:
    break;
  }

}

void Game::loop()
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

void Game::render()
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

  sdl_.withColor(Sdl::GRAY)
      ->withBaseXY({cellSize_ * 8, 0})
      ->fillRect(0, 0, cellSize_ / 2, cellSize_ * 16);

  renderFalling(falling_, falling_.x, falling_.y);
  renderFalling(fallingNext_, 9, 0);
  
  auto scoreStr = std::to_string(score_);
  if(scoreStr.size() < 5)
    scoreStr = std::string(5 - scoreStr.size(), ' ') + scoreStr;


  {
    auto wxy = sdl_.withBaseXY({9 * cellSize_, 14 * cellSize_});
    auto wcl = sdl_.withColor(Sdl::WHITE);
    renderText(sdl_, scoreStr, font_, scale_ * 2);
  }

  sdl_.present();
}

void Game::renderPause()
{
  auto wxy = sdl_.withBaseXY({scale_ * 4, 7 * cellSize_});
  {
    auto wcl = sdl_.withColor(Sdl::BLACK);
    renderText(sdl_, "PAUSE", font_, scale_ * 4, 1.);
  }
  {
    auto wcl = sdl_.withColor(Sdl::WHITE);
    renderText(sdl_, "PAUSE", font_, scale_ * 4);
  }
  
  sdl_.present();
}

namespace
{
  using Scoreboard = std::vector<std::pair<std::string, int>>;

  std::string prepareNameForScoreboard(std::string_view name, int length)
  {
    if(!name.empty())
    {
      std::string res;

      for(auto c : name)
        if(isalpha(c))
          res += char(toupper(c));
      
      return res.substr(0, length);
    }
    else
    {
      return "-";
    }
  }

  Scoreboard readScoreboardFromFile(std::istream &in)
  {
    Scoreboard scoreboard;

    while(!in.eof())
    {
      std::string name;
      int score;
      in >> name >> score;
      if(!in.eof())
        scoreboard.emplace_back(std::move(name), score);
    }
    
    return scoreboard;
  }

  Scoreboard readScoreboardFromFile(const char *filename)
  {
    if(auto in = std::ifstream(filename))
      return readScoreboardFromFile(in);
    else
      return {};
  }
  
  void insertToScoreboard(Scoreboard &scoreboard, std::string_view name, int score)
  {
    auto it = std::ranges::find(scoreboard, name, &std::pair<std::string,int>::first);
    if(it != scoreboard.end())
    {
      if(it->second < score)
        it->second = score;
    }
    else
    {
      scoreboard.emplace_back(name, score);
    }
    
    std::ranges::sort(scoreboard, std::ranges::greater(), &std::pair<std::string,int>::second);
    
    if(scoreboard.size() > 14)
      scoreboard.resize(14);
  }
  
  std::string dumpScoreboardLine(const std::string &name, int score, int nameLen, int scoreLen)
  {
    auto scoreStr = std::to_string(score);
    return name + std::string(nameLen - name.size(), ' ') + " "
         + std::string(scoreLen - scoreStr.size(), ' ') + scoreStr + "\n";
  }
}

void Game::showScoreboard(std::string_view currentName)
{
  auto scoreboard = readScoreboardFromFile("scoreboard");
  
  auto currentNameFixed = prepareNameForScoreboard(currentName, 10);

  insertToScoreboard(scoreboard, currentNameFixed, score_);
  
  auto out = std::ofstream("scoreboard", std::ios::trunc);

  sdl_.setColor(Sdl::BLACK);
  sdl_.clear();

  sdl_.setBaseXY({scale_ * 8, scale_ * 8});

  for(int i = 0; i < scoreboard.size(); ++i)
  {
    const auto &[name, score] = scoreboard[i];

    auto isSelf = name == currentNameFixed;

    auto line = dumpScoreboardLine(name, score, 10, 5);

    out << line;

    sdl_.setColor(isSelf ? Sdl::gray(192) : Sdl::gray(128));
    renderText(sdl_, line, font_, scale_ * 2, 0., i);
  }
  
  sdl_.setColor(Sdl::WHITE);
  renderText(sdl_, dumpScoreboardLine(currentNameFixed, score_, 10, 5), font_, scale_ * 2, 0., 14);

  sdl_.present();
  
  do sdl_.wait(); while(!( sdl_.event().type == SDL_QUIT
                        || sdl_.event().type == SDL_KEYDOWN
                          && ( sdl_.event().key.keysym.sym == 'q'
                            || sdl_.event().key.keysym.sym == SDLK_ESCAPE)));

  std::cout << "SCORE: " << score_ << '\n';
}

void Game::execute(int scale, std::string_view currentName, bool help)
{
  init(scale);

  if(help)
    showHelp();

  while(true)
  {
    while(!quit_ && sdl_.poll())
      handleEvent(sdl_.event());
    
    if(quit_)
      break;

    loop();
    render();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  
  showScoreboard(currentName);
}

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"s", "scale"},
                         {"n", "name"}}, {{"h", "help"}});

  Game().execute(args.getIntO("scale").value_or(1),
                 args.getO("name").value_or(""),
                 args.is("help"));
}
