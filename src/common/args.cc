#include "args.hh"
#include "str2num.hh"

#include <optional>
#include <stdexcept>

class ArgsRange
{
public:
  ArgsRange(int argc, char **argv)
    : argc_(argc)
    , argv_(argv)
  {
  }

  char **begin() { return argv_; }
  char **end() { return argv_ + argc_; }

private:
  int argc_;
  char **argv_;
};

Args::Args(int argc, char **argv, ShortLongNames keyValueArgs, ShortLongNames flagArgs)
{
  auto findArgBy = [](std::string_view name,
                      const ShortLongNames &args,
                      std::string_view ShortLongName::* proj) -> std::optional<std::string_view>
  {
    if(name.empty())
      return {};

    for(auto pair : args)
      if(pair.*proj == name)
        return {pair.second};
    
    return {};
  };
  
  auto findKeyValueArgByShortName = [&](std::string_view name)
  {
    return findArgBy(name, keyValueArgs, &ShortLongName::first);
  };
  
  auto findKeyValueArgByLongName = [&](std::string_view name)
  {
    return findArgBy(name, keyValueArgs, &ShortLongName::second);
  };
  
  auto findFlagArgByShortName = [&](std::string_view name)
  {
    return findArgBy(name, flagArgs, &ShortLongName::first);
  };
  
  auto findFlagArgByLongName = [&](std::string_view name)
  {
    return findArgBy(name, flagArgs, &ShortLongName::second);
  };

  std::string_view currentKey;

  for(std::string_view arg : ArgsRange(argc, argv))
  {
    if(currentKey.empty())
    {
      if(arg[0] == '-')
      {
        if(arg.size() > 1)
        {
          if(arg[1] == '-')
          {
            auto longName = arg.substr(2);

            if(findKeyValueArgByLongName(longName))
            {
              currentKey = longName;
            }
            else if(findFlagArgByLongName(longName))
            {
              flags_.insert(longName);
            }
          }
          else
          {
            auto shortName = arg.substr(1);
            
            if(auto longName = findKeyValueArgByShortName(shortName))
            {
              currentKey = *longName;
            }
            else if(auto longName = findFlagArgByShortName(shortName))
            {
              flags_.insert(*longName);
            }
          }
        }
      }
      else
      {
        targets_.push_back(arg);
      }
    }
    else
    {
      key2value_.emplace(currentKey, arg);
      currentKey = {};
    }
  }
}

std::optional<std::string_view> Args::getO(std::string_view name) const
{
  if(auto it = key2value_.find(name); it != key2value_.end())
    return {it->second};
  else
    return {};
}

std::string_view Args::get(std::string_view name) const
{
  if(auto val = getO(name))
    return *val;
  else
    throw std::runtime_error("Missing expected argument: --" + std::string(name));
}

std::optional<int> Args::getIntO(std::string_view name) const
{
  if(auto val = getO(name))
    return {str2num<int>(*val)};
  else
    return {};
}

int Args::getInt(std::string_view name) const
{
  return str2num<int>(get(name));
}

std::optional<std::string> Args::getStrO(std::string_view name) const
{
  if(auto val = getO(name))
    return {std::string(*val)};
  else
    return {};
}

std::string Args::getStr(std::string_view name) const
{
  return std::string(get(name));
}

bool Args::is(std::string_view name) const
{
  return flags_.contains(name);
}
