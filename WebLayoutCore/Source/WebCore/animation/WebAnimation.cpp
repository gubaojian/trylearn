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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "WebAnimation.h"

#include "AnimationEffect.h"
#include "AnimationTimeline.h"
#include "Document.h"
#include "KeyframeEffect.h"
#include <wtf/text/WTFString.h>

namespace WebCore {

Ref<WebAnimation> WebAnimation::create(Document& document, AnimationEffect* effect, AnimationTimeline* timeline)
{
    auto result = adoptRef(*new WebAnimation());

    result->setEffect(effect);

    // FIXME: the spec mandates distinguishing between an omitted timeline parameter
    // and an explicit null or undefined value (webkit.org/b/179065).
    result->setTimeline(timeline ? timeline : &document.timeline());

    return result;
}

WebAnimation::WebAnimation()
{
}

WebAnimation::~WebAnimation()
{
    if (m_timeline)
        m_timeline->removeAnimation(*this);
}

void WebAnimation::setEffect(RefPtr<AnimationEffect>&& effect)
{
    if (effect == m_effect)
        return;

    if (m_effect) {
        m_effect->setAnimation(nullptr);

        // Update the Element to Animation map.
        if (m_timeline && m_effect->isKeyframeEffect()) {
            auto* keyframeEffect = downcast<KeyframeEffect>(m_effect.get());
            auto* target = keyframeEffect->target();
            if (target)
                m_timeline->animationWasRemovedFromElement(*this, *target);
        }
    }

    if (effect) {
        // An animation effect can only be associated with a single animation.
        if (effect->animation())
            effect->animation()->setEffect(nullptr);

        effect->setAnimation(this);

        if (m_timeline && effect->isKeyframeEffect()) {
            auto* keyframeEffect = downcast<KeyframeEffect>(effect.get());
            auto* target = keyframeEffect->target();
            if (target)
                m_timeline->animationWasAddedToElement(*this, *target);
        }
    }

    m_effect = WTFMove(effect);
}

void WebAnimation::setTimeline(RefPtr<AnimationTimeline>&& timeline)
{
    if (timeline == m_timeline)
        return;

    // FIXME: If the animation start time of animation is resolved, make animation’s
    // hold time unresolved (webkit.org/b/178932).

    if (m_timeline)
        m_timeline->removeAnimation(*this);

    if (timeline)
        timeline->addAnimation(*this);

    if (m_effect && m_effect->isKeyframeEffect()) {
        auto* keyframeEffect = downcast<KeyframeEffect>(m_effect.get());
        auto* target = keyframeEffect->target();
        if (target) {
            if (m_timeline)
                m_timeline->animationWasRemovedFromElement(*this, *target);
            if (timeline)
                timeline->animationWasAddedToElement(*this, *target);
        }
    }

    m_timeline = WTFMove(timeline);
}
    
std::optional<double> WebAnimation::bindingsStartTime() const
{
    if (m_startTime)
        return m_startTime->milliseconds();
    return std::nullopt;
}

void WebAnimation::setBindingsStartTime(std::optional<double> startTime)
{
    if (startTime == std::nullopt)
        setStartTime(std::nullopt);
    else
        setStartTime(Seconds::fromMilliseconds(startTime.value()));
}

std::optional<Seconds> WebAnimation::startTime() const
{
    return m_startTime;
}

void WebAnimation::setStartTime(std::optional<Seconds> startTime)
{
    if (startTime == m_startTime)
        return;

    m_startTime = startTime;
    
    if (m_timeline)
        m_timeline->animationTimingModelDidChange();
}

std::optional<double> WebAnimation::bindingsCurrentTime() const
{
    auto time = currentTime();
    if (!time)
        return std::nullopt;
    return time->milliseconds();
}

ExceptionOr<void> WebAnimation::setBindingsCurrentTime(std::optional<double> currentTime)
{
    if (!currentTime)
        return Exception { TypeError };
    setCurrentTime(Seconds::fromMilliseconds(currentTime.value()));
    return { };
}

std::optional<Seconds> WebAnimation::currentTime() const
{
    // FIXME: return the hold time when we support pausing (webkit.org/b/178932).

    if (!m_timeline || !m_startTime)
        return std::nullopt;

    auto timelineTime = m_timeline->currentTime();
    if (!timelineTime)
        return std::nullopt;

    return (timelineTime.value() - m_startTime.value()) * m_playbackRate;
}

void WebAnimation::setCurrentTime(std::optional<Seconds> seekTime)
{
    // FIXME: account for hold time when we support it (webkit.org/b/178932),
    // including situations where playbackRate is 0.

    if (!m_timeline) {
        setStartTime(std::nullopt);
        return;
    }

    auto timelineTime = m_timeline->currentTime();
    if (!timelineTime) {
        setStartTime(std::nullopt);
        return;
    }

    setStartTime(timelineTime.value() - (seekTime.value() / m_playbackRate));
}

void WebAnimation::setPlaybackRate(double newPlaybackRate)
{
    if (m_playbackRate == newPlaybackRate)
        return;

    // 3.5.17.1. Updating the playback rate of an animation
    // Changes to the playback rate trigger a compensatory seek so that that the animation's current time
    // is unaffected by the change to the playback rate.
    auto previousTime = currentTime();
    m_playbackRate = newPlaybackRate;
    if (previousTime)
        setCurrentTime(previousTime);
}

Seconds WebAnimation::timeToNextRequiredTick(Seconds timelineTime) const
{
    if (!m_timeline || !m_startTime || !m_effect || !m_playbackRate)
        return Seconds::infinity();

    auto startTime = m_startTime.value();
    auto endTime = startTime + (m_effect->timing()->duration() / m_playbackRate);

    // If we haven't started yet, return the interval until our active start time.
    auto activeStartTime = std::min(startTime, endTime);
    if (timelineTime <= activeStartTime)
        return activeStartTime - timelineTime;

    // If we're in the middle of our active duration, we want to be called as soon as possible.
    auto activeEndTime = std::max(startTime, endTime);
    if (timelineTime <= activeEndTime)
        return 0_ms;

    // If none of the previous cases match, then we're already past our active duration
    // and do not need scheduling.
    return Seconds::infinity();
}

void WebAnimation::resolve(RenderStyle& targetStyle)
{
    if (m_effect && currentTime())
        m_effect->applyAtLocalTime(currentTime().value(), targetStyle);
}

void WebAnimation::acceleratedRunningStateDidChange()
{
    if (m_timeline && m_timeline->isDocumentTimeline())
        downcast<DocumentTimeline>(*m_timeline).animationAcceleratedRunningStateDidChange(*this);
}

void WebAnimation::startOrStopAccelerated()
{
    if (m_effect && m_effect->isKeyframeEffect())
        downcast<KeyframeEffect>(*m_effect).startOrStopAccelerated();
}

String WebAnimation::description()
{
    return "Animation";
}

} // namespace WebCore
