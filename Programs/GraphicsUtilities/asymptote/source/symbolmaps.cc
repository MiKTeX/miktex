#include "common.h"

#ifdef HAVE_LSP

#include "symbolmaps.h"
#include "locate.h"
#include <unordered_map>

namespace AsymptoteLsp
{
  [[nodiscard]]
  bool isVirtualFile(std::string const& filename)
  {
    bool isSettings=filename=="settings";
#ifdef HAVE_LIBGSL
    bool isGSL=filename=="gsl" and settings::getSetting<bool>("gsl");
#else
    bool isGSL=false;
#endif
    return isSettings || isGSL;
  }

  std::string getPlainFile()
  {
    return std::string((std::string) settings::locateFile("plain", true).c_str());
  }

  positions::positions(filePos const& positionInFile)
  {
    add(positionInFile);
  }

  void positions::add(const filePos &positionInFile)
  {
    auto fileLoc=pos.find(positionInFile.first);
    if (fileLoc == pos.end())
    {
      pos.emplace(positionInFile.first, std::vector<posInFile>{positionInFile.second});
    } else
    {
      fileLoc->second.push_back(positionInFile.second);
    }
  }

  ostream& operator <<(std::ostream& os, const SymbolMaps& sym)
  {
    os << "var decs:" << endl;
    for (auto const& s : sym.varDec)
    {
      auto const& key=std::get<0>(s);
      auto const& value=std::get<1>(s);
      os << key << " " << value.pos.first << ":" << value.pos.second << endl;
    }
    return os;
  }

  optional<fullSymPosRangeInFile> SymbolMaps::searchSymbol(posInFile const& inputPos)
  {
    // FIXME: can be optimized by binary search.
    for (auto const& s : usageByLines)
    {
      auto const& pos=std::get<0>(s);
      auto const& syLit=std::get<1>(s);
      size_t endCharacter = pos.second + syLit.name.length() - 1;
      bool posMatches =
              pos.first == inputPos.first and
              pos.second <= inputPos.second and
              inputPos.second <= endCharacter;
      bool isOperator = syLit.name.find("operator ") == 0;
      if (posMatches and !isOperator)
      {
        posInFile endPos(pos.first, endCharacter + 1);
        return boost::make_optional(std::make_tuple(syLit, pos, endPos));
      }
    }
    return nullopt;
  }

  FunctionInfo& SymbolMaps::addFunDef(
          std::string const& funcName, posInFile const& position, std::string const& returnType)
  {
    auto fit=std::get<0>(funDec.emplace(std::piecewise_construct,
                                        std::forward_as_tuple(funcName), std::forward_as_tuple()));

    fit->second.emplace_back(funcName, position, returnType);
    return fit->second.back();
  }

  std::pair<optional<fullSymPosRangeInFile>, SymbolContext*> SymbolContext::searchSymbol(posInFile const& inputPos)
  {
    auto currCtxSym = symMap.searchSymbol(inputPos);
    if (currCtxSym.has_value())
    {
      return make_pair(currCtxSym, this);
    }
    // else, not found in currCtx;
    for (auto& subContext : subContexts)
    {
      if (!posLt(inputPos, subContext->contextLoc))
      {
        auto v=subContext->searchSymbol(inputPos);
        auto subCtxSym=std::get<0>(v);;
        auto ctx=std::get<1>(v);
        if (subCtxSym.has_value())
        {
          return make_pair(subCtxSym, ctx);
        }
      }
    }
    return make_pair(nullopt, nullptr);
  }

  SymbolInfo const* SymbolContext::searchVarRaw(std::string const& symbol) const
  {
    // local variable declarations
    auto pt = symMap.varDec.find(symbol);
    if (pt != symMap.varDec.end())
    {
      return &pt->second;
    }

    // otherwise, search parent.
    return parent != nullptr ? parent->searchVarRaw(symbol) : nullptr;
  }

