#pragma once

#if defined(MIKTEX)
#include <miktex/asy-first.h>
#endif
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <utility>

#include "common.h"
#include "makeUnique.h"

#include "LibLsp/lsp/lsPosition.h"
#include "LibLsp/lsp/textDocument/documentColor.h"

#ifndef boost
#define nullopt boost::none
#endif

namespace AsymptoteLsp
{
  struct SymbolLit
  {
    std::string name;
    std::vector<std::string> scopes;

    SymbolLit(std::string symName) :
      name(std::move(symName))
    {
    }

    SymbolLit(std::string symName, std::vector<std::string> scope) :
            name(std::move(symName)), scopes(std::move(scope))
    {
    }

    ~SymbolLit() = default;

    SymbolLit(SymbolLit const& sym) :
      name(sym.name), scopes(sym.scopes)
    {
    }

    SymbolLit& operator=(SymbolLit const& sym)
    {
      name = sym.name;
      scopes = sym.scopes;
      return *this;
    }

    SymbolLit(SymbolLit&& sym) noexcept :
      name(std::move(sym.name)), scopes(std::move(sym.scopes))
    {
    }

    SymbolLit& operator=(SymbolLit&& sym) noexcept
    {
      name = std::move(sym.name);
      scopes = std::move(sym.scopes);
      return *this;
    }

    bool operator==(SymbolLit const& other) const
    {
      return name == other.name and scopes == other.scopes;
    }

    bool matchesRaw(std::string const& sym) const
    {
      return name == sym;
    }
  };
} // namespace AsymptoteLsp

namespace std
{
  using AsymptoteLsp::SymbolLit;

  template<>
  struct hash<SymbolLit>
  {
    std::size_t operator()(SymbolLit const& sym) const
    {
      size_t final_hash = 0;
      final_hash ^= hash<std::string>()(sym.name);
      for (auto const& accessor : sym.scopes)
      {
        final_hash = (final_hash << 1) ^ hash<std::string>()(accessor);
      }
      return final_hash;
    }
  };
} // namespace std

namespace AsymptoteLsp
{
  using std::unordered_map;
  struct SymbolContext;

  typedef std::pair<std::string, SymbolContext*> contextedSymbol;
  typedef std::pair<size_t, size_t> posInFile;
  typedef std::pair<std::string, posInFile> filePos;
  typedef std::tuple<std::string, posInFile, posInFile> posRangeInFile;
  typedef std::tuple<SymbolLit, posInFile, posInFile> fullSymPosRangeInFile;


  // NOTE: lsPosition is zero-indexed, while all Asymptote positions (incl this struct) is 1-indexed.
  inline posInFile fromLsPosition(lsPosition const& inPos)
  {
    return std::make_pair(inPos.line + 1, inPos.character + 1);
  }

  inline lsPosition toLsPosition(posInFile const& inPos)
  {
    return lsPosition(inPos.first - 1, inPos.second - 1);
  }

  inline bool posLt(posInFile const& p1, posInFile const& p2)
  {
    return (p1.first < p2.first) or ((p1.first == p2.first) and (p1.second < p2.second));
  }

  std::string getPlainFile();
  bool isVirtualFile(std::string const& filename);

  // filename to positions
  struct positions
  {
    std::unordered_map<std::string, std::vector<posInFile>> pos;

    positions() = default;
    explicit positions(filePos const& positionInFile);
    void add(filePos const& positionInFile);
  };


  struct SymbolInfo
  {
    std::string name;
    optional<std::string> type;
    posInFile pos;

    SymbolInfo() : type(nullopt), pos(1, 1) {}

    SymbolInfo(std::string inName, posInFile position):
      name(std::move(inName)), type(nullopt), pos(std::move(position)) {}

    SymbolInfo(std::string inName, std::string inType, posInFile position):
      name(std::move(inName)), type(std::move(inType)), pos(std::move(position)) {}

    SymbolInfo(SymbolInfo const& symInfo) = default;

    SymbolInfo& operator=(SymbolInfo const& symInfo) = default;

    SymbolInfo(SymbolInfo&& symInfo) noexcept :
            name(std::move(symInfo.name)), type(std::move(symInfo.type)), pos(std::move(symInfo.pos))
    {
    }

    SymbolInfo& operator=(SymbolInfo&& symInfo) noexcept
    {
      name = std::move(symInfo.name);
      type = std::move(symInfo.type);
      pos = std::move(symInfo.pos);
      return *this;
    }

