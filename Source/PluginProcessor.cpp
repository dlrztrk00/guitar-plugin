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
    layout.add (std::make_unique<AudioParameterBool>  (ParameterID { "irOn", 1 }, "IR Cab", false));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "chorus", 1 }, "Chorus",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "delayMix", 1 }, "Delay",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "delayTime", 1 }, "Delay Time",
        NormalisableRange<float> (0.02f, 1.0f, 0.001f), 0.3f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "delayFb", 1 }, "Feedback",
        NormalisableRange<float> (0.0f, 0.9f, 0.001f), 0.35f));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "delaySync", 1 }, "Delay Sync", false));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "delayDiv", 1 }, "Delay Division",
        StringArray { "1/4", "1/8", "1/8.", "1/16", "1/4." }, 1));
    layout.add (std::make_unique<AudioParameterBool>  (ParameterID { "tremOn", 1 }, "Tremolo On", true));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "tremRate", 1 }, "Tremolo Rate",
        NormalisableRange<float> (0.1f, 12.0f, 0.01f), 5.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "tremDepth", 1 }, "Tremolo Depth",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterBool>  (ParameterID { "phaserOn", 1 }, "Phaser On", true));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "phaserRate", 1 }, "Phaser Rate",
        NormalisableRange<float> (0.05f, 8.0f, 0.01f), 0.8f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "phaserDepth", 1 }, "Phaser Depth",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterBool>  (ParameterID { "flangerOn", 1 }, "Flanger On", true));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "flangerRate", 1 }, "Flanger Rate",
        NormalisableRange<float> (0.05f, 6.0f, 0.01f), 0.3f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "flangerDepth", 1 }, "Flanger Depth",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterBool>  (ParameterID { "wahOn", 1 }, "Auto-Wah On", true));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "wahSens", 1 }, "Wah Sens",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "wahPeak", 1 }, "Wah Peak",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));
    layout.add (std::make_unique<AudioParameterBool>  (ParameterID { "boostOn", 1 }, "Boost On", true));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "boost", 1 }, "Boost",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

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

    phaser.prepare (spec);
    phaser.setCentreFrequency (600.0f);
    phaser.setFeedback (0.4f);
    tremPhase = 0.0;

    flanger.prepare (spec);
    flanger.setCentreDelay (2.0f);
    flanger.setFeedback (0.65f);

    wahFilter.prepare (spec);
    wahFilter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    wahEnv = 0.0f;

    delayLine.setMaximumDelayInSamples ((int) (sampleRate * 1.0) + 2);
    delayLine.prepare (spec);
    delayLine.reset();

    reverb.prepare (spec);
    limiter.prepare (spec);
    limiter.setThreshold (-1.0f);
    limiter.setRelease (200.0f);

    irConv.prepare (spec);
    if (irLoaded.load() && irPath.isNotEmpty())
        loadIR (juce::File (irPath));
}

void SoundBoxProcessor::loadIR (const juce::File& file)
{
    if (! file.existsAsFile())
        return;
    irConv.loadImpulseResponse (file,
                                juce::dsp::Convolution::Stereo::yes,
                                juce::dsp::Convolution::Trim::yes,
                                0,
                                juce::dsp::Convolution::Normalise::yes);
    irPath = file.getFullPathName();
    irName = file.getFileNameWithoutExtension();
    irLoaded.store (true);
}

