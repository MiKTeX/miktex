//========================================================================
//
// SplashXPathScanner.cc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008, 2010, 2014, 2018, 2019, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Paweł Wiejacha <pawel.wiejacha@gmail.com>
// Copyright (C) 2013, 2014, 2021 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "goo/gmem.h"
#include "goo/GooLikely.h"
#include "SplashMath.h"
#include "SplashXPath.h"
#include "SplashBitmap.h"
#include "SplashXPathScanner.h"

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// SplashXPathScanner
//------------------------------------------------------------------------

SplashXPathScanner::SplashXPathScanner(const SplashXPath &xPath, bool eoA, int clipYMin, int clipYMax)
{
    const SplashXPathSeg *seg;
    SplashCoord xMinFP, yMinFP, xMaxFP, yMaxFP;
    int i;

    eo = eoA;
    partialClip = false;

    // compute the bbox
    xMin = yMin = 1;
    xMax = yMax = 0;
    if (xPath.length > 0) {
        seg = &xPath.segs[0];
        if (unlikely(std::isnan(seg->x0) || std::isnan(seg->x1) || std::isnan(seg->y0) || std::isnan(seg->y1))) {
            return;
        }
        if (seg->x0 <= seg->x1) {
            xMinFP = seg->x0;
            xMaxFP = seg->x1;
        } else {
            xMinFP = seg->x1;
            xMaxFP = seg->x0;
        }
        if (seg->flags & splashXPathFlip) {
            yMinFP = seg->y1;
            yMaxFP = seg->y0;
        } else {
            yMinFP = seg->y0;
            yMaxFP = seg->y1;
        }
        for (i = 1; i < xPath.length; ++i) {
            seg = &xPath.segs[i];
            if (unlikely(std::isnan(seg->x0) || std::isnan(seg->x1) || std::isnan(seg->y0) || std::isnan(seg->y1))) {
                return;
            }
            if (seg->x0 < xMinFP) {
                xMinFP = seg->x0;
            } else if (seg->x0 > xMaxFP) {
                xMaxFP = seg->x0;
            }
            if (seg->x1 < xMinFP) {
                xMinFP = seg->x1;
            } else if (seg->x1 > xMaxFP) {
                xMaxFP = seg->x1;
            }
            if (seg->flags & splashXPathFlip) {
                if (seg->y0 > yMaxFP) {
                    yMaxFP = seg->y0;
                }
            } else {
                if (seg->y1 > yMaxFP) {
                    yMaxFP = seg->y1;
                }
            }
        }
        xMin = splashFloor(xMinFP);
        xMax = splashFloor(xMaxFP);
        yMin = splashFloor(yMinFP);
        yMax = splashFloor(yMaxFP);
        if (clipYMin > yMin) {
            yMin = clipYMin;
            partialClip = true;
        }
        if (clipYMax < yMax) {
            yMax = clipYMax;
            partialClip = true;
        }
    }

    computeIntersections(xPath);
}

SplashXPathScanner::~SplashXPathScanner() { }

void SplashXPathScanner::getBBoxAA(int *xMinA, int *yMinA, int *xMaxA, int *yMaxA) const
{
    *xMinA = xMin / splashAASize;
    *yMinA = yMin / splashAASize;
    *xMaxA = xMax / splashAASize;
    *yMaxA = yMax / splashAASize;
}

void SplashXPathScanner::getSpanBounds(int y, int *spanXMin, int *spanXMax) const
{
    if (y < yMin || y > yMax) {
        *spanXMin = xMax + 1;
        *spanXMax = xMax;
        return;
    }
    const auto &line = allIntersections[y - yMin];
    if (!line.empty()) {
        *spanXMin = line[0].x0;
        int xx = line[0].x1;
        for (const SplashIntersect &intersect : line) {
            if (intersect.x1 > xx) {
                xx = intersect.x1;
            }
        }
        *spanXMax = xx;
    } else {
        *spanXMin = xMax + 1;
        *spanXMax = xMax;
    }
}

