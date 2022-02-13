/************
*
*   This file is part of a tool for producing 3D content in the PRC format.
*   Copyright (C) 2008  Orest Shardt <shardtor (at) gmail dot com> and
*                       Michail Vidiassov <master@iaas.msu.ru>
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*************/

#ifndef __O_PRC_FILE_H
#define __O_PRC_FILE_H

#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/CharBuffer>
#endif
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <stack>
#include <string>
#include <cstring>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "PRC.h"
#include "PRCbitStream.h"
#include "writePRC.h"


#ifdef HAVE_RPC_RPC_H
#include "xstream.h"
#endif

namespace prc {

class oPRCFile;
class PRCFileStructure;

// Map [0,1] to [0,255]
inline uint8_t byte(double r)
{
  if(r < 0.0) r=0.0;
  else if(r > 1.0) r=1.0;
  int a=(int)(256.0*r);
  if(a == 256) a=255;
  return a;
}

struct RGBAColour
{
  RGBAColour(double r=0.0, double g=0.0, double b=0.0, double a=1.0) :
    R(r), G(g), B(b), A(a) {}
  double R,G,B,A;

  void Set(double r, double g, double b, double a=1.0)
  {
    R = r; G = g; B = b; A = a;
  }
  bool operator==(const RGBAColour &c) const
  {
    return (R==c.R && G==c.G && B==c.B && A==c.A);
  }
  bool operator!=(const RGBAColour &c) const
  {
    return !(R==c.R && G==c.G && B==c.B && A==c.A);
  }
  bool operator<(const RGBAColour &c) const
  {
    if(R!=c.R)
      return (R<c.R);
    if(G!=c.G)
      return (G<c.G);
    if(B!=c.B)
      return (B<c.B);
    return (A<c.A);
  }

  friend RGBAColour operator * (const RGBAColour& a, const double d)
  { return RGBAColour(a.R*d,a.G*d,a.B*d,a.A*d); }
  friend RGBAColour operator * (const double d, const RGBAColour& a)
  { return RGBAColour(a.R*d,a.G*d,a.B*d,a.A*d); }

#ifdef HAVE_RPC_RPC_H
  friend xdr::oxstream& operator<<(xdr::oxstream& out, RGBAColour const& col)
  {
    out << (float) col.R << (float) col.G << (float) col.B << (float) col.A;
    return out;
  }
#endif

};
typedef std::map<RGBAColour,uint32_t> PRCcolourMap;

struct RGBAColourWidth
{
  RGBAColourWidth(double r=0.0, double g=0.0, double b=0.0, double a=1.0, double w=1.0) :
    R(r), G(g), B(b), A(a), W(w) {}
  double R,G,B,A,W;

  bool operator==(const RGBAColourWidth &c) const
  {
    return (R==c.R && G==c.G && B==c.B && A==c.A && W==c.W);
  }
  bool operator!=(const RGBAColourWidth &c) const
  {
    return !(R==c.R && G==c.G && B==c.B && A==c.A && W==c.W);
  }
  bool operator<(const RGBAColourWidth &c) const
  {
    if(R!=c.R)
      return (R<c.R);
    if(G!=c.G)
      return (G<c.G);
    if(B!=c.B)
      return (B<c.B);
    if(A!=c.A)
      return (A<c.A);
    return (W<c.W);
  }
};
typedef std::map<RGBAColourWidth,uint32_t> PRCcolourwidthMap;

typedef std::map<PRCRgbColor,uint32_t> PRCcolorMap;

struct PRCmaterial
{
  PRCmaterial() : alpha(1.0),shininess(1.0),
      picture_data(NULL), picture_format(KEPRCPicture_BITMAP_RGB_BYTE), picture_width(0), picture_height(0), picture_size(0),
      picture_replace(false), picture_repeat(false) {}
  PRCmaterial(const RGBAColour &a, const RGBAColour &d, const RGBAColour &e,
              const RGBAColour &s, double p, double h,
              const uint8_t* pic=NULL, EPRCPictureDataFormat picf=KEPRCPicture_BITMAP_RGB_BYTE,
              uint32_t picw=0, uint32_t pich=0, uint32_t pics=0, bool picreplace=false, bool picrepeat=false) :
      ambient(a), diffuse(d), emissive(e), specular(s), alpha(p), shininess(h),
      picture_data(pic), picture_format(picf), picture_width(picw), picture_height(pich), picture_size(pics),
      picture_replace(picreplace), picture_repeat(picrepeat) {
        if(picture_size==0)
        {
          if (picture_format==KEPRCPicture_BITMAP_RGB_BYTE)
             picture_size = picture_width*picture_height*3;
          if (picture_format==KEPRCPicture_BITMAP_RGBA_BYTE)
             picture_size = picture_width*picture_height*4;
          if (picture_format==KEPRCPicture_BITMAP_GREY_BYTE)
             picture_size = picture_width*picture_height*1;
          if (picture_format==KEPRCPicture_BITMAP_GREYA_BYTE)
             picture_size = picture_width*picture_height*2;
        }
      }
  RGBAColour ambient,diffuse,emissive,specular;
  double alpha,shininess;
  const uint8_t* picture_data;
  EPRCPictureDataFormat picture_format;
  uint32_t picture_width;
  uint32_t picture_height;
  uint32_t picture_size;
  bool picture_replace; // replace material color with texture color? if false - just modify
  bool picture_repeat;  // repeat texture? if false - clamp to edge

  bool operator==(const PRCmaterial &m) const
  {
    return (ambient==m.ambient && diffuse==m.diffuse && emissive==m.emissive
        && specular==m.specular && alpha==m.alpha && shininess==m.shininess
        && picture_replace==m.picture_replace && picture_repeat==m.picture_repeat
        && picture_format==m.picture_format
        && picture_width==m.picture_width && picture_height==m.picture_height && picture_size==m.picture_size
        && (picture_data==m.picture_data || memcmp(picture_data,m.picture_data,picture_size)==0) );
  }
  bool operator<(const PRCmaterial &m) const
  {
    if(ambient!=m.ambient)
      return (ambient<m.ambient);
    if(diffuse!=m.diffuse)
      return (diffuse<m.diffuse);
    if(emissive!=m.emissive)
      return (emissive<m.emissive);
    if(specular!=m.specular)
      return (specular<m.specular);
    if(alpha!=m.alpha)
      return (alpha<m.alpha);
    if(shininess!=m.shininess)
      return (shininess<m.shininess);
    if(picture_replace!=m.picture_replace)
      return (picture_replace<m.picture_replace);
    if(picture_repeat!=m.picture_repeat)
      return (picture_repeat<m.picture_repeat);
    if(picture_format!=m.picture_format)
      return (picture_format<m.picture_format);
    if(picture_width!=m.picture_width)
      return (picture_width<m.picture_width);
    if(picture_height!=m.picture_height)
      return (picture_height<m.picture_height);
    if(picture_size!=m.picture_size)
      return (picture_size<m.picture_size);
    if(picture_data!=m.picture_data)
      return (memcmp(picture_data,m.picture_data,picture_size)<0);
    return false;
  }
};
typedef std::map<PRCmaterial,uint32_t> PRCmaterialMap;

struct PRCpicture
{
  PRCpicture() :
      data(NULL), format(KEPRCPicture_BITMAP_RGB_BYTE),
      width(0), height(0), size(0) {}
  PRCpicture(const uint8_t* pic, EPRCPictureDataFormat picf,
             uint32_t picw, uint32_t pich, uint32_t pics=0) :
      data(pic), format(picf),
      width(picw), height(pich), size(pics)
  {
    if(size==0)
    {
      if (format==KEPRCPicture_BITMAP_RGB_BYTE)
         size = width*height*3;
      if (format==KEPRCPicture_BITMAP_RGBA_BYTE)
         size = width*height*4;
      if (format==KEPRCPicture_BITMAP_GREY_BYTE)
         size = width*height*1;
      if (format==KEPRCPicture_BITMAP_GREYA_BYTE)
         size = width*height*2;
    }
  }
  PRCpicture(const PRCmaterial& m) :
      data(m.picture_data), format(m.picture_format),
      width(m.picture_width), height(m.picture_height), size(m.picture_size) {}

