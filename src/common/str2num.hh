#pragma once

#include <string>
#include <charconv>

template<typename Num>
Num str2num(std::string_view str)
{
  Num num;
  std::from_chars(str.begin(), str.end(), num);
  return num;
}