bool SplashXPathScanner::test(int x, int y) const
{
    if (y < yMin || y > yMax) {
        return false;
    }
    const auto &line = allIntersections[y - yMin];
    int count = 0;
    for (unsigned int i = 0; i < line.size() && line[i].x0 <= x; ++i) {
        if (x <= line[i].x1) {
            return true;
        }
        count += line[i].count;
    }
    return eo ? (count & 1) : (count != 0);
}

bool SplashXPathScanner::testSpan(int x0, int x1, int y) const
{
    unsigned int i;

    if (y < yMin || y > yMax) {
        return false;
    }
    const auto &line = allIntersections[y - yMin];
    int count = 0;
    for (i = 0; i < line.size() && line[i].x1 < x0; ++i) {
        count += line[i].count;
    }

    // invariant: the subspan [x0,xx1] is inside the path
    int xx1 = x0 - 1;
    while (xx1 < x1) {
        if (i >= line.size()) {
            return false;
        }
        if (line[i].x0 > xx1 + 1 && !(eo ? (count & 1) : (count != 0))) {
            return false;
        }
        if (line[i].x1 > xx1) {
            xx1 = line[i].x1;
        }
        count += line[i].count;
        ++i;
    }

    return true;
}

bool SplashXPathScanIterator::getNextSpan(int *x0, int *x1)
{
    int xx0, xx1;

    if (interIdx >= line.size()) {
        return false;
    }
    xx0 = line[interIdx].x0;
    xx1 = line[interIdx].x1;
    interCount += line[interIdx].count;
    ++interIdx;
    while (interIdx < line.size() && (line[interIdx].x0 <= xx1 || (eo ? (interCount & 1) : (interCount != 0)))) {
        if (line[interIdx].x1 > xx1) {
            xx1 = line[interIdx].x1;
        }
        interCount += line[interIdx].count;
        ++interIdx;
    }
    *x0 = xx0;
    *x1 = xx1;
    return true;
}

SplashXPathScanIterator::SplashXPathScanIterator(const SplashXPathScanner &scanner, int y)
    : line((y < scanner.yMin || y > scanner.yMax) ? scanner.allIntersections[0] : scanner.allIntersections[y - scanner.yMin]), interIdx(0), interCount(0), eo(scanner.eo)
{
    if (y < scanner.yMin || y > scanner.yMax) {
        // set index to line end
        interIdx = line.size();
    }
}

void SplashXPathScanner::computeIntersections(const SplashXPath &xPath)
{
    const SplashXPathSeg *seg;
    SplashCoord segXMin, segXMax, segYMin, segYMax, xx0, xx1;
    int x, y, y0, y1, i;

    if (yMin > yMax) {
        return;
    }

    // build the list of all intersections
    allIntersections.resize(yMax - yMin + 1);

    for (i = 0; i < xPath.length; ++i) {
        seg = &xPath.segs[i];
        if (seg->flags & splashXPathFlip) {
            segYMin = seg->y1;
            segYMax = seg->y0;
        } else {
            segYMin = seg->y0;
            segYMax = seg->y1;
        }
        if (seg->flags & splashXPathHoriz) {
            y = splashFloor(seg->y0);
            if (y >= yMin && y <= yMax) {
                if (!addIntersection(segYMin, segYMax, y, splashFloor(seg->x0), splashFloor(seg->x1), 0)) {
                    break;
                }
            }
        } else if (seg->flags & splashXPathVert) {
            y0 = splashFloor(segYMin);
            if (y0 < yMin) {
                y0 = yMin;
            }
            y1 = splashFloor(segYMax);
            if (y1 > yMax) {
                y1 = yMax;
            }
            x = splashFloor(seg->x0);
            int count = eo || (seg->flags & splashXPathFlip) ? 1 : -1;
            for (y = y0; y <= y1; ++y) {
                if (!addIntersection(segYMin, segYMax, y, x, x, count)) {
                    break;
                }
            }
        } else {
            if (seg->x0 < seg->x1) {
                segXMin = seg->x0;
                segXMax = seg->x1;
            } else {
                segXMin = seg->x1;
                segXMax = seg->x0;
            }
            y0 = splashFloor(segYMin);
            if (y0 < yMin) {
                y0 = yMin;
            }
            y1 = splashFloor(segYMax);
            if (y1 > yMax) {
                y1 = yMax;
            }
            int count = eo || (seg->flags & splashXPathFlip) ? 1 : -1;
            // Calculate the projected intersection of the segment with the
            // X-Axis.
            SplashCoord xbase = seg->x0 - (seg->y0 * seg->dxdy);
            xx0 = xbase + ((SplashCoord)y0) * seg->dxdy;
            // the segment may not actually extend to the top and/or bottom edges
            if (xx0 < segXMin) {
                xx0 = segXMin;
            } else if (xx0 > segXMax) {
                xx0 = segXMax;
            }
            int x0 = splashFloor(xx0);

            for (y = y0; y <= y1; ++y) {
                xx1 = xbase + ((SplashCoord)(y + 1) * seg->dxdy);

                if (xx1 < segXMin) {
                    xx1 = segXMin;
                } else if (xx1 > segXMax) {
                    xx1 = segXMax;
                }
                int x1 = splashFloor(xx1);
                if (!addIntersection(segYMin, segYMax, y, x0, x1, count)) {
                    break;
                }

                xx0 = xx1;
                x0 = x1;
            }
        }
    }
    for (auto &line : allIntersections) {
        std::sort(line.begin(), line.end(), [](const SplashIntersect i0, const SplashIntersect i1) { return i0.x0 < i1.x0; });
    }
}

