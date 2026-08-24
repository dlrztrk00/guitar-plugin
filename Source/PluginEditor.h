#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Full native control panel: 12 knobs + distortion selector + cab toggle.
// (The HTML/CSS pedal UI via WebView is a later step.)
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
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct KnobUI
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<SliderAttachment> attach;
    };

    void addKnob (KnobUI&, const juce::String& paramId, const juce::String& text);

    SoundBoxProcessor& proc;

    KnobUI gate, comp, drive, low, mid, high,
           delayMix, delayTime, delayFb, chorus, reverb, level;

    juce::Label    distLabel;
    juce::ComboBox distBox;
    std::unique_ptr<ComboBoxAttachment> distAtt;

    juce::ToggleButton cabButton { "CAB SIM" };
    std::unique_ptr<ButtonAttachment> cabAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBoxEditor)
};