    virtual ~SymbolInfo() = default;

    bool operator==(SymbolInfo const& sym) const;

    [[nodiscard]]
    virtual std::string signature() const;
  };

  struct FunctionInfo: SymbolInfo
  {
    std::string returnType;
    using typeName = std::pair<std::string, optional<std::string>>;
    std::vector<typeName> arguments;
    optional<typeName> restArgs;

    FunctionInfo(std::string name, posInFile pos, std::string returnTyp):
            SymbolInfo(std::move(name), std::move(pos)),
            returnType(std::move(returnTyp)),
            arguments(), restArgs(nullopt) {}

    ~FunctionInfo() override = default;

    [[nodiscard]]
    std::string signature() const override;

    [[nodiscard]]
    std::string signature(std::vector<std::string> const& scopes) const;
  };


  struct TypeDec
  {
    posInFile position;
    std::string typeName;

    TypeDec(): position(1, 1) {}
    virtual ~TypeDec() = default;

    TypeDec(posInFile pos, std::string typName):
            position(std::move(pos)), typeName(std::move(typName))
    {
    }

    TypeDec(TypeDec const& typDec) = default;
    TypeDec& operator= (TypeDec const& typDec) = default;

    TypeDec(TypeDec&& typDec) noexcept = default;
    TypeDec& operator= (TypeDec&& typDec) = default;

    [[nodiscard]]
    virtual unique_ptr<TypeDec> clone() const
    {
      return make_unique<TypeDec>(*this);
    }
  };

  struct TypedefDec : public TypeDec
  {
    std::string destName;
  };

  struct SymColorInfo
  {
    posInFile rangeBegin;
    posInFile rangeEnd;

    using RGBColor = std::tuple<double, double, double>;
    using RGBAColor = std::tuple<double, double, double, double>;
    void setLastArgPos(posInFile lastArgPos) { lastArgPosition = std::move(lastArgPos); }

    SymColorInfo() = default;
    virtual ~SymColorInfo() = default;
    SymColorInfo(SymColorInfo const& col) = default;
    SymColorInfo& operator=(SymColorInfo const& col) = default;

    SymColorInfo(SymColorInfo&& col) noexcept = default;
    SymColorInfo& operator=(SymColorInfo&& col) noexcept = default;



    [[nodiscard]]
    virtual RGBColor getRGBColor() const = 0;

    [[nodiscard]]
    virtual double getAlpha() const
    {
      return 1;
    }

    [[nodiscard]]
    RGBAColor getRGBAColor() const
    {
      RGBColor c=getRGBColor();
      auto const& red=std::get<0>(c);
      auto const& green=std::get<1>(c);
      auto const& blue=std::get<2>(c);
      return RGBAColor(red,green,blue,getAlpha());
    }

    explicit operator TextDocument::Color() const
    {
      TextDocument::Color col;
      RGBAColor c=getRGBAColor();
      col.red=std::get<0>(c);
      col.green=std::get<1>(c);
      col.blue=std::get<2>(c);
      col.alpha=std::get<3>(c);
      return col;
    }

    [[nodiscard]]
    virtual unique_ptr<SymColorInfo> clone() const = 0;

  public:
    posInFile lastArgPosition;
  };

  struct RGBSymColorInfo : SymColorInfo
  {
    double red, green, blue;

    RGBSymColorInfo(): SymColorInfo(), red(0), green(0), blue(0) {}
    RGBSymColorInfo(double redVal, double greenVal, double blueVal):
      SymColorInfo(),
      red(redVal), green(greenVal), blue(blueVal)
    {
    }

    RGBSymColorInfo(RGBSymColorInfo const& col) = default;
    RGBSymColorInfo& operator=(RGBSymColorInfo const& col) = default;

    [[nodiscard]]
    RGBColor getRGBColor() const override
    {
      return RGBColor(red, green, blue);
    }

    [[nodiscard]]
    unique_ptr<SymColorInfo> clone() const override
    {
      return unique_ptr<SymColorInfo>(new RGBSymColorInfo(*this));
    }
  };

  struct RGBASymColorInfo : RGBSymColorInfo
  {
    double alpha;

    RGBASymColorInfo(): RGBSymColorInfo(), alpha(1) {}
    RGBASymColorInfo(double redVal, double greenVal, double blueVal, double alphaVal):
            RGBSymColorInfo(redVal, greenVal, blueVal), alpha(alphaVal)
    {
    }

