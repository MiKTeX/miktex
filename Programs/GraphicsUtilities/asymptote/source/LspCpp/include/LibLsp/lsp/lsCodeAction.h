#pragma once

#include "LibLsp/JsonRpc/serializer.h"

#include <string>
#include <vector>
#include "lsPosition.h"
#include "lsWorkspaceEdit.h"
#include "lsp_diagnostic.h"
#include "lsCommand.h"

struct CodeAction
{
    /**
         * A short, human-readable, title for this code action.
         */

    std::string title;

    /**
         * The kind of the code action.
         *
         * Used to filter code actions.
         */
    optional<std::string> kind;

    /**
         * The diagnostics that this code action resolves.
         */
    optional<std::vector<lsDiagnostic>> diagnostics;

    /**
         * The workspace edit this code action performs.
         */
    optional<lsWorkspaceEdit> edit;

    /**
         * A command this code action executes. If a code action
         * provides a edit and a command, first the edit is
         * executed and then the command.
         */
    optional<lsCommandWithAny> command;

    MAKE_SWAP_METHOD(CodeAction, title, kind, diagnostics, edit, command)
};
MAKE_REFLECT_STRUCT(CodeAction, title, kind, diagnostics, edit, command)
struct TextDocumentCodeAction

{

    typedef std::pair<optional<lsCommandWithAny>, optional<CodeAction>> Either;
};

extern void Reflect(Reader& visitor, TextDocumentCodeAction::Either& value);
