#include "PluginEditor.h"

SoundBoxEditor::SoundBoxEditor (SoundBoxProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    juce::ignoreUnused (proc);
    setSize (420, 260);
}

void SoundBoxEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0d10));

    auto area = getLocalBounds().reduced (24);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("SOUND//BOX", area.removeFromTop (60),
                juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff7f8a78));
    g.setFont (juce::FontOptions (13.0f));
    g.drawText ("guitar effects — AU plugin",
                area.removeFromTop (24), juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xffe8b64c));
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("step 1 · passthrough · loaded in Logic",
                getLocalBounds().reduced (24).removeFromBottom (30),
                juce::Justification::centredLeft);
}

void SoundBoxEditor::resized() {}
