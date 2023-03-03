#include "font.hh"
#include <stdexcept>
#include <cstring>

Font::~Font()
{
  drop();
}

Font::Font(Font &&other)
{
  *this = std::move(other);
}

Font::Font(const Font &other)
{
  *this = other;
}

Font &Font::operator=(Font &&other)
{
  drop();
  bytes_ = other.bytes_;
  w_ = other.w_;
  h_ = other.h_;
  bytesPerOne_ = other.bytesPerOne_;
  other.bytes_ = nullptr;
  return *this;
}

Font &Font::operator=(const Font &other)
{
  if(other.bytes_)
  {
    if(!bytes_ || bytesPerOne_ != other.bytesPerOne_)
    {
      drop();
      bytesPerOne_ = other.bytesPerOne_;
      bytes_ = new uint8_t[128 * bytesPerOne_];
    }
    memcpy(bytes_, other.bytes_, 128 * bytesPerOne_);
    w_ = other.w_;
    h_ = other.h_;
  }
  else
  {
    drop();
  }
  return *this;
}

void Font::init(int w, int h)
{
  if(bytes_)
    throw std::runtime_error("Font::init(): font already initialized!");

  w_ = w;
  h_ = h;
  bytesPerOne_ = w*h ? (w*h - 1) / 8 + 1 : 0;
  bytes_ = new uint8_t[128 * bytesPerOne_]();
}

void Font::drop()
{
  if(bytes_)
    delete[] bytes_;

  bytes_ = nullptr;
}

void Font::load(std::istream &in)
{
  if(bytes_)
    throw std::runtime_error("Font::load(): font already initialized!");

  uint8_t w, h;
  in.read((char *)&w, 1);
  in.read((char *)&h, 1);

  init(w, h);

  in.read((char *)bytes_, 128 * bytesPerOne_);
}

void Font::store(std::ostream &out) const
{
  if(!bytes_)
    throw std::runtime_error("Font::store(): font not initialized!");

  uint8_t w = w_, h = h_;

  out.write((char *)&w, 1);
  out.write((char *)&h, 1);
  out.write((char *)bytes_, 128 * bytesPerOne_);
}

void Font::erase(int ch)
{
  memset(&bytes_[ch * bytesPerOne_], 0, bytesPerOne_);
}