  optional<posRangeInFile> SymbolContext::searchVarDecl(
          std::string const& symbol, optional<posInFile> const& position)
  {
    auto pt = symMap.varDec.find(symbol);
    if (pt != symMap.varDec.end())
    {
      auto line = std::get<0>(pt->second.pos);
      auto ch = std::get<1>(pt->second.pos);
      if ((not position.has_value()) or (!posLt(position.value(), pt->second.pos)))
      {
        return std::make_tuple(getFileName().value_or(""),
                               pt->second.pos, std::make_pair(line, ch + symbol.length()));
      }
    }

    auto ptFn = symMap.funDec.find(symbol);
    if (ptFn != symMap.funDec.end() and !ptFn->second.empty())
    {
      // FIXME: Right now, we have no way of knowing the exact position of the
      //        start of where the function name is. As an example, we do not know
      //        where the exact position of
      //        real testFunction(...) { ...
      //             ^
      auto ptValue = ptFn->second[0];
      if ((not position.has_value()) or (!posLt(position.value(), ptValue.pos)))
      {
        return std::make_tuple(getFileName().value_or(""), ptValue.pos, ptValue.pos);
      }
    }

    // otherwise, search parent.
    return parent != nullptr ? parent->searchVarDecl(symbol, position) : nullopt;
  }

  void SymbolContext::addPlainFile()
  {
    std::string plainFile = getPlainFile();
    addEmptyExtRef(plainFile);
  }

  SymbolContext::SymbolContext(posInFile loc):
    fileLoc(nullopt), contextLoc(std::move(loc)), parent(nullptr)
  {
    addPlainFile();
  }

  SymbolContext::SymbolContext(posInFile loc, std::string filename):
          fileLoc(std::move(filename)), contextLoc(std::move(loc)), parent(nullptr)
  {
    addPlainFile();
  }

  optional<posRangeInFile>
  SymbolContext::searchVarDeclFull(std::string const& symbol, optional<posInFile> const& position)
  {
    std::unordered_set<SymbolContext*> searched;
    std::unordered_set<std::string> symbolSearch { symbol };

    auto returnVal = _searchVarFull<posRangeInFile>(
            searched, symbolSearch,
            [&position](SymbolContext* ctx, std::string const& sym)
            {
              return ctx->searchVarDecl(sym, position);
            },
            [](SymbolContext* ctx, std::string const& sym)
            {
              return ctx->searchVarDecl(sym);
            },
            fnFromDeclCreateTrav);
    if (returnVal.has_value())
    {
      return returnVal;
    }

    return _searchVarFull<posRangeInFile>(
            symbol,
            [](SymbolContext* ctx, std::string const& sym) -> optional<posRangeInFile>
            {
              for (auto const& unravelVal : ctx->extRefs.unraveledVals)
              {
                if (auto* pctx=ctx->searchStructCtxFull(unravelVal))
                {
                  auto retVal = pctx->searchVarDecl(sym);
                  if (retVal.has_value())
                  {
                    return retVal;
                  }
                }
              }
              return nullopt;
            });
  }

  std::list<ExternalRefs::extRefMap::iterator> SymbolContext::getEmptyRefs()
  {
    std::list<ExternalRefs::extRefMap::iterator> finalList;

    for (auto it = extRefs.extFileRefs.begin(); it != extRefs.extFileRefs.end(); it++)
    {
      if (it->second == nullptr)
      {
        // cerr << it->first << endl;
        finalList.emplace_back(it);
      }
    }

    for (auto& ctx : subContexts)
    {
      finalList.splice(finalList.end(), ctx->getEmptyRefs());
    }

    return finalList;
  }

  optional<std::string> SymbolContext::getFileName() const
  {
    if (fileLoc.has_value())
    {
      return fileLoc;
    }
    else
    {
      return parent == nullptr ? fileLoc : parent->getFileName();
    }
  }

  optional<std::string> SymbolContext::searchVarSignatureFull(std::string const& symbol)
  {
    std::unordered_set<SymbolContext*> searched;
    auto retVal = _searchVarFull<std::string>(
            symbol,
            [](SymbolContext const* ctx, std::string const& sym) -> optional<std::string>
            {
              if (auto const* info = ctx->searchVarRaw(sym))
              {
                return info->signature();
              }
              return nullopt;
            },
            fnFromDeclCreateTrav);

    if (retVal.has_value())
    {
      return retVal;
    }

    // else,
    // search every unravel values possible (not only here, but also in every includes + unravels as well).
    // If an unravel value match a struct, then see if symbol matches the struct. If yes, return otherwise skip.

    return _searchVarFull<std::string>(
            symbol,
            [](SymbolContext* ctx, std::string const& sym) -> optional<std::string>
            {
              if (SymbolInfo* info=ctx->searchVarUnravelStructRaw(sym))
              {
                return info->signature();
              }
              return nullopt;
            });
  }

