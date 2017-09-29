//========================================================================
//
// OptionalContent.cc
//
// Copyright 2007 Brad Hards <bradh@kde.org>
// Copyright 2008 Pino Toscano <pino@kde.org>
// Copyright 2008, 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2008, 2010, 2011, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright 2008 Mark Kaplan <mkaplan@finjan.com>
//
// Released under the GPL (version 2, or later, at your option)
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "Error.h"
// #include "PDFDocEncoding.h"
#include "OptionalContent.h"

// Max depth of nested visibility expressions.  This is used to catch
// infinite loops in the visibility expression object structure.
#define visibilityExprRecursionLimit 50

// Max depth of nested display nodes.  This is used to catch infinite
// loops in the "Order" object structure.
#define displayNodeRecursionLimit 50

//------------------------------------------------------------------------

OCGs::OCGs(Object *ocgObject, XRef *xref) :
  m_xref(xref)
{
  // we need to parse the dictionary here, and build optionalContentGroups
  ok = gTrue;
  optionalContentGroups = new GooList();
  display = NULL;

  Object ocgList = ocgObject->dictLookup("OCGs");
  if (!ocgList.isArray()) {
    error(errSyntaxError, -1, "Expected the optional content group list, but wasn't able to find it, or it isn't an Array");
    ok = gFalse;
    return;
  }

  // we now enumerate over the ocgList, and build up the optionalContentGroups list.
  for(int i = 0; i < ocgList.arrayGetLength(); ++i) {
    Object ocg = ocgList.arrayGet(i);
    if (!ocg.isDict()) {
      break;
    }
    OptionalContentGroup *thisOptionalContentGroup = new OptionalContentGroup(ocg.getDict());
    ocg = ocgList.arrayGetNF(i);
    if (!ocg.isRef()) {
      delete thisOptionalContentGroup;
      break;
    }
    // TODO: we should create a lookup map from Ref to the OptionalContentGroup
    thisOptionalContentGroup->setRef( ocg.getRef() );
    // the default is ON - we change state later, depending on BaseState, ON and OFF
    thisOptionalContentGroup->setState(OptionalContentGroup::On);
    optionalContentGroups->append(thisOptionalContentGroup);
  }

  Object defaultOcgConfig = ocgObject->dictLookup("D");
  if (!defaultOcgConfig.isDict()) {
    error(errSyntaxError, -1, "Expected the default config, but wasn't able to find it, or it isn't a Dictionary");
    ok = gFalse;
    return;
  }

  Object baseState = defaultOcgConfig.dictLookup("BaseState");
  if (baseState.isName("OFF")) {
    for (int i = 0; i < optionalContentGroups->getLength(); ++i) {
      OptionalContentGroup *group;

      group = (OptionalContentGroup *)optionalContentGroups->get(i);
      group->setState(OptionalContentGroup::Off);
    }
  }

  Object on = defaultOcgConfig.dictLookup("ON");
  if (on.isArray()) {
    // ON is an optional element
    for (int i = 0; i < on.arrayGetLength(); ++i) {
      Object reference = on.arrayGetNF(i);
      if (!reference.isRef()) {
	// there can be null entries
	break;
      }
      OptionalContentGroup *group = findOcgByRef( reference.getRef() );
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
      Object reference = off.arrayGetNF(i);
      if (!reference.isRef()) {
	// there can be null entries
	break;
      }
      OptionalContentGroup *group = findOcgByRef( reference.getRef() );
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

OCGs::~OCGs()
{
  deleteGooList(optionalContentGroups, OptionalContentGroup);
  delete display;
}


bool OCGs::hasOCGs()
{
  return ( optionalContentGroups->getLength() > 0 );
}

OptionalContentGroup* OCGs::findOcgByRef( const Ref &ref)
{
  //TODO: make this more efficient
  OptionalContentGroup *ocg = NULL;
  for (int i=0; i < optionalContentGroups->getLength(); ++i) {
    ocg = (OptionalContentGroup*)optionalContentGroups->get(i);
    if ( (ocg->getRef().num == ref.num) && (ocg->getRef().gen == ref.gen) ) {
      return ocg;
    }
  }

  // not found
  return NULL;
}

OCDisplayNode *OCGs::getDisplayRoot()
{
  if (display)
    return display;

  if (order.isArray())
    display = OCDisplayNode::parse(&order, this, m_xref);

  return display;
}

bool OCGs::optContentIsVisible( Object *dictRef )
{
  Dict *dict;
  bool result = true;

  if (dictRef->isNull())
    return result;

  if (dictRef->isRef()) {
    OptionalContentGroup *oc = findOcgByRef(dictRef->getRef());
    if (oc)
      return oc->getState() == OptionalContentGroup::On;
  }

  Object dictObj = dictRef->fetch( m_xref);
  if ( ! dictObj.isDict() ) {
    error(errSyntaxWarning, -1, "Unexpected oc reference target: {0:d}", dictObj.getType() );
    return result;
  }
  dict = dictObj.getDict();
  // printf("checking if optContent is visible\n");
  Object dictType = dict->lookup("Type");
  if (dictType.isName("OCMD")) {
    Object ve = dict->lookup("VE");
    if (ve.isArray()) {
      result = evalOCVisibilityExpr(&ve, 0);
    } else {
      Object ocg = dict->lookupNF("OCGs");
      if (ocg.isArray()) {
        Object policy = dict->lookup("P");
        if (policy.isName("AllOn")) {
          result = allOn( ocg.getArray() );
        } else if (policy.isName("AllOff")) {
          result = allOff( ocg.getArray() );
        } else if (policy.isName("AnyOff")) {
          result = anyOff( ocg.getArray() );
        } else if ( (!policy.isName()) || (policy.isName("AnyOn") ) ) {
          // this is the default
          result = anyOn( ocg.getArray() );
        }
      } else if (ocg.isRef()) {
        OptionalContentGroup *oc = findOcgByRef( ocg.getRef() );
        if ( oc && oc->getState() == OptionalContentGroup::Off ) {
          result = false;
        } else {
          result = true ;
        }
      }
    }
  } else if ( dictType.isName("OCG") ) {
    OptionalContentGroup* oc = findOcgByRef( dictRef->getRef() );
    if ( oc && oc->getState() == OptionalContentGroup::Off ) {
      result=false;
    }
  }
  // printf("visibility: %s\n", result? "on" : "off");
  return result;
}

GBool OCGs::evalOCVisibilityExpr(Object *expr, int recursion) {
  OptionalContentGroup *ocg;
  GBool ret;

  if (recursion > visibilityExprRecursionLimit) {
    error(errSyntaxError, -1,
	  "Loop detected in optional content visibility expression");
    return gTrue;
  }
  if (expr->isRef()) {
    if ((ocg = findOcgByRef(expr->getRef()))) {
      return ocg->getState() == OptionalContentGroup::On;
    }
  }
  Object expr2 = expr->fetch(m_xref);
  if (!expr2.isArray() || expr2.arrayGetLength() < 1) {
    error(errSyntaxError, -1,
	  "Invalid optional content visibility expression");
    return gTrue;
  }
  Object op = expr2.arrayGet(0);
  if (op.isName("Not")) {
    if (expr2.arrayGetLength() == 2) {
      Object obj = expr2.arrayGetNF(1);
      ret = !evalOCVisibilityExpr(&obj, recursion + 1);
    } else {
      error(errSyntaxError, -1,
	    "Invalid optional content visibility expression");
      ret = gTrue;
    }
  } else if (op.isName("And")) {
    ret = gTrue;
    for (int i = 1; i < expr2.arrayGetLength() && ret; ++i) {
      Object obj = expr2.arrayGetNF(i);
      ret = evalOCVisibilityExpr(&obj, recursion + 1);
    }
  } else if (op.isName("Or")) {
    ret = gFalse;
    for (int i = 1; i < expr2.arrayGetLength() && !ret; ++i) {
      Object obj = expr2.arrayGetNF(i);
      ret = evalOCVisibilityExpr(&obj, recursion + 1);
    }
  } else {
    error(errSyntaxError, -1,
	  "Invalid optional content visibility expression");
    ret = gTrue;
  }
  return ret;
}

bool OCGs::allOn( Array *ocgArray )
{
  for (int i = 0; i < ocgArray->getLength(); ++i) {
    Object ocgItem = ocgArray->getNF(i);
    if (ocgItem.isRef()) {
      OptionalContentGroup* oc = findOcgByRef( ocgItem.getRef() );      
      if ( oc && oc->getState() == OptionalContentGroup::Off ) {
	return false;
      }
    }
  }
  return true;
}

bool OCGs::allOff( Array *ocgArray )
{
  for (int i = 0; i < ocgArray->getLength(); ++i) {
    Object ocgItem = ocgArray->getNF(i);
    if (ocgItem.isRef()) {
      OptionalContentGroup* oc = findOcgByRef( ocgItem.getRef() );      
      if ( oc && oc->getState() == OptionalContentGroup::On ) {
	return false;
      }
    }
  }
  return true;
}

bool OCGs::anyOn( Array *ocgArray )
{
  for (int i = 0; i < ocgArray->getLength(); ++i) {
    Object ocgItem = ocgArray->getNF(i);
    if (ocgItem.isRef()) {
      OptionalContentGroup* oc = findOcgByRef( ocgItem.getRef() );      
      if ( oc && oc->getState() == OptionalContentGroup::On ) {
	return true;
      }
    }
  }
  return false;
}

bool OCGs::anyOff( Array *ocgArray )
{
  for (int i = 0; i < ocgArray->getLength(); ++i) {
    Object ocgItem = ocgArray->getNF(i);
    if (ocgItem.isRef()) {
      OptionalContentGroup* oc = findOcgByRef( ocgItem.getRef() );      
      if ( oc && oc->getState() == OptionalContentGroup::Off ) {
	return true;
      }
    }
  }
  return false;
}

//------------------------------------------------------------------------

OptionalContentGroup::OptionalContentGroup(Dict *ocgDict) : m_name(NULL)
{
  Object ocgName = ocgDict->lookup("Name");
  if (!ocgName.isString()) {
    error(errSyntaxWarning, -1, "Expected the name of the OCG, but wasn't able to find it, or it isn't a String");
  } else {
    m_name = new GooString( ocgName.getString() );
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

GooString* OptionalContentGroup::getName() const
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

//------------------------------------------------------------------------

OCDisplayNode *OCDisplayNode::parse(Object *obj, OCGs *oc,
				    XRef *xref, int recursion) {
  OptionalContentGroup *ocgA;
  OCDisplayNode *node, *child;
  int i;

  if (recursion > displayNodeRecursionLimit) {
    error(errSyntaxError, -1, "Loop detected in optional content order");
    return NULL;
  }
  if (obj->isRef()) {
    if ((ocgA = oc->findOcgByRef(obj->getRef()))) {
      return new OCDisplayNode(ocgA);
    }
  }
  Object obj2 = obj->fetch(xref);
  if (!obj2.isArray()) {
    return NULL;
  }
  i = 0;
  if (obj2.arrayGetLength() >= 1) {
    Object obj3 = obj2.arrayGet(0);
    if (obj3.isString()) {
      node = new OCDisplayNode(obj3.getString());
      i = 1;
    } else {
      node = new OCDisplayNode();
    }
  } else {
    node = new OCDisplayNode();
  }
  for (; i < obj2.arrayGetLength(); ++i) {
    Object obj3 = obj2.arrayGetNF(i);
    if ((child = OCDisplayNode::parse(&obj3, oc, xref, recursion + 1))) {
      if (!child->ocg && !child->name && node->getNumChildren() > 0) {
	node->getChild(node->getNumChildren() - 1)->addChildren(child->takeChildren());
	delete child;
      } else {
	node->addChild(child);
      }
    }
  }
  return node;
}

OCDisplayNode::OCDisplayNode() {
  name = NULL;
  ocg = NULL;
  children = NULL;
}

OCDisplayNode::OCDisplayNode(GooString *nameA) {
  name = new GooString(nameA);
  ocg = NULL;
  children = NULL;
}

OCDisplayNode::OCDisplayNode(OptionalContentGroup *ocgA) {
  name = (ocgA->getName()) ? ocgA->getName()->copy() : NULL;
  ocg = ocgA;
  children = NULL;
}

void OCDisplayNode::addChild(OCDisplayNode *child) {
  if (!children) {
    children = new GooList();
  }
  children->append(child);
}

void OCDisplayNode::addChildren(GooList *childrenA) {
  if (!children) {
    children = new GooList();
  }
  children->append(childrenA);
  delete childrenA;
}

GooList *OCDisplayNode::takeChildren() {
  GooList *childrenA;

  childrenA = children;
  children = NULL;
  return childrenA;
}

OCDisplayNode::~OCDisplayNode() {
  gfree(name);
  if (children) {
    deleteGooList(children, OCDisplayNode);
  }
}

int OCDisplayNode::getNumChildren() {
  if (!children) {
    return 0;
  }
  return children->getLength();
}

OCDisplayNode *OCDisplayNode::getChild(int idx) {
  return (OCDisplayNode *)children->get(idx);
}
