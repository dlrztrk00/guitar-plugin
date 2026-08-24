#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Step 2: native JUCE knobs bound to the parameters. The HTML/CSS pedal UI
// (WebView) comes in a later step.
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

    SoundBoxProcessor& proc;

    juce::Slider driveKnob, levelKnob;
    juce::Label  driveLabel, levelLabel, distLabel;
    juce::ComboBox distBox;

    std::unique_ptr<SliderAttachment>   driveAtt, levelAtt;
    std::unique_ptr<ComboBoxAttachment> distAtt;

    void styleKnob (juce::Slider&, juce::Label&, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBoxEditor)
};
