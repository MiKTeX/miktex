#pragma once
#include <string>
#include <functional>

namespace lsp {
    /// The encoding style of the JSON-RPC messages (both input and output).
    enum JSONStreamStyle {
        /// Encoding per the LSP specification, with mandatory Content-Length header.
        Standard,
        /// Messages are delimited by a '// -----' line. Comment lines start with //.
        Delimited
    };
}
class MessageProducer
{
public:
        typedef  std::function< void(std::string&&) >  MessageConsumer;
        virtual  ~MessageProducer() = default;
        virtual void listen(MessageConsumer) = 0;
};
