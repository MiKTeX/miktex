//
// Created by Supakorn on 5/13/2021.
//

#include "common.h"

#ifdef HAVE_LSP

#include "lspserv.h"

#include <LibLsp/JsonRpc/stream.h>

#include <string>
#include <cstdlib>
#include <vector>
#include <memory>
#include <queue>

#include <thread>

#include "dec.h"
#include "process.h"
#include "locate.h"

#define GC_THREADS

#include "gc.h"

#define REGISTER_REQ_FN(typ, fn) remoteEndPoint->registerHandler(\
  [this](typ::request const& req) { return this->fn(req); });
#define REGISTER_NOTIF_FN(typ, handler) remoteEndPoint->registerHandler(\
  [this](typ::notify& notif) { this->handler(notif); });

namespace AsymptoteLsp
{
  using std::unique_ptr;
  using std::shared_ptr;
  using absyntax::block;
  using Level=lsp::Log::Level;

  class SearchPathAddition
  {
  public:
    SearchPathAddition(mem::string const& dir)
    {
      settings::searchPath.push_back(dir);
    }

    SearchPathAddition(SearchPathAddition const&) = delete;
    SearchPathAddition& operator=(SearchPathAddition const&) = delete;

    SearchPathAddition(SearchPathAddition&&) = delete;
    SearchPathAddition& operator=(SearchPathAddition&&) = delete;

    ~SearchPathAddition()
    {
      settings::searchPath.pop_back();
    }
  };

std::string wslDos2Unix(std::string const& dosPath)
  {
    bool isDrivePath=false;
    char drive;
    if (dosPath.length() >= 3)
    {
      if (dosPath[0] == '/' and dosPath[2] == ':')
      {
        isDrivePath=true;
        drive=dosPath[1];
      }
    }

    if (isDrivePath)
    {
      std::stringstream sstream;
      sstream << "/mnt/" << (char) tolower(drive) << dosPath.substr(3);
      return sstream.str();
    } else
    {
      return std::string(dosPath);
    }
  }

std::string wslUnix2Dos(std::string const& unixPath)
  {
    bool isMntPath=false;
    char drive;

#ifdef __GNU__
#define PATH_MAX 4096
#endif
    char actPath[PATH_MAX];
    if(!realpath(unixPath.c_str(), actPath))
      return "";
    std::string fullPath(actPath);

    if (fullPath.length() >= 7) // /mnt/
    {
      if (fullPath.find("/mnt/") == 0)
      {
        isMntPath=true;
        drive=fullPath[5];
      }
    }

    if (isMntPath)
    {
      std::stringstream sstream;
      sstream << "/" << (char) tolower(drive) << ":" << fullPath.substr(6);
      return sstream.str();
    } else
    {
      return std::string(fullPath);
    }
  }

  TextDocumentHover::Either fromString(std::string const& str)
  {
    auto strobj=std::make_pair(boost::make_optional(str), optional<lsMarkedString>());
    std::vector<decltype(strobj)> vec{strobj};
    return std::make_pair(vec, nullopt);
  }

  TextDocumentHover::Either fromMarkedStr(lsMarkedString const& markedString)
  {
    auto strobj=std::make_pair((optional<std::string>) nullopt, make_optional(markedString));
    std::vector<decltype(strobj)> vec{strobj};
    return std::make_pair(vec, nullopt);
  }

  TextDocumentHover::Either fromMarkedStr(std::vector<std::string> const& stringList, std::string const& language)
  {
    std::vector<std::pair<optional<std::string>, optional<lsMarkedString>>> vec;
    std::transform(stringList.begin(), stringList.end(), std::back_inserter(vec),
                   [&language](std::string const& str)
                   {
                     lsMarkedString lms;
                     lms.language=language;
                     lms.value=str;
                     return std::make_pair((optional<std::string>) nullopt, make_optional(lms));
                   });

    return std::make_pair(vec, nullopt);
  }

