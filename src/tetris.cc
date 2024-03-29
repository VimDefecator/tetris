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
#include "common/args.hh"

static constexpr auto NSHAPES = 7;
static constexpr auto NCOLORS = 6;
static constexpr auto GAMEWID = 10;
static constexpr auto GAMEHEI = 20;
static constexpr auto CELLSIZE = 16;
static constexpr auto NAMELIMIT = 12;
static constexpr auto SCOREBOARDLIMIT = 18;
static constexpr auto DIFFICULTYLIMIT = 10;

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
  void init(int scale);
  void execute(bool help);
  bool finalize();

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
  const bool (*getShape(Falling &falling))[4];

  void showHelp();
  void promptDifficulty();
  void handleEvent(const SDL_Event &event);
  void loop();
  void render();
  void renderTextInCenter(std::string_view text, int scale);
  bool promptName();
  void showScoreboard();
  
  void delay(int factor);

private:
  Sdl::Context sdl_;
  Font font_;

  uint8_t cell_[GAMEHEI][GAMEWID];

  Falling falling_, fallingNext_;

  int difficulty_ = 0, clockPeriod_;
  int clock_;
  int score_;

  bool started_;
  bool update_;
  bool pause_;
  bool quit_;

  std::string currentName_;
};

namespace
{
  bool isQuitEvent(const SDL_Event &event)
  {
    return event.type == SDL_QUIT
        || event.type == SDL_KEYDOWN
          && event.key.keysym.sym == SDLK_ESCAPE;
  }

  bool isRetryEvent(const SDL_Event &event)
  {
    return event.type == SDL_KEYDOWN
          && event.key.keysym.sym == SDLK_SPACE;
  }
}

void Game::init(int scale)
{
  sdl_.init("tetris", (GAMEWID + 1 + 4) * CELLSIZE, GAMEHEI * CELLSIZE, scale);
  readFontFromFile(font_, "68.font");
  srand(time(NULL));
}

void Game::execute(bool help)
{
  clock_ = 0;
  score_ = 0;

  started_ = false;
  update_ = true;
  pause_ = false;
  quit_ = false;

  if(help)
    showHelp();

  if(quit_)
    return;

  promptDifficulty();

  if(quit_)
    return;

  clockPeriod_ = DIFFICULTYLIMIT - difficulty_;

  started_ = true;

  memset(cell_, 0, sizeof(cell_));
  spawn();
  spawn();

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
}

bool Game::finalize()
{
  if(!started_ || sdl_.event().type == SDL_QUIT)
    return true;

  if(promptName())
    showScoreboard();

  std::cout << "SCORE: " << score_ << '\n';

  return !isRetryEvent(sdl_.event());
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
  fallingNext_.x = 4;
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
  auto shape = getShape(falling_);

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

  for(int i = 0; i < GAMEHEI; i++)
    if(std::all_of(&cell_[i][0], &cell_[i][GAMEWID], std::identity()))
      fullRows.push_back(i);

  if(fullRows.empty())
    return;

  for(auto i : fullRows)
    memset(cell_[i], 0, sizeof(cell_[i]));

  auto dscore = fullRows.size() * fullRows.size() + difficulty_;

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
  auto shape = getShape(falling_);

  for(int yRel = 0; yRel < 4; yRel++)
    for(int xRel = 0; xRel < 4; xRel++)
      if(shape[yRel][xRel])
        if(auto y = falling_.y + yRel, x = falling_.x + xRel; y >= GAMEHEI || x >= GAMEWID || cell_[y][x])
          return true;

  return false;
}

const bool (*Game::getShape(Falling &falling))[4]
{
  if(falling.shape)
    return falling.shape->views[falling.v];
  else
    return nullptr;
}

void Game::showHelp()
{
  auto wxy = sdl_.withBaseXY({font_.wid(), font_.hei()});
  auto wcl = sdl_.withColor(Sdl::WHITE);

  renderText("LEFT,RIGHT | MOVE  \n"
             "-----------|-------\n"
             "UP         | TURN  \n"
             "-----------|-------\n"
             "DOWN       | SKIP  \n"
             "-----------|-------\n"
             "ESC        | QUIT  \n"
             "-----------|-------\n"
             "SPACE      | PAUSE \n"
             "-----------|-------\n"
             "SPACE (END)| RETRY \n", {.sdl = sdl_,
                                       .font = font_,
                                       .scale = 2});
  
  sdl_.present();

  while(sdl_.wait(), sdl_.event().type != SDL_KEYDOWN && sdl_.event().type != SDL_QUIT);
  
  quit_ = isQuitEvent(sdl_.event());
}

