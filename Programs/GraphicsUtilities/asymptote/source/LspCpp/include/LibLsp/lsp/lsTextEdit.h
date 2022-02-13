#pragma once

#include "LibLsp/JsonRpc/serializer.h"



#include <string>
#include "lsRange.h"


//Since 3.16.0 there is also the concept of an annotated text edit which supports to add an annotation to a text edit.
//The annotation can add information describing the change to the text edit.
/**
 * Additional information that describes document changes.
 *
 * @since 3.16.0
 */
struct lsChangeAnnotation
{
	/**
	 * A human-readable string describing the actual change. The string
	 * is rendered prominent in the user interface.
	 */
	std::string label;

	/**
	 * A flag which indicates that user confirmation is needed
	 * before applying the change.
	 */
	boost::optional<bool>  needsConfirmation;

	/**
	 * A human-readable string which is rendered less prominent in
	 * the user interface.
	 */
	boost::optional < std::string >  description;
	MAKE_REFLECT_STRUCT(lsChangeAnnotation, label, needsConfirmation, description)
};
MAKE_REFLECT_STRUCT(lsChangeAnnotation, label, needsConfirmation, description)


//Usually clients provide options to group the changes along the annotations they are associated with.
//To support this in the protocol an edit or resource operation refers to a change annotation
//using an identifier and not the change annotation literal directly.This allows servers to use
//the identical annotation across multiple edits or resource operations which then allows clients
//to group the operations under that change annotation.The actual change annotations together with
//their identifers are managed by the workspace edit via the new property changeAnnotations.



/**
 * An identifier referring to a change annotation managed by a workspace
 * edit.
 *
 * @since 3.16.0.
 */
using lsChangeAnnotationIdentifier = std::string;
/**
 * A special text edit with an additional change annotation.
 *
 * @since 3.16.0.
 */


//A textual edit applicable to a text document.
struct lsTextEdit {
	// The range of the text document to be manipulated. To insert
	// text into a document create a range where start === end.
	lsRange range;

	// The string to be inserted. For delete operations use an
	// empty string.
	std::string newText;

	/**
 * The actual annotation identifier.
 */
	boost::optional<lsChangeAnnotationIdentifier>  annotationId;
	

		bool operator==(const lsTextEdit& that);
	std::string ToString() const;
	MAKE_SWAP_METHOD(lsTextEdit, range, newText, annotationId)
};
MAKE_REFLECT_STRUCT(lsTextEdit, range, newText, annotationId)

using  lsAnnotatedTextEdit = lsTextEdit;