  TextDocumentHover::Either fromMarkedStr(std::string const& str, std::string const& language)
  {
    lsMarkedString lms;
    lms.language=language;
    lms.value=str;
    return fromMarkedStr(lms);
  }

  std::string getDocIdentifierRawPath(lsTextDocumentIdentifier const& textDocIdentifier)
  {
    lsDocumentUri fileUri(textDocIdentifier.uri);
    std::string rawPath=settings::getSetting<bool>("wsl") ?
      wslDos2Unix(fileUri.GetRawPath()) : std::string(fileUri.GetRawPath());
    return static_cast<std::string>(rawPath);
  }

  void AsymptoteLspServer::generateMissingTrees(std::string const& inputFile)
  {
    using extRefMap=std::unordered_map<std::string, SymbolContext*>;
    //using extRefMapLoc = std::pair<extRefMap*, std::string>;
    std::queue<extRefMap::iterator> procList;
    std::unordered_set<std::string> processing;

    processing.emplace(inputFile);
    SymbolContext* ctx=symmapContextsPtr->at(inputFile).get();

    for (auto const& locPair : ctx->getEmptyRefs())
    {
      procList.emplace(locPair);
    }

    // standard BFS algorithm
    while (not procList.empty())
    {
      auto it=procList.front();
      procList.pop();

      std::string filename(it->first);
      processing.emplace(filename);

      auto mapIt=symmapContextsPtr->find(filename);
      if (mapIt != symmapContextsPtr->end())
      {
        it->second=mapIt->second.get();
      }
      else
      {
        block* blk=ifile(mem::string(filename.c_str())).getTree();
        auto s=symmapContextsPtr->emplace(
                filename,
                make_unique<SymbolContext>(posInFile(1, 1), filename));
        auto fit=std::get<0>(s);

        if(blk == nullptr)
        {
          // dead end. file cannot be parsed. no new paths.
          continue;
        }

        blk->createSymMap(fit->second.get()); // parse symbol from there.
        // set plain.asy to plain
        if (plainCtx != nullptr)
        {
          fit->second->extRefs.extFileRefs[plainFile]=plainCtx;
        }

        // also parse its neighbors
        for (auto const& sit : fit->second->getEmptyRefs())
        {
          if (processing.find(sit->first) == processing.end())
          {
            procList.emplace(sit);
          }
          else
          {
            // import cycles detected!
            logWarning("Import cycles detected!");
          }
        }
        it->second=fit->second.get();
      }
    }
  }

  void LspLog::log(Level level, std::string&& msg)
  {
    if ((uint32_t)Level::WARNING + settings::verbose >= (uint32_t)level || level == Level::ALL)
    {
      cerr << msg << std::endl;
    }
  }

  void LspLog::log(Level level, std::wstring&& msg)
  {
    if ((uint32_t)Level::WARNING + settings::verbose >= (uint32_t)level || level == Level::ALL)
    {
      std::wcerr << msg << std::endl;
    }
  }

  void LspLog::log(Level level, const std::string& msg)
  {
    if ((uint32_t)Level::WARNING + settings::verbose >= (uint32_t)level || level == Level::ALL)
    {
      cerr << msg << std::endl;
    }
  }

  void LspLog::log(Level level, const std::wstring& msg)
  {
    if ((uint32_t)Level::WARNING + settings::verbose >= (uint32_t)level || level == Level::ALL)
    {
      std::wcerr << msg << std::endl;
    }
  }

  AsymptoteLspServer::AsymptoteLspServer(
          shared_ptr<lsp::ProtocolJsonHandler> const& jsonHandler,
          shared_ptr<GenericEndpoint> const& endpoint, LspLog& log) :
          internalREP(make_unique<RemoteEndPoint>(jsonHandler, endpoint, log)), remoteEndPoint(internalREP.get()),
          pjh(jsonHandler), ep(endpoint), _log(log)
  {
    initializeRequestFn();
    initializeNotifyFn();
  }