void Game::promptDifficulty()
{
  auto stepWid = sdl_.wid() / DIFFICULTYLIMIT;
  auto stepHei = sdl_.hei() / DIFFICULTYLIMIT;

  auto renderDifficulty = [&]
  {
    sdl_.setColor(Sdl::BLACK);
    sdl_.clear();

    for(int step = 0; step <= difficulty_; step++)
    {
      Sdl::Color col;
      col.r = step * 255 / (DIFFICULTYLIMIT - 1);
      col.g = 255 - col.r;
      col.b = 0;

      sdl_.setColor(col);

      sdl_.fillRect(step * stepWid + 1,
                    (DIFFICULTYLIMIT - 1 - step) * stepHei + 1,
                    stepWid - 2,
                    (step + 1) * stepHei - 2);
    };

    auto difficultyStr = std::to_string(difficulty_);
    {
      auto wcl = sdl_.withColor(sdl_.getColor() / 2);
      renderTextAt(difficultyStr, {.sdl = sdl_, .font = font_, .scale = 8, .pixelOverlap = 1.}, {.pos = {8, 8}});
    }
    renderTextAt(difficultyStr, {.sdl = sdl_, .font = font_, .scale = 8}, {.pos = {8, 8}});

    sdl_.present();
  };

  renderDifficulty();

  while(sdl_.wait(), !isQuitEvent(sdl_.event()))
  {
    if(sdl_.event().type == SDL_KEYDOWN)
    {
      switch(sdl_.event().key.keysym.sym)
      {
        case SDLK_LEFT:
        case SDLK_DOWN:
          if(difficulty_ > 0)
          {
            --difficulty_;
            renderDifficulty();
          }
        break;

        case SDLK_RIGHT:
        case SDLK_UP:
          if(difficulty_ < DIFFICULTYLIMIT - 1)
          {
            ++difficulty_;
            renderDifficulty();
          }
        break;

        case SDLK_RETURN:
          return;
        break;

        default:
        break;
      }
    }
  }

  quit_ = true;
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
  clock_ = (clock_ + 1) % clockPeriod_;

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
    auto col = idx2col(colIdx);

    sdl_.setColor(col / 2);
    sdl_.pixArtPut(x, y, CELLSIZE, 0.875);

    sdl_.setColor(col);
    sdl_.pixArtPut(x, y, CELLSIZE, 0.750);
  };

  auto renderFalling = [&](Falling &falling, int x, int y)
  {
    if(auto shape = getShape(falling))
      for(int yRel = 0; yRel < 4; yRel++)
        for(int xRel = 0; xRel < 4; xRel++)
          if(shape[yRel][xRel])
            renderCell(x + xRel, y + yRel, falling.col);
  };

  for(int y = 0; y < GAMEHEI; y++)
    for(int x = 0; x < GAMEWID; x++)
      if(auto col = cell_[y][x])
        renderCell(x, y, col);

  sdl_.withColor(Sdl::GRAY)
      ->withBaseXY({CELLSIZE * GAMEWID, 0})
      ->fillRect(0, 0, CELLSIZE / 2, CELLSIZE * GAMEHEI);

  renderFalling(falling_, falling_.x, falling_.y);
  renderFalling(fallingNext_, GAMEWID + 1, 0);
  
  sdl_.setColor(Sdl::WHITE);
  renderTextAt(std::to_string(score_), {.sdl = sdl_,
                                        .font = font_,
                                        .scale = 2}, {.pos = {sdl_.wid() - 3, sdl_.hei() - 4},
                                                      .hAlign = HAlign::Right,
                                                      .vAlign = VAlign::Down});
}

void Game::renderTextInCenter(std::string_view text, int scale)
{
  auto rp = TextRenderParams{
    .sdl = sdl_,
    .font = font_,
    .scale = scale};

  auto pp = TextPositionParams{
    .pos = {CELLSIZE * GAMEWID / 2, CELLSIZE * GAMEHEI / 2},
    .hAlign = HAlign::Center,
    .vAlign = VAlign::Center};

  auto resultScale = scale;
  auto centerPos = Sdl::XY{CELLSIZE * GAMEWID / 2, CELLSIZE * GAMEHEI / 2};

  rp.pixelOverlap = 1.;
  {
    auto wcl = sdl_.withColor(Sdl::BLACK);
    renderTextAt(text, rp, pp);
  }

  rp.pixelOverlap = 0.;
  {
    auto wcl = sdl_.withColor(Sdl::WHITE);
    renderTextAt(text, rp, pp);
  }
}

