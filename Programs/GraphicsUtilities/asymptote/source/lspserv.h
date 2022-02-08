#pragma once
#include "common.h"
#include "symbolmaps.h"

#include "LibLsp/lsp/ProtocolJsonHandler.h"
#include "LibLsp/lsp/AbsolutePath.h"

#include "LibLsp/JsonRpc/Endpoint.h"
#include "LibLsp/JsonRpc/TcpServer.h"
#include "LibLsp/JsonRpc/Condition.h"

// header for requests
#include "LibLsp/lsp/textDocument/hover.h"
#include "LibLsp/lsp/general/initialize.h"
#include "LibLsp/lsp/general/shutdown.h"
#include "LibLsp/lsp/textDocument/declaration_definition.h"
#include "LibLsp/lsp/textDocument/colorPresentation.h"

//header for notifs
#include "LibLsp/lsp/general/exit.h"
#include "LibLsp/lsp/general/initialized.h"
#include "LibLsp/lsp/textDocument/did_open.h"
#include "LibLsp/lsp/textDocument/did_change.h"
#include "LibLsp/lsp/textDocument/did_save.h"
#include "LibLsp/lsp/textDocument/did_close.h"

//everything else
#include <functional>
#include <cctype>
#include <unordered_map>
#include "LibLsp/JsonRpc/stream.h"

namespace absyntax
{
  class block;
}

namespace AsymptoteLsp
{
  class istream : public lsp::base_istream<std::istream>
  {
  public:
    istream(std::istream& ist) : lsp::base_istream<std::istream>(ist)
    {
    }

    std::string what() override
    {
      return "AsymptoteLSP_istream";
    }
  };

  class ostream : public lsp::base_ostream<std::ostream>
  {
  public:
    ostream(std::ostream& ost) : lsp::base_ostream<std::ostream>(ost)
    {
    }

    std::string what() override
    {
      return "AsymptoteLSP_ostream";
    }
  };

  template<typename TLeft, typename TRight>
  inline optional<std::pair<optional<TLeft>, optional<TRight>>> opt_left(TLeft const& opt)
  {
    return boost::make_optional(std::make_pair(optional<TLeft>(opt), optional<TRight>()));
  }

  template<typename TLeft, typename TRight>
  inline optional<std::pair<optional<TLeft>, optional<TRight>>> opt_right(TRight const& opt)
  {
    return boost::make_optional(std::make_pair(optional<TLeft>(), optional<TRight>(opt)));
  }

  TextDocumentHover::Either fromString(std::string const &str);
  TextDocumentHover::Either fromMarkedStr(lsMarkedString const& markedString);
  TextDocumentHover::Either fromMarkedStr(std::string const& str, std::string const& language="asymptote");
  TextDocumentHover::Either fromMarkedStr(std::vector<std::string> const& stringList,
                                          std::string const& language="asymptote");

  std::string wslDos2Unix(std::string const& dosPath);
  std::string wslUnix2Dos(std::string const& unixPath);
  std::string getDocIdentifierRawPath(lsTextDocumentIdentifier const&);

  typedef std::unordered_map<std::string, std::unique_ptr<SymbolContext>> SymContextFilemap;

  class LspLog: public lsp::Log
  {
  public:
    void log(Level level, std::string&& msg) override;
    void log(Level level, const std::string& msg) override;
    void log(Level level, std::wstring&& msg) override;
    void log(Level level, const std::wstring& msg) override;
  };


  class AsymptoteLspServer
  {
  public:
    AsymptoteLspServer(shared_ptr<lsp::ProtocolJsonHandler> const& jsonHandler,
                       shared_ptr<GenericEndpoint> const& endpoint, LspLog& log);
    AsymptoteLspServer(RemoteEndPoint* remoteEndPt,
                       shared_ptr<lsp::ProtocolJsonHandler> const& jsonHandler,
                       shared_ptr<GenericEndpoint> const& endpoint, LspLog& log);
    virtual ~AsymptoteLspServer();

    // copy constructors + copy assignment op
    AsymptoteLspServer(AsymptoteLspServer& sv) = delete;
    AsymptoteLspServer& operator=(AsymptoteLspServer const& sv) = delete;

    // move constructors and move assignment op
    AsymptoteLspServer(AsymptoteLspServer&& sv) = delete;
    AsymptoteLspServer& operator=(AsymptoteLspServer&& sv) = delete;

    virtual void start();
    void startIO(std::istream& in=cin, std::ostream& out=cout);

  protected:
    td_hover::response handleHoverRequest(td_hover::request const&);
    virtual td_initialize::response handleInitailizeRequest(td_initialize::request const&);
    virtual td_shutdown::response handleShutdownRequest(td_shutdown::request const&);
    td_definition::response handleDefnRequest(td_definition::request const&);
    td_documentColor::response handleDocColorRequest(td_documentColor::request const&);
    td_colorPresentation::response handleColorPresRequest(td_colorPresentation::request const&);


    virtual void onInitialized(Notify_InitializedNotification::notify& notify);
    virtual void onExit(Notify_Exit::notify& notify);
    void onChange(Notify_TextDocumentDidChange::notify& notify);
    void onOpen(Notify_TextDocumentDidOpen::notify& notify);
    void onSave(Notify_TextDocumentDidSave::notify& notify);
    void onClose(Notify_TextDocumentDidClose::notify& notify);

    void generateMissingTrees(std::string const& inputFile);

    void initializeRequestFn();
    void initializeNotifyFn();

    void reloadFile(std::string const&);
    void updateFileContentsTable(std::string const& filename);
    void updateFileContentsTable(std::string const& filename, std::istream& in);

    // logging functions
    void log(lsp::Log::Level const& level, std::string const& message);
    void logInfo(std::string const& message);
    void logWarning(std::string const& message);
    void logError(std::string const& message);

    SymbolContext* reloadFileRaw(std::string const&, bool const& fillTree=true);
    SymbolContext* fromRawPath(lsTextDocumentIdentifier const& identifier);
    SymbolContext* reloadFileRaw(absyntax::block* blk, std::string const& rawPath, bool const& fillTree=true);
    virtual void clearVariables();
    Condition<bool> serverClosed;

  private:
    unique_ptr<RemoteEndPoint> internalREP;
  protected:
    // [owned, ptr]
    RemoteEndPoint* const remoteEndPoint;

  private:
    shared_ptr<lsp::ProtocolJsonHandler> pjh;
    shared_ptr<GenericEndpoint> ep;
    SymbolContext* plainCtx=NULL;
    LspLog& _log;

    unique_ptr<SymContextFilemap> symmapContextsPtr;
    unique_ptr<unordered_map<std::string, std::vector<std::string>>> fileContentsPtr;
    std::string plainFile;
  };

  class TCPAsymptoteLSPServer : public lsp::TcpServer, public AsymptoteLspServer
  {
  public:
    TCPAsymptoteLSPServer(
            std::string const& addr, std::string const& port,
            shared_ptr<lsp::ProtocolJsonHandler> const& jsonHandler,
            shared_ptr<GenericEndpoint> const& endpoint, LspLog& log);
    ~TCPAsymptoteLSPServer() override;

  protected:
    void start() override;
    Condition<bool> serverInitialized;
  };
}
