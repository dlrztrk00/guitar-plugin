#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

// The full SoundBox engine, ported from the web version.
// Chain: gate → comp → drive → EQ → cab → chorus → delay → reverb → level → limiter.
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
    double getTailLengthSeconds() const override { return 2.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // pedal chain order (0=comp,1=drive,2=chorus,3=delay), packed 2 bits per slot
    void setPedalOrder (int s0, int s1, int s2, int s3)
    {
        orderPacked.store ((s0 & 3) | ((s1 & 3) << 2) | ((s2 & 3) << 4) | ((s3 & 3) << 6));
    }

    // impulse-response cabinet (real speaker convolution)
    void loadIR (const juce::File& file);
    void clearIR();
    juce::String getIRName() const { return irName; }
    bool hasIR() const { return irLoaded.load(); }

    // output peak level (0..1), read by the editor's meter
    std::atomic<float> outPeak { 0.0f };

    // live spectrum for the amp screen — log-spaced bands, 0..1, read by the editor
    static constexpr int fftOrder = 11;          // 2048-point FFT
    static constexpr int fftSize  = 1 << fftOrder;
    static constexpr int numBands = 32;
    std::array<std::atomic<float>, numBands> spectrum {};

private:
    void pushToAnalyser (float s) noexcept;
    void computeSpectrum() noexcept;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize,
                                                 juce::dsp::WindowingFunction<float>::hann };
    std::array<float, fftSize>     fifo {};
    std::array<float, fftSize * 2> fftData {};
    int fifoIndex = 0;

    using Filter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;

    double fs = 44100.0;
    std::atomic<int> orderPacked { 0 | (1 << 2) | (2 << 4) | (3 << 6) }; // comp,drive,chorus,delay

    juce::dsp::NoiseGate<float> gate;
    juce::dsp::Compressor<float> comp;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler; // for drive
    Filter dcBlock;   // 80 Hz high-pass: removes DC, rumble and mains hum
    Filter toneLP;    // gentle post-drive low-pass to tame fizz
    Filter lowShelf, midPeak, highShelf;                          // EQ
    Filter cabHP, cabLP, cabDip;                                  // cabinet sim
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Chorus<float> flanger;
    juce::dsp::Phaser<float> phaser;
    double tremPhase = 0.0;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 96000 };
    juce::dsp::Reverb reverb;
    juce::dsp::Limiter<float> limiter;

    juce::dsp::Convolution irConv { juce::dsp::Convolution::Latency { 0 } };
    std::atomic<bool> irLoaded { false };
    juce::String irPath, irName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBoxProcessor)
};
