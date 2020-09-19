#ifndef MATERIAL_H
#define MATERIAL_H

#ifdef HAVE_LIBGLM

#include <iostream>
#include <fstream>

#include "common.h"
#include "triple.h"

#include <glm/glm.hpp>

namespace camp {

inline bool operator < (const glm::vec4& m1, const glm::vec4& m2) {
  return m1[0] < m2[0] || 
                 (m1[0] == m2[0] &&
                  (m1[1] < m2[1] || 
                   (m1[1] == m2[1] &&
                    (m1[2] < m2[2] || 
                     (m1[2] == m2[2] &&
                      (m1[3] < m2[3]))))));
}

inline glm::vec4 GLparameters(GLfloat shininess, GLfloat metallic,
                             GLfloat fresnel0) {
  return glm::vec4(shininess,metallic,fresnel0,0.0);
}

inline ostream& operator << (ostream& out, const glm::vec4& v)
{
  out << "[" << v[0] << "," << v[1] << "," << v[2] << "," << v[3]
      << "]";
  return out;
}

struct Material {
public:
  glm::vec4 diffuse,emissive,specular;
  glm::vec4 parameters;
  Material() {}

  Material(const glm::vec4& diffuse, const glm::vec4& emissive,
           const glm::vec4& specular, double shininess, double metallic, double fresnel0) : 
    diffuse(diffuse), emissive(emissive), specular(specular),
    parameters(GLparameters(shininess,metallic,fresnel0)) {}

  Material(Material const& m):
    diffuse(m.diffuse), emissive(m.emissive),
    specular(m.specular), parameters(m.parameters) {}
  ~Material() {}

  Material& operator=(Material const& m)
  {
    diffuse=m.diffuse;
    emissive=m.emissive;
    specular=m.specular;
    parameters=m.parameters;
    return *this; 
  }
  
  friend bool operator < (const Material& m1, const Material& m2) {
    return m1.diffuse < m2.diffuse ||
                        (m1.diffuse == m2.diffuse && 
                         (m1.emissive < m2.emissive ||
                        (m1.emissive == m2.emissive && 
                         (m1.specular < m2.specular ||
                        (m1.specular == m2.specular && 
                         (m1.parameters < m2.parameters))))));
  }
  
  friend ostream& operator << (ostream& out, const Material& m) {
    out << "diffuse=" << m.diffuse << "," << newl
        << "emissive=" << m.emissive << "," << newl
        << "specular=" << m.specular << "," << newl
        << "shininess=" << m.parameters[0] << "," << newl
        << "metallic=" << m.parameters[1] << "," << newl
        << "fresnel0=" << m.parameters[2] << newl;
    return out;
  }

}; 

extern size_t Nmaterials; // Number of materials compiled in shader
extern size_t nmaterials; // Current size of materials buffer
extern size_t Maxmaterials; // Maxinum size of materials buffer
}

#endif
#endif
