#pragma once
#include <cctype> 
#include <clocale>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "string_ref.h"

namespace lsp
{
	
/// Holds text and knows how to lay it out. Multiple blocks can be grouped to
/// form a document. Blocks include their own trailing newlines,  std::string_ref
/// should trim them if need be.
class Block {
public:
  virtual void renderMarkdown(std::ostringstream &OS) const = 0;
  virtual void renderPlainText(std::ostringstream &OS) const = 0;
  virtual std::unique_ptr<Block> clone() const = 0;
   std::string_ref asMarkdown() const;
   std::string_ref asPlainText() const;

  virtual bool isRuler() const { return false; }
  virtual ~Block() = default;
};

/// Represents parts of the markup that can contain strings, like inline code,
/// code block or plain text.
/// One must introduce different paragraphs to create separate blocks.
class Paragraph : public Block {
public:
  void renderMarkdown(std::ostringstream &OS) const override;
  void renderPlainText(std::ostringstream &OS) const override;
  std::unique_ptr<Block> clone() const override;

  /// Append plain text to the end of the string.
  Paragraph &appendText(const std::string_ref& Text);

  /// Append inline code, this translates to the ` block in markdown.
  /// \p Preserve indicates the code span must be apparent even in plaintext.
  Paragraph &appendCode(const std::string_ref& Code, bool Preserve = false);

  /// Ensure there is space between the surrounding chunks.
  /// Has no effect at the beginning or end of a paragraph.
  Paragraph &appendSpace();

private:
  struct Chunk {
    enum {
      PlainText,
      InlineCode,
    } Kind = PlainText;
    // Preserve chunk markers in plaintext.
    bool Preserve = false;
     std::string_ref Contents;
    // Whether this chunk should be surrounded by whitespace.
    // Consecutive SpaceAfter and SpaceBefore will be collapsed into one space.
    // Code spans don't usually set this: their spaces belong "inside" the span.
    bool SpaceBefore = false;
    bool SpaceAfter = false;
  };
  std::vector<Chunk> Chunks;
};

/// Represents a sequence of one or more documents. Knows how to print them in a
/// list like format, e.g. by prepending with "- " and indentation.
class BulletList : public Block {
public:
  void renderMarkdown(std::ostringstream &OS) const override;
  void renderPlainText(std::ostringstream &OS) const override;
  std::unique_ptr<Block> clone() const override;

  class Document &addItem();

private:
  std::vector<class Document> Items;
};

/// A format-agnostic representation for structured text. Allows rendering into
/// markdown and plaintext.
class Document {
public:
  Document() = default;
  Document(const Document &Other) { *this = Other; }
  Document &operator=(const Document &);
  Document(Document &&) = default;
  Document &operator=(Document &&) = default;

  void append(Document Other);

  /// Adds a semantical block that will be separate from others.
  Paragraph &addParagraph();
  /// Inserts a horizontal separator to the document.
  void addRuler();
  /// Adds a block of code. This translates to a ``` block in markdown. In plain
  /// text representation, the code block will be surrounded by newlines.
  void addCodeBlock( std::string_ref Code,  std::string_ref Language = "cpp");
  /// Heading is a special type of paragraph that will be prepended with \p
  /// Level many '#'s in markdown.
  Paragraph &addHeading(size_t Level);

  BulletList &addBulletList();

  /// Doesn't contain any trailing newlines.
  /// We try to make the markdown human-readable, e.g. avoid extra escaping.
  /// At least one client (coc.nvim) displays the markdown verbatim!
   std::string_ref asMarkdown() const;
  /// Doesn't contain any trailing newlines.
   std::string_ref asPlainText() const;

private:
  std::vector<std::unique_ptr<Block>> Children;
};


}
