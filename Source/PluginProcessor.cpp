#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

// The same three distortion curves as the web SoundBox.
static inline float shapeSample (float x, float drive, int type)
{
    if (type == 1) // hard — softened clip (a touch of round-off, not a raw square)
    {
        const float g = 1.0f + drive * 16.0f;
        const float y = g * x;
        return juce::jlimit (-1.0f, 1.0f, 1.5f * y - 0.5f * y * y * y);
    }
    if (type == 2) // fuzz — asymmetric, but gentler than before
    {
        const float gp = 1.0f + drive * 45.0f;
        const float gn = 1.0f + drive * 24.0f;
        return x >= 0.0f ? std::tanh (gp * x) : std::tanh (gn * x);
    }
    const float k = 1.0f + drive * 20.0f; // soft — smoother overdrive
    return std::tanh (k * x) / std::tanh (k);
}

juce::AudioProcessorValueTreeState::ParameterLayout
SoundBoxProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "gate", 1 }, "Gate",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "comp", 1 }, "Comp",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 }, "Drive",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

    // per-pedal footswitches (true = engaged)
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "compOn", 1 },   "Comp On",   true));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "driveOn", 1 },  "Drive On",  true));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "chorusOn", 1 }, "Chorus On", true));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "delayOn", 1 },  "Delay On",  true));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "distType", 1 }, "Distortion",
        StringArray { "Soft", "Hard", "Fuzz" }, 0));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "low", 1 }, "Low",
        NormalisableRange<float> (-18.0f, 18.0f, 0.1f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "mid", 1 }, "Mid",
        NormalisableRange<float> (-18.0f, 18.0f, 0.1f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "high", 1 }, "High",
        NormalisableRange<float> (-18.0f, 18.0f, 0.1f), 0.0f));
    layout.add (std::make_unique<AudioParameterBool>  (ParameterID { "cab", 1 }, "Cab Sim", false));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "chorus", 1 }, "Chorus",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "delayMix", 1 }, "Delay",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "delayTime", 1 }, "Delay Time",
        NormalisableRange<float> (0.02f, 1.0f, 0.001f), 0.3f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "delayFb", 1 }, "Feedback",
        NormalisableRange<float> (0.0f, 0.9f, 0.001f), 0.35f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "reverb", 1 }, "Reverb",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "level", 1 }, "Level",
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
    fs = sampleRate;
    const auto channels = (juce::uint32) juce::jmax (1, getTotalNumInputChannels());
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, channels };

    gate.prepare (spec);
    gate.setRatio (10.0f);
    gate.setAttack (2.0f);
    gate.setRelease (120.0f);

    comp.prepare (spec);

    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        (size_t) channels, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
    oversampler->initProcessing ((size_t) samplesPerBlock);
    setLatencySamples ((int) oversampler->getLatencyInSamples());

    dcBlock.prepare (spec);
    toneLP.prepare (spec);
    lowShelf.prepare (spec); midPeak.prepare (spec); highShelf.prepare (spec);
    cabHP.prepare (spec);    cabLP.prepare (spec);   cabDip.prepare (spec);

    chorus.prepare (spec);
    chorus.setRate (0.8f);
    chorus.setDepth (0.3f);
    chorus.setCentreDelay (18.0f);
    chorus.setFeedback (0.0f);

    delayLine.setMaximumDelayInSamples ((int) (sampleRate * 1.0) + 2);
    delayLine.prepare (spec);
    delayLine.reset();

    reverb.prepare (spec);
    limiter.prepare (spec);
    limiter.setThreshold (-1.0f);
    limiter.setRelease (200.0f);
}

bool SoundBoxProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    return mainIn == mainOut;
}

void SoundBoxProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    // 1) noise gate
    const float gateAmt = get ("gate");
    if (gateAmt > 0.001f)
    {
        gate.setThreshold (juce::Decibels::gainToDecibels (juce::jmax (1.0e-4f, gateAmt * gateAmt * 0.5f)));
        gate.process (ctx);
    }

    // 2) compressor (+ makeup) — mirrors the web mapping
    const float compAmt = get ("comp");
    if (get ("compOn") > 0.5f && compAmt > 0.001f)
    {
        comp.setThreshold (-40.0f * compAmt);
        comp.setRatio (1.0f + 11.0f * compAmt);
        comp.setAttack (3.0f);
        comp.setRelease (250.0f);
        comp.process (ctx);
        buffer.applyGain (1.0f + 1.5f * compAmt);
    }

    // 3) distortion (oversampled) — only when engaged and turned up
    const float drive = get ("drive");
    const int   type  = (int) get ("distType");
    const bool  driveOn = get ("driveOn") > 0.5f && drive > 0.001f;
    if (driveOn)
    {
        auto os = oversampler->processSamplesUp (block);
        for (size_t ch = 0; ch < os.getNumChannels(); ++ch)
        {
            auto* d = os.getChannelPointer (ch);
            for (size_t i = 0; i < os.getNumSamples(); ++i)
                d[i] = shapeSample (d[i], drive, type);
        }
        oversampler->processSamplesDown (block);

        // smooth the fizz the drive adds (tone roll-off), scaled with drive
        *toneLP.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
            fs, juce::jmap (drive, 0.0f, 1.0f, 9000.0f, 5500.0f));
        toneLP.process (ctx);
    }

    // always-on 70 Hz high-pass: kills DC, rumble and most mains hum (ground noise)
    *dcBlock.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (fs, 70.0f);
    dcBlock.process (ctx);

    // 4) 3-band EQ
    *lowShelf.state  = *juce::dsp::IIR::Coefficients<float>::makeLowShelf  (fs, 200.0f,  0.707f, juce::Decibels::decibelsToGain (get ("low")));
    *midPeak.state   = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (fs, 1000.0f, 1.0f,   juce::Decibels::decibelsToGain (get ("mid")));
    *highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (fs, 3500.0f, 0.707f, juce::Decibels::decibelsToGain (get ("high")));
    lowShelf.process (ctx); midPeak.process (ctx); highShelf.process (ctx);

    // 5) cabinet / speaker sim (switchable)
    if (get ("cab") > 0.5f)
    {
        *cabHP.state  = *juce::dsp::IIR::Coefficients<float>::makeHighPass  (fs, 85.0f);
        *cabLP.state  = *juce::dsp::IIR::Coefficients<float>::makeLowPass   (fs, 4500.0f);
        *cabDip.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (fs, 2800.0f, 1.5f, juce::Decibels::decibelsToGain (-4.0f));
        cabHP.process (ctx); cabLP.process (ctx); cabDip.process (ctx);
    }

    // 6) chorus (only when engaged)
    const float chAmt = get ("chorus");
    if (get ("chorusOn") > 0.5f && chAmt > 0.001f)
    {
        chorus.setMix (chAmt);
        chorus.process (ctx);
    }

    // 7) delay (echo) with feedback — wet added on top of dry
    const float dmix = get ("delayMix");
    if (get ("delayOn") > 0.5f && dmix > 0.0001f)
    {
        delayLine.setDelay (juce::jlimit (1.0f, (float) (fs * 1.0), get ("delayTime") * (float) fs));
        const float dfb = get ("delayFb");
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                const float in = data[i];
                const float echo = delayLine.popSample (ch, -1.0f, true);
                delayLine.pushSample (ch, in + echo * dfb);
                data[i] = in + echo * dmix;
            }
        }
    }

    // 8) reverb (only when engaged; dry kept full, wet scaled by the knob)
    const float rvAmt = get ("reverb");
    if (rvAmt > 0.001f)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.75f;
        rp.damping  = 0.4f;
        rp.width    = 1.0f;
        rp.dryLevel = 1.0f;
        rp.wetLevel = rvAmt;
        reverb.setParameters (rp);
        reverb.process (ctx);
    }

    // 9) output level
    buffer.applyGain (get ("level"));

    // 10) safety limiter
    limiter.process (ctx);
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

juce::AudioProcessorEditor* SoundBoxProcessor::createEditor() { return new SoundBoxEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SoundBoxProcessor(); }
