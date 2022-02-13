// shader handling
// Author: Supakorn "Jamie" Rassameemasmuang

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#define UW_(x) MiKTeX::Util::CharBuffer<wchar_t>(x).GetData()
#endif
#include "common.h"

#ifdef HAVE_GL

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

#include "shaders.h"

int GLSLversion;

GLuint compileAndLinkShader(std::vector<ShaderfileModePair> const& shaders,
                            std::vector<std::string> const& defineflags,
                            bool ssbo, bool interlock, bool compute)
{
  GLuint shader = glCreateProgram();
  std::vector<GLuint> compiledShaders;

  size_t n=shaders.size();
  for(size_t i=0; i < n; ++i) {
    GLint newshader=createShaderFile(shaders[i].first,shaders[i].second,
                                     defineflags,ssbo,interlock,compute);
    if((ssbo || interlock || compute) && newshader == 0) return 0;
    glAttachShader(shader,newshader);
    compiledShaders.push_back(newshader);
  }

  glBindAttribLocation(shader,positionAttrib,"position");
  glBindAttribLocation(shader,normalAttrib,"normal");
  glBindAttribLocation(shader,materialAttrib,"material");
  glBindAttribLocation(shader,colorAttrib,"color");
  glBindAttribLocation(shader,widthAttrib,"width");

  glLinkProgram(shader);

  for(size_t i=0; i < n; ++i) {
    glDetachShader(shader,compiledShaders[i]);
    glDeleteShader(compiledShaders[i]);
  }

  return shader;
}

GLuint createShader(const std::string& src, int shaderType,
                    const std::string& filename, bool ssbo, bool interlock,
                    bool compute)
{
  const GLchar *source=src.c_str();
  GLuint shader=glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if(status != GL_TRUE) {
    if(ssbo || interlock || compute) return 0;
    GLint length;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    std::vector<GLchar> msg(length);

    glGetShaderInfoLog(shader, length, &length, msg.data());

    size_t n=msg.size();
    for(size_t i=0; i < n; ++i)
      std::cerr << msg[i];

    std::cerr << std::endl << "GL Compile error" << std::endl;
    std::stringstream s(src);
    std::string line;
    unsigned int k=0;
    while(getline(s,line))
      std::cerr << ++k << ": " << line << std::endl;
    exit(-1);
  }
  return shader;
}

GLuint createShaderFile(std::string file, int shaderType,
                        std::vector<std::string> const& defineflags,
                        bool ssbo, bool interlock, bool compute)
{
  std::ifstream shaderFile;
#if defined(MIKTEX_WINDOWS)
  shaderFile.open(UW_(file));
#else
  shaderFile.open(file.c_str());
#endif
  std::stringstream shaderSrc;

  shaderSrc << "#version " << GLSLversion << "\n";
#ifndef __APPLE__
  shaderSrc << "#extension GL_ARB_uniform_buffer_object : enable" << "\n";
#ifdef HAVE_SSBO
  if(ssbo) {
    shaderSrc << "#extension GL_ARB_shader_storage_buffer_object : enable" << "\n";
    if(interlock)
      shaderSrc << "#extension GL_ARB_fragment_shader_interlock : enable"
                << "\n";
    if(compute)
      shaderSrc << "#extension GL_ARB_compute_shader : enable" << "\n";
  }
#endif
#endif

  size_t n=defineflags.size();
  for(size_t i=0; i < n; ++i)
    shaderSrc << "#define " << defineflags[i] << "\n";

  if(shaderFile) {
    shaderSrc << shaderFile.rdbuf();
    shaderFile.close();
  } else {
    std::cerr << "Cannot read from shader file " << file << std::endl;
    exit(-1);
  }

  return createShader(shaderSrc.str(),shaderType,file,ssbo,interlock,compute);
}
#endif