    [[nodiscard]]
    double getAlpha() const override
    {
      return alpha;
    }

    [[nodiscard]]
    unique_ptr<SymColorInfo> clone() const override
    {
      return unique_ptr<SymColorInfo>(new RGBASymColorInfo(*this));
    }
  };

  struct StructDecs : public TypeDec
  {
    SymbolContext* ctx;

    StructDecs(): TypeDec(), ctx(nullptr) {}
    ~StructDecs() override = default;

    StructDecs(posInFile pos, std::string typName) :
            TypeDec(std::move(pos), std::move(typName)), ctx(nullptr)
    {
    }

    StructDecs(posInFile pos, std::string typName, SymbolContext* ctx) :
            TypeDec(std::move(pos), std::move(typName)), ctx(ctx)
    {
    }

    [[nodiscard]]
    unique_ptr<TypeDec> clone() const override
    {
      return std::unique_ptr<TypeDec>(new StructDecs(*this));
    }
  };

  struct SymbolMaps
  {
    unordered_map <std::string, SymbolInfo> varDec;
    unordered_map <std::string, std::vector<FunctionInfo>> funDec;
    // can refer to other files
    unordered_map <SymbolLit, positions> varUsage;
    unordered_map <std::string, unique_ptr<TypeDec>> typeDecs;

    // python equivalent of dict[str, list[tuple(pos, sym)]]
    // filename -> list[(position, symbol)]

    std::vector<std::pair<posInFile, SymbolLit>> usageByLines;

    SymbolMaps() = default;
    ~SymbolMaps() = default;

    SymbolMaps(SymbolMaps const& symMap) :
    varDec(symMap.varDec), funDec(symMap.funDec), varUsage(symMap.varUsage), typeDecs(),
    usageByLines(symMap.usageByLines)
    {
      for(auto const& t : symMap.typeDecs)
      {
        auto const& ty=std::get<0>(t);
        auto const& tyDec=std::get<1>(t);
        typeDecs.emplace(ty, tyDec != nullptr ? tyDec->clone() : nullptr);
      }
    }

    SymbolMaps& operator=(SymbolMaps const& symMap)
    {
      varDec = symMap.varDec;
      funDec = symMap.funDec;
      varUsage = symMap.varUsage;
      usageByLines = symMap.usageByLines;

      typeDecs.clear();
      for(auto const& t : symMap.typeDecs)
      {
        auto const& ty=std::get<0>(t);
        auto const& tyDec=std::get<1>(t);
        typeDecs.emplace(ty, tyDec != nullptr ? tyDec->clone() : nullptr);
      }
      return *this;
    }

    SymbolMaps(SymbolMaps&& symMap) noexcept:
            varDec(std::move(symMap.varDec)), funDec(std::move(symMap.funDec)), varUsage(std::move(symMap.varUsage)),
            typeDecs(std::move(symMap.typeDecs)), usageByLines(std::move(symMap.usageByLines))
    {
    }

    SymbolMaps& operator=(SymbolMaps&& symMap) noexcept
    {
      varDec = std::move(symMap.varDec);
      funDec = std::move(symMap.funDec);
      varUsage = std::move(symMap.varUsage);
      usageByLines = std::move(symMap.usageByLines);
      typeDecs = std::move(symMap.typeDecs);

      return *this;
    }

    inline void clear()
    {
      varDec.clear();
      funDec.clear();
      varUsage.clear();
      usageByLines.clear();
      typeDecs.clear();
    }
    optional<fullSymPosRangeInFile> searchSymbol(posInFile const& inputPos);
    FunctionInfo& addFunDef(std::string const& funcName, posInFile const& position, std::string const& returnType);

  private:
    friend ostream& operator<<(std::ostream& os, const SymbolMaps& sym);
  };

  struct ExternalRefs
  {

    // file interactions
    // access -> (file, id)
    // unravel -> id
    // include -> file
    // import = acccess + unravel

    using extRefMap = std::unordered_map<std::string, SymbolContext*>;
    extRefMap extFileRefs;
    std::unordered_map<std::string, std::string> fileIdPair;
    std::unordered_set<std::string> includeVals;
    std::unordered_set<std::string> unraveledVals;
    std::unordered_set<std::string> accessVals;
    std::unordered_map<std::string, std::pair<std::string, std::string>> fromAccessVals;

