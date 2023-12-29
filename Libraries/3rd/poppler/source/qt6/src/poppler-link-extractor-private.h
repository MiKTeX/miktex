/* poppler-link-extractor_p.h: qt interface to poppler
 * Copyright (C) 2007, 2008, 2011, Pino Toscano <pino@kde.org>
 * Copyright (C) 2021, Oliver Sander <oliver.sander@tu-dresden.de>
 * Copyright (C) 2021, Albert Astals Cid <aacid@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _POPPLER_LINK_EXTRACTOR_H_
#define _POPPLER_LINK_EXTRACTOR_H_

#include <memory>
#include <vector>

#include <Object.h>
#include <OutputDev.h>

#include <QtCore/QList>

namespace Poppler {

class Link;
class PageData;

class LinkExtractorOutputDev : public OutputDev
{
public:
    explicit LinkExtractorOutputDev(PageData *data);
    ~LinkExtractorOutputDev() override;

    // inherited from OutputDev
    bool upsideDown() override { return false; }
    bool useDrawChar() override { return false; }
    bool interpretType3Chars() override { return false; }
    void processLink(::AnnotLink *link) override;

    // our stuff
    std::vector<std::unique_ptr<Link>> links();

private:
    PageData *m_data;
    double m_pageCropWidth;
    double m_pageCropHeight;
    std::vector<std::unique_ptr<Link>> m_links;
};

}

#endif
