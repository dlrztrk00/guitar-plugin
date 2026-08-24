#include "PluginEditor.h"

SoundBoxEditor::SoundBoxEditor (SoundBoxProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    styleKnob (driveKnob, driveLabel, "DRIVE");
    styleKnob (levelKnob, levelLabel, "LEVEL");

    distLabel.setText ("DIST", juce::dontSendNotification);
    distLabel.setJustificationType (juce::Justification::centred);
    distLabel.setColour (juce::Label::textColourId, juce::Colour (0xffc6ccd4));
    distLabel.setFont (juce::FontOptions (12.0f));
    addAndMakeVisible (distLabel);

    distBox.addItemList ({ "Soft", "Hard", "Fuzz" }, 1);
    distBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff14171a));
    distBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xffe6ede2));
    addAndMakeVisible (distBox);

    driveAtt = std::make_unique<SliderAttachment>   (proc.apvts, "drive",    driveKnob);
    levelAtt = std::make_unique<SliderAttachment>   (proc.apvts, "level",    levelKnob);
    distAtt  = std::make_unique<ComboBoxAttachment> (proc.apvts, "distType", distBox);

    setSize (460, 300);
}

void SoundBoxEditor::styleKnob (juce::Slider& s, juce::Label& l, const juce::String& text)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffe8b64c));
    s.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff1e6cf));
    s.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe8b64c));
    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (s);

    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, juce::Colour (0xffc6ccd4));
    l.setFont (juce::FontOptions (12.0f));
    addAndMakeVisible (l);
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
    g.drawText ("distortion — AU plugin",
                getLocalBounds().reduced (24).removeFromTop (66).removeFromBottom (20),
                juce::Justification::centredLeft);
}

void SoundBoxEditor::resized()
{
    auto area = getLocalBounds().reduced (24);
    area.removeFromTop (72); // header

    auto row = area.removeFromTop (150);
    const int knobW = 120;

    auto driveArea = row.removeFromLeft (knobW);
    driveLabel.setBounds (driveArea.removeFromTop (18));
    driveKnob.setBounds (driveArea);

    auto levelArea = row.removeFromLeft (knobW);
    levelLabel.setBounds (levelArea.removeFromTop (18));
    levelKnob.setBounds (levelArea);

    auto distArea = row.removeFromLeft (knobW + 20);
    distLabel.setBounds (distArea.removeFromTop (18));
    distBox.setBounds (distArea.removeFromTop (30).reduced (4, 0));
}