    ExternalRefs() = default;
    virtual ~ExternalRefs() = default;

    ExternalRefs(ExternalRefs const& exRef) = default;
    ExternalRefs& operator=(ExternalRefs const& exRef) = default;

    ExternalRefs(ExternalRefs&& exRef) noexcept = default;
    ExternalRefs& operator=(ExternalRefs&& exRef) noexcept = default;


    void clear()
    {
      extFileRefs.clear();
      fileIdPair.clear();
      includeVals.clear();
      unraveledVals.clear();
      accessVals.clear();
      fromAccessVals.clear();
    }

    bool addEmptyExtRef(std::string const& fileName)
    {
      auto success=std::get<1>(extFileRefs.emplace(fileName, nullptr));
      return success;
    }

    bool addAccessVal(std::string const& symbol)
    {
      auto success=std::get<1>(accessVals.emplace(symbol));
      return success;
    }

    bool addUnravelVal(std::string const& symbol)
    {
      auto success=std::get<1>(unraveledVals.emplace(symbol));
      return success;
    }

    bool addFromAccessVal(std::string const& fileName, std::string const& symbolSrc, std::string const& symbolDest)
    {
      auto success=std::get<1>(fromAccessVals.emplace(symbolDest, make_pair(symbolSrc, fileName)));
      return success;
    }
  };


  struct SymbolContext
  {
    optional<std::string> fileLoc;
    posInFile contextLoc;
    SymbolContext* parent;
    SymbolMaps symMap;

    // file interactions
    // access -> (file, id)
    // unravel -> id
    // include -> file
    // import = acccess + unravel

    ExternalRefs extRefs;

    std::vector<std::unique_ptr<SymColorInfo>> colorInformation;
    std::vector<std::unique_ptr<SymbolContext>> subContexts;

    SymbolContext():
      parent(nullptr)
    {
    }

    virtual ~SymbolContext() = default;

    explicit SymbolContext(posInFile loc);
    explicit SymbolContext(posInFile loc, std::string filename);

    SymbolContext(posInFile loc, SymbolContext* contextParent):
      fileLoc(nullopt), contextLoc(std::move(loc)), parent(contextParent)
    {
    }

    template<typename T=SymbolContext, typename=std::enable_if<std::is_base_of<SymbolContext, T>::value>>
    T* newContext(posInFile const& loc)
    {
      subContexts.emplace_back(make_unique<T>(loc, this));
      return static_cast<T*>(subContexts.back().get());
    }

    template<typename T=TypeDec, typename=std::enable_if<std::is_base_of<TypeDec, T>::value>>
    T* newTypeDec(std::string const& tyName, posInFile const& loc)
    {
      auto s=symMap.typeDecs.emplace(tyName, make_unique<T>(loc, tyName));
      auto it=std::get<0>(s);
      auto succ=std::get<1>(s);
      return succ ? static_cast<T*>(it->second.get()) : static_cast<T*>(nullptr);
    }

    SymbolContext(SymbolContext const& symCtx) :
      fileLoc(symCtx.fileLoc), contextLoc(symCtx.contextLoc),
      parent(symCtx.parent), symMap(symCtx.symMap),
      extRefs(symCtx.extRefs)
    {
      for (auto& ctx : symCtx.subContexts)
      {
        subContexts.push_back(make_unique<SymbolContext>(*ctx));
      }

      for (auto& col : symCtx.colorInformation)
      {
        colorInformation.emplace_back(col != nullptr ? col->clone() : nullptr);
      }
    }

    SymbolContext& operator= (SymbolContext const& symCtx)
    {
      fileLoc = symCtx.fileLoc;
      contextLoc = symCtx.contextLoc;
      parent = symCtx.parent;
      symMap = symCtx.symMap;
      extRefs = symCtx.extRefs;

      subContexts.clear();
      for (auto& ctx : symCtx.subContexts)
      {
        subContexts.push_back(make_unique<SymbolContext>(*ctx));
      }

      colorInformation.clear();
      for (auto& col : symCtx.colorInformation)
      {
        colorInformation.emplace_back(col != nullptr ? col->clone() : nullptr);
      }

      return *this;
    }

    SymbolContext(SymbolContext&& symCtx) noexcept :
            fileLoc(std::move(symCtx.fileLoc)), contextLoc(std::move(symCtx.contextLoc)),
            parent(symCtx.parent), symMap(std::move(symCtx.symMap)),
            extRefs(std::move(symCtx.extRefs)),
            colorInformation(std::move(symCtx.colorInformation)), subContexts(std::move(symCtx.subContexts))
    {
    }

