//========================================================================
//
// This file comes from pdftohtml project
// http://pdftohtml.sourceforge.net
//
// Copyright from:
// Gueorgui Ovtcharov
// Rainer Dorsch <http://www.ra.informatik.uni-stuttgart.de/~rainer/>
// Mikhail Kruk <meshko@cs.brandeis.edu>
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008 Boris Toloknov <tlknv@yandex.ru>
// Copyright (C) 2010, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Julien Nabet <serval2412@yahoo.fr>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "HtmlLinks.h"

extern bool xml;

HtmlLink::HtmlLink(const HtmlLink &x)
{
    Xmin = x.Xmin;
    Ymin = x.Ymin;
    Xmax = x.Xmax;
    Ymax = x.Ymax;
    dest = new GooString(x.dest);
}

HtmlLink::HtmlLink(double xmin, double ymin, double xmax, double ymax, GooString *_dest)
{
    if (xmin < xmax) {
        Xmin = xmin;
        Xmax = xmax;
    } else {
        Xmin = xmax;
        Xmax = xmin;
    }
    if (ymin < ymax) {
        Ymin = ymin;
        Ymax = ymax;
    } else {
        Ymin = ymax;
        Ymax = ymin;
    }
    dest = new GooString(_dest);
}

HtmlLink::~HtmlLink()
{
    delete dest;
}

bool HtmlLink::isEqualDest(const HtmlLink &x) const
{
    return (!strcmp(dest->c_str(), x.dest->c_str()));
}

bool HtmlLink::inLink(double xmin, double ymin, double xmax, double ymax) const
{
    double y = (ymin + ymax) / 2;
    if (y > Ymax) {
        return false;
    }
    return (y > Ymin) && (xmin < Xmax) && (xmax > Xmin);
}

static GooString *EscapeSpecialChars(GooString *s)
{
    GooString *tmp = nullptr;
    for (int i = 0, j = 0; i < s->getLength(); i++, j++) {
        const char *replace = nullptr;
        switch (s->getChar(i)) {
        case '"':
            replace = "&quot;";
            break;
        case '&':
            replace = "&amp;";
            break;
        case '<':
            replace = "&lt;";
            break;
        case '>':
            replace = "&gt;";
            break;
        default:
            continue;
        }
        if (replace) {
            if (!tmp) {
                tmp = new GooString(s);
            }
            if (tmp) {
                tmp->del(j, 1);
                int l = strlen(replace);
                tmp->insert(j, replace, l);
                j += l - 1;
            }
        }
    }
    return tmp ? tmp : s;
}

GooString *HtmlLink::getLinkStart() const
{
    GooString *res = new GooString("<a href=\"");
    GooString *d = xml ? EscapeSpecialChars(dest) : dest;
    res->append(d);
    if (d != dest) {
        delete d;
    }
    res->append("\">");
    return res;
}

/*GooString* HtmlLink::Link(GooString* content){
  //GooString* _dest=new GooString(dest);
  GooString *tmp=new GooString("<a href=\"");
  tmp->append(dest);
  tmp->append("\">");
  tmp->append(content);
  tmp->append("</a>");
  //delete _dest;
  return tmp;
  }*/

HtmlLinks::HtmlLinks() { }

HtmlLinks::~HtmlLinks() { }

bool HtmlLinks::inLink(double xmin, double ymin, double xmax, double ymax, size_t &p) const
{

    for (std::vector<HtmlLink>::const_iterator i = accu.begin(); i != accu.end(); ++i) {
        if (i->inLink(xmin, ymin, xmax, ymax)) {
            p = (i - accu.begin());
            return true;
        }
    }
    return false;
}

const HtmlLink *HtmlLinks::getLink(size_t i) const
{
    return &accu[i];
}