inline bool SplashXPathScanner::addIntersection(double segYMin, double segYMax, int y, int x0, int x1, int count)
{
    SplashIntersect intersect;
    intersect.y = y;
    if (x0 < x1) {
        intersect.x0 = x0;
        intersect.x1 = x1;
    } else {
        intersect.x0 = x1;
        intersect.x1 = x0;
    }
    if (segYMin <= y && (SplashCoord)y < segYMax) {
        intersect.count = count;
    } else {
        intersect.count = 0;
    }

    auto &line = allIntersections[y - yMin];
#ifndef USE_BOOST_HEADERS
    if (line.empty()) {
        line.reserve(4);
    }
#endif
    line.push_back(intersect);

    return true;
}

void SplashXPathScanner::renderAALine(SplashBitmap *aaBuf, int *x0, int *x1, int y, bool adjustVertLine) const
{
    int xx0, xx1, xx, xxMin, xxMax, yy, yyMax, interCount;
    size_t interIdx;
    unsigned char mask;
    SplashColorPtr p;

    memset(aaBuf->getDataPtr(), 0, aaBuf->getRowSize() * aaBuf->getHeight());
    xxMin = aaBuf->getWidth();
    xxMax = -1;
    if (yMin <= yMax) {
        yy = 0;
        yyMax = splashAASize - 1;
        // clamp start and end position
        if (yMin > splashAASize * y) {
            yy = yMin - splashAASize * y;
        }
        if (yyMax + splashAASize * y > yMax) {
            yyMax = yMax - splashAASize * y;
        }

        for (; yy <= yyMax; ++yy) {
            const auto &line = allIntersections[splashAASize * y + yy - yMin];
            interIdx = 0;
            interCount = 0;
            while (interIdx < line.size()) {
                xx0 = line[interIdx].x0;
                xx1 = line[interIdx].x1;
                interCount += line[interIdx].count;
                ++interIdx;
                while (interIdx < line.size() && (line[interIdx].x0 <= xx1 || (eo ? (interCount & 1) : (interCount != 0)))) {
                    if (line[interIdx].x1 > xx1) {
                        xx1 = line[interIdx].x1;
                    }
                    interCount += line[interIdx].count;
                    ++interIdx;
                }
                if (xx0 < 0) {
                    xx0 = 0;
                }
                ++xx1;
                if (xx1 > aaBuf->getWidth()) {
                    xx1 = aaBuf->getWidth();
                }
                // set [xx0, xx1) to 1
                if (xx0 < xx1) {
                    xx = xx0;
                    p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + (xx >> 3);
                    if (xx & 7) {
                        mask = adjustVertLine ? 0xff : 0xff >> (xx & 7);
                        if (!adjustVertLine && (xx & ~7) == (xx1 & ~7)) {
                            mask &= (unsigned char)(0xff00 >> (xx1 & 7));
                        }
                        *p++ |= mask;
                        xx = (xx & ~7) + 8;
                    }
                    for (; xx + 7 < xx1; xx += 8) {
                        *p++ |= 0xff;
                    }
                    if (xx < xx1) {
                        *p |= adjustVertLine ? 0xff : (unsigned char)(0xff00 >> (xx1 & 7));
                    }
                }
                if (xx0 < xxMin) {
                    xxMin = xx0;
                }
                if (xx1 > xxMax) {
                    xxMax = xx1;
                }
            }
        }
    }
    if (xxMin > xxMax) {
        xxMin = xxMax;
    }
    *x0 = xxMin / splashAASize;
    *x1 = (xxMax - 1) / splashAASize;
}

