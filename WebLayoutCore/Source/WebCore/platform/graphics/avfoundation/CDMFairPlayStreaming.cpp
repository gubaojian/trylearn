/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CDMFairPlayStreaming.h"

#if ENABLE(ENCRYPTED_MEDIA)

#include "CDMClearKey.h"
#include "CDMKeySystemConfiguration.h"
#include "CDMRestrictions.h"
#include "CDMSessionType.h"
#include "ISOSchemeInformationBox.h"
#include "ISOSchemeTypeBox.h"
#include "ISOTrackEncryptionBox.h"
#include "InitDataRegistry.h"
#include "NotImplemented.h"
#include <runtime/ArrayBuffer.h>
#include <runtime/DataView.h>
#include <wtf/Algorithms.h>
#include <wtf/JSONValues.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/Base64.h>

#if HAVE(AVCONTENTKEYSESSION)
#include "CDMInstanceFairPlayStreamingAVFObjC.h"
#endif

namespace WebCore {

static Vector<Ref<SharedBuffer>> extractSinfData(const SharedBuffer& buffer)
{
    // JSON of the format: "{ sinf: [ <base64-encoded-string> ] }"
    if (buffer.size() > std::numeric_limits<unsigned>::max())
        return { };
    String json { buffer.data(), static_cast<unsigned>(buffer.size()) };

    RefPtr<JSON::Value> value;
    if (!JSON::Value::parseJSON(json, value))
        return { };

    RefPtr<JSON::Object> object;
    if (!value->asObject(object))
        return { };

    RefPtr<JSON::Array> sinfArray;
    if (!object->getArray("sinf", sinfArray))
        return { };

    Vector<Ref<SharedBuffer>> sinfs;
    sinfs.reserveInitialCapacity(sinfArray->length());

    for (auto& value : *sinfArray) {
        String keyID;
        if (!value->asString(keyID))
            continue;

        Vector<char> sinfData;
        if (!WTF::base64URLDecode(keyID, { sinfData }))
            continue;

        sinfs.uncheckedAppend(SharedBuffer::create(WTFMove(sinfData)));
    }

    return sinfs;
}

using SchemeAndKeyResult = Vector<std::pair<FourCC, Vector<uint8_t>>>;
static SchemeAndKeyResult extractSchemeAndKeyIdFromSinf(const SharedBuffer& buffer)
{
    auto buffers = extractSinfData(buffer);
    if (!buffers.size())
        return { };

    SchemeAndKeyResult result;
    for (auto& buffer : buffers) {
        unsigned offset = 0;
        std::optional<FourCC> scheme;
        std::optional<Vector<uint8_t>> keyID;

        auto view = JSC::DataView::create(buffer->tryCreateArrayBuffer(), offset, buffer->size());
        while (auto optionalBoxType = ISOBox::peekBox(view, offset)) {
            auto& boxTypeName = optionalBoxType.value().first;
            auto& boxSize = optionalBoxType.value().second;

            if (boxTypeName == ISOSchemeTypeBox::boxTypeName()) {
                ISOSchemeTypeBox schemeTypeBox;
                if (!schemeTypeBox.read(view, offset))
                    break;

                scheme = schemeTypeBox.schemeType();
                continue;
            }

            if (boxTypeName == ISOSchemeInformationBox::boxTypeName()) {
                ISOSchemeInformationBox schemeInfoBox;
                if (!schemeInfoBox.read(view, offset))
                    break;

                auto trackEncryptionBox = downcast<ISOTrackEncryptionBox>(schemeInfoBox.schemeSpecificData());
                if (trackEncryptionBox)
                    keyID = trackEncryptionBox->defaultKID();
                continue;
            }

            offset += boxSize;
        }
        if (scheme && keyID)
            result.append(std::make_pair(scheme.value(), WTFMove(keyID.value())));
    }

    return result;
}

static Vector<Ref<SharedBuffer>> extractKeyIDsSinf(const SharedBuffer& buffer)
{
    auto sinfs = extractSinfData(buffer);
    if (sinfs.isEmpty())
        return { };

    Vector<Ref<SharedBuffer>> keyIDs;
    for (auto& sinf : sinfs) {
        auto results = extractSchemeAndKeyIdFromSinf(sinf);
        if (!results.size())
            continue;

        for (auto& result : results) {
            if (result.first == 'cbcs')
                keyIDs.append(SharedBuffer::create(result.second.data(), result.second.size()));
        }
    }

    return keyIDs;
}

static RefPtr<SharedBuffer> sanitizeSinf(const SharedBuffer& buffer)
{
    // Common SINF Box Format
    UNUSED_PARAM(buffer);
    notImplemented();
    return buffer.copy();
}

void CDMFactory::platformRegisterFactories(Vector<CDMFactory*>& factories)
{
    factories.append(&CDMFactoryClearKey::singleton());
    factories.append(&CDMFactoryFairPlayStreaming::singleton());

    InitDataRegistry::shared().registerInitDataType("sinf", { sanitizeSinf, extractKeyIDsSinf });
}

CDMFactoryFairPlayStreaming& CDMFactoryFairPlayStreaming::singleton()
{
    static NeverDestroyed<CDMFactoryFairPlayStreaming> s_factory;
    return s_factory;
}

CDMFactoryFairPlayStreaming::CDMFactoryFairPlayStreaming() = default;
CDMFactoryFairPlayStreaming::~CDMFactoryFairPlayStreaming() = default;

std::unique_ptr<CDMPrivate> CDMFactoryFairPlayStreaming::createCDM(const String& keySystem)
{
    if (!supportsKeySystem(keySystem))
        return nullptr;

    return std::make_unique<CDMPrivateFairPlayStreaming>();
}

bool CDMFactoryFairPlayStreaming::supportsKeySystem(const String& keySystem)
{
    // https://w3c.github.io/encrypted-media/#key-system
    // "Key System strings are compared using case-sensitive matching."
    return keySystem == "com.apple.fps" || keySystem.startsWith(ASCIILiteral("com.apple.fps."));
}

CDMPrivateFairPlayStreaming::CDMPrivateFairPlayStreaming() = default;
CDMPrivateFairPlayStreaming::~CDMPrivateFairPlayStreaming() = default;

bool CDMPrivateFairPlayStreaming::supportsInitDataType(const AtomicString& initDataType) const
{
    return initDataType == "sinf";
}

bool CDMPrivateFairPlayStreaming::supportsConfiguration(const CDMKeySystemConfiguration& configuration) const
{
    if (!configuration.initDataTypes.contains("sinf"))
        return false;

#if HAVE(AVCONTENTKEYSESSION)
    // FIXME: verify that FairPlayStreaming does not (and cannot) expose a distinctive identifier to the client
    if (configuration.distinctiveIdentifier == CDMRequirement::Required)
        return false;
    if (configuration.persistentState == CDMRequirement::Required && !CDMInstanceFairPlayStreamingAVFObjC::supportsPersistableState())
        return false;

    if (configuration.sessionTypes.contains(CDMSessionType::PersistentLicense)
        && !configuration.sessionTypes.contains(CDMSessionType::Temporary)
        && !CDMInstanceFairPlayStreamingAVFObjC::supportsPersistentKeys())
        return false;

    if (!configuration.audioCapabilities.isEmpty()
        && !WTF::anyOf(configuration.audioCapabilities, [](auto& capability) {
            return CDMInstanceFairPlayStreamingAVFObjC::mimeTypeIsPlayable(capability.contentType);
        }))
        return false;

    if (!configuration.videoCapabilities.isEmpty()
        && !WTF::anyOf(configuration.videoCapabilities, [](auto& capability) {
            return CDMInstanceFairPlayStreamingAVFObjC::mimeTypeIsPlayable(capability.contentType);
        }))
        return false;

    return true;
#else
    return false;
#endif    
}

bool CDMPrivateFairPlayStreaming::supportsConfigurationWithRestrictions(const CDMKeySystemConfiguration& configuration, const CDMRestrictions& restrictions) const
{
    if (restrictions.persistentStateDenied
        && !configuration.sessionTypes.isEmpty()
        && !configuration.sessionTypes.contains(CDMSessionType::Temporary))
        return false;

    if (restrictions.persistentStateDenied && configuration.persistentState == CDMRequirement::Required)
        return false;

    if (WTF::allOf(configuration.sessionTypes, [restrictions](auto& sessionType) {
        return restrictions.deniedSessionTypes.contains(sessionType);
    }))
        return false;

    return supportsConfiguration(configuration);
}

bool CDMPrivateFairPlayStreaming::supportsSessionTypeWithConfiguration(CDMSessionType& sessionType, const CDMKeySystemConfiguration& configuration) const
{
    if (sessionType == CDMSessionType::Temporary) {
        if (configuration.persistentState == CDMRequirement::Required)
            return false;
    } else if (configuration.persistentState == CDMRequirement::NotAllowed)
        return false;

    return supportsConfiguration(configuration);
}

bool CDMPrivateFairPlayStreaming::supportsRobustness(const String& robustness) const
{
    if (robustness.isEmpty())
        return true;

    // FIXME: Determine an enumerated list of robustness values supported by FPS.
    return false;
}

CDMRequirement CDMPrivateFairPlayStreaming::distinctiveIdentifiersRequirement(const CDMKeySystemConfiguration&, const CDMRestrictions&) const
{
    // FIXME: verify that FairPlayStreaming does not (and cannot) expose a distinctive identifier to the client
    return CDMRequirement::NotAllowed;
}

CDMRequirement CDMPrivateFairPlayStreaming::persistentStateRequirement(const CDMKeySystemConfiguration&, const CDMRestrictions&) const
{
    return CDMRequirement::Optional;
}

bool CDMPrivateFairPlayStreaming::distinctiveIdentifiersAreUniquePerOriginAndClearable(const CDMKeySystemConfiguration&) const
{
    return true;
}

RefPtr<CDMInstance> CDMPrivateFairPlayStreaming::createInstance()
{
#if HAVE(AVCONTENTKEYSESSION)
    return adoptRef(new CDMInstanceFairPlayStreamingAVFObjC());
#else
    return nullptr;
#endif
}

void CDMPrivateFairPlayStreaming::loadAndInitialize()
{
}

bool CDMPrivateFairPlayStreaming::supportsServerCertificates() const
{
    return true;
}

bool CDMPrivateFairPlayStreaming::supportsSessions() const
{
    return true;
}

bool CDMPrivateFairPlayStreaming::supportsInitData(const AtomicString& initDataType, const SharedBuffer& initData) const
{
    if (!supportsInitDataType(initDataType))
        return false;

    return WTF::anyOf(extractSchemeAndKeyIdFromSinf(initData), [](auto& result) {
        return result.first == 'cbcs' || result.first == 'cbc2' || result.first == 'cbc1';
    });
}

RefPtr<SharedBuffer> CDMPrivateFairPlayStreaming::sanitizeResponse(const SharedBuffer& response) const
{
    return response.copy();
}

std::optional<String> CDMPrivateFairPlayStreaming::sanitizeSessionId(const String& sessionId) const
{
    return sessionId;
}

} // namespace WebCore

#endif // ENABLE(ENCRYPTED_MEDIA)