    SymbolContext& operator= (SymbolContext&& symCtx) noexcept
    {
      fileLoc = std::move(symCtx.fileLoc);
      contextLoc = std::move(symCtx.contextLoc);
      parent = symCtx.parent;
      symMap = std::move(symCtx.symMap);
      extRefs = std::move(symCtx.extRefs);
      colorInformation = std::move(symCtx.colorInformation);
      subContexts = std::move(symCtx.subContexts);
      return *this;
    }

    // [file, start, end]
    virtual std::pair<optional<fullSymPosRangeInFile>, SymbolContext*> searchSymbol(posInFile const& inputPos);



    // declarations
    optional<posRangeInFile> searchVarDecl(std::string const& symbol)
    {
      return searchVarDecl(symbol, nullopt);
    }
    virtual optional<posRangeInFile> searchVarDecl(std::string const& symbol,
                                                   optional<posInFile> const& position);
    virtual optional<posRangeInFile> searchVarDeclFull(std::string const& symbol,
                                                       optional<posInFile> const& position=nullopt);
    virtual SymbolInfo const* searchVarRaw(std::string const& symbol) const;

    std::list<posRangeInFile> searchFuncDecls(std::string const& symbol);
    virtual std::list<posRangeInFile> searchFuncDecls(
            std::string const& symbol, optional<posInFile> const& position);
    std::list<posRangeInFile> searchFuncDeclsFull(std::string const& symbol,
                                                optional<posInFile> const& position=nullopt);

    // variable signatures
    optional<std::string> searchVarSignatureFull(std::string const& symbol);
    virtual std::list<std::string> searchFuncSignature(std::string const& symbol);
    virtual std::list<std::string> searchFuncSignatureFull(std::string const& symbol);

    optional<std::string> searchLitSignature(SymbolLit const& symbol);
    std::list<std::string> searchLitFuncSignature(SymbolLit const& symbol);

    optional<posRangeInFile> searchLitPosition(
            SymbolLit const& symbol, optional<posInFile> const& position=nullopt);
    std::list<posRangeInFile> searchLitFuncPositions(
            SymbolLit const& symbol, optional<posInFile> const& position=nullopt);

    virtual std::list<ExternalRefs::extRefMap::iterator> getEmptyRefs();

    optional<std::string> getFileName() const;

    SymbolContext* getParent()
    {
      return parent == nullptr ? this : parent->getParent();
    }

    bool addEmptyExtRef(std::string const& fileName)
    {
      return extRefs.addEmptyExtRef(fileName);
    }

    void reset(std::string const& newFile)
    {
      fileLoc = newFile;
      contextLoc = std::make_pair(1,1);
      clear();
    }

    void clear()
    {
      parent = nullptr;
      symMap.clear();
      extRefs.clear();
      clearColorInformation();
      subContexts.clear();
    }

    void clearColorInformation()
    {
      colorInformation.clear();
    }


    void addRGBColor(
            std::tuple<double, double, double> const& c,
            posInFile const& posBegin,
            posInFile const& lastArgs)
    {
      auto const& red=std::get<0>(c);
      auto const& green=std::get<1>(c);
      auto const& blue=std::get<2>(c);
      colorInformation.emplace_back(make_unique<RGBSymColorInfo>(red,green,blue));
      auto const& ptr=colorInformation.back();
      ptr->rangeBegin = posBegin;
      ptr->setLastArgPos(lastArgs);
    }

    void addRGBAColor(
            std::tuple<double, double, double, double> const& c,
            posInFile const& posBegin,
            posInFile const& lastArgs)
    {
      auto const& red=std::get<0>(c);
      auto const& green=std::get<1>(c);
      auto const& blue=std::get<2>(c);
      auto const& alpha=std::get<3>(c);
      colorInformation.emplace_back(make_unique<RGBASymColorInfo>(red,green,blue,alpha));
      auto const& ptr=colorInformation.back();
      ptr->rangeBegin = posBegin;
      ptr->setLastArgPos(lastArgs);
    }

  protected:
    using SymCtxSet = std::unordered_set<SymbolContext*>;

    // search var full

