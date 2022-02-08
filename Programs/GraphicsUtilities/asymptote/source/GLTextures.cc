//
// Created by jamie on 8/23/21.
//

#include "GLTextures.h"

#ifdef HAVE_GL

namespace gl
{

#ifdef __CYGWIN__
#define glDeleteTextures glDeleteTexturesEXT  // Missing in 32-bit CYGWIN
#endif

AGLTexture::~AGLTexture()
{
  if (textureId != 0)
    {
      glDeleteTextures(1, &textureId);
    }
}

AGLTexture& AGLTexture::operator=(AGLTexture&& glTex) noexcept
{
  textureId = glTex.textureId;
  textureNumber = glTex.textureNumber;
  glTex.textureId = 0;
  glTex.textureNumber = -1;

  return *this;
}

AGLTexture::AGLTexture(AGLTexture&& glTex) noexcept:
  textureId(glTex.textureId), textureNumber(glTex.textureNumber)
{
}

AGLTexture::AGLTexture(int textureNumber) : textureNumber(textureNumber) {}

void AGLTexture::setActive() const
{
  glActiveTexture(GL_TEXTURE0+textureNumber);
}

} // namespace gl

#endif
