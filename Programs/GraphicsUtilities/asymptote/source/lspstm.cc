/**
 * @file lspstm.cc
 * @brief For createSymMap and other functions specific to Lsp in *stm classes.
 * @author Supakorn 'Jamie' Rassameemasmuang (jamievlin at outlook.com)
 */

#include "common.h"
#include "exp.h"
#include "stm.h"

#define DEC_CREATE_SYM_MAP_FUNCTION_STM(derived_class)                         \
  void derived_class::createSymMap(AsymptoteLsp::SymbolContext* symContext)


namespace absyntax
{
#ifdef HAVE_LSP

DEC_CREATE_SYM_MAP_FUNCTION_STM(forStm)
{

  AsymptoteLsp::SymbolContext* ctx(symContext);
  if (init) {
    auto* declCtx(symContext->newContext(getPos().LineColumn()));
    init->createSymMap(declCtx);
    ctx= declCtx;
  }
  if (test) {
    test->createSymMap(ctx);
  }
  if (update) {
    update->createSymMap(ctx);
  }
  body->createSymMap(ctx);
}

DEC_CREATE_SYM_MAP_FUNCTION_STM(extendedForStm)
{

  auto* declCtx(symContext->newContext(getPos().LineColumn()));

  std::string varName(var);

  // FIXME: How do we get the position of the actual variable name?
  //        Right now, we only get the starting position of the type declaration
  declCtx->symMap.varDec.emplace(
          std::piecewise_construct, std::forward_as_tuple(varName),
          std::forward_as_tuple(
                  varName, static_cast<std::string>(*start),
                  start->getPos().LineColumn()
          )
  );
  set->createSymMap(symContext);
  body->createSymMap(declCtx);
}

DEC_CREATE_SYM_MAP_FUNCTION_STM(returnStm)
{

  if (value) {
    value->createSymMap(symContext);
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_STM(blockStm)
{

  base->createSymMap(symContext->newContext(getPos().LineColumn()));
}

DEC_CREATE_SYM_MAP_FUNCTION_STM(expStm) { body->createSymMap(symContext); }
DEC_CREATE_SYM_MAP_FUNCTION_STM(ifStm)
{

  test->createSymMap(symContext);
  onTrue->createSymMap(symContext);

  if (onFalse) {
    onFalse->createSymMap(symContext);
  }
}

DEC_CREATE_SYM_MAP_FUNCTION_STM(whileStm)
{

  // while (<xyz>) { <body> }
  // the <xyz> part belongs in the main context as the while statement,
  // as it cannot declare new variables and only knows the symbols from that
  // context.

  test->createSymMap(symContext);

  // for the body part, { <body> } are encapsulated in
  // the blockStm, while <body> are direct statements.
  // If the while block does not use { <body> }, then the body
  // can be considered the same context as it cannot declare new variables and
  // again, can only uses the variable already known before this while
  // statement.

  body->createSymMap(symContext);
}

DEC_CREATE_SYM_MAP_FUNCTION_STM(doStm)
{

  body->createSymMap(symContext);
  test->createSymMap(symContext);
}

#else
#  define DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(derived_class)                 \
    DEC_CREATE_SYM_MAP_FUNCTION_STM(derived_class) {}

DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(forStm)
DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(extendedForStm)
DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(returnStm)
DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(blockStm)
DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(expStm)
DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(ifStm)
DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(whileStm)
DEC_CREATE_SYM_MAP_FUNCTION_STM_EMPTY(doStm)

#endif
}// namespace absyntax
