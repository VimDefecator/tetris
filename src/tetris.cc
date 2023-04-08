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

static constexpr auto NSHAPES = 7;
static constexpr auto NCOLORS = 6;

struct Shape
{
  Shape(std::string_view input)
  {
    bool view[4][4];

    for(int y = 0; y < 4; y++)
      for(int x = 0; x < 4; x++)
        view[y][x] = input[4*y + x] != ' ';

    for(int v = 0; v < 4; v++)
    {
      int upmostY = 4, leftmostX = 4;

      for(int y = 0; y < 4; y++)
        for(int x = 0; x < 4; x++)
          if(view[y][x])
          {
            if(y < upmostY)
              upmostY = y;
            if(x < leftmostX)
              leftmostX = x;
          }

      memcpy(&views[v][0][0],
             &view[upmostY][leftmostX],
             &view[4][0] - &view[upmostY][leftmostX]);

      for(int y = 0; y < 4; y++)
        for(int x = 0; x < 4; x++)
          view[3-x][y] = views[v][y][x];
    }
  }

  bool views[4][4][4] = {};
};

static const std::array<Shape, NSHAPES> g_shapes
{{
  Shape("00  "
        "00  "
        "    "
        "    "),

  Shape("0   "
        "0   "
        "0   "
        "0   "),

  Shape("000 "
        " 0  "
        "    "
        "    "),

  Shape(" 00 "
        "00  "
        "    "
        "    "),

  Shape("00  "
        " 00 "
        "    "
        "    "),

  Shape("0   "
        "000 "
        "    "
        "    "),

  Shape("  0 "
        "000 "
        "    "
        "    ")
}};

class Game
{
public:
  void execute(int scale, std::string currentName, bool help);

private:
  struct Falling
  {
    const Shape *shape;
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
  void renderTextInCenter(std::string_view text, int scale);
  bool promptName(std::string &name);
  void showScoreboard(std::string_view currentName);
  
  void delay(int factor);

private:
  Sdl::Context sdl_;
  Font font_;

  int scale_, cellSize_;

  uint8_t cell_[16][8] = {};

  Falling falling_, fallingNext_;

  int clock_ = 0;

  int score_ = 0;

  bool update_ = false;
  bool pause_ = false;
  bool quit_ = false;
};

namespace
{
  bool isQuitEvent(const SDL_Event &event)
  {
    return event.type == SDL_QUIT
        || event.type == SDL_KEYDOWN
          && event.key.keysym.sym == SDLK_ESCAPE;
  }
}

void Game::spawn()
{
  auto r = rand();

  falling_ = fallingNext_;

  fallingNext_.shape = &g_shapes[r % NSHAPES];
  r /= NSHAPES;

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
  
    update_ = true;
  }
}

void Game::moveRight()
{
  falling_.x++;
  if(collides())
    falling_.x--;

  update_ = true;
}

void Game::turn()
{
  falling_.v = (falling_.v + 1) % 4;
  if(collides())
    falling_.v = (falling_.v + 3) % 4;
  
  update_ = true;
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
    
    update_ = true;
  }

  falling_.y--;
}

void Game::land()
{
  auto shape = getFig(falling_);

  for(int yRel = 0; yRel < 4; yRel++)
    for(int xRel = 0; xRel < 4; xRel++)
      if(shape[yRel][xRel])
        cell_[falling_.y + yRel][falling_.x + xRel] = falling_.col;
  
  falling_.shape = nullptr;
}

void Game::reduce()
{
  int numReduced = 0;

  std::vector<int> fullRows;
  fullRows.reserve(4);

  for(int i = 0; i < 16; i++)
    if(std::all_of(&cell_[i][0], &cell_[i][8], std::identity()))
      fullRows.push_back(i);

  if(fullRows.empty())
    return;

  for(auto i : fullRows)
    memset(cell_[i], 0, sizeof(cell_[i]));

  auto dscore = fullRows.size() * fullRows.size();

  score_ += dscore;
  
  render();
  renderTextInCenter("+" + std::to_string(dscore), 8);
  sdl_.present();
  delay(5);

  for(auto i : fullRows)
    memmove(cell_[1], cell_[0], (uint8_t *)cell_[i] - (uint8_t *)cell_);
  
  memset(cell_[0], 0, sizeof(cell_[0]));
}

bool Game::collides()
{
  auto shape = getFig(falling_);

  for(int yRel = 0; yRel < 4; yRel++)
    for(int xRel = 0; xRel < 4; xRel++)
      if(shape[yRel][xRel])
        if(auto y = falling_.y + yRel, x = falling_.x + xRel; y >= 16 || x >= 8 || cell_[y][x])
          return true;

  return false;
}

const bool (*Game::getFig(Falling &falling))[4]
{
  if(falling.shape)
    return falling.shape->views[falling.v];
  else
    return nullptr;
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

  renderText("LEFT,RIGHT|MOVE\n"
             "UP        |ROTATE\n"
             "DOWN      |SKIP\n"
             "SPACE     |PAUSE\n"
             "Q,ESC     |QUIT\n", {.sdl = sdl_,
                                   .font = font_,
                                   .scale = scale_ * 2});
  
  sdl_.present();

  while(sdl_.wait(), sdl_.event().type != SDL_KEYDOWN && sdl_.event().type != SDL_QUIT);
  
  quit_ = isQuitEvent(sdl_.event());
}

