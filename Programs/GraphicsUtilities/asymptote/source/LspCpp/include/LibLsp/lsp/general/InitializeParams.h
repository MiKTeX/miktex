#pragma once

#include "LibLsp/lsp/lsDocumentUri.h"
#include "LibLsp/lsp/lsAny.h"
#include "lsClientCapabilities.h"
#include "LibLsp/lsp/workspace/workspaceFolders.h"

struct ClientInfo {
	std::string name;
	boost::optional<std::string> version;
	
	MAKE_SWAP_METHOD(ClientInfo,name,version);
};
MAKE_REFLECT_STRUCT(ClientInfo,name,version);

struct lsInitializeParams {
  // The process Id of the parent process that started
  // the server. Is null if the process has not been started by another process.
  // If the parent process is not alive then the server should exit (see exit
  // notification) its process.
  boost::optional<int> processId;
	
  /**
   * Information about the client
   *
   * @since 3.15.0
   */
  boost::optional<ClientInfo> clientInfo;
  /**
   * The locale the client is currently showing the user interface
   * in. This must not necessarily be the locale of the operating
   * system.
   *
   * Uses IETF language tags as the value's syntax
   * (See https://en.wikipedia.org/wiki/IETF_language_tag)
   *
   * @since 3.16.0
   */
  boost::optional<std::string> locale;
	
  // The rootPath of the workspace. Is null
  // if no folder is open.
  //
  // @deprecated in favour of rootUri.
  boost::optional<std::string> rootPath;

  // The rootUri of the workspace. Is null if no
  // folder is open. If both `rootPath` and `rootUri` are set
  // `rootUri` wins.
  boost::optional<lsDocumentUri> rootUri;

  // User provided initialization options.
  boost::optional<lsp::Any> initializationOptions;

  // The capabilities provided by the client (editor or tool)
  lsClientCapabilities capabilities;


  /**
 * An boost::optional extension to the protocol.
 * To tell the server what client (editor) is talking to it.
 */
 // @Deprecated
  boost::optional< std::string >clientName;


	
  enum class lsTrace {
    // NOTE: serialized as a string, one of 'off' | 'messages' | 'verbose';
    Off,       // off
    Messages,  // messages
    Verbose    // verbose
  	
  };

  // The initial trace setting. If omitted trace is disabled ('off').
  lsTrace trace = lsTrace::Off;


  /**
 * The workspace folders configured in the client when the server starts.
 * This property is only available if the client supports workspace folders.
 * It can be `null` if the client supports workspace folders but none are
 * configured.
 *
 * Since 3.6.0
 */
  boost::optional< std::vector<WorkspaceFolder> >  workspaceFolders;

  MAKE_SWAP_METHOD(lsInitializeParams,
      processId,
      rootPath,
      rootUri,
      initializationOptions,
      capabilities, clientName, clientInfo,
      trace, workspaceFolders, locale)
};

void Reflect(Reader& reader, lsInitializeParams::lsTrace& value);


void Reflect(Writer& writer, lsInitializeParams::lsTrace& value);


MAKE_REFLECT_STRUCT(lsInitializeParams,
                    processId,
                    rootPath,
                    rootUri,
                    initializationOptions,
                    capabilities, clientName, clientInfo,
                    trace, workspaceFolders, locale)

struct lsInitializeError {
  // Indicates whether the client should retry to send the
  // initilize request after showing the message provided
  // in the ResponseError.
  bool retry;
  void swap(lsInitializeError& arg) noexcept
  {
	  auto tem = retry;
	  retry = arg.retry;
	  arg.retry = tem;
  }
};
MAKE_REFLECT_STRUCT(lsInitializeError, retry);



