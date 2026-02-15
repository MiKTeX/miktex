/*****
 * drawgroup.h
 * John Bowman
 *
 * Group elements in a picture to be deconstructed as a single object.
 *****/

#ifndef DRAWGROUP_H
#define DRAWGROUP_H

#include "drawelement.h"

namespace camp {

class drawBegin : public drawElement {
public:
  drawBegin() {}

  virtual ~drawBegin() {}

  bool begingroup() override {return true;}
};

class drawEnd : public drawElement {
public:
  drawEnd() {}

  virtual ~drawEnd() {}

  bool endgroup() override {return true;}
};

class drawBegin3 : public drawElementLC {
  string name;
  double compression;
  double granularity;
  bool closed;   // render the surface as one-sided; may yield faster rendering
  bool tessellate; // use tessellated mesh to store straight patches
  bool dobreak; // force breaking
  bool nobreak; // force grouping for transparent patches

  triple center;
  int interaction;
public:
  drawBegin3(string name, double compression, double granularity,
             bool closed, bool tessellate, bool dobreak, bool nobreak,
             triple center, int interaction) :
    name(name), compression(compression), granularity(granularity),
    closed(closed), tessellate(tessellate), dobreak(dobreak), nobreak(nobreak),
    center(center), interaction(interaction) {}

  virtual ~drawBegin3() {}

  bool begingroup() override {return true;}
  bool begingroup3() override {return true;}

  bool write(prcfile *out, unsigned int *count, double compressionlimit,
             groupsmap& groups) override {
    groupmap& group=groups.back();
    if(name.empty()) name="group";
    groupmap::const_iterator p=group.find(name);

    unsigned c=(p != group.end()) ? p->second+1 : 0;
    group[name]=c;

    ostringstream buf;
    buf << name;
    if(c > 0) buf << "-" << (c+1);

    if(interaction == BILLBOARD)
      buf << "-" << (*count)++ << "\001";

    prc::PRCoptions options(compression > 0.0 ?
                            max(compression,compressionlimit) : 0.0,
                            granularity,closed,tessellate,dobreak,nobreak);

    groups.push_back(groupmap());
    const string& s=buf.str();
    out->begingroup(s.c_str(),&options,T);
    return true;
  }

  drawBegin3(const double* t, const drawBegin3 *s) :
    drawElementLC(t, s), name(s->name), compression(s->compression),
    granularity(s->granularity), closed(s->closed), tessellate(s->tessellate),
    dobreak(s->dobreak), nobreak(s->nobreak), interaction(s->interaction)  {
    center=t*s->center;
  }

  drawElement *transformed(const double* t) override {
    return new drawBegin3(t,this);
  }
};

class drawEnd3 : public drawElement {
public:
  drawEnd3() {}

  virtual ~drawEnd3() {}

  bool endgroup() override {return true;}
  bool endgroup3() override {return true;}

  bool write(prcfile *out, unsigned int *, double, groupsmap& groups)
    override {
    groups.pop_back();
    out->endgroup();
    return true;
  }
};

class drawBeginTransform : public drawElement {
  string geometry;
  string color;
  double duration;
  bool autoplay;
public:
  drawBeginTransform(string geometry, string color,
                     double duration, bool autoplay) :
    geometry(geometry), color(color),
    duration(duration), autoplay(autoplay){}

  virtual ~drawBeginTransform() {}

  bool write(abs3Doutfile *out) override {
    out->initTransform();
    out->write("\nbeginTransform(");
    out->write(geometry.empty() ? "null" : geometry);
    out->write(",");
    out->write(color.empty() ? "null" : color);
    out->write(",");
    out->write(duration);
    out->write(",");
    out->write(autoplay ? "true" : "false");
    out->write(");\n");
    return true;
  }
};

class drawEndTransform : public drawElement {
public:
  drawEndTransform() {}

  virtual ~drawEndTransform() {}

  bool write(abs3Doutfile *out) override {
    out->write("endTransform();\n\n");
    return false;
  }
};

}

GC_DECLARE_PTRFREE(camp::drawBegin);
GC_DECLARE_PTRFREE(camp::drawEnd);

#endif