void SoundBoxProcessor::clearIR()
{
    irLoaded.store (false);
    irPath = {};
    irName = {};
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

    // noise gate (fixed, first)
    const float gateAmt = get ("gate");
    if (gateAmt > 0.001f)
    {
        gate.setThreshold (juce::Decibels::gainToDecibels (juce::jmax (1.0e-4f, gateAmt * gateAmt * 0.5f)));
        gate.process (ctx);
    }

    // clean boost (drives the whole chain harder — good for solos)
    const float boostAmt = get ("boost");
    if (get ("boostOn") > 0.5f && boostAmt > 0.001f)
        buffer.applyGain (juce::Decibels::decibelsToGain (boostAmt * 12.0f));

    // ── the four reorderable pedals ──
    auto applyComp = [&]
    {
        const float a = get ("comp");
        if (get ("compOn") > 0.5f && a > 0.001f)
        {
            comp.setThreshold (-40.0f * a);
            comp.setRatio (1.0f + 11.0f * a);
            comp.setAttack (3.0f);
            comp.setRelease (250.0f);
            comp.process (ctx);
            buffer.applyGain (1.0f + 1.5f * a);
        }
    };
    auto applyDrive = [&]
    {
        const float d = get ("drive");
        const int   t = (int) get ("distType");
        if (get ("driveOn") > 0.5f && d > 0.001f)
        {
            auto os = oversampler->processSamplesUp (block);
            for (size_t ch = 0; ch < os.getNumChannels(); ++ch)
            {
                auto* p = os.getChannelPointer (ch);
                for (size_t i = 0; i < os.getNumSamples(); ++i)
                    p[i] = shapeSample (p[i], d, t);
            }
            oversampler->processSamplesDown (block);
            *toneLP.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                fs, juce::jmap (d, 0.0f, 1.0f, 9000.0f, 5500.0f));
            toneLP.process (ctx);
        }
    };
    auto applyChorus = [&]
    {
        const float a = get ("chorus");
        if (get ("chorusOn") > 0.5f && a > 0.001f) { chorus.setMix (a); chorus.process (ctx); }
    };
    // delay time: free (knob) or synced to the host tempo
    float delaySeconds = get ("delayTime");
    if (get ("delaySync") > 0.5f)
    {
        double bpm = 120.0;
        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
                if (auto b = pos->getBpm())
                    bpm = *b;
        static const float factors[] = { 1.0f, 0.5f, 0.75f, 0.25f, 1.5f }; // 1/4,1/8,1/8.,1/16,1/4.
        const int div = juce::jlimit (0, 4, (int) get ("delayDiv"));
        delaySeconds = juce::jlimit (0.02f, 1.0f, (float) (60.0 / bpm) * factors[div]);
    }

    auto applyDelay = [&]
    {
        const float m = get ("delayMix");
        if (get ("delayOn") > 0.5f && m > 0.0001f)
        {
            delayLine.setDelay (juce::jlimit (1.0f, (float) (fs * 1.0), delaySeconds * (float) fs));
            const float dfb = get ("delayFb");
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* data = buffer.getWritePointer (ch);
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    const float in = data[i];
                    const float echo = delayLine.popSample (ch, -1.0f, true);
                    delayLine.pushSample (ch, in + echo * dfb);
                    data[i] = in + echo * m;
                }
            }
        }
    };

    const int packed = orderPacked.load();
    for (int s = 0; s < 4; ++s)
    {
        switch ((packed >> (s * 2)) & 3)
        {
            case 0: applyComp();   break;
            case 1: applyDrive();  break;
            case 2: applyChorus(); break;
            default: applyDelay(); break;
        }
    }

    // tremolo (amplitude LFO) — same modulation across channels
    if (get ("tremOn") > 0.5f && get ("tremDepth") > 0.001f)
    {
        const double inc = juce::MathConstants<double>::twoPi * get ("tremRate") / fs;
        const float depth = get ("tremDepth");
        const int numSm = buffer.getNumSamples();
        const int numCh = buffer.getNumChannels();
        for (int i = 0; i < numSm; ++i)
        {
            const float g = 1.0f - depth * 0.5f * (1.0f - (float) std::cos (tremPhase));
            for (int ch = 0; ch < numCh; ++ch)
                buffer.getWritePointer (ch)[i] *= g;
            tremPhase += inc;
            if (tremPhase > juce::MathConstants<double>::twoPi) tremPhase -= juce::MathConstants<double>::twoPi;
        }
    }

    // phaser
    if (get ("phaserOn") > 0.5f && get ("phaserDepth") > 0.001f)
    {
        phaser.setRate (get ("phaserRate"));
        phaser.setDepth (get ("phaserDepth"));
        phaser.setMix (0.5f);
        phaser.process (ctx);
    }

    // flanger
    if (get ("flangerOn") > 0.5f && get ("flangerDepth") > 0.001f)
    {
        flanger.setRate (get ("flangerRate"));
        flanger.setDepth (get ("flangerDepth"));
        flanger.setMix (0.5f);
        flanger.process (ctx);
    }

    // auto-wah — a bandpass whose cutoff rides the playing envelope
    if (get ("wahOn") > 0.5f && get ("wahSens") > 0.001f)
    {
        const float sens = get ("wahSens");
        wahFilter.setResonance (1.0f + get ("wahPeak") * 7.0f);
        const int nSm = buffer.getNumSamples();
        const int nCh = buffer.getNumChannels();
        for (int i = 0; i < nSm; ++i)
        {
            float mag = 0.0f;
            for (int ch = 0; ch < nCh; ++ch)
                mag = juce::jmax (mag, std::abs (buffer.getReadPointer (ch)[i]));
            // envelope follower: fast attack, slower release
            const float coeff = (mag > wahEnv) ? 0.25f : 0.02f;
            wahEnv += (mag - wahEnv) * coeff;
            const float cf = juce::jlimit (250.0f, 3000.0f, 300.0f + sens * wahEnv * 4500.0f);
            wahFilter.setCutoffFrequency (cf);
            for (int ch = 0; ch < nCh; ++ch)
            {
                auto* d = buffer.getWritePointer (ch);
                d[i] = wahFilter.processSample (ch, d[i]);
            }
        }
    }

    // always-on 70 Hz high-pass: kills DC, rumble and most mains hum (ground noise)
    *dcBlock.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (fs, 70.0f);
    dcBlock.process (ctx);

    // 3-band EQ
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

    // impulse-response cabinet (real speaker convolution, when loaded + engaged)
    if (get ("irOn") > 0.5f && irLoaded.load())
        irConv.process (ctx);

    // reverb (only when engaged; dry kept full, wet scaled by the knob)
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

    // output peak meter (with a little decay) for the UI
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
    outPeak.store (juce::jmax (peak, outPeak.load() * 0.85f));

    // feed the spectrum analyser (post-FX, mono sum)
    {
        const int nCh = juce::jmax (1, buffer.getNumChannels());
        const int nSm = buffer.getNumSamples();
        for (int i = 0; i < nSm; ++i)
        {
            float s = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                s += buffer.getReadPointer (ch)[i];
            pushToAnalyser (s / (float) nCh);
        }
    }
}

