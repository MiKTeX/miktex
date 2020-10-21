/*****
 * primitives.h
 * Andy Hammerlindl 2007/04/27
 *
 * A list of the primative types in Asymptote, defined using the
 * PRIMITIVE(name,Name,asyName) macro.  This macro should be defined in by the
 * code including this file for the context at hand.
 *
 * name - the name of the type in C++ code ex: boolean
 * Name - the same name capitalized        ex: Boolean
 * asyName - the name in Asymptote code    ex: bool
 *
 *****/

// No ifndef because this file may be included multiple times in different
// contexts.

PRIMITIVE(void,Void,void)
PRIMITIVE(inferred,Inferred,var)
/* null is not a primitive type. */
#ifdef PRIMERROR
PRIMITIVE(error,Error,<error>)
#endif

PRIMITIVE(boolean,Boolean,bool)
PRIMITIVE(Int,Int,int)
PRIMITIVE(real,Real,real)
PRIMITIVE(string,String,string)
PRIMITIVE(pair,Pair,pair)
PRIMITIVE(triple,Triple,triple)
PRIMITIVE(transform,Transform,transform)
PRIMITIVE(guide,Guide,guide)
PRIMITIVE(path,Path,path)
PRIMITIVE(path3,Path3,path3)
PRIMITIVE(cycleToken,CycleToken,cycleToken)
PRIMITIVE(tensionSpecifier,TensionSpecifier,tensionSpecifier)
PRIMITIVE(curlSpecifier,CurlSpecifier,curlSpecifier)
  PRIMITIVE(pen,Pen,pen)
PRIMITIVE(picture,Picture,frame)
PRIMITIVE(file,File,file)
PRIMITIVE(code,Code,code)
