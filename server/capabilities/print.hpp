/**
 * @brief
 * Handle protocol messages
**/

#pragma once

// Common headers
#include "../../core/common/headers.hpp"

namespace Capabilities {
    template<typename MessageType>
    void printMessageMethod() {
        const auto type = lsp::message::IsNotification<MessageType> ? "notification" : "request";
        std::cerr << "Server received " << type << " '" << MessageType::Method << '\'' << std::endl;
    }

    template<typename MessageType>
    void printMessagePayload(const typename MessageType::Params& params) {
        const auto json = lsp::toJson(typename MessageType::Params(params));
        std::cerr << "payload: " << lsp::json::stringify(json, true) << std::endl;
    }

    template<typename MessageType>
    void printMessage(const typename MessageType::Params& params) {
        printMessageMethod<MessageType>();
        printMessagePayload<MessageType>(params);
    }

    template<typename MessageType>
    void printMessage() {
        printMessageMethod<MessageType>();
    }
}