  AsymptoteLspServer::AsymptoteLspServer(
          RemoteEndPoint* remoteEndPt,
          shared_ptr<lsp::ProtocolJsonHandler> const& jsonHandler,
          shared_ptr<GenericEndpoint> const& endpoint, LspLog& log) :
          internalREP(nullptr), remoteEndPoint(remoteEndPt),
          pjh(jsonHandler), ep(endpoint), _log(log)
  {
    initializeRequestFn();
    initializeNotifyFn();
  }

  void AsymptoteLspServer::initializeRequestFn()
  {
    REGISTER_REQ_FN(td_initialize, handleInitailizeRequest);
    REGISTER_REQ_FN(td_hover, handleHoverRequest);
    REGISTER_REQ_FN(td_shutdown, handleShutdownRequest);
    REGISTER_REQ_FN(td_definition, handleDefnRequest);
    REGISTER_REQ_FN(td_documentColor, handleDocColorRequest);
    REGISTER_REQ_FN(td_colorPresentation, handleColorPresRequest);
  }

  void AsymptoteLspServer::initializeNotifyFn()
  {
    REGISTER_NOTIF_FN(Notify_InitializedNotification, onInitialized);
    REGISTER_NOTIF_FN(Notify_TextDocumentDidChange, onChange);
    REGISTER_NOTIF_FN(Notify_TextDocumentDidOpen, onOpen);
    REGISTER_NOTIF_FN(Notify_TextDocumentDidSave, onSave);
    REGISTER_NOTIF_FN(Notify_TextDocumentDidClose, onClose);
    REGISTER_NOTIF_FN(Notify_Exit, onExit);
  }

//#pragma region notifications

  void AsymptoteLspServer::onInitialized(Notify_InitializedNotification::notify& notify)
  {
    logInfo("server initialized notification");
  }

  void AsymptoteLspServer::onExit(Notify_Exit::notify& notify)
  {
    logInfo("server exit notification");
    serverClosed.notify(make_unique<bool>(true));
  }

  void AsymptoteLspServer::onChange(Notify_TextDocumentDidChange::notify& notify)
  {
    logInfo("text change notification");

    auto& fileChange = notify.params.contentChanges;
    if (not fileChange.empty())
    {
      bool updatable = true;
      block* codeBlk;
      try
      {
        codeBlk=istring(mem::string(fileChange[0].text.c_str())).getTree();
      }
      catch (handled_error const&)
      {
        updatable = false;
      }

      if (updatable)
      {
        std::string rawPath=getDocIdentifierRawPath(notify.params.textDocument.AsTextDocumentIdentifier());
        std::istringstream iss(fileChange[0].text);
        updateFileContentsTable(rawPath, iss);
        reloadFileRaw(codeBlk, rawPath);
      }
    }

    logInfo("changed text data");
  }

  void AsymptoteLspServer::onOpen(Notify_TextDocumentDidOpen::notify& notify)
  {
    logInfo("onOpen notification");
    lsDocumentUri fileUri(notify.params.textDocument.uri);
    reloadFile(fileUri.GetRawPath());
  }

  void AsymptoteLspServer::onSave(Notify_TextDocumentDidSave::notify& notify)
  {
    logInfo("onSave notification");
//    lsDocumentUri fileUri(notify.params.textDocument.uri);
//    reloadFile(fileUri.GetRawPath());
  }

  void AsymptoteLspServer::onClose(Notify_TextDocumentDidClose::notify& notify)
  {
    logInfo("onClose notification");
  }

//#pragma endregion

//#pragma region requests
  td_initialize::response AsymptoteLspServer::handleInitailizeRequest(td_initialize::request const& req)
  {
    clearVariables();

    symmapContextsPtr=make_unique<SymContextFilemap>();
    fileContentsPtr=make_unique<
            std::remove_reference<decltype(*fileContentsPtr)>::type>();
    plainFile=settings::locateFile("plain", true).c_str();
    plainCtx=reloadFileRaw(plainFile, false);
    generateMissingTrees(plainFile);

    td_initialize::response rsp;
    rsp.id=req.id;
    rsp.result.capabilities.hoverProvider=true;

    lsTextDocumentSyncOptions tdso;
    tdso.openClose=true;
    tdso.change=lsTextDocumentSyncKind::Full;
    lsSaveOptions so;
    so.includeText=true;
    tdso.save=so;
    rsp.result.capabilities.textDocumentSync=opt_right<lsTextDocumentSyncKind>(tdso);
    rsp.result.capabilities.definitionProvider=std::make_pair(true, nullopt);
    rsp.result.capabilities.colorProvider=std::make_pair(true, nullopt);
    return rsp;
  }