void SoundBoxProcessor::pushToAnalyser (float s) noexcept
{
    if (fifoIndex == fftSize)
    {
        computeSpectrum();
        fifoIndex = 0;
    }
    fifo[(size_t) fifoIndex++] = s;
}

void SoundBoxProcessor::computeSpectrum() noexcept
{
    std::memcpy (fftData.data(), fifo.data(), sizeof (float) * fftSize);
    std::fill (fftData.begin() + fftSize, fftData.end(), 0.0f);
    window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    const float minHz   = 40.0f;
    const float maxHz    = 12000.0f;
    const float nyquist  = (float) fs * 0.5f;
    const int   halfBins = fftSize / 2;

    for (int b = 0; b < numBands; ++b)
    {
        const float f0 = minHz * std::pow (maxHz / minHz, (float) b       / numBands);
        const float f1 = minHz * std::pow (maxHz / minHz, (float) (b + 1) / numBands);
        int i0 = juce::jlimit (0, halfBins - 1, (int) (f0 / nyquist * halfBins));
        int i1 = juce::jlimit (i0 + 1, halfBins, (int) (f1 / nyquist * halfBins));

        float mag = 0.0f;
        for (int i = i0; i < i1; ++i)
            mag = juce::jmax (mag, fftData[(size_t) i]);

        const float db = juce::Decibels::gainToDecibels (mag / (float) fftSize + 1.0e-9f);
        const float v  = juce::jlimit (0.0f, 1.0f, (db + 66.0f) / 66.0f);
        const float prev = spectrum[(size_t) b].load();
        spectrum[(size_t) b].store (juce::jmax (v, prev * 0.75f)); // fast attack, slow decay
    }
}

void SoundBoxProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("irPath", irPath, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void SoundBoxProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        apvts.replaceState (tree);
        const auto p = tree.getProperty ("irPath").toString();
        if (p.isNotEmpty())
            loadIR (juce::File (p));
    }
}

juce::AudioProcessorEditor* SoundBoxProcessor::createEditor() { return new SoundBoxEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SoundBoxProcessor(); }
