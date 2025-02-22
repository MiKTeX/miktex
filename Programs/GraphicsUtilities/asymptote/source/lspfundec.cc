/**
 * @file lspfundec.cc
 * @brief For createSymMap and other lsp-related functions
 * specific to Lsp in *fundef classes.
 * @author Supakorn 'Jamie' Rassameemasmuang (jamievlin at outlook.com)
 */

#include "common.h"
#include "fundec.h"
#include "stm.h"

#define DEC_CREATE_SYM_MAP_FUNDEC(derived_class)                               \
  void derived_class::createSymMap(AsymptoteLsp::SymbolContext* symContext)

namespace absyntax
{
#ifdef HAVE_LSP

DEC_CREATE_SYM_MAP_FUNDEC(formals)
{
  for (auto& field : fields) {
    field->createSymMap(symContext);
  }

  if (rest) {
    rest->createSymMap(symContext);
  }
}

void formals::addArgumentsToFnInfo(AsymptoteLsp::FunctionInfo& fnInfo)
{
  for (auto const& field : fields) {
    fnInfo.arguments.emplace_back(field->fnInfo());
  }

  if (rest) {
    fnInfo.restArgs= rest->fnInfo();
  }
  // handle rest case as well
}

void fundef::addArgumentsToFnInfo(AsymptoteLsp::FunctionInfo& fnInfo)
{
  params->addArgumentsToFnInfo(fnInfo);
  // handle rest case as well
}


DEC_CREATE_SYM_MAP_FUNDEC(fundef)
{
  auto* declCtx(symContext->newContext<AsymptoteLsp::AddDeclContexts>(
          getPos().LineColumn()
  ));
  params->createSymMap(declCtx);
  body->createSymMap(declCtx);
}

DEC_CREATE_SYM_MAP_FUNDEC(fundec)
{
  AsymptoteLsp::FunctionInfo& fnInfo= symContext->symMap.addFunDef(
          static_cast<std::string>(id), getPos().LineColumn(),
          static_cast<std::string>(*fun.result)
  );
  fun.addArgumentsToFnInfo(fnInfo);
  fun.createSymMap(symContext);
}

DEC_CREATE_SYM_MAP_FUNDEC(formal)
{
  if (start) {
    start->createSymMap(symContext);
  }
}

std::pair<std::string, optional<std::string>> formal::fnInfo() const
{
  std::string typeName(static_cast<std::string>(*base));
  return start != nullptr
                 ? std::make_pair(
                           typeName,
                           make_optional(
                                   static_cast<std::string>(start->getName())
                           )
                   )
                 : std::make_pair(typeName, nullopt);
}

#else

#  define DEC_CREATE_SYM_MAP_FUNDEC_EMPTY(derived_class)                       \
    DEC_CREATE_SYM_MAP_FUNDEC(derived_class) {}

DEC_CREATE_SYM_MAP_FUNDEC_EMPTY(formals)
DEC_CREATE_SYM_MAP_FUNDEC_EMPTY(fundef)
DEC_CREATE_SYM_MAP_FUNDEC_EMPTY(fundec)
DEC_CREATE_SYM_MAP_FUNDEC_EMPTY(formal)

void formals::addArgumentsToFnInfo(AsymptoteLsp::FunctionInfo& fnInfo) {}
void fundef::addArgumentsToFnInfo(AsymptoteLsp::FunctionInfo& fnInfo) {}

std::pair<std::string, optional<std::string>> formal::fnInfo() const
{
  return std::make_pair("", nullopt);
}

#endif

}// namespace absyntax