  SymbolInfo* SymbolContext::searchVarUnravelStructRaw(std::string const& symbol)
  {
    return searchVarUnravelStructRaw(symbol, nullopt);
  }

  SymbolInfo* SymbolContext::searchVarUnravelStructRaw(std::string const& symbol, optional<posInFile> const& position)
  {
    for (auto const& unravelVal : extRefs.unraveledVals)
    {
      if (auto* ctx=searchStructCtxFull(unravelVal))
      {
        // ctx points to a struct.
        auto it=ctx->symMap.varDec.find(symbol);
        if (it != ctx->symMap.varDec.end())
        {
          if (((not position.has_value()) or (!posLt(position.value(), it->second.pos))))
          {
            return &it->second;
          }
        }
      }
    }
    return parent != nullptr ? parent->searchVarUnravelStructRaw(symbol) : nullptr;
  }

  std::list<std::string> SymbolContext::searchFuncUnravelStruct(std::string const& symbol)
  {
    std::list<std::string> finalList;

    for (auto const& unravelVal : extRefs.unraveledVals)
    {
      if (auto* ctx=searchStructCtxFull(unravelVal))
      {
        // ctx points to a struct.
        auto it=ctx->symMap.funDec.find(symbol);
        if (it != ctx->symMap.funDec.end() and not it->second.empty())
        {
          std::transform(
                  it->second.begin(), it->second.end(), std::back_inserter(finalList),
                  [](FunctionInfo const& fnInfo)
                  {
                    return fnInfo.signature();
                  });
        }
      }
    }
    if (parent != nullptr)
    {
      finalList.splice(finalList.end(), parent->searchFuncUnravelStruct(symbol));
    }
    return finalList;
  }


  std::list<std::string> SymbolContext::searchFuncSignature(std::string const& symbol)
  {
    std::list<std::string> funcSigs;
    auto pt = symMap.funDec.find(symbol);
    if (pt != symMap.funDec.end())
    {
      std::transform(pt->second.begin(), pt->second.end(),
                     std::back_inserter(funcSigs),
                     [](FunctionInfo const& fnInfo) {
        return fnInfo.signature();
      });
    }

    if (parent != nullptr)
    {
      funcSigs.splice(funcSigs.end(), parent->searchFuncSignature(symbol));
    }
    return funcSigs;
  }

  std::list<std::string> SymbolContext::searchFuncSignatureFull(std::string const& symbol)
  {
    auto base_list = _searchAllVarFull<std::string>(
            symbol, std::mem_fn(&SymbolContext::searchFuncSignature), fnFromDeclCreateTrav);

    base_list.splice(base_list.end(), _searchAllVarFull<std::string>(
            symbol, std::mem_fn(&SymbolContext::searchFuncUnravelStruct)));
    return base_list;
  }

  optional<SymbolContext*> SymbolContext::searchStructContext(std::string const& tyVal) const
  {
    auto stCtx = symMap.typeDecs.find(tyVal);
    if (stCtx != symMap.typeDecs.end())
    {
      if (auto* stDec = dynamic_cast<StructDecs*>(stCtx->second.get()))
      {
        return make_optional(stDec->ctx);
      }
    }

    return parent != nullptr ? parent->searchStructContext(tyVal) : nullopt;
  };

  optional<std::string> SymbolContext::searchLitSignature(SymbolLit const& symbol)
  {
    if (symbol.scopes.empty())
    {
      return searchVarSignatureFull(symbol.name);
    }
    else
    {
      auto ctx=std::get<0>(searchLitContext(symbol));
      auto isStruct=std::get<1>(searchLitContext(symbol));
      if (ctx)
      {
        if (isStruct)
        {
          // ctx is a struct declaration. should not search beyond this struct.
          auto varDec=ctx->symMap.varDec.find(symbol.name);
          return varDec != ctx->symMap.varDec.end() ?
                 optional<std::string>(varDec->second.signature()) : nullopt;
        }
        else
        {
          return ctx->searchVarSignatureFull(symbol.name);
        }
      }
    }

    return nullopt;
  }

