/* Sound.cc - an object that holds the sound structure
 * Copyright (C) 2006-2007, Pino Toscano <pino@kde.org>
 * Copyright (C) 2009, 2017-2020, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2020, Oliver Sander <oliver.sander@tu-dresden.de>
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

#include "Object.h"
#include "Sound.h"
#include "Stream.h"
#include "FileSpec.h"

std::unique_ptr<Sound> Sound::parseSound(Object *obj)
{
    // let's try to see if this Object is a Sound, according to the PDF specs
    // (section 9.2)
    Stream *str = nullptr;
    // the Object must be a Stream
    if (obj->isStream()) {
        str = obj->getStream();
    } else {
        return nullptr;
    }
    // the Stream must have a Dict
    Dict *dict = str->getDict();
    if (dict == nullptr) {
        return nullptr;
    }
    // the Dict must have the 'R' key of type num
    Object tmp = dict->lookup("R");
    if (tmp.isNum()) {
        return std::unique_ptr<Sound>(new Sound(obj));
    } else {
        return nullptr;
    }
}

Sound::Sound(const Object *obj, bool readAttrs)
{
    streamObj = obj->copy();

    samplingRate = 0.0;
    channels = 1;
    bitsPerSample = 8;
    encoding = soundRaw;

    if (readAttrs) {
        Dict *dict = streamObj.getStream()->getDict();
        Object tmp = dict->lookup("F");
        if (!tmp.isNull()) {
            // valid 'F' key -> external file
            kind = soundExternal;
            Object obj1 = getFileSpecNameForPlatform(&tmp);
            if (obj1.isString()) {
                fileName = obj1.getString()->toStr();
            }
        } else {
            // no file specification, then the sound data have to be
            // extracted from the stream
            kind = soundEmbedded;
        }
        // sampling rate
        samplingRate = dict->lookup("R").getNumWithDefaultValue(0);
        // sound channels
        tmp = dict->lookup("C");
        if (tmp.isInt()) {
            channels = tmp.getInt();
        }
        // bits per sample
        tmp = dict->lookup("B");
        if (tmp.isInt()) {
            bitsPerSample = tmp.getInt();
        }
        // encoding format
        tmp = dict->lookup("E");
        if (tmp.isName()) {
            const char *enc = tmp.getName();
            if (strcmp("Raw", enc) == 0) {
                encoding = soundRaw;
            } else if (strcmp("Signed", enc) == 0) {
                encoding = soundSigned;
            } else if (strcmp("muLaw", enc) == 0) {
                encoding = soundMuLaw;
            } else if (strcmp("ALaw", enc) == 0) {
                encoding = soundALaw;
            }
        }
    }
}

Sound::~Sound() { }

Stream *Sound::getStream()
{
    return streamObj.getStream();
}

Sound *Sound::copy() const
{
    Sound *newsound = new Sound(&streamObj, false);

    newsound->kind = kind;
    newsound->fileName = fileName;
    newsound->samplingRate = samplingRate;
    newsound->channels = channels;
    newsound->bitsPerSample = bitsPerSample;
    newsound->encoding = encoding;

    return newsound;
}