  const uint8_t* data;
  EPRCPictureDataFormat format;
  uint32_t width;
  uint32_t height;
  uint32_t size;
  bool operator==(const PRCpicture& p) const
  {
    return ( format==p.format
        && width==p.width && height==p.height && size==p.size
        && (data==p.data || memcmp(data,p.data,size)==0) );
  }
  bool operator<(const PRCpicture& p) const
  {
    if(format!=p.format)
      return (format<p.format);
    if(width!=p.width)
      return (width<p.width);
    if(height!=p.height)
      return (height<p.height);
    if(size!=p.size)
      return (size<p.size);
    if(data!=p.data)
      return (memcmp(data,p.data,size)<0);
    return false;
  }
};

typedef std::map<PRCpicture,uint32_t> PRCpictureMap;

struct PRCmaterialgeneric
{
  PRCmaterialgeneric() : alpha(1.0),shininess(1.0) {}
  PRCmaterialgeneric(const RGBAColour& a, const RGBAColour& d, const RGBAColour& e,
              const RGBAColour& s, double p, double h) :
      ambient(a), diffuse(d), emissive(e), specular(s), alpha(p), shininess(h) {}
  PRCmaterialgeneric(const PRCmaterial& m) :
      ambient(m.ambient), diffuse(m.diffuse), emissive(m.emissive), specular(m.specular), alpha(m.alpha), shininess(m.shininess) {}
  RGBAColour ambient,diffuse,emissive,specular;
  double alpha,shininess;

  bool operator==(const PRCmaterialgeneric& m) const
  {
    return (ambient==m.ambient && diffuse==m.diffuse && emissive==m.emissive
        && specular==m.specular && alpha==m.alpha && shininess==m.shininess);
  }
  bool operator<(const PRCmaterialgeneric& m) const
  {
    if(ambient!=m.ambient)
      return (ambient<m.ambient);
    if(diffuse!=m.diffuse)
      return (diffuse<m.diffuse);
    if(emissive!=m.emissive)
      return (emissive<m.emissive);
    if(specular!=m.specular)
      return (specular<m.specular);
    if(alpha!=m.alpha)
      return (alpha<m.alpha);
    if(shininess!=m.shininess)
      return (shininess<m.shininess);
    return false;
  }
};
typedef std::map<PRCmaterialgeneric,uint32_t> PRCmaterialgenericMap;

struct PRCtexturedefinition
{
  PRCtexturedefinition() :
      picture_index(m1), picture_replace(false), picture_repeat(false) {}
  PRCtexturedefinition(uint32_t picindex, bool picreplace=false, bool picrepeat=false) :
      picture_index(picindex), picture_replace(picreplace), picture_repeat(picrepeat) {}
  PRCtexturedefinition(uint32_t picindex, const PRCmaterial& m) :
      picture_index(picindex), picture_replace(m.picture_replace), picture_repeat(m.picture_repeat) {}
  uint32_t picture_index;
  bool picture_replace; // replace material color with texture color? if false - just modify
  bool picture_repeat;  // repeat texture? if false - clamp to edge

  bool operator==(const PRCtexturedefinition& t) const
  {
    return (picture_index==t.picture_index
        && picture_replace==t.picture_replace && picture_repeat==t.picture_repeat);
  }
  bool operator<(const PRCtexturedefinition& t) const
  {
    if(picture_index!=t.picture_index)
      return (picture_index<t.picture_index);
    if(picture_replace!=t.picture_replace)
      return (picture_replace<t.picture_replace);
    if(picture_repeat!=t.picture_repeat)
      return (picture_repeat<t.picture_repeat);
    return false;
  }
};
typedef std::map<PRCtexturedefinition,uint32_t> PRCtexturedefinitionMap;

struct PRCtextureapplication
{
  PRCtextureapplication() :
      material_generic_index(m1), texture_definition_index(m1) {}
  PRCtextureapplication(uint32_t matindex, uint32_t texindex) :
      material_generic_index(matindex), texture_definition_index(texindex) {}
  uint32_t material_generic_index;
  uint32_t texture_definition_index;

  bool operator==(const PRCtextureapplication& t) const
  {
    return (material_generic_index==t.material_generic_index
        && texture_definition_index==t.texture_definition_index);
  }
  bool operator<(const PRCtextureapplication& t) const
  {
    if(material_generic_index!=t.material_generic_index)
      return (material_generic_index<t.material_generic_index);
    if(texture_definition_index!=t.texture_definition_index)
      return (texture_definition_index<t.texture_definition_index);
    return false;
  }
};
typedef std::map<PRCtextureapplication,uint32_t> PRCtextureapplicationMap;

struct PRCstyle
{
  PRCstyle() :
      line_width(0), alpha(1), is_material(false), color_material_index(m1) {}
  PRCstyle(double linewidth, double alph, bool ismat, uint32_t colindex=m1) :
      line_width(linewidth), alpha(alph), is_material(ismat), color_material_index(colindex) {}
  double line_width;
  double alpha;
  bool is_material;
  uint32_t color_material_index;

  bool operator==(const PRCstyle& s) const
  {
    return (line_width==s.line_width && alpha==s.alpha && is_material==s.is_material
        && color_material_index==s.color_material_index);
  }
  bool operator<(const PRCstyle& s) const
  {
    if(line_width!=s.line_width)
      return (line_width<s.line_width);
    if(alpha!=s.alpha)
      return (alpha<s.alpha);
    if(is_material!=s.is_material)
      return (is_material<s.is_material);
    if(color_material_index!=s.color_material_index)
      return (color_material_index<s.color_material_index);
    return false;
  }
};
typedef std::map<PRCstyle,uint32_t> PRCstyleMap;

struct PRCtessrectangle // rectangle
{
  PRCVector3d vertices[4];
  uint32_t style;
};
typedef std::vector<PRCtessrectangle> PRCtessrectangleList;

struct PRCtessquad // rectangle
{
  PRCVector3d vertices[4];
  RGBAColour  colours[4];
};
typedef std::vector<PRCtessquad> PRCtessquadList;
/*
struct PRCtesstriangle // textured triangle
{
  PRCtesstriangle() :
  style(m1) {}
  PRCVector3d vertices[3];
// PRCVector3d normals[3];
// RGBAColour  colors[3];
  PRCVector2d texcoords[3];
  uint32_t style;
};
typedef std::vector<PRCtesstriangle> PRCtesstriangleList;
*/
struct PRCtessline // polyline
{
  std::vector<PRCVector3d> point;
  PRCRgbColor color;
};
typedef std::list<PRCtessline> PRCtesslineList;
typedef std::map<double, PRCtesslineList> PRCtesslineMap;

struct PRCface
{
  PRCface() : transform(NULL), face(NULL) {}
  uint32_t style;
  bool transparent;
  PRCGeneralTransformation3d*  transform;
  PRCFace* face;
};
typedef std::vector <PRCface>  PRCfaceList;

struct PRCcompface
{
  PRCcompface() : face(NULL) {}
  uint32_t style;
  bool transparent;
  PRCCompressedFace* face;
};
typedef std::vector <PRCcompface>  PRCcompfaceList;

struct PRCwire
{
  PRCwire() : style(m1), transform(NULL), curve(NULL) {}
  uint32_t style;
  PRCGeneralTransformation3d*  transform;
  PRCCurve* curve;
};
typedef std::vector <PRCwire>  PRCwireList;

typedef std::map <uint32_t,std::vector<PRCVector3d> >  PRCpointsetMap;

class PRCoptions
{
public:
  double compression;
  double granularity;

