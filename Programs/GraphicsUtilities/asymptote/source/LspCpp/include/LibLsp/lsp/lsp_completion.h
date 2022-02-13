#pragma once
#include "lsTextEdit.h"
#include "lsMarkedString.h"
#include "lsCommand.h"


// The kind of a completion entry.
enum class lsCompletionItemKind {
  Text = 1,
  Method = 2,
  Function = 3,
  Constructor = 4,
  Field = 5,
  Variable = 6,
  Class = 7,
  Interface = 8,
  Module = 9,
  Property = 10,
  Unit = 11,
  Value = 12,
  Enum = 13,
  Keyword = 14,
  Snippet = 15,
  Color = 16,
  File = 17,
  Reference = 18,
  Folder = 19,
  EnumMember = 20,
  Constant = 21,
  Struct = 22,
  Event = 23,
  Operator = 24,
  TypeParameter = 25,
};
MAKE_REFLECT_TYPE_PROXY(lsCompletionItemKind);



// Defines whether the insert text in a completion item should be interpreted as
// plain text or a snippet.
enum class lsInsertTextFormat {
  // The primary text to be inserted is treated as a plain string.
  PlainText = 1,

  // The primary text to be inserted is treated as a snippet.
  //
  // A snippet can define tab stops and placeholders with `$1`, `$2`
  // and `${3:foo}`. `$0` defines the final tab stop, it defaults to
  // the end of the snippet. Placeholders with equal identifiers are linked,
  // that is typing in one will update others too.
  //
  // See also:
  // https://github.com/Microsoft/vscode/blob/master/src/vs/editor/contrib/snippet/common/snippet.md
  Snippet = 2
};
MAKE_REFLECT_TYPE_PROXY(lsInsertTextFormat);

namespace lsp
{
	std::string ToString(lsCompletionItemKind);
	std::string ToString(lsInsertTextFormat);
}
/**
 * The Completion request is sent from the client to the server to compute completion items at a given cursor position.
 * Completion items are presented in the IntelliSense user class. If computing complete completion items is expensive
 * servers can additional provide a handler for the resolve completion item request. This request is send when a
 * completion item is selected in the user class.
 */
struct lsCompletionItem {
 
  // The label of this completion item. By default
  // also the text that is inserted when selecting
  // this completion.
  std::string label;

  // The kind of this completion item. Based of the kind
  // an icon is chosen by the editor.
  boost::optional<lsCompletionItemKind>  kind ;

  // A human-readable string with additional information
  // about this item, like type or symbol information.
  boost::optional < std::string > detail;

  // A human-readable string that represents a doc-comment.
  boost::optional< std::pair<boost::optional< std::string> , boost::optional<MarkupContent> > > documentation;


  /**
   * Indicates if this item is deprecated.
   */
  boost::optional< bool >deprecated;
	

   /**
   * Select this item when showing.
   * 
   * *Note* that only one completion item can be selected and that the
   * tool / client decides which item that is. The rule is that the *first
   * item of those that match best is selected.
   */
  boost::optional< bool > preselect;


  // Internal information to order candidates.
  int relevance = 0;

  // A string that shoud be used when comparing this item
  // with other items. When `falsy` the label is used.
  boost::optional< std::string > sortText;

  // A string that should be used when filtering a set of
  // completion items. When `falsy` the label is used.
  boost::optional<std::string> filterText;

  // A string that should be inserted a document when selecting
  // this completion. When `falsy` the label is used.
  boost::optional<std::string> insertText;

  // The format of the insert text. The format applies to both the `insertText`
  // property and the `newText` property of a provided `textEdit`.
  boost::optional< lsInsertTextFormat> insertTextFormat ;

  // An edit which is applied to a document when selecting this completion. When
  // an edit is provided the value of `insertText` is ignored.
  //
  // *Note:* The range of the edit must be a single line range and it must
  // contain the position at which completion has been requested.
  boost::optional<lsTextEdit> textEdit;

  // An boost::optional array of additional text edits that are applied when
  // selecting this completion. Edits must not overlap with the main edit
  // nor with themselves.
  // std::vector<TextEdit> additionalTextEdits;

  // An boost::optional command that is executed *after* inserting this completion.
  // *Note* that additional modifications to the current document should be
  // described with the additionalTextEdits-property. Command command;

  // An data entry field that is preserved on a completion item between
  // a completion and a completion resolve request.
  // data ? : any

  // Use this helper to figure out what content the completion item will insert
  // into the document, as it could live in either |textEdit|, |insertText|, or
  // |label|.
  const std::string& InsertedContent() const;

  std::string DisplayText();
  /**
 * An boost::optional array of additional text edits that are applied when
 * selecting this completion. Edits must not overlap (including the same insert position)
 * with the main edit nor with themselves.
 *
 * Additional text edits should be used to change text unrelated to the current cursor position
 * (for example adding an import statement at the top of the file if the completion item will
 * insert an unqualified type).
 */
  boost::optional<std::vector<lsTextEdit> >additionalTextEdits;

  /**
* An boost::optional set of characters that when pressed while this completion is active will accept it first and
* then type that character. *Note* that all commit characters should have `length=1` and that superfluous
* characters will be ignored.
*/
  boost::optional< std::vector<std::string> > commitCharacters;

  /**
* An boost::optional command that is executed *after* inserting this completion. *Note* that
* additional modifications to the current document should be described with the
* additionalTextEdits-property.
*/
  boost::optional<lsCommandWithAny> command;

  /**
* An data entry field that is preserved on a completion item between a completion and a completion resolve request.
*/
  boost::optional<lsp::Any> data;
  std::string ToString();
  MAKE_SWAP_METHOD(lsCompletionItem,
	  label,
	  kind,
	  detail,
	  documentation,
	  sortText,
	  insertText,
	  filterText,
	  insertTextFormat,
	  textEdit,
	  deprecated, preselect, additionalTextEdits, commitCharacters,
	  command, data);
	
};



MAKE_REFLECT_STRUCT(lsCompletionItem,
                    label,
                    kind,
                    detail,
                    documentation,
                    sortText,
                    insertText,
                    filterText,
                    insertTextFormat,
                    textEdit,
                    deprecated, preselect, additionalTextEdits, commitCharacters,
	command, data);



struct CompletionList {
	// This list it not complete. Further typing should result in recomputing
	// this list.
	bool isIncomplete = false;
	// The completion items.
	std::vector<lsCompletionItem> items;

	void swap(CompletionList& arg) noexcept
	{
		items.swap(arg.items);
		std::swap(isIncomplete, arg.isIncomplete);
	}
};
MAKE_REFLECT_STRUCT(CompletionList, isIncomplete, items);