    template<
            typename TArg,
            template<typename...> class TMapTraverse=std::unordered_map,
            template<typename...> class TContainerArg=std::unordered_set
    >
    using SymbolArgContainer = TMapTraverse<std::string, TContainerArg<TArg>>;

    template<
            typename TArg,
            template<typename...> class TMapTraverse=std::unordered_map,
            template<typename...> class TContainerArg=std::unordered_set,
            template<typename...> class TArgContainer=std::unordered_set
    >
    using FnCreateSymbolArgContainer =
    std::function<SymbolArgContainer<TArg, TMapTraverse, TContainerArg>(SymbolContext*, TArgContainer<TArg> const&)>;

    template<typename TArg>
    SymbolArgContainer<TArg> defaultCreateTraverse(std::unordered_set<TArg> const& searchSet)
    {
      SymbolArgContainer<TArg> retVal;
      for (auto const& traverseVal : createTraverseSet())
      {
        retVal.emplace(traverseVal, searchSet);
      }
      return retVal;
    }

    SymbolArgContainer<std::string> fromDeclCreateTraverse(std::unordered_set<std::string> const& symbols)
    {
      // base
      SymbolArgContainer<std::string> base=defaultCreateTraverse(symbols);

      for (auto const& sym : symbols)
      {
        auto aliasSearch = extRefs.fromAccessVals.find(sym);
        if (aliasSearch != extRefs.fromAccessVals.end())
        {
          // there's an alias to dest -> [src, ctx].
          auto const& src=std::get<0>(aliasSearch->second);
          auto const& fileName=std::get<1>(aliasSearch->second);

          auto baseTrav = base.find(fileName);
          if (baseTrav == base.end())
          {
            base.emplace(fileName, std::unordered_set<std::string> { src });
          }
          else
          {
            baseTrav->second.emplace(src);
          }
        }
      }

      return base;
    }

    FnCreateSymbolArgContainer<std::string> const fnFromDeclCreateTrav =
            std::mem_fn(&SymbolContext::fromDeclCreateTraverse);

    template<typename TRet, typename TArg, typename TFn>
    optional<TRet> _searchVarFull(
            TArg init, TFn const& fnLocalPredicate,
            FnCreateSymbolArgContainer<TArg> const& fnCreateTraverse=
                    std::mem_fn(&SymbolContext::defaultCreateTraverse<TArg>))
    {
      std::unordered_set<SymbolContext*> searched;
      std::unordered_set<TArg> initSet { init };
      return _searchVarFull<TRet, TArg, TFn, TFn>(
              searched, initSet, fnLocalPredicate, fnLocalPredicate, fnCreateTraverse);
    }

    template<typename TRet, typename TArg, typename TFn, typename TFn2>
    optional<TRet> _searchVarFull(
            std::unordered_set<SymbolContext*>& searched,
            std::unordered_set<TArg> const& searchArgs,
            TFn const& fnLocalPredicate, TFn2 const& fnLocalPredicateFirst,
            FnCreateSymbolArgContainer<TArg> const& fnCreateTraverse)
    {
      auto p=searched.emplace(getParent());
      auto const& notSearched=std::get<1>(p);
      if (not notSearched)
      {
        // a loop in the search path. Stop now.
        return nullopt;
      }

      // local search first
      for (TArg const& arg : searchArgs)
      {
        optional<TRet> returnVal=fnLocalPredicateFirst(this, arg);
        if (returnVal.has_value())
        {
          return returnVal;
        }
      }

      return searchVarExt<TRet, TArg, TFn>(searched, searchArgs, fnLocalPredicate, fnCreateTraverse);
    }

    template<typename TRet, typename TArg, typename TFn>
    optional<TRet> searchVarExt(
            std::unordered_set<SymbolContext*>& searched,
            std::unordered_set<TArg> const& searchArgs,
            TFn const& fnLocalPredicate,
            FnCreateSymbolArgContainer<TArg> const& fnCreateTraverse)
    {
      using travType = std::pair<std::string, std::unordered_set<TArg>>;
      for (travType const travArg : fnCreateTraverse(this, searchArgs))
      {
        std::string const traverseVal = travArg.first;
        std::unordered_set<TArg> const argSet = travArg.second;
        if (traverseVal == getFileName())
        {
          continue;
        }

        if (SymbolContext* ref=getExternalRef(traverseVal))
        {
          optional<TRet> returnValF = ref->_searchVarFull<TRet, TArg, TFn, TFn>(
                  searched, argSet,
                  fnLocalPredicate, fnLocalPredicate,
                  fnCreateTraverse);
          if (returnValF.has_value())
          {
            return returnValF;
          }
        }
      }
      return nullopt;
    }

