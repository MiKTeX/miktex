//========================================================================
//
// CryptoSignBackend.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//========================================================================
#if defined(MIKTEX_WINDOWS)
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include "CryptoSignBackend.h"
#include "config.h"
#ifdef ENABLE_GPGME
#    include "GPGMECryptoSignBackend.h"
#endif
#ifdef ENABLE_NSS3
#    include "NSSCryptoSignBackend.h"
#endif

namespace CryptoSign {

void Factory::setPreferredBackend(CryptoSign::Backend::Type backend)
{
    preferredBackend = backend;
}
static std::string_view toStringView(const char *str)
{
    if (str) {
        return std::string_view(str);
    }
    return {};
}

std::optional<CryptoSign::Backend::Type> Factory::typeFromString(std::string_view string)
{
    if (string.empty()) {
        return std::nullopt;
    }
    if ("GPG" == string) {
        return Backend::Type::GPGME;
    }
    if ("NSS" == string) {
        return Backend::Type::NSS3;
    }
    return std::nullopt;
}

std::optional<CryptoSign::Backend::Type> Factory::getActive()
{
    if (preferredBackend) {
        return *preferredBackend;
    }
    static auto backendFromEnvironment = typeFromString(toStringView(getenv("POPPLER_SIGNATURE_BACKEND")));
    if (backendFromEnvironment) {
        return *backendFromEnvironment;
    }
    static auto backendFromCompiledDefault = typeFromString(toStringView(DEFAULT_SIGNATURE_BACKEND));
    if (backendFromCompiledDefault) {
        return *backendFromCompiledDefault;
    }

    return std::nullopt;
}
static std::vector<Backend::Type> createAvailableBackends()
{
    std::vector<Backend::Type> backends;
#ifdef ENABLE_NSS3
    backends.push_back(Backend::Type::NSS3);
#endif
#ifdef ENABLE_GPGME
    if (GpgSignatureBackend::hasSufficientVersion()) {
        backends.push_back(Backend::Type::GPGME);
    }
#endif
    return backends;
}
std::vector<Backend::Type> Factory::getAvailable()
{
    static std::vector<Backend::Type> availableBackends = createAvailableBackends();
    return availableBackends;
}
std::unique_ptr<Backend> Factory::createActive()
{
    auto active = getActive();
    if (active) {
        return create(active.value());
    }
    return nullptr;
}
std::unique_ptr<CryptoSign::Backend> CryptoSign::Factory::create(Backend::Type backend)
{
    switch (backend) {
    case Backend::Type::NSS3:
#ifdef ENABLE_NSS3
        return std::make_unique<NSSCryptoSignBackend>();
#else
        return nullptr;
#endif
    case Backend::Type::GPGME: {
#ifdef ENABLE_GPGME
        return std::make_unique<GpgSignatureBackend>();
#else
        return nullptr;
#endif
    }
    }
    return nullptr;
}
/// backend specific settings

// Android build wants some methods out of line in the interfaces
Backend::~Backend() = default;
SigningInterface::~SigningInterface() = default;
VerificationInterface::~VerificationInterface() = default;

std::optional<Backend::Type> Factory::preferredBackend = std::nullopt;

} // namespace Signature;