  SymbolContext* AsymptoteLspServer::fromRawPath(lsTextDocumentIdentifier const& identifier)
  {
    std::string rawPath=getDocIdentifierRawPath(identifier);
    auto fileSymIt=symmapContextsPtr->find(rawPath);

    return fileSymIt != symmapContextsPtr->end() ? fileSymIt->second.get() : nullptr;
  }

  td_hover::response AsymptoteLspServer::handleHoverRequest(td_hover::request const& req)
  {
    td_hover::response rsp;
    SymbolContext* fileSymPtr=fromRawPath(req.params.textDocument);
    std::vector<std::pair<optional<std::string>, optional<lsMarkedString>>> nullVec;

    if (!fileSymPtr)
    {
      rsp.result.contents.first=nullVec;
      return rsp;
    }

      auto s=fileSymPtr->searchSymbol(fromLsPosition(req.params.position));
      auto st=std::get<0>(s);
      auto ctx=std::get<1>(s);

    if (not st.has_value())
    {
      rsp.result.contents.first=nullVec;
      return rsp;
    }

    auto v=st.value();
    auto symText=std::get<0>(v);
    auto startPos=std::get<1>(v);
    auto endPos=std::get<2>(v);

    rsp.result.range=make_optional(lsRange(toLsPosition(startPos), toLsPosition(endPos)));

    auto typ=ctx->searchLitSignature(symText);
    std::list<std::string> endResultList;
    if (typ.has_value())
    {
      endResultList.push_back(typ.value());
    }
    endResultList.splice(endResultList.end(), ctx->searchLitFuncSignature(symText));

    std::vector<std::string> endResult;
    std::copy(endResultList.begin(), endResultList.end(), std::back_inserter(endResult));

    rsp.result.contents=endResult.empty() ?
                        fromMarkedStr("<decl-unknown> " + symText.name + ";") :
                        fromMarkedStr(endResult);
    return rsp;
  }

  td_documentColor::response AsymptoteLspServer::handleDocColorRequest(td_documentColor::request const& req)
  {
    td_documentColor::response rsp;

    if (SymbolContext* fileSymPtr=fromRawPath(req.params.textDocument))
    {
      logInfo("Got Document color request.");
      auto& colorsInfo = fileSymPtr->colorInformation;
      for (auto const& colorPtr : colorsInfo)
      {
        ColorInformation cif;

        cif.color = static_cast<TextDocument::Color>(*colorPtr);
        cif.range.start=toLsPosition(colorPtr->rangeBegin);

        auto s=colorPtr->lastArgPosition;
        auto& line=std::get<0>(s);
        auto& colm=std::get<1>(s);
        size_t offset = 0;
        size_t lineOffset = 0;

        auto& strLines = fileContentsPtr->at(getDocIdentifierRawPath(req.params.textDocument));
        char ch=strLines[line + lineOffset + 1][colm - 1 + offset];

        while (
                ch != ')' and ch != ';'
                and line + lineOffset <= strLines.size()
                )
        {
          ++offset;
          if (offset > strLines[line+lineOffset-1].size())
          {
            ++lineOffset;
            offset = 0;
          }
          if (line+lineOffset <= strLines.size())
          {
            ch=strLines[line + lineOffset - 1][colm - 1 + offset];
          }
        }

        if (ch != ')' or line + lineOffset > strLines.size())
        {
          continue;
        }

        cif.range.end=toLsPosition(make_pair(line+lineOffset, colm+offset+1));
        rsp.result.emplace_back(cif);
      }
    }

    return rsp;
  }

