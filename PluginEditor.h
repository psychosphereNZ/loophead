#pragma once

#include "PluginProcessor.h"

//==============================================================================
// Skin der "G Audio Vintage Series"
class DustLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DustLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool isDown,
                       int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool isHighlighted, bool isDown) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool isHighlighted, bool isDown) override;
};

//==============================================================================
class LoopheadAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit LoopheadAudioProcessorEditor (LoopheadAudioProcessor&);
    ~LoopheadAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SliderAtt> attachment;
    };

    void addKnob (Knob&, const juce::String& paramId, const juce::String& name, int textW = 80);
    void addToggle (juce::TextButton&, const juce::String& paramId, std::unique_ptr<ButtonAtt>&,
                    const juce::String& componentId = {});
    void attachTimeKnob (bool sync);
    void timerCallback() override;
    void drawTapePath (juce::Graphics&);

    LoopheadAudioProcessor& proc;
    DustLookAndFeel lnf;

    Knob time;
    Knob feedback, drive, color, wobble, age;   // LOOP
    Knob spring, duck, width, mix, output;      // OUTPUT

    juce::TextButton headBtn[4] { juce::TextButton ("1"), juce::TextButton ("2"),
                                  juce::TextButton ("3"), juce::TextButton ("4") };
    juce::TextButton revBtn { "REV 4" }, syncBtn { "SYNC" }, throwBtn { "THROW" }, freezeBtn { "FREEZE" };
    std::unique_ptr<ButtonAtt> headAtt[4], revAtt, syncAtt, freezeAtt;
    juce::ComboBox presetBox;

    juce::Rectangle<int> sectionA, loopSec, outSec, tapeArea;
    juce::Path tapeLoop;
    float tapeOffset = 0.0f;
    float glow[4] {};
    int timeIsSync = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopheadAudioProcessorEditor)
};
