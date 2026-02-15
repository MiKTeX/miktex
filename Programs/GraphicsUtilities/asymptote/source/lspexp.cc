/**
 * @file lspexp.cc
 * @brief For createSymMap and other lsp-related functions
 * specific to Lsp in *exp classes.
 * @author Supakorn 'Jamie' Rassameemasmuang (jamievlin at outlook.com)
 */

#include "common.h"
#include "exp.h"

#define DEC_CREATE_SYM_MAP_EXP(derived_class)                                  \
  void derived_class::createSymMap(AsymptoteLsp::SymbolContext* symContext)


namespace absyntax
{

#ifdef HAVE_LSP

DEC_CREATE_SYM_MAP_EXP(nameExp)
{
  AsymptoteLsp::SymbolLit accessedName(value->getLit());
  position basePos= getPos();
  AsymptoteLsp::filePos castedPos=
          dynamic_cast<qualifiedName*>(value)
                  ? std::make_pair(
                            mem::stdString(basePos.filename()),
                            std::make_pair(basePos.Line(), basePos.Column() + 1)
                    )
                  : static_cast<AsymptoteLsp::filePos>(basePos);

  auto varUsageIt= symContext->symMap.varUsage.find(accessedName);
  if (varUsageIt == symContext->symMap.varUsage.end()) {
    symContext->symMap.varUsage.emplace(accessedName, castedPos);
  } else {
    varUsageIt->second.add(castedPos);
  }

  symContext->symMap.usageByLines.emplace_back(castedPos.second, accessedName);
}

DEC_CREATE_SYM_MAP_EXP(argument) { val->createSymMap(symContext); }

DEC_CREATE_SYM_MAP_EXP(arglist)
{
  for (auto& p : args) {
    p.createSymMap(symContext);
  }
}

DEC_CREATE_SYM_MAP_EXP(callExp)
{
  callee->createSymMap(symContext);
  args->createSymMap(symContext);

  if (auto col= getColorInformation()) {
    auto const& v= col.value();
    auto const& colVal= std::get<0>(v);
    auto const& alpha= std::get<1>(v);
    auto const& beginArgPos= std::get<2>(v);
    auto const& lastArgPos= std::get<3>(v);
    if (alpha.has_value()) {

      auto const& red= std::get<0>(colVal);
      auto const& green= std::get<1>(colVal);
      auto const& blue= std::get<2>(colVal);
      std::tuple<double, double, double, double> rgba(
              red, green, blue, alpha.value()
      );

      symContext->addRGBAColor(rgba, beginArgPos, lastArgPos);
    } else {
      symContext->addRGBColor(colVal, beginArgPos, lastArgPos);
    }
  }
}

DEC_CREATE_SYM_MAP_EXP(castExp) { castee->createSymMap(symContext); }

DEC_CREATE_SYM_MAP_EXP(assignExp)
{
  dest->createSymMap(symContext);
  value->createSymMap(symContext);
}

optional<std::tuple<
        callExp::colorInfo, optional<double>, AsymptoteLsp::posInFile,
        AsymptoteLsp::posInFile>>
callExp::getColorInformation()
{
  auto* namedCallee= dynamic_cast<nameExp*>(callee);
  if (namedCallee == nullptr) {
    return nullopt;
  }

  std::string calleeName= static_cast<std::string>(namedCallee->getName());
  std::vector<double> colors;

  auto getLineColumn= [&argsval= args->args](int const& idx) {
    return argsval[idx].val->getPos().LineColumn();
  };

  if (calleeName == "rgb" || calleeName == "rgba") {
    for (auto const& expVec : args->args) {
      if (auto* valExp= dynamic_cast<realExp*>(expVec.val)) {
        colors.push_back(valExp->getValue<double>());
      } else if (auto* valExpI= dynamic_cast<intExp*>(expVec.val)) {
        colors.push_back(valExpI->getValue<double>());
      }
    }
  }
  if (calleeName == "rgb" && colors.size() == 3) {
    callExp::colorInfo col(colors[0], colors[1], colors[2]);
    return std::make_tuple(
            col, optional<double>(), callee->getPos().LineColumn(),
            getLineColumn(2)
    );
  } else if (calleeName == "rgba" && colors.size() == 4) {
    callExp::colorInfo col(colors[0], colors[1], colors[2]);
    return std::make_tuple(
            col, optional<double>(colors[3]), callee->getPos().LineColumn(),
            getLineColumn(3)
    );
  }
  return nullopt;
}

#else

#  define DEC_CREATE_SYM_MAP_EXP_EMPTY(derived_class)                          \
    DEC_CREATE_SYM_MAP_EXP(derived_class) {}

DEC_CREATE_SYM_MAP_EXP_EMPTY(nameExp)
DEC_CREATE_SYM_MAP_EXP_EMPTY(argument)
DEC_CREATE_SYM_MAP_EXP_EMPTY(arglist)
DEC_CREATE_SYM_MAP_EXP_EMPTY(callExp)
DEC_CREATE_SYM_MAP_EXP_EMPTY(castExp)
DEC_CREATE_SYM_MAP_EXP_EMPTY(assignExp)

optional<std::tuple<
        callExp::colorInfo, optional<double>, AsymptoteLsp::posInFile,
        AsymptoteLsp::posInFile>>
callExp::getColorInformation()
{
  return nullopt;
}
#endif
}// namespace absyntax
