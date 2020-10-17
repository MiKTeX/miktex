/* poppler-qiodeviceinstream-private.h: Qt5 interface to poppler
 * Copyright (C) 2019 Alexander Volkov <a.volkov@rusbitech.ru>
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

#ifndef POPPLER_QIODEVICEINSTREAM_PRIVATE_H
#define POPPLER_QIODEVICEINSTREAM_PRIVATE_H

#include "Object.h"
#include "Stream.h"

class QIODevice;

namespace Poppler {

class QIODeviceInStream : public BaseSeekInputStream
{
public:
    QIODeviceInStream(QIODevice *device, Goffset startA, bool limitedA, Goffset lengthA, Object &&dictA);
    ~QIODeviceInStream() override;

    BaseStream *copy() override;
    Stream *makeSubStream(Goffset startA, bool limitedA, Goffset lengthA, Object &&dictA) override;

private:
    Goffset currentPos() const override;
    void setCurrentPos(Goffset offset) override;
    Goffset read(char *buffer, Goffset count) override;

    QIODevice *m_device;
};

}

#endif
