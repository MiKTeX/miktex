//
// Created by jamie on 8/23/21.
//

#pragma once

#include "common.h"

#ifdef HAVE_GL

#include "GL/glew.h"

namespace gl
{
struct GLTexturesFmt
{
  GLint minFilter=GL_LINEAR_MIPMAP_LINEAR;
  GLint magFilter=GL_LINEAR;
  GLint wrapS=GL_REPEAT;
  GLint wrapT=GL_REPEAT;
  GLint wrapR=GL_REPEAT;

  GLuint format=GL_RGBA;
  GLuint internalFmt=GL_RGBA;
};

class AGLTexture
{
protected:
  AGLTexture() = default;
  explicit AGLTexture(int textureNumber);

  AGLTexture(AGLTexture const&) = delete;
  AGLTexture& operator=(AGLTexture const&) = delete;

  AGLTexture(AGLTexture&& glTex) noexcept;
  AGLTexture& operator=(AGLTexture&& glTex) noexcept;

public:
  virtual ~AGLTexture();
  void setActive() const;
  virtual void setUniform(GLint uniformNumber) const = 0;

protected:
  GLuint textureId=0;
  int textureNumber=-1;
};

template<typename T=float, GLuint GLDataType=GL_FLOAT>
class GLTexture2 : public AGLTexture
{
public:
  GLTexture2() = default;
  GLTexture2(GLTexture2 const& glTex)=delete;
  GLTexture2& operator= (GLTexture2 const& glTex)=delete;

  GLTexture2(GLTexture2&& glTex) noexcept: AGLTexture(std::move(glTex))
  {
  }

  GLTexture2& operator=(GLTexture2&& glTex) noexcept
  {
    AGLTexture::operator=(std::move(glTex));
    return *this;
  }

  GLTexture2(T const* data, std::pair<int, int> size,
             int textureNumber, GLTexturesFmt const& fmt) : AGLTexture(textureNumber)
  {
    glGenTextures(1, &textureId);

    glActiveTexture(GL_TEXTURE0+textureNumber);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, fmt.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, fmt.magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, fmt.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, fmt.wrapT);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt.internalFmt, size.first, size.second, 0,
                 fmt.format, GLDataType, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glActiveTexture(0);
  }

  void setUniform(GLint uniformNumber) const override
  {
    glActiveTexture(GL_TEXTURE0+textureNumber);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(uniformNumber, textureNumber);
    glActiveTexture(0);
  }
};

template<typename T=float, GLuint GLDataType=GL_FLOAT>
class GLTexture3 : public AGLTexture
{
public:
  GLTexture3() = default;
  GLTexture3(T const* data, std::tuple<int, int, int> size,
             int textureNumber, GLTexturesFmt const& fmt) : AGLTexture(textureNumber)
  {
    int width=std::get<0>(size);
    int height=std::get<1>(size);
    int depth=std::get<2>(size);
    glEnable(GL_TEXTURE_3D);

    glGenTextures(1, &textureId);
    glActiveTexture(GL_TEXTURE0+textureNumber);
    glBindTexture(GL_TEXTURE_3D, textureId);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, fmt.wrapS);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, fmt.wrapT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, fmt.wrapR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, fmt.minFilter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, fmt.magFilter);

    glTexImage3D(GL_TEXTURE_3D, 0, fmt.internalFmt, width, height, depth, 0, fmt.format,
                 GLDataType, data);
    glGenerateMipmap(GL_TEXTURE_3D);
    glActiveTexture(0);
  }

  void setUniform(GLint uniformNumber) const override
  {
    glActiveTexture(GL_TEXTURE0+textureNumber);
    glBindTexture(GL_TEXTURE_3D, textureId);
    glUniform1i(uniformNumber, textureNumber);
    glActiveTexture(0);
  }
};

} // namespace gl

#endif
