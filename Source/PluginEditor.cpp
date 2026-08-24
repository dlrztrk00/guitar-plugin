#include "PluginEditor.h"

SoundBoxEditor::SoundBoxEditor (SoundBoxProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    addKnob (gate,      "gate",      "GATE");
    addKnob (comp,      "comp",      "COMP");
    addKnob (drive,     "drive",     "DRIVE");
    addKnob (low,       "low",       "LOW");
    addKnob (mid,       "mid",       "MID");
    addKnob (high,      "high",      "HIGH");
    addKnob (delayMix,  "delayMix",  "DELAY");
    addKnob (delayTime, "delayTime", "TIME");
    addKnob (delayFb,   "delayFb",   "F.BACK");
    addKnob (chorus,    "chorus",    "CHORUS");
    addKnob (reverb,    "reverb",    "REVERB");
    addKnob (level,     "level",     "LEVEL");

    distLabel.setText ("DIST", juce::dontSendNotification);
    distLabel.setJustificationType (juce::Justification::centredRight);
    distLabel.setColour (juce::Label::textColourId, juce::Colour (0xffc6ccd4));
    distLabel.setFont (juce::FontOptions (12.0f));
    addAndMakeVisible (distLabel);

    distBox.addItemList ({ "Soft", "Hard", "Fuzz" }, 1);
    distBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff14171a));
    distBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xffe6ede2));
    addAndMakeVisible (distBox);
    distAtt = std::make_unique<ComboBoxAttachment> (proc.apvts, "distType", distBox);

    cabButton.setColour (juce::ToggleButton::textColourId, juce::Colour (0xffc6ccd4));
    cabButton.setColour (juce::ToggleButton::tickColourId, juce::Colour (0xffe8b64c));
    addAndMakeVisible (cabButton);
    cabAtt = std::make_unique<ButtonAttachment> (proc.apvts, "cab", cabButton);

    setSize (560, 520);
}

void SoundBoxEditor::addKnob (KnobUI& k, const juce::String& paramId, const juce::String& text)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 16);
    k.slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffe8b64c));
    k.slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff1e6cf));
    k.slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe8b64c));
    k.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (k.slider);

    k.label.setText (text, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setColour (juce::Label::textColourId, juce::Colour (0xffc6ccd4));
    k.label.setFont (juce::FontOptions (11.0f));
    addAndMakeVisible (k.label);

    k.attach = std::make_unique<SliderAttachment> (proc.apvts, paramId, k.slider);
}

void SoundBoxEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0d10));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    g.drawText ("SOUND//BOX", getLocalBounds().reduced (22).removeFromTop (40),
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff7f8a78));
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("guitar multi-fx — AU plugin",
                getLocalBounds().reduced (22).removeFromTop (60).removeFromBottom (20),
                juce::Justification::centredLeft);
}

void SoundBoxEditor::resized()
{
    auto area = getLocalBounds().reduced (22);
    area.removeFromTop (54); // header

    auto placeRow = [&] (juce::Rectangle<int> row, std::initializer_list<KnobUI*> knobs)
    {
        const int knobW = row.getWidth() / (int) knobs.size();
        for (auto* k : knobs)
        {
            auto col = row.removeFromLeft (knobW);
            k->label.setBounds (col.removeFromTop (16));
            k->slider.setBounds (col);
        }
    };

    placeRow (area.removeFromTop (120), { &gate, &comp, &drive, &low });
    area.removeFromTop (8);
    placeRow (area.removeFromTop (120), { &mid, &high, &delayMix, &delayTime });
    area.removeFromTop (8);
    placeRow (area.removeFromTop (120), { &delayFb, &chorus, &reverb, &level });

    area.removeFromTop (10);
    auto bottom = area.removeFromTop (30);
    distLabel.setBounds (bottom.removeFromLeft (44));
    distBox.setBounds (bottom.removeFromLeft (130));
    bottom.removeFromLeft (20);
    cabButton.setBounds (bottom.removeFromLeft (120));
}
