#pragma once


#include "LibLsp/JsonRpc/RequestInMessage.h"
#include "LibLsp/JsonRpc/lsResponseMessage.h"
#include "LibLsp/lsp/lsMarkedString.h"
#include "LibLsp/lsp/lsTextDocumentPositionParams.h"

extern  void Reflect(Reader& visitor, 
	std::pair<boost::optional<std::string>, boost::optional<MarkupContent>>& value);



// Represents a parameter of a callable-signature. A parameter can
// have a label and a doc-comment.
struct lsParameterInformation {
  // The label of this parameter. Will be shown in
  // the UI.
  std::string label;

  // The human-readable doc-comment of this parameter. Will be shown
  // in the UI but can be omitted.
  boost::optional< std::pair<  boost::optional<std::string>  , boost::optional <MarkupContent>  > > documentation;

  MAKE_SWAP_METHOD(lsParameterInformation, label, documentation)
};
MAKE_REFLECT_STRUCT(lsParameterInformation, label, documentation);

// Represents the signature of something callable. A signature
// can have a label, like a function-name, a doc-comment, and
// a set of parameters.
struct lsSignatureInformation {
  // The label of this signature. Will be shown in
  // the UI.
  std::string label;

  // The human-readable doc-comment of this signature. Will be shown
  // in the UI but can be omitted.
  boost::optional< std::pair<  boost::optional<std::string>, boost::optional <MarkupContent>  > > documentation;

  // The parameters of this signature.
  std::vector<lsParameterInformation> parameters;

  MAKE_SWAP_METHOD(lsSignatureInformation, label, documentation, parameters)
};
MAKE_REFLECT_STRUCT(lsSignatureInformation, label, documentation, parameters);

// Signature help represents the signature of something
// callable. There can be multiple signature but only one
// active and only one active parameter.
struct lsSignatureHelp {
  // One or more signatures.
  std::vector<lsSignatureInformation> signatures;

  // The active signature. If omitted or the value lies outside the
  // range of `signatures` the value defaults to zero or is ignored if
  // `signatures.length === 0`. Whenever possible implementors should
  // make an active decision about the active signature and shouldn't
  // rely on a default value.
  // In future version of the protocol this property might become
  // mandantory to better express this.
  boost::optional<int> activeSignature;

  // The active parameter of the active signature. If omitted or the value
  // lies outside the range of `signatures[activeSignature].parameters`
  // defaults to 0 if the active signature has parameters. If
  // the active signature has no parameters it is ignored.
  // In future version of the protocol this property might become
  // mandantory to better express the active parameter if the
  // active signature does have any.
  boost::optional<int> activeParameter;


  MAKE_SWAP_METHOD(lsSignatureHelp,
	  signatures,
	  activeSignature,
	  activeParameter)
};
MAKE_REFLECT_STRUCT(lsSignatureHelp,
                    signatures,
                    activeSignature,
                    activeParameter);



/**
 * The signature help request is sent from the client to the server to
 * request signature information at a given cursor position.
 *
 * Registration Options: SignatureHelpRegistrationOptions
 */
DEFINE_REQUEST_RESPONSE_TYPE(td_signatureHelp, lsTextDocumentPositionParams, lsSignatureHelp, "textDocument/signatureHelp");