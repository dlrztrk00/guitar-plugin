#include "PluginEditor.h"
#include "BinaryData.h"

using Options = juce::WebBrowserComponent::Options;

SoundBoxEditor::SoundBoxEditor (SoundBoxProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      webView (Options{}
                   .withNativeIntegrationEnabled()
                   .withResourceProvider ([this] (const auto& url) { return getResource (url); })
                   .withNativeFunction (juce::Identifier ("setOrder"),
                       [this] (const juce::Array<juce::var>& args, auto complete)
                       {
                           if (args.size() >= 4)
                               proc.setPedalOrder ((int) args[0], (int) args[1],
                                                   (int) args[2], (int) args[3]);
                           complete (juce::var());
                       })
                   .withOptionsFrom (gateRelay)
                   .withOptionsFrom (compRelay)
                   .withOptionsFrom (driveRelay)
                   .withOptionsFrom (lowRelay)
                   .withOptionsFrom (midRelay)
                   .withOptionsFrom (highRelay)
                   .withOptionsFrom (delayMixRelay)
                   .withOptionsFrom (delayTimeRelay)
                   .withOptionsFrom (delayFbRelay)
                   .withOptionsFrom (chorusRelay)
                   .withOptionsFrom (reverbRelay)
                   .withOptionsFrom (levelRelay)
                   .withOptionsFrom (distRelay)
                   .withOptionsFrom (delayDivRelay)
                   .withOptionsFrom (delaySyncRelay)
                   .withOptionsFrom (cabRelay)
                   .withOptionsFrom (compOnRelay)
                   .withOptionsFrom (driveOnRelay)
                   .withOptionsFrom (chorusOnRelay)
                   .withOptionsFrom (delayOnRelay)
                   .withOptionsFrom (tremRateRelay)
                   .withOptionsFrom (tremDepthRelay)
                   .withOptionsFrom (phaserRateRelay)
                   .withOptionsFrom (phaserDepthRelay)
                   .withOptionsFrom (flangerRateRelay)
                   .withOptionsFrom (flangerDepthRelay)
                   .withOptionsFrom (boostRelay)
                   .withOptionsFrom (tremOnRelay)
                   .withOptionsFrom (phaserOnRelay)
                   .withOptionsFrom (flangerOnRelay)
                   .withOptionsFrom (boostOnRelay))
{
    auto& apvts = proc.apvts;
    auto bindSlider = [&] (const juce::String& id, juce::WebSliderRelay& relay)
    {
        sliderAtts.push_back (std::make_unique<juce::WebSliderParameterAttachment> (
            *apvts.getParameter (id), relay, nullptr));
    };

    bindSlider ("gate", gateRelay);
    bindSlider ("comp", compRelay);
    bindSlider ("drive", driveRelay);
    bindSlider ("low", lowRelay);
    bindSlider ("mid", midRelay);
    bindSlider ("high", highRelay);
    bindSlider ("delayMix", delayMixRelay);
    bindSlider ("delayTime", delayTimeRelay);
    bindSlider ("delayFb", delayFbRelay);
    bindSlider ("chorus", chorusRelay);
    bindSlider ("reverb", reverbRelay);
    bindSlider ("level", levelRelay);
    bindSlider ("tremRate", tremRateRelay);
    bindSlider ("tremDepth", tremDepthRelay);
    bindSlider ("phaserRate", phaserRateRelay);
    bindSlider ("phaserDepth", phaserDepthRelay);
    bindSlider ("flangerRate", flangerRateRelay);
    bindSlider ("flangerDepth", flangerDepthRelay);
    bindSlider ("boost", boostRelay);

    distAtt = std::make_unique<juce::WebComboBoxParameterAttachment> (
        *apvts.getParameter ("distType"), distRelay, nullptr);
    delayDivAtt = std::make_unique<juce::WebComboBoxParameterAttachment> (
        *apvts.getParameter ("delayDiv"), delayDivRelay, nullptr);

    auto bindToggle = [&] (const juce::String& id, juce::WebToggleButtonRelay& relay)
    {
        toggleAtts.push_back (std::make_unique<juce::WebToggleButtonParameterAttachment> (
            *apvts.getParameter (id), relay, nullptr));
    };
    bindToggle ("cab", cabRelay);
    bindToggle ("delaySync", delaySyncRelay);
    bindToggle ("compOn", compOnRelay);
    bindToggle ("driveOn", driveOnRelay);
    bindToggle ("chorusOn", chorusOnRelay);
    bindToggle ("delayOn", delayOnRelay);
    bindToggle ("tremOn", tremOnRelay);
    bindToggle ("phaserOn", phaserOnRelay);
    bindToggle ("flangerOn", flangerOnRelay);
    bindToggle ("boostOn", boostOnRelay);

    addAndMakeVisible (webView);
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    setResizable (true, true);
    setResizeLimits (760, 460, 1600, 1000);
    setSize (1000, 530);

    startTimerHz (24);
}

void SoundBoxEditor::timerCallback()
{
    const float lvl = juce::jlimit (0.0f, 1.0f, proc.outPeak.load());
    webView.evaluateJavascript ("window.__meter && window.__meter(" + juce::String (lvl, 3) + ");");
}

static const char* mimeForExtension (const juce::String& ext)
{
    if (ext == "html") return "text/html";
    if (ext == "js")   return "text/javascript";
    if (ext == "css")  return "text/css";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png")  return "image/png";
    return "application/octet-stream";
}

std::optional<juce::WebBrowserComponent::Resource>
SoundBoxEditor::getResource (const juce::String& url)
{
    // the embedded resources are keyed by base filename (index.html, juce.js, top.jpg…)
    const juce::String name = (url == "/" ? juce::String ("index.html")
                                          : url.fromLastOccurrenceOf ("/", false, false));

    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        if (name == juce::String (BinaryData::originalFilenames[i]))
        {
            int size = 0;
            const char* data = BinaryData::getNamedResource (BinaryData::namedResourceList[i], size);
            if (data == nullptr)
                break;

            const auto ext = name.fromLastOccurrenceOf (".", false, false);
            std::vector<std::byte> bytes ((size_t) size);
            std::memcpy (bytes.data(), data, (size_t) size);
            return juce::WebBrowserComponent::Resource { std::move (bytes),
                                                         juce::String (mimeForExtension (ext)) };
        }
    }
    return std::nullopt;
}

void SoundBoxEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0d10));
}

void SoundBoxEditor::resized()
{
    webView.setBounds (getLocalBounds());
}