void Game::handleEvent(const SDL_Event &event)
{
  if(isQuitEvent(event))
  {
    quit_ = true;
    return;
  }

  if(event.type == SDL_KEYDOWN)
  {
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
          renderTextInCenter("PAUSE", 4);
          sdl_.present();

          while(pause_ && !quit_)
          {
            sdl_.wait();
            handleEvent(sdl_.event());
          }
        }
      break;
    }
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
    
    update_ = true;
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
    if(auto shape = getFig(falling))
      for(int yRel = 0; yRel < 4; yRel++)
        for(int xRel = 0; xRel < 4; xRel++)
          if(shape[yRel][xRel])
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
    renderText(scoreStr, {.sdl = sdl_,
                          .font = font_,
                          .scale = scale_ * 2});
  }
}

void Game::renderTextInCenter(std::string_view text, int scale)
{
  auto resultScale = scale_ * scale;
  auto centerPos = Sdl::XY{cellSize_ * 4, cellSize_ * 8};

  {
    auto wcl = sdl_.withColor(Sdl::BLACK);
    renderTextAt(text, {.sdl = sdl_,
                        .font = font_,
                        .scale = resultScale,
                        .pixelOverlap = 1.}, centerPos, true);
  }
  {
    auto wcl = sdl_.withColor(Sdl::WHITE);
    renderTextAt(text, {.sdl = sdl_,
                        .font = font_,
                        .scale = resultScale}, centerPos, true);
  }
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
        if(isalpha(c) || isdigit(c))
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

bool Game::promptName(std::string &name)
{
  auto wxy = sdl_.withBaseXY({scale_ * 8, scale_ * 8});

  auto render = [&]
  {
    sdl_.setColor(Sdl::BLACK);
    sdl_.clear();
    
    sdl_.setColor(Sdl::WHITE);

    renderText("YOUR NAME:", {.sdl = sdl_,
                              .font = font_,
                              .scale = scale_ * 2});

    renderText(name, {.sdl = sdl_,
                      .font = font_,
                      .scale = scale_ * 2,
                      .skipRows = 1});

    sdl_.present();
  };

  render();

  bool isEntered = false;

  SDL_StartTextInput();

  while(sdl_.wait())
  {
    if(isQuitEvent(sdl_.event()))
      break;

    if(sdl_.event().type == SDL_TEXTINPUT)
    {
      auto input = std::string_view(sdl_.event().text.text);

      if(input.size() == 1 && name.size() < 10)
      {
        name += char(toupper(input[0]));
        render();
      }
    }
    else if(sdl_.event().type == SDL_KEYDOWN)
    {
      if(sdl_.event().key.keysym.sym == SDLK_RETURN)
      {
        isEntered = true;
        break;
      }

      if(sdl_.event().key.keysym.sym == SDLK_BACKSPACE && !name.empty())
      {
        name.pop_back();
        render();
      }
    }
  }

  SDL_StopTextInput();

  return isEntered;
}

void Game::showScoreboard(std::string_view currentName)
{
  auto wxy = sdl_.withBaseXY({scale_ * 8, scale_ * 8});

  auto scoreboard = readScoreboardFromFile("scoreboard");
  
  auto out = std::ofstream("scoreboard", std::ios::trunc);
  
  auto currentNameFixed = prepareNameForScoreboard(currentName, 10);

  insertToScoreboard(scoreboard, currentNameFixed, score_);

  sdl_.setColor(Sdl::BLACK);
  sdl_.clear();

  for(int i = 0; i < scoreboard.size(); ++i)
  {
    const auto &[name, score] = scoreboard[i];

    auto isSelf = name == currentNameFixed;

    auto line = dumpScoreboardLine(name, score, 10, 5);

    out << line;

    sdl_.setColor(isSelf ? Sdl::gray(192) : Sdl::gray(128));
    renderText(line, {.sdl = sdl_,
                      .font = font_,
                      .scale = scale_ * 2,
                      .skipRows = i});
  }
  
  sdl_.setColor(Sdl::WHITE);
  renderText(dumpScoreboardLine(currentNameFixed, score_, 10, 5), {.sdl = sdl_,
                                                                   .font = font_,
                                                                   .scale = scale_ * 2,
                                                                   .skipRows = 14});

  sdl_.present();
  
  do sdl_.wait(); while(!isQuitEvent(sdl_.event()));
}

void Game::delay(int factor)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(factor * 50));
}

void Game::execute(int scale, std::string currentName, bool help)
{
  init(scale);

  if(help)
    showHelp();

  if(quit_)
    return;

  while(true)
  {
    while(!quit_ && sdl_.poll())
      handleEvent(sdl_.event());
    
    if(quit_)
      break;

    loop();

    if(update_)
    {
      render();
      sdl_.present();
      update_ = false;
    }
    
    delay(1);
  }

  if(sdl_.event().type == SDL_QUIT)
    return;

  if(currentName.empty())
    if(!promptName(currentName))
      return;
  
  showScoreboard(currentName);

  std::cout << "SCORE: " << score_ << '\n';
}

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"s", "scale"},
                         {"n", "name"}}, {{"h", "help"}});

  Game().execute(args.getIntO("scale").value_or(1),
                 args.getStrO("name").value_or(""),
                 args.is("help"));
}