  std::list<std::string> SymbolContext::searchLitFuncSignature(SymbolLit const& symbol)
  {
    std::list<std::string> signatures;
    if (symbol.scopes.empty())
    {
      signatures.splice(signatures.end(), searchFuncSignatureFull(symbol.name));
    }
    else
    {
      auto ctx=std::get<0>(searchLitContext(symbol));
      auto isStruct=std::get<1>(searchLitContext(symbol));
      if (ctx)
      {
        if (isStruct)
        {
          // ctx is a struct declaration. should not search beyond this struct.
          auto fnDecs=ctx->symMap.funDec.find(symbol.name);
          if (fnDecs != ctx->symMap.funDec.end())
          {
            std::transform(
                    fnDecs->second.begin(), fnDecs->second.end(),
                    std::back_inserter(signatures),
                    [&scopes=symbol.scopes](FunctionInfo const& fnInf)
                    {
                      return fnInf.signature(scopes);
                    });
          }
        }
        else
        {
          signatures.splice(signatures.end(), searchFuncSignatureFull(symbol.name));
        }
      }
    }

    return signatures;
  }

  optional<posRangeInFile> SymbolContext::searchLitPosition(
          SymbolLit const& symbol,
          optional<posInFile> const& position)
  {
    if (symbol.scopes.empty())
    {
      return searchVarDeclFull(symbol.name, position);
    }
    else
    {
      auto ctx=std::get<0>(searchLitContext(symbol));
      auto isStruct=std::get<1>(searchLitContext(symbol));
      if (ctx) // ctx is a struct declaration. should not search beyond this struct.
      {
        if (isStruct)
        {
          auto varDec=ctx->symMap.varDec.find(symbol.name);
          if (varDec != ctx->symMap.varDec.end())
          {
            auto line=std::get<0>(varDec->second.pos);
            auto ch=std::get<1>(varDec->second.pos);
            return std::make_tuple(ctx->getFileName().value_or(""),
                                   varDec->second.pos, std::make_pair(line, ch + symbol.name.length()));
          }
        }
        else
        {
          return searchVarDeclFull(symbol.name, position);
        }
      }
      return nullopt;
    }
  }

  std::list<posRangeInFile>
  SymbolContext::searchLitFuncPositions(SymbolLit const& symbol, optional<posInFile> const& position)
  {
    if (symbol.scopes.empty())
    {
      return searchFuncDeclsFull(symbol.name, position);
    }
    else
    {
      std::list<posRangeInFile> fnDecls;
      auto v=searchLitContext(symbol);
      auto ctx=std::get<0>(v);
      auto isStruct=std::get<1>(v);
      if (ctx) // ctx is a struct declaration. should not search beyond this struct.
      {
        if (isStruct)
        {
          auto fnDec=ctx->symMap.funDec.find(symbol.name);
          if (fnDec != ctx->symMap.funDec.end())
          {
            for (auto const& ptValue : fnDec->second)
            {
              fnDecls.emplace_back(getFileName().value_or(""), ptValue.pos, ptValue.pos);
            }
          }
        }
        else
        {
          return searchFuncDeclsFull(symbol.name, position);
        }
      }
      return fnDecls;
    }
  }

  SymbolContext* SymbolContext::searchStructCtxFull(std::string const& symbol)
  {
    std::unordered_set<SymbolContext*> searched;
    optional<std::string> tyInfo=_searchVarFull<std::string, std::string>(
            symbol,
            [](SymbolContext const* ctx, std::string const& sym) -> optional<std::string>
            {
              if (auto const* info = ctx->searchVarRaw(sym))
              {
                return info->type;
              }
              return nullopt;
            },
            fnFromDeclCreateTrav);


    if (not tyInfo.has_value())
    {
      return nullptr;
    }

    return _searchVarFull<SymbolContext*>(
            tyInfo.value(), std::mem_fn(&SymbolContext::searchStructContext)).value_or(nullptr);
  }

