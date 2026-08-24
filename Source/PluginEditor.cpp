#include "PluginEditor.h"

SoundBoxEditor::SoundBoxEditor (SoundBoxProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    addKnob (drive, "drive", "DRIVE");
    addKnob (low,   "low",   "LOW");
    addKnob (mid,   "mid",   "MID");
    addKnob (high,  "high",  "HIGH");
    addKnob (level, "level", "LEVEL");

    distLabel.setText ("DIST", juce::dontSendNotification);
    distLabel.setJustificationType (juce::Justification::centred);
    distLabel.setColour (juce::Label::textColourId, juce::Colour (0xffc6ccd4));
    distLabel.setFont (juce::FontOptions (12.0f));
    addAndMakeVisible (distLabel);

    distBox.addItemList ({ "Soft", "Hard", "Fuzz" }, 1);
    distBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff14171a));
    distBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xffe6ede2));
    addAndMakeVisible (distBox);
    distAtt = std::make_unique<ComboBoxAttachment> (proc.apvts, "distType", distBox);

    setSize (620, 300);
}

void SoundBoxEditor::addKnob (KnobUI& k, const juce::String& paramId, const juce::String& text)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    k.slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffe8b64c));
    k.slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff1e6cf));
    k.slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe8b64c));
    k.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (k.slider);

    k.label.setText (text, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setColour (juce::Label::textColourId, juce::Colour (0xffc6ccd4));
    k.label.setFont (juce::FontOptions (12.0f));
    addAndMakeVisible (k.label);

    k.attach = std::make_unique<SliderAttachment> (proc.apvts, paramId, k.slider);
}

void SoundBoxEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0d10));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    g.drawText ("SOUND//BOX", getLocalBounds().reduced (24).removeFromTop (44),
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff7f8a78));
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("distortion + 3-band EQ — AU plugin",
                getLocalBounds().reduced (24).removeFromTop (66).removeFromBottom (20),
                juce::Justification::centredLeft);
}

void SoundBoxEditor::resized()
{
    auto area = getLocalBounds().reduced (24);
    area.removeFromTop (60); // header

    auto row = area.removeFromTop (150);
    const int knobW = row.getWidth() / 5;

    auto place = [&] (KnobUI& k)
    {
        auto col = row.removeFromLeft (knobW);
        k.label.setBounds (col.removeFromTop (18));
        k.slider.setBounds (col);
    };
    place (drive);
    place (low);
    place (mid);
    place (high);
    place (level);

    // DIST selector below the drive knob
    auto bottom = area.removeFromTop (34);
    distLabel.setBounds (bottom.removeFromLeft (44));
    distBox.setBounds (bottom.removeFromLeft (140));
}
