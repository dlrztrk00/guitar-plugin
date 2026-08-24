#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

// The same three curves as the web SoundBox:
//   soft = round tanh clip, hard = square-ish clip, fuzz = asymmetric high-gain.
static inline float shapeSample (float x, float drive, int type)
{
    if (type == 1) // hard
    {
        const float g = 1.0f + drive * 30.0f;
        return juce::jlimit (-1.0f, 1.0f, g * x);
    }
    if (type == 2) // fuzz
    {
        const float gp = 1.0f + drive * 90.0f;
        const float gn = 1.0f + drive * 45.0f;
        return x >= 0.0f ? std::tanh (gp * x) : std::tanh (gn * x);
    }
    // soft
    const float k = 1.0f + drive * 40.0f;
    return std::tanh (k * x) / std::tanh (k);
}

juce::AudioProcessorValueTreeState::ParameterLayout
SoundBoxProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "drive", 1 }, "Drive",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { "distType", 1 }, "Distortion",
        StringArray { "Soft", "Hard", "Fuzz" }, 0));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { "level", 1 }, "Level",
        NormalisableRange<float> (0.0f, 1.5f, 0.001f), 1.0f));

    return layout;
}

SoundBoxProcessor::SoundBoxProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

void SoundBoxProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate);

    const auto channels = (size_t) juce::jmax (1, getTotalNumInputChannels());
    // 4x oversampling (factor 2 = 2^2) — tames the harsh aliasing distortion makes
    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        channels, 2,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
    oversampler->initProcessing ((size_t) samplesPerBlock);
    setLatencySamples ((int) oversampler->getLatencyInSamples());
}

bool SoundBoxProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainIn == mainOut;
}

void SoundBoxProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float drive = apvts.getRawParameterValue ("drive")->load();
    const int   type  = (int) apvts.getRawParameterValue ("distType")->load();
    const float level = apvts.getRawParameterValue ("level")->load();

    juce::dsp::AudioBlock<float> block (buffer);

    if (oversampler != nullptr)
    {
        auto osBlock = oversampler->processSamplesUp (block);
        for (size_t ch = 0; ch < osBlock.getNumChannels(); ++ch)
        {
            auto* data = osBlock.getChannelPointer (ch);
            for (size_t i = 0; i < osBlock.getNumSamples(); ++i)
                data[i] = shapeSample (data[i], drive, type);
        }
        oversampler->processSamplesDown (block);
    }

    buffer.applyGain (level);
}

void SoundBoxProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState().createXml())
        copyXmlToBinary (*state, destData);
}

void SoundBoxProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* SoundBoxProcessor::createEditor()
{
    return new SoundBoxEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoundBoxProcessor();
}