  optional<SymbolContext*> SymbolContext::searchAccessDecls(std::string const& accessVal)
  {
    // fileIdPair includes accessVals

    auto src=extRefs.fileIdPair.find(accessVal);
    if (src != extRefs.fileIdPair.end())
    {
      std::string& fileSrc=src->second;
      auto ctxMap=extRefs.extFileRefs.find(fileSrc);
      if (ctxMap != extRefs.extFileRefs.end() && ctxMap->second != nullptr)
      {
        return ctxMap->second;
      }
    }

    return parent != nullptr ? parent->searchAccessDecls(accessVal) : nullopt;
  }

  /**
   * Fully search context for source structs or source file in access
   * @param symbol Symbol to search
   * @return pair of <code>[context, isStruct]</code>, where <code>isStruct</code> is true
   *    if the <code>context</code> is a struct, and false if the context is a file from access declaration.
   *    If context is nullptr, <code>isStruct</code>'s return does not have defined behavior.
   */
  std::pair<SymbolContext*, bool> SymbolContext::searchLitContext(SymbolLit const& symbol)
  {
    if (symbol.scopes.empty())
    {
      // if the symbol is empty, then we can access every variable, not just those in the struct context
      // and hence should be treated as "not a struct", even if the variable is accessed in the struct.
      // for example,
      // int z; struct St { int x; int y=x+z; }
      // here, x and z can be accessed in the struct context.
      return std::make_pair(this, false);
    }
    else
    {
      std::vector<std::string> scopes(symbol.scopes);
      bool isStruct=false;

      // search in struct
      auto* ctx=searchStructCtxFull(scopes.back());
      if (ctx)
      {
        isStruct = true;
        scopes.pop_back();
      }
      else
      {
        // search in access declarations
        ctx =_searchVarFull<SymbolContext*>(
                scopes.back(), std::mem_fn(&SymbolContext::searchAccessDecls)).value_or(nullptr);

        if (ctx)
        {
          scopes.pop_back();
        }
      }

      for (auto it = scopes.rbegin(); it != scopes.rend() and ctx != nullptr; it++)
      {
        // FIXME: Impelemnt scope searching
        //        example:
        //        varx.vary.varz => go into varx's type context (struct or external file), repeat.
        //        struct and extfile handled very differently

        // get next variable declaration loc.
        // assumes struct, hence we do not search entire workspace
        SymbolContext* newCtx=nullptr;

        auto locVarDec = ctx->symMap.varDec.find(*it);
        if (locVarDec != ctx->symMap.varDec.end() and locVarDec->second.type.has_value())
        {
          newCtx = ctx->_searchVarFull<SymbolContext*>(
                  locVarDec->second.type.value(), std::mem_fn(&SymbolContext::searchStructContext)
                  ).value_or(nullptr);
        }

        // here, we searched for a struct context and did not find anything.
        // If we are in a struct context, we hit a dead end.
        // otherwise, we search for access declaration contexts.
        if (newCtx == nullptr)
        {
          if (isStruct)
          {
            return make_pair(nullptr, false);
          }
          else
          {
            ctx =_searchVarFull<SymbolContext*>(
                    *it, std::mem_fn(&SymbolContext::searchAccessDecls)).value_or(nullptr);
          }
        }
        else
        {
          isStruct = true;
        }

        ctx = newCtx;
      }
      return make_pair(ctx, isStruct);
    }
  }

  std::unordered_set<std::string> SymbolContext::createTraverseSet()
  {
    std::unordered_set<std::string> traverseSet(extRefs.includeVals);
    traverseSet.emplace(getPlainFile());
    for (auto const& unravelVal : extRefs.unraveledVals)
    {
      auto it = extRefs.fileIdPair.find(unravelVal);
      if (it != extRefs.fileIdPair.end())
      {
        traverseSet.emplace(it->second);
      }
    }
    return traverseSet;
  }

  SymbolContext* SymbolContext::getExternalRef(std::string const& symbol)
  {
    auto symRef = extRefs.extFileRefs.find(symbol);
    if (symRef != extRefs.extFileRefs.end())
    {
      return symRef->second;
    }

    return parent != nullptr ? parent->getExternalRef(symbol) : nullptr;
  }

  std::list<posRangeInFile> SymbolContext::searchFuncDecls(std::string const& symbol)
  {
    return searchFuncDecls(symbol, nullopt);
  }

