#pragma once

#include <iostream>
#include <cstddef>
#include <cstdint>

class BitView
{
public:
  BitView(uint8_t *bytePtr, int bitNo)
    : bytePtr_(bytePtr)
    , bitMask_(1 << bitNo)
  {
  }

  operator bool() const
  {
    return *bytePtr_ & bitMask_;
  }

  bool operator=(bool value)
  {
    if(value)
      *bytePtr_ |= bitMask_;
    else
      *bytePtr_ &= ~bitMask_;

    return value;
  }
  
  void flip()
  {
    *bytePtr_ ^= bitMask_;
  }
  
private:
  uint8_t *bytePtr_;
  uint8_t bitMask_;
};

class BitsView
{
public:
  BitsView(uint8_t *bytes, int shift = 0)
    : bytes_(bytes)
    , shift_(shift)
  {
  }

  BitView operator[](int index)
  {
    auto shiftedIndex = index + shift_;
    return BitView(&bytes_[shiftedIndex / 8], shiftedIndex % 8);
  }

private:
  uint8_t *bytes_;
  int shift_;
};

class BitsView2D
{
public:
  BitsView2D(uint8_t *bytes, int w, int h)
    : bytes_(bytes)
    , w_(w)
    , h_(h)
  {
  }

  BitsView operator[](int colNo) const
  {
    auto absNo = colNo * h_;
    return BitsView(&bytes_[absNo / 8], absNo % 8);
  }

private:
  uint8_t *bytes_;
  int w_, h_;
};

class Font
{
public:
  Font() = default;
  ~Font();
  
  void init(int w, int h);

  void load(std::istream &in);

  void store(std::ostream &out) const;
  
  BitsView2D operator[](int ch)
  {
    return BitsView2D(&bytes_[ch * bytesPerOne_], w_, h_);
  }
  
  int width() const { return w_; }
  int height() const { return h_; }
  
  void erase(int ch);
    
private:
  uint8_t *bytes_ = nullptr;
  int w_, h_, bytesPerOne_;
};