  bool closed;   // render the surface as one-sided; may yield faster rendering
  bool tess;     // use tessellated mesh to store straight patches
  bool do_break; //
  bool no_break; // do not render transparent patches as one-faced nodes
  double crease_angle; // crease angle for meshes

  PRCoptions(double compression=0.0, double granularity=0.0, bool closed=false,
             bool tess=false, bool do_break=true, bool no_break=false, double crease_angle=25.8419)
    : compression(compression), granularity(granularity), closed(closed),
      tess(tess), do_break(do_break), no_break(no_break), crease_angle(crease_angle) {}
};

class PRCgroup
{
 public:
  PRCgroup() :
    product_occurrence(NULL), parent_product_occurrence(NULL), part_definition(NULL), parent_part_definition(NULL), transform(NULL) {}
  PRCgroup(const std::string& name) :
    product_occurrence(NULL), parent_product_occurrence(NULL), part_definition(NULL), parent_part_definition(NULL), transform(NULL), name(name) {}
  PRCProductOccurrence *product_occurrence, *parent_product_occurrence;
  PRCPartDefinition *part_definition, *parent_part_definition;
  PRCfaceList       faces;
  PRCcompfaceList   compfaces;
  PRCtessrectangleList  rectangles;
// PRCtesstriangleList   triangles;
  PRCtessquadList       quads;
  PRCtesslineMap        lines;
  PRCwireList           wires;
  PRCpointsetMap        points;
  std::vector<PRCPointSet*>      pointsets;
  std::vector<PRCPolyBrepModel*> polymodels;
  std::vector<PRCPolyWire*>      polywires;
  PRCGeneralTransformation3d*  transform;
  std::string name;
  PRCoptions options;
};

void makeFileUUID(PRCUniqueId&);
void makeAppUUID(PRCUniqueId&);

class PRCUncompressedFile
{
  public:
    PRCUncompressedFile() : file_size(0), data(NULL) {}
    PRCUncompressedFile(uint32_t fs, uint8_t *d) : file_size(fs), data(d) {}
    ~PRCUncompressedFile() { if(data != NULL) delete[] data; }
    uint32_t file_size;
    uint8_t *data;

    void write(std::ostream&) const;

    uint32_t getSize() const;
};
typedef std::deque <PRCUncompressedFile*>  PRCUncompressedFileList;

class PRCStartHeader
{
  public:
    uint32_t minimal_version_for_read; // PRCVersion
    uint32_t authoring_version; // PRCVersion
    PRCUniqueId file_structure_uuid;
    PRCUniqueId application_uuid; // should be 0

    PRCStartHeader() :
      minimal_version_for_read(PRCVersion), authoring_version(PRCVersion) {}
    void serializeStartHeader(std::ostream&) const;

    uint32_t getStartHeaderSize() const;
};

class PRCFileStructure : public PRCStartHeader
{
  public:
    uint32_t number_of_referenced_file_structures;
    double tessellation_chord_height_ratio;
    double tessellation_angle_degree;
    std::string default_font_family_name;
    std::vector<PRCRgbColor> colors;
    std::vector<PRCPicture> pictures;
    PRCUncompressedFileList uncompressed_files;
    PRCTextureDefinitionList texture_definitions;
    PRCMaterialList materials;
    PRCStyleList styles;
    PRCCoordinateSystemList reference_coordinate_systems;
    std::vector<PRCFontKeysSameFont> font_keys_of_font;
    PRCPartDefinitionList part_definitions;
    PRCProductOccurrenceList product_occurrences;
//  PRCMarkupList markups;
//  PRCAnnotationItemList annotation_entities;
    double unit;
    PRCTopoContextList contexts;
    PRCTessList tessellations;

    uint32_t sizes[6];
    uint8_t *globals_data;
    PRCbitStream globals_out; // order matters: PRCbitStream must be initialized last
    uint8_t *tree_data;
    PRCbitStream tree_out;
    uint8_t *tessellations_data;
    PRCbitStream tessellations_out;
    uint8_t *geometry_data;
    PRCbitStream geometry_out;
    uint8_t *extraGeometry_data;
    PRCbitStream extraGeometry_out;

    ~PRCFileStructure () {
      for(PRCUncompressedFileList::iterator  it=uncompressed_files.begin();  it!=uncompressed_files.end();  ++it) delete *it;
      for(PRCTextureDefinitionList::iterator it=texture_definitions.begin(); it!=texture_definitions.end(); ++it) delete *it;
      for(PRCMaterialList::iterator          it=materials.begin();           it!=materials.end();           ++it) delete *it;
      for(PRCStyleList::iterator             it=styles.begin();              it!=styles.end();              ++it) delete *it;
      for(PRCTopoContextList::iterator       it=contexts.begin();            it!=contexts.end();            ++it) delete *it;
      for(PRCTessList::iterator              it=tessellations.begin();       it!=tessellations.end();       ++it) delete *it;
      for(PRCPartDefinitionList::iterator    it=part_definitions.begin();    it!=part_definitions.end();    ++it) delete *it;
      for(PRCProductOccurrenceList::iterator it=product_occurrences.begin(); it!=product_occurrences.end(); ++it) delete *it;
      for(PRCCoordinateSystemList::iterator  it=reference_coordinate_systems.begin(); it!=reference_coordinate_systems.end(); it++)
        delete *it;

      free(globals_data);
      free(tree_data);
      free(tessellations_data);
      free(geometry_data);
      free(extraGeometry_data);
    }

