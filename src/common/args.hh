#pragma once

#include <initializer_list>
#include <tuple>
#include <map>
#include <set>
#include <vector>
#include <string>

class Args
{
public:
  using ShortLongName = std::pair<std::string_view, std::string_view>;
  using ShortLongNames = std::initializer_list<ShortLongName>;

  Args(int argc, char **argv, ShortLongNames keyValueArgs, ShortLongNames flagArgs);

  std::optional<std::string_view> getO(std::string_view name) const;
  std::string_view get(std::string_view name) const;

  std::optional<int> getIntO(std::string_view name) const;
  int getInt(std::string_view name) const;

  std::optional<std::string> getStrO(std::string_view name) const;
  std::string getStr(std::string_view name) const;

  bool is(std::string_view name) const;
  
  const std::vector<std::string_view> &targets() const { return targets_; }
  
private:
  std::map<std::string_view, std::string_view> key2value_;
  std::set<std::string_view> flags_;
  std::vector<std::string_view> targets_;
};