bool Game::promptName()
{
  auto wxy = sdl_.withBaseXY({font_.wid(), font_.hei()});

  auto renderPrompt = [&]
  {
    sdl_.setColor(Sdl::BLACK);
    sdl_.clear();
    
    sdl_.setColor(Sdl::gray(128));

    renderText("YOUR NAME:", {.sdl = sdl_,
                              .font = font_,
                              .scale = 2});

    sdl_.setColor(Sdl::WHITE);

    renderText(currentName_, {.sdl = sdl_,
                              .font = font_,
                              .scale = 2,
                              .skipRows = 1});

    sdl_.present();
  };

  renderPrompt();

  bool isEntered = false;

  SDL_StartTextInput();

  while(sdl_.wait())
  {
    if(isQuitEvent(sdl_.event()) || isRetryEvent(sdl_.event()))
      break;

    if(sdl_.event().type == SDL_TEXTINPUT)
    {
      auto input = std::string_view(sdl_.event().text.text);

      if(input.size() == 1 && currentName_.size() < NAMELIMIT)
      {
        auto ch = input[0];

        if(isalpha(ch) || isdigit(ch))
        {
          currentName_ += char(toupper(ch));
          renderPrompt();
        }
      }
    }
    else if(sdl_.event().type == SDL_KEYDOWN)
    {
      auto sym = sdl_.event().key.keysym.sym;

      if(sym == SDLK_RETURN)
      {
        isEntered = true;
        break;
      }

      if(sym == SDLK_SPACE)
      {
        break;
      }

      if(sym == SDLK_BACKSPACE && !currentName_.empty())
      {
        currentName_.pop_back();
        renderPrompt();
      }
    }
  }

  SDL_StopTextInput();

  return isEntered;
}

namespace
{
  using Scoreboard = std::vector<std::pair<std::string, int>>;

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
    
    if(scoreboard.size() > SCOREBOARDLIMIT)
      scoreboard.resize(SCOREBOARDLIMIT);
  }
  
  std::string dumpScoreboardLine(const std::string &name, int score, int nameLen, int scoreLen)
  {
    auto scoreStr = std::to_string(score);
    return name + std::string(nameLen - name.size(), ' ') + " "
         + std::string(scoreLen - scoreStr.size(), ' ') + scoreStr + "\n";
  }
}

void Game::showScoreboard()
{
  auto trp = TextRenderParams{
    .sdl = sdl_,
    .font = font_,
    .scale = 2};

  auto tppNames = TextPositionParams{
    .pos = {font_.wid(), font_.hei()},
    .hAlign = HAlign::Left,
    .vAlign = VAlign::Up};

  auto tppScores = TextPositionParams{
    .pos = {sdl_.wid() - font_.wid(), font_.hei()},
    .hAlign = HAlign::Right,
    .vAlign = VAlign::Up};

  auto tppYourName = TextPositionParams{
    .pos = {font_.wid(), sdl_.hei() - font_.hei()},
    .hAlign = HAlign::Left,
    .vAlign = VAlign::Down};

  auto tppYourScore = TextPositionParams{
    .pos = {sdl_.wid() - font_.wid(), sdl_.hei() - font_.hei()},
    .hAlign = HAlign::Right,
    .vAlign = VAlign::Down};

  auto scoreboard = readScoreboardFromFile("scoreboard");
  
  auto out = std::ofstream("scoreboard", std::ios::trunc);
  
  auto currentNameFixed = !currentName_.empty()
                        ? std::string_view(currentName_)
                        : std::string_view("-");

  insertToScoreboard(scoreboard, currentNameFixed, score_);

  sdl_.setColor(Sdl::BLACK);
  sdl_.clear();

  for(int i = 0; i < scoreboard.size(); ++i)
  {
    const auto &[name, score] = scoreboard[i];

    out << name << ' ' << score << '\n';

    sdl_.setColor((name == currentNameFixed) ? Sdl::gray(192) : Sdl::gray(128));

    trp.skipRows = i;
    renderTextAt(name, trp, tppNames);
    renderTextAt(std::to_string(score), trp, tppScores);
  }
  
  sdl_.setColor(Sdl::WHITE);

  trp.skipRows = 0;
  renderTextAt(currentNameFixed, trp, tppYourName);
  renderTextAt(std::to_string(score_), trp, tppYourScore);

  sdl_.present();
  
  while(sdl_.wait(), sdl_.event().type != SDL_QUIT && sdl_.event().type != SDL_KEYDOWN);
}

void Game::delay(int factor)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(factor * 50));
}

int main(int argc, char **argv)
{
  Args args(argc, argv, {{"s", "scale"}}, {{"h", "help"}});

  Game game;
  game.init(args.getIntO("scale").value_or(1));

  game.execute(args.is("help"));

  while(!game.finalize())
    game.execute(false);
}
