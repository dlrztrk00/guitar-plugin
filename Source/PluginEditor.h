#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Step 3: native JUCE knobs (drive, EQ, level) bound to the parameters.
// The HTML/CSS pedal UI (WebView) comes in a later step.
class SoundBoxEditor : public juce::AudioProcessorEditor
{
public:
    explicit SoundBoxEditor (SoundBoxProcessor&);
    ~SoundBoxEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct KnobUI
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<SliderAttachment> attach;
    };

    void addKnob (KnobUI&, const juce::String& paramId, const juce::String& text);

    SoundBoxProcessor& proc;

    KnobUI drive, low, mid, high, delayMix, delayTime, delayFb, level;

    juce::Label    distLabel;
    juce::ComboBox distBox;
    std::unique_ptr<ComboBoxAttachment> distAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBoxEditor)
};