  std::list<posRangeInFile>
  SymbolContext::searchFuncDecls(std::string const& symbol, optional<posInFile> const& position)
  {
    std::list<posRangeInFile> funcDecls;
    auto pt = symMap.funDec.find(symbol);
    if (pt != symMap.funDec.end())
    {
      for (auto const& fnInfo : pt->second)
      {
        if ((not position.has_value()) or (!posLt(position.value(), fnInfo.pos)))
        {
          funcDecls.emplace_back(getFileName().value_or(""), fnInfo.pos, fnInfo.pos);
        }
      }
    }

    if (parent != nullptr)
    {
      funcDecls.splice(funcDecls.end(), parent->searchFuncDecls(symbol, position));
    }
    return funcDecls;
  }

  std::list<posRangeInFile>
  SymbolContext::searchFuncDeclsFull(std::string const& symbol, optional<posInFile> const& position)
  {
    std::unordered_set<SymbolContext*> searched;
    auto retVal = _searchAllVarFull<posRangeInFile>(
            searched,
            std::unordered_set<std::string> { symbol },
            [&position](SymbolContext* ctx, std::string const& symbol)
            {
              return ctx->searchFuncDecls(symbol, position);
            },
            [](SymbolContext* ctx, std::string const& symbol)
            {
              return ctx->searchFuncDecls(symbol);
            },
            fnFromDeclCreateTrav);

    auto retValStruct =_searchAllVarFull<posRangeInFile>(
            symbol,
            [](SymbolContext* ctx, std::string const& sym) -> std::list<posRangeInFile>
            {
              std::list<posRangeInFile> finalList;
              for (auto const& unravelVal : ctx->extRefs.unraveledVals)
              {
                if (auto* pctx=ctx->searchStructCtxFull(unravelVal))
                {
                  finalList.splice(finalList.end(), pctx->searchFuncDecls(sym));
                }
              }
              return finalList;
            });
    retVal.splice(retVal.end(), std::move(retValStruct));
    return retVal;
  }

  optional<posRangeInFile> AddDeclContexts::searchVarDecl(
          std::string const& symbol, optional<posInFile> const& position)
  {
    auto pt = additionalDecs.find(symbol);
    if (pt != additionalDecs.end())
    {
      auto line=std::get<0>(pt->second.pos);
      auto ch=std::get<1>(pt->second.pos);
      if ((not position.has_value()) or (!posLt(position.value(), pt->second.pos)))
      {
        return std::make_tuple(getFileName().value_or(""),
                               pt->second.pos,
                               std::make_pair(line, ch + symbol.length()));
      }
    }

    return SymbolContext::searchVarDecl(symbol, position);
  }

  SymbolInfo const* AddDeclContexts::searchVarRaw(std::string const& symbol) const
  {
    auto pt = additionalDecs.find(symbol);
    return pt != additionalDecs.end() ? &pt->second : SymbolContext::searchVarRaw(symbol);
  }

  bool SymbolInfo::operator==(SymbolInfo const& sym) const
  {
    return name==sym.name and type==sym.type and pos == sym.pos;
  }

  std::string SymbolInfo::signature() const
  {
    return type.value_or("<decl-unknown>") + " " + name + ";";
  }

  std::string FunctionInfo::signature() const
  {
    return signature(std::vector<std::string>());
  }

  std::string FunctionInfo::signature(std::vector<std::string> const& scopes) const
  {
    std::stringstream ss;
    ss << returnType << " ";
    for (auto it=scopes.crbegin(); it!=scopes.crend(); it++)
    {
      ss << *it << ".";
    }
    ss << name << "(";
    for (auto it = arguments.begin(); it != arguments.end(); it++)
    {
      auto const& argtype=std::get<0>(*it);
      auto const& argname=std::get<1>(*it);
      ss << argtype;
      if (argname.has_value())
      {
        ss << " " << argname.value();
      }

      if (std::next(it) != arguments.end())
      {
        ss << ",";
      }

      if (std::next(it) != arguments.end() or restArgs.has_value())
      {
        ss << " ";
      }
    }

    if (restArgs.has_value())
    {
      auto const& v=restArgs.value();
      auto const& argtype=std::get<0>(v);
      auto const& argname=std::get<1>(v);
      ss << "... " << argtype;

      if (argname.has_value())
      {
        ss << " " << argname.value();
      }
    }
    ss << ");";
    return ss.str();
  }
}

#endif