void SplashXPathScanner::clipAALine(SplashBitmap *aaBuf, int *x0, int *x1, int y) const
{
    int xx0, xx1, xx, yy, yyMin, yyMax, interCount;
    size_t interIdx;
    unsigned char mask;
    SplashColorPtr p;

    yyMin = 0;
    yyMax = splashAASize - 1;
    // clamp start and end position
    if (yMin > splashAASize * y) {
        yyMin = yMin - splashAASize * y;
    }
    if (yyMax + splashAASize * y > yMax) {
        yyMax = yMax - splashAASize * y;
    }
    for (yy = 0; yy < splashAASize; ++yy) {
        xx = *x0 * splashAASize;
        if (yy >= yyMin && yy <= yyMax) {
            const int intersectionIndex = splashAASize * y + yy - yMin;
            if (unlikely(intersectionIndex < 0 || (unsigned)intersectionIndex >= allIntersections.size())) {
                break;
            }
            const auto &line = allIntersections[intersectionIndex];
            interIdx = 0;
            interCount = 0;
            while (interIdx < line.size() && xx < (*x1 + 1) * splashAASize) {
                xx0 = line[interIdx].x0;
                xx1 = line[interIdx].x1;
                interCount += line[interIdx].count;
                ++interIdx;
                while (interIdx < line.size() && (line[interIdx].x0 <= xx1 || (eo ? (interCount & 1) : (interCount != 0)))) {
                    if (line[interIdx].x1 > xx1) {
                        xx1 = line[interIdx].x1;
                    }
                    interCount += line[interIdx].count;
                    ++interIdx;
                }
                if (xx0 > aaBuf->getWidth()) {
                    xx0 = aaBuf->getWidth();
                }
                // set [xx, xx0) to 0
                if (xx < xx0) {
                    p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + (xx >> 3);
                    if (xx & 7) {
                        mask = (unsigned char)(0xff00 >> (xx & 7));
                        if ((xx & ~7) == (xx0 & ~7)) {
                            mask |= 0xff >> (xx0 & 7);
                        }
                        *p++ &= mask;
                        xx = (xx & ~7) + 8;
                    }
                    for (; xx + 7 < xx0; xx += 8) {
                        *p++ = 0x00;
                    }
                    if (xx < xx0) {
                        *p &= 0xff >> (xx0 & 7);
                    }
                }
                if (xx1 >= xx) {
                    xx = xx1 + 1;
                }
            }
        }
        xx0 = (*x1 + 1) * splashAASize;
        if (xx0 > aaBuf->getWidth()) {
            xx0 = aaBuf->getWidth();
        }
        // set [xx, xx0) to 0
        if (xx < xx0 && xx >= 0) {
            p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() + (xx >> 3);
            if (xx & 7) {
                mask = (unsigned char)(0xff00 >> (xx & 7));
                if ((xx & ~7) == (xx0 & ~7)) {
                    mask &= 0xff >> (xx0 & 7);
                }
                *p++ &= mask;
                xx = (xx & ~7) + 8;
            }
            for (; xx + 7 < xx0; xx += 8) {
                *p++ = 0x00;
            }
            if (xx < xx0) {
                *p &= 0xff >> (xx0 & 7);
            }
        }
    }
}
