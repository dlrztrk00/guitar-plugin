#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// The audio engine. Effects get ported from the web SoundBox one at a time.
// Step 2: distortion (drive + soft/hard/fuzz) with 4x oversampling, plus output level.
class SoundBoxProcessor : public juce::AudioProcessor
{
public:
    SoundBoxProcessor();
    ~SoundBoxProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SoundBox"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // parameters (drive, distortion type, output level) — visible to the editor
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    // 3-band EQ (stereo) — low shelf, mid peak, high shelf
    using Filter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    Filter lowShelf, midPeak, highShelf;
    double fs = 44100.0;

    // delay (echo) with feedback
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 96000 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBoxProcessor)
};
