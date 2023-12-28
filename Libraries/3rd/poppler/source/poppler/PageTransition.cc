/* PageTransition.cc
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2010, 2017, 2020, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
 * Copyright (C) 2015, Arseniy Lartsev <arseniy@alumni.chalmers.se>
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

#include "PageTransition.h"

//------------------------------------------------------------------------
// PageTransition
//------------------------------------------------------------------------

PageTransition::PageTransition(Object *trans)
{
    Object obj;
    Dict *dict;

    type = transitionReplace;
    duration = 1;
    alignment = transitionHorizontal;
    direction = transitionInward;
    angle = 0;
    scale = 1.0;
    ok = true;

    if (!trans || !trans->isDict()) {
        ok = false;
        return;
    }

    dict = trans->getDict();

    // get type
    obj = dict->lookup("S");
    if (obj.isName()) {
        const char *s = obj.getName();

        if (strcmp("R", s) == 0) {
            type = transitionReplace;
        } else if (strcmp("Split", s) == 0) {
            type = transitionSplit;
        } else if (strcmp("Blinds", s) == 0) {
            type = transitionBlinds;
        } else if (strcmp("Box", s) == 0) {
            type = transitionBox;
        } else if (strcmp("Wipe", s) == 0) {
            type = transitionWipe;
        } else if (strcmp("Dissolve", s) == 0) {
            type = transitionDissolve;
        } else if (strcmp("Glitter", s) == 0) {
            type = transitionGlitter;
        } else if (strcmp("Fly", s) == 0) {
            type = transitionFly;
        } else if (strcmp("Push", s) == 0) {
            type = transitionPush;
        } else if (strcmp("Cover", s) == 0) {
            type = transitionCover;
        } else if (strcmp("Uncover", s) == 0) {
            type = transitionUncover;
        } else if (strcmp("Fade", s) == 0) {
            type = transitionFade;
        }
    }

    // get duration
    obj = dict->lookup("D");
    if (obj.isNum()) {
        duration = obj.getNum();
    }

    // get alignment
    obj = dict->lookup("Dm");
    if (obj.isName()) {
        const char *dm = obj.getName();

        if (strcmp("H", dm) == 0) {
            alignment = transitionHorizontal;
        } else if (strcmp("V", dm) == 0) {
            alignment = transitionVertical;
        }
    }

    // get direction
    obj = dict->lookup("M");
    if (obj.isName()) {
        const char *m = obj.getName();

        if (strcmp("I", m) == 0) {
            direction = transitionInward;
        } else if (strcmp("O", m) == 0) {
            direction = transitionOutward;
        }
    }

    // get angle
    obj = dict->lookup("Di");
    if (obj.isInt()) {
        angle = obj.getInt();
    }

    obj = dict->lookup("Di");
    if (obj.isName()) {
        if (strcmp("None", obj.getName()) == 0) {
            angle = 0;
        }
    }

    // get scale
    obj = dict->lookup("SS");
    if (obj.isNum()) {
        scale = obj.getNum();
    }

    // get rectangular
    rectangular = dict->lookup("B").getBoolWithDefaultValue(false);
}

PageTransition::~PageTransition() { }
