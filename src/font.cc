#include "font.hh"
#include <stdexcept>
#include <cstring>

Font::~Font()
{
  if(bytes_)
    delete[] bytes_;
}

void Font::init(int w, int h)
{
  w_ = w;
  h_ = h;
  bytesPerOne_ = w*h ? (w*h - 1) / 8 + 1 : 0;
  bytes_ = new uint8_t[128 * bytesPerOne_]();
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
