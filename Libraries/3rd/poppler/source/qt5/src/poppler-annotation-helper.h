/* poppler-annotation-helper.h: qt interface to poppler
 * Copyright (C) 2006, 2008, 2017-2019, 2021, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2012, Fabio D'Urso <fabiodurso@hotmail.it>
 * Copyright (C) 2018, Dileep Sankhla <sankhla.dileep96@gmail.com>
 * Copyright (C) 2018, Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2018, 2019, Oliver Sander <oliver.sander@tu-dresden.de>
 * Adapting code from
 *   Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>
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

#ifndef _POPPLER_ANNOTATION_HELPER_H_
#define _POPPLER_ANNOTATION_HELPER_H_

#include <memory>

#include <QtCore/QDebug>

#include <Object.h>

class QColor;

class AnnotColor;

namespace Poppler {

class XPDFReader
{
public:
    // transform from user coords to normalized ones using the matrix M
    static inline void transform(double *M, double x, double y, QPointF &res);
    static inline void invTransform(const double *M, const QPointF p, double &x, double &y);
};

void XPDFReader::transform(double *M, double x, double y, QPointF &res)
{
    res.setX(M[0] * x + M[2] * y + M[4]);
    res.setY(M[1] * x + M[3] * y + M[5]);
}

void XPDFReader::invTransform(const double *M, const QPointF p, double &x, double &y)
{
    const double det = M[0] * M[3] - M[1] * M[2];
    if (det == 0) {
        qWarning("Tried to invert singular matrix, something won't work");
        x = 0;
        y = 0;
        return;
    }

    const double invM[4] = { M[3] / det, -M[1] / det, -M[2] / det, M[0] / det };
    const double xt = p.x() - M[4];
    const double yt = p.y() - M[5];

    x = invM[0] * xt + invM[2] * yt;
    y = invM[1] * xt + invM[3] * yt;
}

QColor convertAnnotColor(const AnnotColor *color);
std::unique_ptr<AnnotColor> convertQColor(const QColor &color);

}

#endif
