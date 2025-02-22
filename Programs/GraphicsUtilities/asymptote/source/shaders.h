#ifndef __TOGL_SHADERSPROC
#define __TOGL_SHADERSPROC

#define GLEW_NO_GLU
#include <GL/glew.h>

#include <string>

extern int GLSLversion;

typedef std::pair<std::string, int> ShaderfileModePair;

GLuint compileAndLinkShader(
  std::vector<ShaderfileModePair> const& shaders,
  std::vector<std::string> const& defineflags, bool ssbo=true,
  bool interlock=false, bool compute=false, bool test=false);

GLuint createShaderFile(std::string file, int shaderType,
                        std::vector<std::string> const& constflags,
                        bool ssbo, bool interlock, bool compute, bool test);

enum attrib {positionAttrib=0,normalAttrib,materialAttrib,colorAttrib,
             widthAttrib};

#endif
