#pragma once

template<class Obj, class Param>
class WithSetTmp
{
public:
  WithSetTmp(Obj *obj, Param tmp, Param (Obj::*getter)(), void (Obj::*setter)(Param))
    : obj_(obj)
    , prevVal_(((*obj).*getter)())
    , setter_(setter)
  {
    ((*obj_).*setter_)(tmp);
  }

  ~WithSetTmp()
  {
    ((*obj_).*setter_)(prevVal_);
  }

  Obj& operator*() { return *obj_; }
  Obj* operator->() { return obj_; }
  
  WithSetTmp(WithSetTmp&&) = delete;
  WithSetTmp(const WithSetTmp&) = delete;
  WithSetTmp &operator=(WithSetTmp&&) = delete;
  WithSetTmp &operator=(const WithSetTmp&) = delete;
    
private:
  Obj *obj_;
  Param prevVal_;
  void (Obj::*setter_)(Param);
};
