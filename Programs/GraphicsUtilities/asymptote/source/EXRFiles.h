//
// Created by jamie on 8/23/21.
//

#pragma once
#include "common.h"
#include "cudareflect/tinyexr/tinyexr.h"

namespace camp
{
class IEXRFile
{
public:
  IEXRFile(string const& file);
  ~IEXRFile();

  [[nodiscard]]
  float const* getData() const {
    return data;
  }

  [[nodiscard]]
  std::pair<int,int> size() const {
    return std::make_pair(width, height);
  }

private:
  float* data=nullptr;
  int width=0, height=0;
};

} // namespace gl;
