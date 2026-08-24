#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Step 1: a simple painted editor. The real pedal UI (HTML/CSS via WebView)
// gets wired in a later step.
class SoundBoxEditor : public juce::AudioProcessorEditor
{
public:
    explicit SoundBoxEditor (SoundBoxProcessor&);
    ~SoundBoxEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SoundBoxProcessor& proc;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBoxEditor)
};
