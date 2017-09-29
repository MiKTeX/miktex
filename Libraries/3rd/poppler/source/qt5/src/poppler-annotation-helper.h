/* poppler-annotation-helper.h: qt interface to poppler
 * Copyright (C) 2006, 2008, 2017, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2012, Fabio D'Urso <fabiodurso@hotmail.it>
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

#include <QtCore/QDebug>

#include <Object.h>

class QColor;

class AnnotColor;

namespace Poppler {

class XPDFReader
{
    public:
        // find named symbol and parse it
        static inline void lookupName( Dict *, char *, QString & dest );
        static inline void lookupString( Dict *, char *, QString & dest );
        static inline void lookupBool( Dict *, char *, bool & dest );
        static inline void lookupInt( Dict *, char *, int & dest );
        static inline void lookupNum( Dict *, char *, double & dest );
        static inline int lookupNumArray( Dict *, char *, double * dest, int len );
        static inline void lookupColor( Dict *, char *, QColor & color );
        static inline void lookupIntRef( Dict *, char *, int & dest );
        static inline void lookupDate( Dict *, char *, QDateTime & dest );
        // transform from user coords to normalized ones using the matrix M
        static inline void transform( double * M, double x, double y, QPointF &res );
        static inline void invTransform( double * M, const QPointF &p, double &x, double &y );
};

void XPDFReader::lookupName( Dict * dict, char * type, QString & dest )
{
    Object nameObj = dict->lookup( type );
    if ( nameObj.isNull() )
        return;
    if ( nameObj.isName() )
        dest = nameObj.getName();
    else
        qDebug() << type << " is not Name." << endl;
}

void XPDFReader::lookupString( Dict * dict, char * type, QString & dest )
{
    Object stringObj = dict->lookup( type );
    if ( stringObj.isNull() )
        return;
    if ( stringObj.isString() )
        dest = stringObj.getString()->getCString();
    else
        qDebug() << type << " is not String." << endl;
}

void XPDFReader::lookupBool( Dict * dict, char * type, bool & dest )
{
    Object boolObj = dict->lookup( type );
    if ( boolObj.isNull() )
        return;
    if ( boolObj.isBool() )
        dest = boolObj.getBool() == gTrue;
    else
        qDebug() << type << " is not Bool." << endl;
}

void XPDFReader::lookupInt( Dict * dict, char * type, int & dest )
{
    Object intObj = dict->lookup( type );
    if ( intObj.isNull() )
        return;
    if ( intObj.isInt() )
        dest = intObj.getInt();
    else
        qDebug() << type << " is not Int." << endl;
}

void XPDFReader::lookupNum( Dict * dict, char * type, double & dest )
{
    Object numObj = dict->lookup( type );
    if ( numObj.isNull() )
        return;
    if ( numObj.isNum() )
        dest = numObj.getNum();
    else
        qDebug() << type << " is not Num." << endl;
}

int XPDFReader::lookupNumArray( Dict * dict, char * type, double * dest, int len )
{
    Object arrObj = dict->lookup( type );
    if ( arrObj.isNull() )
        return 0;
    if ( arrObj.isArray() )
    {
        len = qMin( len, arrObj.arrayGetLength() );
        for ( int i = 0; i < len; i++ )
        {
            Object numObj = arrObj.arrayGet( i );
            dest[i] = numObj.getNum();
        }
    }
    else
    {
        len = 0;
        qDebug() << type << "is not Array." << endl;
    }
    return len;
}

void XPDFReader::lookupColor( Dict * dict, char * type, QColor & dest )
{
    double c[3];
    if ( XPDFReader::lookupNumArray( dict, type, c, 3 ) == 3 )
        dest = QColor( (int)(c[0]*255.0), (int)(c[1]*255.0), (int)(c[2]*255.0));
}

void XPDFReader::lookupIntRef( Dict * dict, char * type, int & dest )
{
    Object refObj = dict->lookupNF( type );
    if ( refObj.isNull() )
        return;
    if ( refObj.isRef() )
        dest = refObj.getRefNum();
    else
        qDebug() << type << " is not Ref." << endl;
}

void XPDFReader::lookupDate( Dict * dict, char * type, QDateTime & dest )
{
    Object dateObj = dict->lookup( type );
    if ( dateObj.isNull() )
        return;
    if ( dateObj.isString() )
    {
        dest = convertDate( dateObj.getString()->getCString() );
    }
    else
        qDebug() << type << " is not Date" << endl;
}

void XPDFReader::transform( double * M, double x, double y, QPointF &res )
{
    res.setX( M[0] * x + M[2] * y + M[4] );
    res.setY( M[1] * x + M[3] * y + M[5] );
}

void XPDFReader::invTransform( double * M, const QPointF &p, double &x, double &y )
{
    const double det = M[0]*M[3] - M[1]*M[2];
    Q_ASSERT(det != 0);

    const double invM[4] = { M[3]/det, -M[1]/det, -M[2]/det, M[0]/det };
    const double xt = p.x() - M[4];
    const double yt = p.y() - M[5];

    x = invM[0] * xt + invM[2] * yt;
    y = invM[1] * xt + invM[3] * yt;
}

QColor convertAnnotColor( AnnotColor *color );
AnnotColor* convertQColor( const QColor &color );

}