    PRCFileStructure() :
      number_of_referenced_file_structures(0),
      tessellation_chord_height_ratio(2000.0),tessellation_angle_degree(40.0),
      default_font_family_name(""),
      unit(1),
      globals_data(NULL),globals_out(globals_data,0),
      tree_data(NULL),tree_out(tree_data,0),
      tessellations_data(NULL),tessellations_out(tessellations_data,0),
      geometry_data(NULL),geometry_out(geometry_data,0),
      extraGeometry_data(NULL),extraGeometry_out(extraGeometry_data,0) {}
    void write(std::ostream&);
    void prepare();
    uint32_t getSize();
    void serializeFileStructureGlobals(PRCbitStream&);
    void serializeFileStructureTree(PRCbitStream&);
    void serializeFileStructureTessellation(PRCbitStream&);
    void serializeFileStructureGeometry(PRCbitStream&);
    void serializeFileStructureExtraGeometry(PRCbitStream&);
    uint32_t addPicture(EPRCPictureDataFormat format, uint32_t size, const uint8_t *picture, uint32_t width=0, uint32_t height=0, std::string name="");
    uint32_t addTextureDefinition(PRCTextureDefinition*& pTextureDefinition);
    uint32_t addRgbColor(const PRCRgbColor &color);
    uint32_t addRgbColorUnique(const PRCRgbColor &color);
    uint32_t addMaterialGeneric(PRCMaterialGeneric*& pMaterialGeneric);
    uint32_t addTextureApplication(PRCTextureApplication*& pTextureApplication);
    uint32_t addStyle(PRCStyle*& pStyle);
    uint32_t addPartDefinition(PRCPartDefinition*& pPartDefinition);
    uint32_t addProductOccurrence(PRCProductOccurrence*& pProductOccurrence);
    uint32_t addTopoContext(PRCTopoContext*& pTopoContext);
    uint32_t getTopoContext(PRCTopoContext*& pTopoContext);
    uint32_t add3DTess(PRC3DTess*& p3DTess);
    uint32_t add3DWireTess(PRC3DWireTess*& p3DWireTess);
/*
    uint32_t addMarkupTess(PRCMarkupTess*& pMarkupTess);
    uint32_t addMarkup(PRCMarkup*& pMarkup);
    uint32_t addAnnotationItem(PRCAnnotationItem*& pAnnotationItem);
 */
    uint32_t addCoordinateSystem(PRCCoordinateSystem*& pCoordinateSystem);
    uint32_t addCoordinateSystemUnique(PRCCoordinateSystem*& pCoordinateSystem);
};

class PRCFileStructureInformation
{
  public:
    PRCUniqueId UUID;
    uint32_t reserved; // 0
    uint32_t number_of_offsets;
    uint32_t *offsets;

    void write(std::ostream&);

    uint32_t getSize();
};

class PRCHeader : public PRCStartHeader
{
  public :
    uint32_t number_of_file_structures;
    PRCFileStructureInformation *fileStructureInformation;
    uint32_t model_file_offset;
    uint32_t file_size; // not documented
    PRCUncompressedFileList uncompressed_files;

    void write(std::ostream&);
    uint32_t getSize();
};

typedef std::map <PRCGeneralTransformation3d,uint32_t> PRCtransformMap;

inline double X(const double *v) {return v[0];}
inline double Y(const double *v) {return v[1];}
inline double Z(const double *v) {return v[2];}

class oPRCFile
{
  public:
    oPRCFile(std::ostream &os, double u=1, uint32_t n=1) :
      number_of_file_structures(n),
      fileStructures(new PRCFileStructure*[n]),
      unit(u),
      modelFile_data(NULL),modelFile_out(modelFile_data,0),
      fout(NULL),output(os)
      {
        for(uint32_t i = 0; i < number_of_file_structures; ++i)
        {
          fileStructures[i] = new PRCFileStructure();
          fileStructures[i]->minimal_version_for_read = PRCVersion;
          fileStructures[i]->authoring_version = PRCVersion;
          makeFileUUID(fileStructures[i]->file_structure_uuid);
          makeAppUUID(fileStructures[i]->application_uuid);
          fileStructures[i]->unit = u;
        }

        groups.push(PRCgroup());
        PRCgroup &group = groups.top();
        group.name="root";
        group.transform = NULL;
        group.product_occurrence = new PRCProductOccurrence(group.name);
        group.parent_product_occurrence = NULL;
        group.part_definition = new PRCPartDefinition;
        group.parent_part_definition = NULL;
      }

    oPRCFile(const std::string &name, double u=1, uint32_t n=1) :
      number_of_file_structures(n),
      fileStructures(new PRCFileStructure*[n]),
      unit(u),
      modelFile_data(NULL),modelFile_out(modelFile_data,0),
#if defined(MIKTEX_WINDOWS)
      fout(new std::ofstream(MiKTeX::Util::CharBuffer<wchar_t>(name).GetData(), std::ios::out|std::ios::binary|std::ios::trunc)),

#else
      fout(new std::ofstream(name.c_str(),
                             std::ios::out|std::ios::binary|std::ios::trunc)),
#endif
      output(*fout)
      {
        for(uint32_t i = 0; i < number_of_file_structures; ++i)
        {
          fileStructures[i] = new PRCFileStructure();
          fileStructures[i]->minimal_version_for_read = PRCVersion;
          fileStructures[i]->authoring_version = PRCVersion;
          makeFileUUID(fileStructures[i]->file_structure_uuid);
          makeAppUUID(fileStructures[i]->application_uuid);
          fileStructures[i]->unit = u;
        }

        groups.push(PRCgroup());
        PRCgroup &group = groups.top();
        group.name="root";
        group.transform = NULL;
        group.product_occurrence = new PRCProductOccurrence(group.name);
        group.parent_product_occurrence = NULL;
        group.part_definition = new PRCPartDefinition;
        group.parent_part_definition = NULL;
      }

    ~oPRCFile()
    {
      for(uint32_t i = 0; i < number_of_file_structures; ++i)
        delete fileStructures[i];
      delete[] fileStructures;
      if(fout != NULL)
        delete fout;
      free(modelFile_data);
      for(PRCpictureMap::iterator it=pictureMap.begin(); it!=pictureMap.end(); ++it) delete it->first.data;
    }

    void begingroup(const char *name, PRCoptions *options=NULL,
                    const double* t=NULL);
    void endgroup();

    std::string lastgroupname;
    std::vector<std::string> lastgroupnames;
    std::string calculate_unique_name(const ContentPRCBase *prc_entity,const ContentPRCBase *prc_occurence);

    bool finish();
    uint32_t getSize();

