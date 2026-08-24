#include "PluginEditor.h"
#include "BinaryData.h"

using Options = juce::WebBrowserComponent::Options;

SoundBoxEditor::SoundBoxEditor (SoundBoxProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      webView (Options{}
                   .withNativeIntegrationEnabled()
                   .withResourceProvider ([this] (const auto& url) { return getResource (url); })
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
                   .withOptionsFrom (cabRelay))
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

    distAtt = std::make_unique<juce::WebComboBoxParameterAttachment> (
        *apvts.getParameter ("distType"), distRelay, nullptr);
    cabAtt = std::make_unique<juce::WebToggleButtonParameterAttachment> (
        *apvts.getParameter ("cab"), cabRelay, nullptr);

    addAndMakeVisible (webView);
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    setSize (380, 380);
}

static const char* mimeForExtension (const juce::String& ext)
{
    if (ext == "html") return "text/html";
    if (ext == "js")   return "text/javascript";
    if (ext == "css")  return "text/css";
    return "application/octet-stream";
}

std::optional<juce::WebBrowserComponent::Resource>
SoundBoxEditor::getResource (const juce::String& url)
{
    // map the requested path to an embedded resource
    juce::String name = (url == "/" ? juce::String ("index.html")
                                     : url.fromFirstOccurrenceOf ("/", false, false));

    int size = 0;
    const char* data = nullptr;

    if (name == "index.html")                    data = BinaryData::index_html,               size = BinaryData::index_htmlSize;
    else if (name == "js/juce.js")               data = BinaryData::juce_js,                  size = BinaryData::juce_jsSize;
    else if (name == "js/check_native_interop.js") data = BinaryData::check_native_interop_js, size = BinaryData::check_native_interop_jsSize;

    if (data == nullptr)
        return std::nullopt;

    const auto ext = name.fromLastOccurrenceOf (".", false, false);
    std::vector<std::byte> bytes ((size_t) size);
    std::memcpy (bytes.data(), data, (size_t) size);
    return juce::WebBrowserComponent::Resource { std::move (bytes), juce::String (mimeForExtension (ext)) };
}

void SoundBoxEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0d10));
}

void SoundBoxEditor::resized()
{
    webView.setBounds (getLocalBounds());
}
