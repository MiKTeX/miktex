/**
 * @file lspdec.cc
 * @brief For createSymMap and other functions specific to Lsp in *dec classes.
 * @author Supakorn 'Jamie' Rassameemasmuang (jamievlin at outlook.com)
 */

#include "common.h"
#include "dec.h"


#ifdef HAVE_LSP
#  include "locate.h"
#endif

#define DEC_CREATE_SYM_MAP_FUNCTION_DEF(derived_class)                         \
  void derived_class::createSymMap(AsymptoteLsp::SymbolContext* symContext)


namespace absyntax
{

#ifdef HAVE_LSP
DEC_CREATE_SYM_MAP_FUNCTION_DEF(unraveldec)
{
  std::string fileName= static_cast<std::string>(id->getName());
  if (not AsymptoteLsp::isVirtualFile(fileName)) {
    symContext->extRefs.addUnravelVal(fileName);
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(fromaccessdec)
{
  // filename is id;
  std::string idStr(id);
  symContext->extRefs.addEmptyExtRef(idStr);

  auto* f= this->fields;
  if (f) {
    // add [dest] -> [src, filename] to fromAccessDecls;
    f->processListFn([&symContext,
                      &idStr](symbol const& src, symbol const& dest) {
      std::string srcId(src);
      std::string destId(dest);
      symContext->extRefs.addFromAccessVal(idStr, srcId, destId);
    });
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(importdec) { base.createSymMap(symContext); }

DEC_CREATE_SYM_MAP_FUNCTION_DEF(includedec)
{
  std::string fullname(
          (std::string) settings::locateFile(filename, true).c_str()
  );
  if (not AsymptoteLsp::isVirtualFile(fullname)) {
    symContext->addEmptyExtRef(fullname);
    symContext->extRefs.includeVals.emplace(fullname);
  }
}


DEC_CREATE_SYM_MAP_FUNCTION_DEF(recorddec)
{
  auto* newCtx= symContext->newContext(getPos().LineColumn());
  auto* structTyInfo= symContext->newTypeDec<AsymptoteLsp::StructDecs>(
          static_cast<std::string>(id), getPos().LineColumn()
  );
  if (structTyInfo != nullptr) {
    structTyInfo->ctx= newCtx;
    body->createSymMap(newCtx);
  } else {
    cerr << "Cannot create new struct context" << endl;
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(block)
{
  for (auto const& p : stms) {
    p->createSymMap(symContext);
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(decidstart)
{
  std::string name(static_cast<std::string>(getName()));
  AsymptoteLsp::posInFile pos(getPos().LineColumn());
  if (auto decCtx= dynamic_cast<AsymptoteLsp::AddDeclContexts*>(symContext)) {
    decCtx->additionalDecs.emplace(
            std::piecewise_construct, std::forward_as_tuple(name),
            std::forward_as_tuple(name, pos)
    );
  } else {
    symContext->symMap.varDec[name]= AsymptoteLsp::SymbolInfo(name, pos);
  }
}


void decidstart::createSymMapWType(
        AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
)
{
  std::string name(static_cast<std::string>(getName()));
  AsymptoteLsp::posInFile pos(getPos().LineColumn());
  if (auto decCtx= dynamic_cast<AsymptoteLsp::AddDeclContexts*>(symContext)) {
    if (base == nullptr) {
      decCtx->additionalDecs.emplace(
              std::piecewise_construct, std::forward_as_tuple(name),
              std::forward_as_tuple(name, pos)
      );
    } else {
      decCtx->additionalDecs.emplace(
              std::piecewise_construct, std::forward_as_tuple(name),
              std::forward_as_tuple(name, static_cast<std::string>(*base), pos)
      );
    }
  } else {
    symContext->symMap.varDec[name]=
            base == nullptr ? AsymptoteLsp::SymbolInfo(name, pos)
                            : AsymptoteLsp::SymbolInfo(
                                      name, static_cast<std::string>(*base), pos
                              );
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(decid)
{
  start->createSymMap(symContext);
  if (init) {
    init->createSymMap(symContext);
  }
}

void decid::createSymMapWType(
        AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
)
{
  start->createSymMapWType(symContext, base);
  if (init) {
    init->createSymMap(symContext);
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(decidlist)
{
  for (auto const& p : decs) {
    p->createSymMap(symContext);
  }
}

void decidlist::createSymMapWType(
        AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
)
{
  for (auto const& p : decs) {
    p->createSymMapWType(symContext, base);
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(vardec)
{
  decs->createSymMapWType(symContext, base);
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(idpair)
{
  if (valid) {
    string fullSrc(settings::locateFile(src, true));
    if (not AsymptoteLsp::isVirtualFile((std::string) (fullSrc.c_str()))) {
      if (not fullSrc.empty()) {
        symContext->addEmptyExtRef((std::string) (fullSrc.c_str()));
      }

      // add (dest, source) to reference map.
      auto s= symContext->extRefs.fileIdPair.emplace(
              dest, (std::string) fullSrc.c_str()
      );
      auto it= std::get<0>(s);
      auto success= std::get<1>(s);
      if (not success) {
        it->second= (std::string) (fullSrc.c_str());
      }

      symContext->extRefs.addAccessVal(static_cast<std::string>(dest));
    }
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_DEF(accessdec) { base->createSymMap(symContext); }

DEC_CREATE_SYM_MAP_FUNCTION_DEF(idpairlist)
{
  for (auto& idp : base) {
    idp->createSymMap(symContext);
  }
}

#else

#  define DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(derived_class)                 \
    DEC_CREATE_SYM_MAP_FUNCTION_DEF(derived_class) {}

void decidstart::createSymMapWType(
        AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
)
{}

void decid::createSymMapWType(
        AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
)
{}

void decidlist::createSymMapWType(
        AsymptoteLsp::SymbolContext* symContext, absyntax::astType* base
)
{}

DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(unraveldec)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(fromaccessdec)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(importdec)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(includedec)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(recorddec)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(block)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(decidstart)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(decid)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(decidlist)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(vardec)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(idpair)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(accessdec)
DEC_CREATE_SYM_MAP_FUNCTION_DEF_EMPTY(idpairlist)

#endif
}// namespace absyntax