    const uint32_t number_of_file_structures;
    PRCFileStructure **fileStructures;
    PRCHeader header;
    PRCUnit unit;
    uint8_t *modelFile_data;
    PRCbitStream modelFile_out; // order matters: PRCbitStream must be initialized last
    PRCcolorMap colorMap;
    PRCcolourMap colourMap;
    PRCcolourwidthMap colourwidthMap;
    PRCmaterialgenericMap materialgenericMap;
    PRCtexturedefinitionMap texturedefinitionMap;
    PRCtextureapplicationMap textureapplicationMap;
    PRCstyleMap styleMap;
    PRCpictureMap pictureMap;
    PRCgroup rootGroup;
    PRCtransformMap transformMap;
    std::stack<PRCgroup> groups;
    PRCgroup& findGroup();
    void doGroup(PRCgroup& group);
    uint32_t addColor(const PRCRgbColor &color);
    uint32_t addColour(const RGBAColour &colour);
    uint32_t addColourWidth(const RGBAColour &colour, double width);
    uint32_t addLineMaterial(const RGBAColour& c, double width)
               { return addColourWidth(c,width); }
    uint32_t addMaterial(const PRCmaterial &material);
    uint32_t addTransform(PRCGeneralTransformation3d*& transform);
    uint32_t addTransform(const double* t);
    uint32_t addTransform(const double origin[3], const double x_axis[3], const double y_axis[3], double scale);
template<class V>
void addPoint(const V P, const RGBAColour &c, double w=1.0)
{
  PRCgroup &group = findGroup();
  group.points[addColourWidth(c,w)].push_back(PRCVector3d(X(P),Y(P),Z(P)));
}
    void addPoints(uint32_t n, const double P[][3], const RGBAColour &c, double w=1.0);
    void addLines(uint32_t nP, const double P[][3], uint32_t nI, const uint32_t PI[],
                      const RGBAColour& c, double w,
                      bool segment_color, uint32_t nC, const RGBAColour C[], uint32_t nCI, const uint32_t CI[]);
    uint32_t createLines(uint32_t nP, const double P[][3], uint32_t nI, const uint32_t PI[],
                      bool segment_color, uint32_t nC, const RGBAColour C[], uint32_t nCI, const uint32_t CI[]);
template<class V>
    void addTriangles(uint32_t nP, const V P[], uint32_t nI, const uint32_t PI[][3], const PRCmaterial &m,
 uint32_t nN, const V N[],   const uint32_t NI[][3],
 uint32_t nT, const double T[][2],   const uint32_t TI[][3],
 uint32_t nC, const RGBAColour C[],  const uint32_t CI[][3],
 uint32_t nM, const PRCmaterial M[], const uint32_t MI[], double ca)
{
  if(nP==0 || P==NULL || nI==0 || PI==NULL)
     return;
  const uint32_t tess_index = createTriangleMesh(nP, P, nI, PI, m, nN, N, NI, nT, T, TI, nC, C, CI, nM, M, MI, ca);
  useMesh(tess_index,m1);
}
template<class V>
    uint32_t createTriangleMesh(uint32_t nP, const V P[], uint32_t nI, const uint32_t PI[][3], const PRCmaterial& m,
                      uint32_t nN, const V N[],   const uint32_t NI[][3],
                      uint32_t nT, const double T[][2],   const uint32_t TI[][3],
                      uint32_t nC, const RGBAColour C[],  const uint32_t CI[][3],
                      uint32_t nM, const PRCmaterial M[], const uint32_t MI[], double ca)
            {
               const uint32_t style = addMaterial(m);
               if(M!=NULL && nM>0)
               {
                 uint32_t* const styles = new uint32_t[nM];
                 for(uint32_t i=0; i<nM; i++)
                   styles[i]=addMaterial(M[i]);
                 const uint32_t meshid =  createTriangleMesh(nP, P, nI, PI, style, nN, N, NI, nT, T, TI, nC, C, CI, nM, styles, MI, ca);
                 delete[] styles;
                 return meshid;
               }
               else
                 return createTriangleMesh(nP, P, nI, PI, style, nN, N, NI, nT, T, TI, nC, C, CI, 0, NULL, NULL, ca);
            }

template<class V>
uint32_t createTriangleMesh(uint32_t nP, const V P[], uint32_t nI, const uint32_t PI[][3], const uint32_t style_index,
uint32_t nN, const V N[],  const uint32_t NI[][3],
 uint32_t nT, const double T[][2],  const uint32_t TI[][3],
 uint32_t nC, const RGBAColour C[], const uint32_t CI[][3],
 uint32_t nS, const uint32_t S[],   const uint32_t SI[], double ca)
{
  if(nP==0 || P==NULL || nI==0 || PI==NULL)
     return m1;

  const bool triangle_color = (nS != 0 && S != NULL && SI != NULL);
  const bool vertex_color   = (nC != 0 && C != NULL && CI != NULL);
  const bool has_normals    = (nN != 0 && N != NULL && NI != NULL);
  const bool textured       = (nT != 0 && T != NULL && TI != NULL);

  PRC3DTess *tess = new PRC3DTess();
  PRCTessFace *tessFace = new PRCTessFace();
  tessFace->used_entities_flag = textured ? PRC_FACETESSDATA_TriangleTextured : PRC_FACETESSDATA_Triangle;
  tessFace->number_of_texture_coordinate_indexes = textured ? 1 : 0;
  tess->coordinates.reserve(3*nP);
  for(uint32_t i=0; i<nP; i++)
  {
    tess->coordinates.push_back(X(P[i]));
    tess->coordinates.push_back(Y(P[i]));
    tess->coordinates.push_back(Z(P[i]));
  }
  if(has_normals)
  {
    tess->normal_coordinate.reserve(3*nN);
  for(uint32_t i=0; i<nN; i++)
  {
    tess->normal_coordinate.push_back(X(N[i]));
    tess->normal_coordinate.push_back(Y(N[i]));
    tess->normal_coordinate.push_back(Z(N[i]));
  }
  }
  else
    tess->crease_angle = ca;
  if(textured)
  {
    tess->texture_coordinate.reserve(2*nT);
  for(uint32_t i=0; i<nT; i++)
  {
    tess->texture_coordinate.push_back(T[i][0]);
    tess->texture_coordinate.push_back(T[i][1]);
  }
  }
  tess->triangulated_index.reserve(3*nI+(has_normals?3:0)*nI+(textured?3:0)*nI);
  for(uint32_t i=0; i<nI; i++)
  {
    if(has_normals)
    tess->triangulated_index.push_back(3*NI[i][0]);
    if(textured)
    tess->triangulated_index.push_back(2*TI[i][0]);
    tess->triangulated_index.push_back(3*PI[i][0]);
    if(has_normals)
    tess->triangulated_index.push_back(3*NI[i][1]);
    if(textured)
    tess->triangulated_index.push_back(2*TI[i][1]);
    tess->triangulated_index.push_back(3*PI[i][1]);
    if(has_normals)
    tess->triangulated_index.push_back(3*NI[i][2]);
    if(textured)
    tess->triangulated_index.push_back(2*TI[i][2]);
    tess->triangulated_index.push_back(3*PI[i][2]);
  }
  tessFace->sizes_triangulated.push_back(nI);
  if(triangle_color)
  {
    tessFace->line_attributes.reserve(nI);
    for(uint32_t i=0; i<nI; i++)
       tessFace->line_attributes.push_back(SI[i]);
  }
  else if (style_index != m1 )
  {
      tessFace->line_attributes.push_back(style_index);
  }
  if(vertex_color)
  {
    tessFace->is_rgba=false;
    for(uint32_t i=0; i<nI; i++)
      if(1.0 != C[CI[i][0]].A || 1.0 != C[CI[i][1]].A || 1.0 != C[CI[i][2]].A)
      {
         tessFace->is_rgba=true;
         break;
      }

    tessFace->rgba_vertices.reserve((tessFace->is_rgba?4:3)*3*nI);
    for(uint32_t i=0; i<nI; i++)
    {
       tessFace->rgba_vertices.push_back(byte(C[CI[i][0]].R));
       tessFace->rgba_vertices.push_back(byte(C[CI[i][0]].G));
       tessFace->rgba_vertices.push_back(byte(C[CI[i][0]].B));
       if(tessFace->is_rgba)
       tessFace->rgba_vertices.push_back(byte(C[CI[i][0]].A));
       tessFace->rgba_vertices.push_back(byte(C[CI[i][1]].R));
       tessFace->rgba_vertices.push_back(byte(C[CI[i][1]].G));
       tessFace->rgba_vertices.push_back(byte(C[CI[i][1]].B));
       if(tessFace->is_rgba)
       tessFace->rgba_vertices.push_back(byte(C[CI[i][1]].A));
       tessFace->rgba_vertices.push_back(byte(C[CI[i][2]].R));
       tessFace->rgba_vertices.push_back(byte(C[CI[i][2]].G));
       tessFace->rgba_vertices.push_back(byte(C[CI[i][2]].B));
       if(tessFace->is_rgba)
       tessFace->rgba_vertices.push_back(byte(C[CI[i][2]].A));
    }
  }
  tess->addTessFace(tessFace);
  const uint32_t tess_index = add3DTess(tess);
  return tess_index;
}