  td_colorPresentation::response AsymptoteLspServer::handleColorPresRequest(td_colorPresentation::request const& req)
  {
    td_colorPresentation::response rsp;

    if (SymbolContext* fileSymPtr=fromRawPath(req.params.textDocument))
    {
      logInfo("Got color presentation request.");
      ColorPresentation clp;

      for (auto& colPtr : fileSymPtr->colorInformation)
      {
        auto& incomingColor = req.params.color;
        std::ostringstream ssargs;
        std::ostringstream labelargs;

        bool opaque=std::fabs(incomingColor.alpha - 1) < std::numeric_limits<double>::epsilon();
        std::string fnName = opaque ? "rgb" : "rgba";

        labelargs << std::setprecision(3) << incomingColor.red << "," << incomingColor.green << "," <<
                                          incomingColor.blue;
        ssargs << incomingColor.red << "," << incomingColor.green << "," <<
           incomingColor.blue;
        if (!opaque)
        {
          ssargs << "," << incomingColor.alpha;
          labelargs << "," << incomingColor.alpha;
        }
        std::ostringstream ss;
        ss << fnName << "(" << ssargs.str() << ")";
        clp.textEdit.newText = ss.str();

        std::ostringstream lss;
        lss << fnName << "(" << labelargs.str() << ")";
        clp.label = lss.str();

        if (colPtr->rangeBegin == fromLsPosition(req.params.range.start))
        {
          clp.textEdit.range = req.params.range;
          rsp.result.emplace_back(std::move(clp));
          break;
        }
      }
    }
    return rsp;
  }

  td_shutdown::response AsymptoteLspServer::handleShutdownRequest(td_shutdown::request const& req)
  {
    logInfo("got shut down request");
    td_shutdown::response rsp;
    JsonNull nullrsp;
    lsp::Any anyrsp;
    anyrsp.Set(nullrsp);
    rsp.result = make_optional<lsp::Any>(std::move(anyrsp));

    serverClosed.notify(make_unique<bool>(true));
    return rsp;
  }

  td_definition::response AsymptoteLspServer::handleDefnRequest(td_definition::request const& req)
  {
    td_definition::response rsp;
    std::list<posRangeInFile> posRanges;
    if (SymbolContext* fileSymPtr=fromRawPath(req.params.textDocument))
    {
      posInFile pos = fromLsPosition(req.params.position);
      auto s=fileSymPtr->searchSymbol(pos);
      auto st=std::get<0>(s);
      auto ctx=std::get<1>(s);
      if (st.has_value())
      {
        optional<posRangeInFile> posRange=ctx->searchLitPosition(std::get<0>(st.value()), pos);
        if (posRange.has_value())
        {
          posRanges.push_back(posRange.value());
        }

        posRanges.splice(posRanges.begin(), ctx->searchLitFuncPositions(std::get<0>(st.value()), pos));
      }
    }
    rsp.result.first=boost::make_optional(std::vector<lsLocation>());
    std::transform(
            posRanges.begin(), posRanges.end(), std::back_inserter(rsp.result.first.value()),
            [](posRangeInFile const& posRange)
            {
              auto& fil=std::get<0>(posRange);
              auto& posBegin=std::get<1>(posRange);
              auto& posEnd=std::get<2>(posRange);
              lsRange rng(toLsPosition(posBegin), toLsPosition(posEnd));

              std::string filReturn(
                      settings::getSetting<bool>("wsl") ? static_cast<std::string>(wslUnix2Dos(fil)) : fil);

              lsDocumentUri uri(filReturn);
              return lsLocation(uri, rng);
            });
    return rsp;
  }

//#pragma endregion
  void AsymptoteLspServer::reloadFile(std::string const& filename)
  {
    std::string rawPath=settings::getSetting<bool>("wsl") ? wslDos2Unix(filename) : std::string(filename);
    reloadFileRaw(static_cast<std::string>(rawPath));
  }

