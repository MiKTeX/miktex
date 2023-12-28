//========================================================================
//
// OptionalContent.cc
//
// Copyright 2007 Brad Hards <bradh@kde.org>
// Copyright 2008 Pino Toscano <pino@kde.org>
// Copyright 2008, 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2008, 2010, 2011, 2017-2019 Albert Astals Cid <aacid@kde.org>
// Copyright 2008 Mark Kaplan <mkaplan@finjan.com>
// Copyright 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
// Released under the GPL (version 2, or later, at your option)
//
//========================================================================

#include <config.h>

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "Error.h"
#include "OptionalContent.h"

// Max depth of nested visibility expressions.  This is used to catch
// infinite loops in the visibility expression object structure.
#define visibilityExprRecursionLimit 50

// Max depth of nested display nodes.  This is used to catch infinite
// loops in the "Order" object structure.
#define displayNodeRecursionLimit 50

//------------------------------------------------------------------------

OCGs::OCGs(Object *ocgObject, XRef *xref) : m_xref(xref)
{
    // we need to parse the dictionary here, and build optionalContentGroups
    ok = true;

    Object ocgList = ocgObject->dictLookup("OCGs");
    if (!ocgList.isArray()) {
        error(errSyntaxError, -1, "Expected the optional content group list, but wasn't able to find it, or it isn't an Array");
        ok = false;
        return;
    }

    // we now enumerate over the ocgList, and build up the optionalContentGroups list.
    for (int i = 0; i < ocgList.arrayGetLength(); ++i) {
        Object ocgDict = ocgList.arrayGet(i);
        if (!ocgDict.isDict()) {
            break;
        }
        auto thisOptionalContentGroup = std::make_unique<OptionalContentGroup>(ocgDict.getDict());
        const Object &ocgRef = ocgList.arrayGetNF(i);
        if (!ocgRef.isRef()) {
            break;
        }
        thisOptionalContentGroup->setRef(ocgRef.getRef());
        // the default is ON - we change state later, depending on BaseState, ON and OFF
        thisOptionalContentGroup->setState(OptionalContentGroup::On);
        optionalContentGroups.emplace(ocgRef.getRef(), std::move(thisOptionalContentGroup));
    }

    Object defaultOcgConfig = ocgObject->dictLookup("D");
    if (!defaultOcgConfig.isDict()) {
        error(errSyntaxError, -1, "Expected the default config, but wasn't able to find it, or it isn't a Dictionary");
        ok = false;
        return;
    }

    Object baseState = defaultOcgConfig.dictLookup("BaseState");
    if (baseState.isName("OFF")) {
        for (auto &group : optionalContentGroups) {
            group.second->setState(OptionalContentGroup::Off);
        }
    }

    Object on = defaultOcgConfig.dictLookup("ON");
    if (on.isArray()) {
        // ON is an optional element
        for (int i = 0; i < on.arrayGetLength(); ++i) {
            const Object &reference = on.arrayGetNF(i);
            if (!reference.isRef()) {
                // there can be null entries
                break;
            }
            OptionalContentGroup *group = findOcgByRef(reference.getRef());
            if (!group) {
                error(errSyntaxWarning, -1, "Couldn't find group for reference");
                break;
            }
            group->setState(OptionalContentGroup::On);
        }
    }

    Object off = defaultOcgConfig.dictLookup("OFF");
    if (off.isArray()) {
        // OFF is an optional element
        for (int i = 0; i < off.arrayGetLength(); ++i) {
            const Object &reference = off.arrayGetNF(i);
            if (!reference.isRef()) {
                // there can be null entries
                break;
            }
            OptionalContentGroup *group = findOcgByRef(reference.getRef());
            if (!group) {
                error(errSyntaxWarning, -1, "Couldn't find group for reference to set OFF");
                break;
            }
            group->setState(OptionalContentGroup::Off);
        }
    }

    order = defaultOcgConfig.dictLookup("Order");
    rbgroups = defaultOcgConfig.dictLookup("RBGroups");
}

bool OCGs::hasOCGs() const
{
    return !(optionalContentGroups.empty());
}

OptionalContentGroup *OCGs::findOcgByRef(const Ref ref)
{
    const auto ocg = optionalContentGroups.find(ref);
    return ocg != optionalContentGroups.end() ? ocg->second.get() : nullptr;
}

bool OCGs::optContentIsVisible(const Object *dictRef)
{
    Dict *dict;
    bool result = true;

    if (dictRef->isNull()) {
        return result;
    }

    if (dictRef->isRef()) {
        OptionalContentGroup *oc = findOcgByRef(dictRef->getRef());
        if (oc) {
            return oc->getState() == OptionalContentGroup::On;
        }
    }

    Object dictObj = dictRef->fetch(m_xref);
    if (!dictObj.isDict()) {
        error(errSyntaxWarning, -1, "Unexpected oc reference target: {0:d}", dictObj.getType());
        return result;
    }
    dict = dictObj.getDict();
    Object dictType = dict->lookup("Type");
    if (dictType.isName("OCMD")) {
        Object ve = dict->lookup("VE");
        if (ve.isArray()) {
            result = evalOCVisibilityExpr(&ve, 0);
        } else {
            const Object &ocg = dict->lookupNF("OCGs");
            if (ocg.isArray()) {
                Object policy = dict->lookup("P");
                if (policy.isName("AllOn")) {
                    result = allOn(ocg.getArray());
                } else if (policy.isName("AllOff")) {
                    result = allOff(ocg.getArray());
                } else if (policy.isName("AnyOff")) {
                    result = anyOff(ocg.getArray());
                } else if ((!policy.isName()) || (policy.isName("AnyOn"))) {
                    // this is the default
                    result = anyOn(ocg.getArray());
                }
            } else if (ocg.isRef()) {
                OptionalContentGroup *oc = findOcgByRef(ocg.getRef());
                if (oc && oc->getState() == OptionalContentGroup::Off) {
                    result = false;
                } else {
                    result = true;
                }
            }
        }
    } else if (dictType.isName("OCG") && dictRef->isRef()) {
        OptionalContentGroup *oc = findOcgByRef(dictRef->getRef());
        if (oc && oc->getState() == OptionalContentGroup::Off) {
            result = false;
        }
    }
    return result;
}