    void addQuads(uint32_t nP, const double P[][3], uint32_t nI, const uint32_t PI[][4], const PRCmaterial &m,
                  uint32_t nN, const double N[][3],   const uint32_t NI[][4],
                  uint32_t nT, const double T[][2],   const uint32_t TI[][4],
                  uint32_t nC, const RGBAColour C[],  const uint32_t CI[][4],
                  uint32_t nM, const PRCmaterial M[], const uint32_t MI[], double ca);
    uint32_t createQuadMesh(uint32_t nP, const double P[][3], uint32_t nI, const uint32_t PI[][4], uint32_t style_index,
                      uint32_t nN, const double N[][3],   const uint32_t NI[][4],
                      uint32_t nT, const double T[][2],   const uint32_t TI[][4],
                      uint32_t nC, const RGBAColour C[],  const uint32_t CI[][4],
                      uint32_t nS, const uint32_t S[],    const uint32_t SI[], double ca);
    uint32_t createQuadMesh(uint32_t nP, const double P[][3], uint32_t nI, const uint32_t PI[][4], const PRCmaterial& m,
                      uint32_t nN, const double N[][3],   const uint32_t NI[][4],
                      uint32_t nT, const double T[][2],   const uint32_t TI[][4],
                      uint32_t nC, const RGBAColour C[],  const uint32_t CI[][4],
                      uint32_t nM, const PRCmaterial M[], const uint32_t MI[], double ca)
            {
               const uint32_t style = addMaterial(m);
               if(M!=NULL && nM>0)
               {
                 uint32_t* const styles = new uint32_t[nM];
                 for(uint32_t i=0; i<nM; i++)
                   styles[i]=addMaterial(M[i]);
                 const uint32_t meshid =  createQuadMesh(nP, P, nI, PI, style, nN, N, NI, nT, T, TI, nC, C, CI, nM, styles, MI, ca);
                 delete[] styles;
                 return meshid;
               }
               else
                 return createQuadMesh(nP, P, nI, PI, style, nN, N, NI, nT, T, TI, nC, C, CI, 0, NULL, NULL, ca);
            }
#define PRCTRANSFORM const double origin[3]=NULL, const double x_axis[3]=NULL, const double y_axis[3]=NULL, double scale=1, const double* t=NULL
#define PRCCARTRANSFORM const double origin[3], const double x_axis[3], const double y_axis[3], double scale
#define PRCGENTRANSFORM const double* t=NULL
#define PRCNOMATERIALINDEX m1

#define ADDWIRE(curvtype)                                 \
  PRCgroup &group = findGroup();                          \
  group.wires.push_back(PRCwire());                       \
  PRCwire &wire = group.wires.back();                     \
  curvtype *curve = new curvtype;                         \
  wire.curve = curve;                                     \
  wire.style = addColour(c);

#define ADDFACE(surftype)                                 \
  PRCgroup &group = findGroup();                          \
  group.faces.push_back(PRCface());                       \
  PRCface& face = group.faces.back();                     \
  surftype *surface = new surftype;                       \
  face.face = new PRCFace;                                \
  face.face->base_surface = surface;                      \
  face.transparent = m.alpha < 1.0;                       \
  face.style = addMaterial(m);

#define ADDCOMPFACE                                       \
  PRCgroup &group = findGroup();                          \
  group.compfaces.push_back(PRCcompface());               \
  PRCcompface& face = group.compfaces.back();             \
  PRCCompressedFace *compface = new PRCCompressedFace;    \
  face.face = compface;                                   \
  face.transparent = m.alpha < 1.0;                       \
  face.style = addMaterial(m);

inline bool isid(const double* t)
{
  return(
         t[0]==1 && t[1]==0 && t[2]==0 && t[3]==0 &&
         t[4]==0 && t[5]==1 && t[6]==0 && t[7]==0 &&
         t[8]==0 && t[9]==0 && t[10]==1 && t[11]==0 &&
         t[12]==0 && t[13]==0 && t[14]==0 && t[15]==1);
}

#define SETTRANSF \
  if(t&&!isid(t))                                                             \
    face.transform = new PRCGeneralTransformation3d(t);                       \
  if(origin) surface->origin.Set(origin[0],origin[1],origin[2]);              \
  if(x_axis) surface->x_axis.Set(x_axis[0],x_axis[1],x_axis[2]);              \
  if(y_axis) surface->y_axis.Set(y_axis[0],y_axis[1],y_axis[2]);              \
  surface->scale = scale;                                                     \
  surface->geometry_is_2D = false;                                            \
  if(surface->origin!=PRCVector3d(0.0,0.0,0.0))                                     \
    surface->behaviour = surface->behaviour | PRC_TRANSFORMATION_Translate;   \
  if(surface->x_axis!=PRCVector3d(1.0,0.0,0.0)||surface->y_axis!=PRCVector3d(0.0,1.0,0.0)) \
    surface->behaviour = surface->behaviour | PRC_TRANSFORMATION_Rotate;      \
  if(surface->scale!=1)                                                       \
    surface->behaviour = surface->behaviour | PRC_TRANSFORMATION_Scale;       \
  surface->has_transformation = (surface->behaviour != PRC_TRANSFORMATION_Identity);

    void useMesh(uint32_t tess_index, uint32_t style_index,            PRCGENTRANSFORM);
    void useMesh(uint32_t tess_index, const PRCmaterial& m,            PRCGENTRANSFORM)
           { useMesh(tess_index,addMaterial(m),t); }
    void useMesh(uint32_t tess_index, uint32_t style_index,            PRCCARTRANSFORM);
    void useMesh(uint32_t tess_index, const PRCmaterial& m,            PRCCARTRANSFORM)
           { useMesh(tess_index,addMaterial(m),origin, x_axis, y_axis, scale); }

