#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

// The pedal UI, rendered from embedded HTML/CSS/JS in a WebView, with each
// control two-way bound to a plugin parameter via JUCE's WebView relays.
class SoundBoxEditor : public juce::AudioProcessorEditor
{
public:
    explicit SoundBoxEditor (SoundBoxProcessor&);
    ~SoundBoxEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url);

    SoundBoxProcessor& proc;

    // one relay per parameter (constructed before the WebView below)
    juce::WebSliderRelay gateRelay { "gate" }, compRelay { "comp" }, driveRelay { "drive" },
        lowRelay { "low" }, midRelay { "mid" }, highRelay { "high" },
        delayMixRelay { "delayMix" }, delayTimeRelay { "delayTime" }, delayFbRelay { "delayFb" },
        chorusRelay { "chorus" }, reverbRelay { "reverb" }, levelRelay { "level" };
    juce::WebComboBoxRelay     distRelay { "distType" };
    juce::WebToggleButtonRelay cabRelay  { "cab" };
    juce::WebToggleButtonRelay compOnRelay { "compOn" }, driveOnRelay { "driveOn" },
        chorusOnRelay { "chorusOn" }, delayOnRelay { "delayOn" };

    juce::WebBrowserComponent webView;

    std::vector<std::unique_ptr<juce::WebSliderParameterAttachment>> sliderAtts;
    std::vector<std::unique_ptr<juce::WebToggleButtonParameterAttachment>> toggleAtts;
    std::unique_ptr<juce::WebComboBoxParameterAttachment> distAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBoxEditor)
};