    // search all var full

    template<typename TRet, typename TArg, typename TFn>
    std::list<TRet> _searchAllVarFull(
            TArg init,
            TFn const& fnLocalPredicate,
            FnCreateSymbolArgContainer<TArg> const& fnCreateTraverse=
                    std::mem_fn(&SymbolContext::defaultCreateTraverse<TArg>))
    {
      std::unordered_set<SymbolContext*> searched;
      std::unordered_set<TArg> initSet { init };
      return _searchAllVarFull<TRet, TArg, TFn, TFn>(
              searched, initSet, fnLocalPredicate, fnLocalPredicate, fnCreateTraverse);
    }

    template<typename TRet, typename TArg, typename TFn, typename TFn2>
    std::list<TRet> _searchAllVarFull(
            std::unordered_set<SymbolContext*>& searched,
            std::unordered_set<TArg> const& searchArgs,
            TFn const& fnLocalPredicate, TFn2 const& fnLocalPredicateFirst,
            FnCreateSymbolArgContainer<TArg> const& fnCreateTraverse)
    {
      auto p=searched.emplace(getParent());
      auto const& notSearched=std::get<1>(p);
      if (not notSearched)
      {
        // a loop in the search path. Stop now.
        return std::list<TRet>();
      }

      std::list<TRet> returnVal;
      // local search first
      for (TArg const& arg : searchArgs)
      {
        returnVal.splice(returnVal.end(), fnLocalPredicateFirst(this, arg));
      }
      returnVal.splice(returnVal.end(), searchAllVarExt<TRet, TArg, TFn>(
              searched, searchArgs, fnLocalPredicate, fnCreateTraverse));
      return returnVal;
    }

    template<typename TRet, typename TArg, typename TFn>
    std::list<TRet> searchAllVarExt(
            std::unordered_set<SymbolContext*>& searched,
            std::unordered_set<TArg> const& searchArgs,
            TFn const& fnLocalPredicate,
            FnCreateSymbolArgContainer<TArg> const& fnCreateTraverse)
    {
      using travType = std::pair<std::string, std::unordered_set<TArg>>;
      std::list<TRet> finalList;
      for (travType const travArg : fnCreateTraverse(this, searchArgs))
      {
        std::string const traverseVal = travArg.first;
        std::unordered_set<TArg> const argSet = travArg.second;
        if (traverseVal == getFileName())
        {
          continue;
        }

        if (SymbolContext* ref=getExternalRef(traverseVal))
        {
          auto returnValF=ref->_searchAllVarFull<TRet, TArg, TFn, TFn>(
                  searched, argSet,
                  fnLocalPredicate, fnLocalPredicate,
                  fnCreateTraverse);

          finalList.splice(finalList.end(), std::move(returnValF));
        }
      }
      return finalList;
    }

    virtual optional<SymbolContext*> searchStructContext(std::string const& tyVal) const;
    SymbolContext* searchStructCtxFull(std::string const&);

    optional<SymbolContext*> searchAccessDecls(std::string const&);
    virtual std::pair<SymbolContext*, bool> searchLitContext(SymbolLit const& symbol);

    virtual std::unordered_set<std::string> createTraverseSet();

    virtual SymbolContext* getExternalRef(std::string const&);
    std::list<std::string> searchFuncUnravelStruct(std::string const& symbol);

    SymbolInfo* searchVarUnravelStructRaw(std::string const& symbol);
    SymbolInfo* searchVarUnravelStructRaw(std::string const& symbol, optional<posInFile> const& position);

    void addPlainFile();
  };

  struct AddDeclContexts: SymbolContext
  {
    unordered_map <std::string, SymbolInfo> additionalDecs;
    AddDeclContexts(): SymbolContext() {}

    explicit AddDeclContexts(posInFile loc):
      SymbolContext(loc) {}

    AddDeclContexts(posInFile loc, SymbolContext* contextParent):
      SymbolContext(loc, contextParent) {}

    ~AddDeclContexts() override = default;
    optional<posRangeInFile> searchVarDecl(std::string const& symbol, optional<posInFile> const& position) override;
    SymbolInfo const* searchVarRaw(std::string const& symbol) const override;
  };
}