    void useLines(uint32_t tess_index, uint32_t style_index,           PRCGENTRANSFORM);
    void useLines(uint32_t tess_index, const RGBAColour& c,  double w, PRCGENTRANSFORM)
           { useLines(tess_index, addLineMaterial(c,w), t); }
    void useLines(uint32_t tess_index, uint32_t style_index,           PRCCARTRANSFORM);
    void useLines(uint32_t tess_index, const RGBAColour& c,  double w, PRCCARTRANSFORM)
           { useLines(tess_index,addLineMaterial(c,w),origin, x_axis, y_axis, scale); }

//  void addTriangle(const double P[][3], const double T[][2], uint32_t style_index);

template<class V>
void addLine(uint32_t n, const V P[], const RGBAColour &c, double w=1.0)
{
  PRCgroup &group = findGroup();
  if(group.options.tess)
  {
    group.lines[w].push_back(PRCtessline());
    PRCtessline& line = group.lines[w].back();
    line.color.red   = c.R;
    line.color.green = c.G;
    line.color.blue  = c.B;
    for(uint32_t i=0; i<n; i++)
      line.point.push_back(PRCVector3d(X(P[i]),Y(P[i]),Z(P[i])));
  }
  else
  {
    ADDWIRE(PRCPolyLine)
    curve->point.resize(n);
    for(uint32_t i=0; i<n; i++)
      curve->point[i].Set(X(P[i]),Y(P[i]),Z(P[i]));
    curve->interval.min = 0;
    curve->interval.max = curve->point.size()-1;
  }
}
template<class V>
void addBezierCurve(uint32_t n, const V cP[], const RGBAColour &c)
{
  ADDWIRE(PRCNURBSCurve)
  curve->is_rational = false;
  curve->degree = 3;
  const size_t NUMBER_OF_POINTS = n;
  curve->control_point.resize(NUMBER_OF_POINTS);
  for(size_t i = 0; i < NUMBER_OF_POINTS; ++i)
    curve->control_point[i].Set(X(cP[i]),Y(cP[i]),Z(cP[i]));
  curve->knot.resize(3+NUMBER_OF_POINTS+1);
  curve->knot[0] = 1;
  for(size_t i = 1; i < 3+NUMBER_OF_POINTS; ++i)
    curve->knot[i] = (i+2)/3; // integer division is intentional
  curve->knot[3+NUMBER_OF_POINTS] = (3+NUMBER_OF_POINTS+1)/3;
}

template<class V>
void addCurve(uint32_t d, uint32_t n, const V cP[], const double *k, const RGBAColour &c, const double w[])
{
  ADDWIRE(PRCNURBSCurve)
  curve->is_rational = (w!=NULL);
  curve->degree = d;
  curve->control_point.resize(n);
  for(uint32_t i = 0; i < n; i++)
    if(w)
      curve->control_point[i].Set(X(cP[i])*w[i],Y(cP[i])*w[i],Z(cP[i])*w[i],
                                  w[i]);
    else
      curve->control_point[i].Set(X(cP[i]),Y(cP[i]),Z(cP[i]));
  curve->knot.resize(d+n+1);
  for(uint32_t i = 0; i < d+n+1; i++)
    curve->knot[i] = k[i];
}

template<class V>
void addQuad(const V P[], const RGBAColour C[])
{
  PRCgroup &group = findGroup();

  group.quads.push_back(PRCtessquad());
  PRCtessquad &quad = group.quads.back();
  for(size_t i = 0; i < 4; i++)
  {
    quad.vertices[i].x = X(P[i]);
    quad.vertices[i].y = Y(P[i]);
    quad.vertices[i].z = Z(P[i]);
    quad.colours[i] = C[i];
  }
}

template<class V>
void addRectangle(const V P[], const PRCmaterial &m)
{
  PRCgroup &group = findGroup();
  if(group.options.tess)
  {
    group.rectangles.push_back(PRCtessrectangle());
    PRCtessrectangle &rectangle = group.rectangles.back();
    rectangle.style = addMaterial(m);
    for(size_t i = 0; i < 4; i++)
    {
      rectangle.vertices[i].x = X(P[i]);
      rectangle.vertices[i].y = Y(P[i]);
      rectangle.vertices[i].z = Z(P[i]);
    }
  }
  else if(group.options.compression == 0.0)
  {
    ADDFACE(PRCNURBSSurface)

    surface->is_rational = false;
    surface->degree_in_u = 1;
    surface->degree_in_v = 1;
    surface->control_point.resize(4);
    for(size_t i = 0; i < 4; ++i)
    {
      surface->control_point[i].x = X(P[i]);
      surface->control_point[i].y = Y(P[i]);
      surface->control_point[i].z = Z(P[i]);
    }
    surface->knot_u.resize(4);
    surface->knot_v.resize(4);
    surface->knot_v[0] = surface->knot_u[0] = 1;
    surface->knot_v[1] = surface->knot_u[1] = 3;
    surface->knot_v[2] = surface->knot_u[2] = 4;
    surface->knot_v[3] = surface->knot_u[3] = 4;
  }
  else
  {
    ADDCOMPFACE

    compface->degree = 1;
    compface->control_point.resize(4);
    for(size_t i = 0; i < 4; ++i)
    {
      compface->control_point[i].x = X(P[i]);
      compface->control_point[i].y = Y(P[i]);
      compface->control_point[i].z = Z(P[i]);
    }
  }
}

template<class V>
void addPatch(const V cP[], const PRCmaterial &m)
{
  PRCgroup &group = findGroup();
  if(group.options.compression == 0.0)
  {
    ADDFACE(PRCNURBSSurface)

    surface->is_rational = false;
    surface->degree_in_u = 3;
    surface->degree_in_v = 3;
    surface->control_point.resize(16);
    for(size_t i = 0; i < 16; ++i)
    {
      surface->control_point[i].x = X(cP[i]);
      surface->control_point[i].y = Y(cP[i]);
      surface->control_point[i].z = Z(cP[i]);
    }
    surface->knot_u.resize(8);
    surface->knot_v.resize(8);
    surface->knot_v[0] = surface->knot_u[0] = 1;
    surface->knot_v[1] = surface->knot_u[1] = 1;
    surface->knot_v[2] = surface->knot_u[2] = 1;
    surface->knot_v[3] = surface->knot_u[3] = 1;
    surface->knot_v[4] = surface->knot_u[4] = 2;
    surface->knot_v[5] = surface->knot_u[5] = 2;
    surface->knot_v[6] = surface->knot_u[6] = 2;
    surface->knot_v[7] = surface->knot_u[7] = 2;
  }
  else
  {
    ADDCOMPFACE

    compface->degree = 3;
    compface->control_point.resize(16);
    for(size_t i = 0; i < 16; ++i)
    {
      compface->control_point[i].x = X(cP[i]);
      compface->control_point[i].y = Y(cP[i]);
      compface->control_point[i].z = Z(cP[i]);
    }
  }
}

template<class V>
void addSurface(uint32_t dU, uint32_t dV, uint32_t nU, uint32_t nV,
                const V cP[], const double *kU,
                const double *kV, const PRCmaterial &m,
                const double w[])
{
  ADDFACE(PRCNURBSSurface)

  surface->is_rational = (w!=NULL);
  surface->degree_in_u = dU;
  surface->degree_in_v = dV;
  surface->control_point.resize(nU*nV);
  for(size_t i = 0; i < nU*nV; i++)
    if(w)
      surface->control_point[i]=PRCControlPoint(X(cP[i])*w[i],Y(cP[i])*w[i],Z(cP[i])*w[i],w[i]);
    else
      surface->control_point[i]=PRCControlPoint(X(cP[i]),Y(cP[i]),Z(cP[i]));
  surface->knot_u.insert(surface->knot_u.end(), kU, kU+(dU+nU+1));
  surface->knot_v.insert(surface->knot_v.end(), kV, kV+(dV+nV+1));
}
template<class V>
void addTube(uint32_t n, const V cP[], const V oP[], bool straight, const PRCmaterial &m, PRCTRANSFORM)
{
  ADDFACE(PRCBlend01)
  SETTRANSF
  if(straight)
  {
    PRCPolyLine *center_curve = new PRCPolyLine;
    center_curve->point.resize(n);
    for(uint32_t i=0; i<n; i++)
      center_curve->point[i].Set(X(cP[i]),Y(cP[i]),Z(cP[i]));
    center_curve->interval.min = 0;
    center_curve->interval.max = center_curve->point.size()-1;
    surface->center_curve = center_curve;

    PRCPolyLine *origin_curve = new PRCPolyLine;
    origin_curve->point.resize(n);
    for(uint32_t i=0; i<n; i++)
      origin_curve->point[i].Set(X(oP[i]),Y(oP[i]),Z(oP[i]));
    origin_curve->interval.min = 0;
    origin_curve->interval.max = origin_curve->point.size()-1;
    surface->origin_curve = origin_curve;

    surface->uv_domain.min.x = 0;
    surface->uv_domain.max.x = 2*pi;
    surface->uv_domain.min.y = 0;
    surface->uv_domain.max.y = n-1;
  }
  else
  {
    PRCNURBSCurve *center_curve = new PRCNURBSCurve;
    center_curve->is_rational = false;
    center_curve->degree = 3;
    const uint32_t CENTER_NUMBER_OF_POINTS = n;
    center_curve->control_point.resize(CENTER_NUMBER_OF_POINTS);
    for(uint32_t i = 0; i < CENTER_NUMBER_OF_POINTS; ++i)
      center_curve->control_point[i].Set(X(cP[i]),Y(cP[i]),Z(cP[i]));
    center_curve->knot.resize(3+CENTER_NUMBER_OF_POINTS+1);
    center_curve->knot[0] = 1;
    for(uint32_t i = 1; i < 3+CENTER_NUMBER_OF_POINTS; ++i)
      center_curve->knot[i] = (i+2)/3; // integer division is intentional
    center_curve->knot[3+CENTER_NUMBER_OF_POINTS] = (3+CENTER_NUMBER_OF_POINTS+1)/3;
    surface->center_curve = center_curve;

    PRCNURBSCurve *origin_curve = new PRCNURBSCurve;
    origin_curve->is_rational = false;
    origin_curve->degree = 3;
    const uint32_t ORIGIN_NUMBER_OF_POINTS = n;
    origin_curve->control_point.resize(ORIGIN_NUMBER_OF_POINTS);
    for(uint32_t i = 0; i < ORIGIN_NUMBER_OF_POINTS; ++i)
      origin_curve->control_point[i].Set(X(oP[i]),Y(oP[i]),Z(oP[i]));
    origin_curve->knot.resize(3+ORIGIN_NUMBER_OF_POINTS+1);
    origin_curve->knot[0] = 1;
    for(size_t i = 1; i < 3+ORIGIN_NUMBER_OF_POINTS; ++i)
      origin_curve->knot[i] = (i+2)/3; // integer division is intentional
    origin_curve->knot[3+ORIGIN_NUMBER_OF_POINTS] = (3+ORIGIN_NUMBER_OF_POINTS+1)/3;
    surface->origin_curve = origin_curve;

    surface->uv_domain.min.x = 0;
    surface->uv_domain.max.x = 2*pi;
    surface->uv_domain.min.y = 1; // first knot
    surface->uv_domain.max.y = (3+CENTER_NUMBER_OF_POINTS+1)/3; // last knot
  }
}
    void addHemisphere(double radius, const PRCmaterial& m, PRCTRANSFORM);
    void addSphere(double radius, const PRCmaterial& m, PRCTRANSFORM);
    void addDisk(double radius, const PRCmaterial& m, PRCTRANSFORM);
    void addCylinder(double radius, double height, const PRCmaterial& m, PRCTRANSFORM);
    void addCone(double radius, double height, const PRCmaterial& m, PRCTRANSFORM);
    void addTorus(double major_radius, double minor_radius, double angle1, double angle2, const PRCmaterial& m, PRCTRANSFORM);
#undef PRCTRANSFORM
#undef PRCCARTRANSFORM
#undef PRCGENTRANSFORM
#undef ADDCOMPFACE

