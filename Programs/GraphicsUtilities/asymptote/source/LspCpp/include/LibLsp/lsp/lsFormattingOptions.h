#pragma once

#include "LibLsp/JsonRpc/serializer.h"

struct lsFormattingOptions {
        struct KeyData {
                optional<bool> _boolean;
                optional<int32_t> _integer;
                optional<std::string> _string;
        };

        // Size of a tab in spaces.
        int tabSize =4;
        // Prefer spaces over tabs.
        bool insertSpaces = true;

        /**
                 * Trim trailing whitespace on a line.
                 *
                 * @since 3.15.0
                 */
        optional<bool> trimTrailingWhitespace;

        /**
         * Insert a newline character at the end of the file if one does not exist.
         *
         * @since 3.15.0
         */
        optional<bool> insertFinalNewline;

        /**
         * Trim all newlines after the final newline at the end of the file.
         *
         * @since 3.15.0
         */
        optional<bool> trimFinalNewlines;
        optional<KeyData> key;
    MAKE_SWAP_METHOD(lsFormattingOptions, tabSize, insertSpaces, trimTrailingWhitespace, insertFinalNewline, trimFinalNewlines, key)
};
MAKE_REFLECT_STRUCT(lsFormattingOptions, tabSize, insertSpaces, trimTrailingWhitespace, insertFinalNewline, trimFinalNewlines, key);

extern void Reflect(Reader& visitor, lsFormattingOptions::KeyData& value);
extern  void Reflect(Writer& visitor, lsFormattingOptions::KeyData& value);