  void AsymptoteLspServer::updateFileContentsTable(std::string const& filename)
  {
    std::ifstream fil(filename, std::ifstream::in);
    return updateFileContentsTable(filename, fil);
  }

  void AsymptoteLspServer::updateFileContentsTable(std::string const& filename, std::istream& in)
  {
    auto& fileContents = *fileContentsPtr;
    fileContents[filename].clear();

    std::string line;
    while (std::getline(in, line))
    {
      fileContents[filename].emplace_back(line);
    }
  }

  SymbolContext* AsymptoteLspServer::reloadFileRaw(block* blk, std::string const& rawPath, bool const& fillTree)
  {
    if (blk != nullptr)
    {
      SearchPathAddition sp(stripFile(string(rawPath.c_str())));
      auto it=symmapContextsPtr->find(rawPath);
      if (it != symmapContextsPtr->end())
      {
        *(it->second)=SymbolContext(posInFile(1, 1), rawPath);
      }
      else
      {
        auto s = symmapContextsPtr->emplace(
                rawPath, make_unique<SymbolContext>(posInFile(1, 1), rawPath));
        it=std::get<0>(s);
      }

      SymbolContext* newPtr=it->second.get();

      cerr << rawPath << endl;

      blk->createSymMap(newPtr);

      if (plainCtx != nullptr)
      {
        it->second->extRefs.extFileRefs[plainFile]=plainCtx;
      }
      else if (rawPath == plainFile)
      {
        it->second->extRefs.extFileRefs[plainFile]=newPtr;
      }

      if (fillTree)
      {
        generateMissingTrees(rawPath);
      }
      return it->second.get();
    }
    else
    {
      return nullptr;
    }
  }

  SymbolContext* AsymptoteLspServer::reloadFileRaw(std::string const& rawPath, bool const& fillTree)
  {
    updateFileContentsTable(rawPath);
    block* blk=ifile(mem::string(rawPath.c_str())).getTree();
    return reloadFileRaw(blk, rawPath, fillTree);
  }

  void AsymptoteLspServer::start()
  {
    return startIO(cin, cout);
  }

  AsymptoteLspServer::~AsymptoteLspServer()
  {
  }

  void AsymptoteLspServer::startIO(std::istream& in, std::ostream& out)
  {
    auto inPtr=make_shared<AsymptoteLsp::istream>(in);
    auto outPtr=make_shared<AsymptoteLsp::ostream>(out);
    remoteEndPoint->startProcessingMessages(inPtr,outPtr);
    serverClosed.wait();
  }

  void AsymptoteLspServer::log(lsp::Log::Level const& level, std::string const& message)
  {
    _log.log(level, message);
  }

  void AsymptoteLspServer::logError(std::string const& message)
  {
    log(lsp::Log::Level::SEVERE, message);
  }

  void AsymptoteLspServer::logWarning(std::string const& message)
  {
    log(lsp::Log::Level::WARNING, message);
  }

  void AsymptoteLspServer::logInfo(std::string const& message)
  {
    log(lsp::Log::Level::INFO, message);
  }

  void AsymptoteLspServer::clearVariables()
  {
  }

  // TCP Asymptote Server

  TCPAsymptoteLSPServer::TCPAsymptoteLSPServer(
          std::string const& addr, std::string const& port, shared_ptr<lsp::ProtocolJsonHandler> const& jsonHandler,
          shared_ptr<GenericEndpoint> const& endpoint, LspLog& log) :
          lsp::TcpServer(addr, port, jsonHandler, endpoint, log),
          AsymptoteLspServer(&point, jsonHandler, endpoint, log)
  {
  }

  TCPAsymptoteLSPServer::~TCPAsymptoteLSPServer()
  {
    logInfo("Destroying server...");
    this->stop();
  }


  void TCPAsymptoteLSPServer::start()
  {
    std::thread([this]() {this->run();}).detach();
    serverClosed.wait();
    logInfo("Got server closed notification.");
  }
}

#endif