    uint32_t addPicture(EPRCPictureDataFormat format, uint32_t size, const uint8_t *picture, uint32_t width=0, uint32_t height=0,
      std::string name="", uint32_t fileStructure=0)
      { return fileStructures[fileStructure]->addPicture(format, size, picture, width, height, name); }
    uint32_t addPicture(const PRCpicture& pic,
      std::string name="", uint32_t fileStructure=0)
      { return fileStructures[fileStructure]->addPicture(pic.format, pic.size, pic.data, pic.width, pic.height, name); }
    uint32_t addTextureDefinition(PRCTextureDefinition*& pTextureDefinition, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addTextureDefinition(pTextureDefinition);
      }
    uint32_t addTextureApplication(PRCTextureApplication*& pTextureApplication, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addTextureApplication(pTextureApplication);
      }
    uint32_t addRgbColor(const PRCRgbColor &color,
       uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addRgbColor(color);
      }
    uint32_t addRgbColorUnique(const PRCRgbColor &color,
       uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addRgbColorUnique(color);
      }
    uint32_t addMaterialGeneric(PRCMaterialGeneric*& pMaterialGeneric,
       uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addMaterialGeneric(pMaterialGeneric);
      }
    uint32_t addStyle(PRCStyle*& pStyle, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addStyle(pStyle);
      }
    uint32_t addPartDefinition(PRCPartDefinition*& pPartDefinition, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addPartDefinition(pPartDefinition);
      }
    uint32_t addProductOccurrence(PRCProductOccurrence*& pProductOccurrence, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addProductOccurrence(pProductOccurrence);
      }
    uint32_t addTopoContext(PRCTopoContext*& pTopoContext, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addTopoContext(pTopoContext);
      }
    uint32_t getTopoContext(PRCTopoContext*& pTopoContext, uint32_t fileStructure=0)
    {
      return fileStructures[fileStructure]->getTopoContext(pTopoContext);
    }
    uint32_t add3DTess(PRC3DTess*& p3DTess, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->add3DTess(p3DTess);
      }
    uint32_t add3DWireTess(PRC3DWireTess*& p3DWireTess, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->add3DWireTess(p3DWireTess);
      }
/*
    uint32_t addMarkupTess(PRCMarkupTess*& pMarkupTess, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addMarkupTess(pMarkupTess);
      }
    uint32_t addMarkup(PRCMarkup*& pMarkup, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addMarkup(pMarkup);
      }
    uint32_t addAnnotationItem(PRCAnnotationItem*& pAnnotationItem, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addAnnotationItem(pAnnotationItem);
      }
 */
    uint32_t addCoordinateSystem(PRCCoordinateSystem*& pCoordinateSystem, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addCoordinateSystem(pCoordinateSystem);
      }
    uint32_t addCoordinateSystemUnique(PRCCoordinateSystem*& pCoordinateSystem, uint32_t fileStructure=0)
      {
        return fileStructures[fileStructure]->addCoordinateSystemUnique(pCoordinateSystem);
      }
  private:
    void serializeModelFileData(PRCbitStream&);
    std::ofstream *fout;
    std::ostream &output;
};

}

#endif // __O_PRC_FILE_H