bool OCGs::evalOCVisibilityExpr(const Object *expr, int recursion)
{
    OptionalContentGroup *ocg;
    bool ret;

    if (recursion > visibilityExprRecursionLimit) {
        error(errSyntaxError, -1, "Loop detected in optional content visibility expression");
        return true;
    }
    if (expr->isRef()) {
        if ((ocg = findOcgByRef(expr->getRef()))) {
            return ocg->getState() == OptionalContentGroup::On;
        }
    }
    Object expr2 = expr->fetch(m_xref);
    if (!expr2.isArray() || expr2.arrayGetLength() < 1) {
        error(errSyntaxError, -1, "Invalid optional content visibility expression");
        return true;
    }
    Object op = expr2.arrayGet(0);
    if (op.isName("Not")) {
        if (expr2.arrayGetLength() == 2) {
            const Object &obj = expr2.arrayGetNF(1);
            ret = !evalOCVisibilityExpr(&obj, recursion + 1);
        } else {
            error(errSyntaxError, -1, "Invalid optional content visibility expression");
            ret = true;
        }
    } else if (op.isName("And")) {
        ret = true;
        for (int i = 1; i < expr2.arrayGetLength() && ret; ++i) {
            const Object &obj = expr2.arrayGetNF(i);
            ret = evalOCVisibilityExpr(&obj, recursion + 1);
        }
    } else if (op.isName("Or")) {
        ret = false;
        for (int i = 1; i < expr2.arrayGetLength() && !ret; ++i) {
            const Object &obj = expr2.arrayGetNF(i);
            ret = evalOCVisibilityExpr(&obj, recursion + 1);
        }
    } else {
        error(errSyntaxError, -1, "Invalid optional content visibility expression");
        ret = true;
    }
    return ret;
}

bool OCGs::allOn(Array *ocgArray)
{
    for (int i = 0; i < ocgArray->getLength(); ++i) {
        const Object &ocgItem = ocgArray->getNF(i);
        if (ocgItem.isRef()) {
            OptionalContentGroup *oc = findOcgByRef(ocgItem.getRef());
            if (oc && oc->getState() == OptionalContentGroup::Off) {
                return false;
            }
        }
    }
    return true;
}

bool OCGs::allOff(Array *ocgArray)
{
    for (int i = 0; i < ocgArray->getLength(); ++i) {
        const Object &ocgItem = ocgArray->getNF(i);
        if (ocgItem.isRef()) {
            OptionalContentGroup *oc = findOcgByRef(ocgItem.getRef());
            if (oc && oc->getState() == OptionalContentGroup::On) {
                return false;
            }
        }
    }
    return true;
}

bool OCGs::anyOn(Array *ocgArray)
{
    for (int i = 0; i < ocgArray->getLength(); ++i) {
        const Object &ocgItem = ocgArray->getNF(i);
        if (ocgItem.isRef()) {
            OptionalContentGroup *oc = findOcgByRef(ocgItem.getRef());
            if (oc && oc->getState() == OptionalContentGroup::On) {
                return true;
            }
        }
    }
    return false;
}

bool OCGs::anyOff(Array *ocgArray)
{
    for (int i = 0; i < ocgArray->getLength(); ++i) {
        const Object &ocgItem = ocgArray->getNF(i);
        if (ocgItem.isRef()) {
            OptionalContentGroup *oc = findOcgByRef(ocgItem.getRef());
            if (oc && oc->getState() == OptionalContentGroup::Off) {
                return true;
            }
        }
    }
    return false;
}

//------------------------------------------------------------------------

OptionalContentGroup::OptionalContentGroup(Dict *ocgDict) : m_name(nullptr)
{
    Object ocgName = ocgDict->lookup("Name");
    if (!ocgName.isString()) {
        error(errSyntaxWarning, -1, "Expected the name of the OCG, but wasn't able to find it, or it isn't a String");
    } else {
        m_name = new GooString(ocgName.getString());
    }

    viewState = printState = ocUsageUnset;
    Object obj1 = ocgDict->lookup("Usage");
    if (obj1.isDict()) {
        Object obj2 = obj1.dictLookup("View");
        if (obj2.isDict()) {
            Object obj3 = obj2.dictLookup("ViewState");
            if (obj3.isName()) {
                if (obj3.isName("ON")) {
                    viewState = ocUsageOn;
                } else {
                    viewState = ocUsageOff;
                }
            }
        }
        obj2 = obj1.dictLookup("Print");
        if (obj2.isDict()) {
            Object obj3 = obj2.dictLookup("PrintState");
            if (obj3.isName()) {
                if (obj3.isName("ON")) {
                    printState = ocUsageOn;
                } else {
                    printState = ocUsageOff;
                }
            }
        }
    }
}

OptionalContentGroup::OptionalContentGroup(GooString *label)
{
    m_name = label;
    m_state = On;
}

const GooString *OptionalContentGroup::getName() const
{
    return m_name;
}

void OptionalContentGroup::setRef(const Ref ref)
{
    m_ref = ref;
}

Ref OptionalContentGroup::getRef() const
{
    return m_ref;
}

OptionalContentGroup::~OptionalContentGroup()
{
    delete m_name;
}
