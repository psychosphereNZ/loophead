#include "PluginEditor.h"

namespace Col
{
    const juce::Colour body      { 0xff1d1c1a };
    const juce::Colour plate     { 0xffd8cdb4 };
    const juce::Colour plateDark { 0xffb9ad92 };
    const juce::Colour ink       { 0xff2a2724 };
    const juce::Colour orange    { 0xffe8742a };
    const juce::Colour knob      { 0xff2e2c29 };
    const juce::Colour lcdBg     { 0xff1f2c16 };
    const juce::Colour lcdText   { 0xffa6dc5c };
}

//==============================================================================
DustLookAndFeel::DustLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       Col::ink);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,               Col::ink);
    setColour (juce::ComboBox::textColourId,            Col::lcdText);
    setColour (juce::ComboBox::arrowColourId,           Col::lcdText);
    setColour (juce::PopupMenu::backgroundColourId,     Col::lcdBg);
    setColour (juce::PopupMenu::textColourId,           Col::lcdText);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Col::lcdText.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, Col::lcdText.brighter (0.3f));
}

void DustLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                        float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);

    // Skalenstriche
    g.setColour (Col::ink.withAlpha (0.55f));
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (float) i / 10.0f * (endAngle - startAngle);
        const float r1 = radius * 0.92f, r2 = radius * (i % 5 == 0 ? 1.0f : 0.97f);
        g.drawLine (c.x + r1 * std::sin (a), c.y - r1 * std::cos (a),
                    c.x + r2 * std::sin (a), c.y - r2 * std::cos (a), i % 5 == 0 ? 1.6f : 1.0f);
    }

    // Wert-Bogen
    const float arcR = radius * 0.82f;
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (Col::plateDark);
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path val;
    val.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour (Col::orange);
    g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knopfkappe mit Schatten
    const float kr = radius * 0.66f;
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (c.x - kr + 1.5f, c.y - kr + 3.0f, kr * 2.0f, kr * 2.0f);

    juce::ColourGradient grad (Col::knob.brighter (0.35f), c.x, c.y - kr,
                               Col::knob.darker (0.4f),     c.x, c.y + kr, false);
    g.setGradientFill (grad);
    g.fillEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawEllipse (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f, 1.0f);

    // Zeiger
    juce::Path ptr;
    ptr.addRoundedRectangle (-1.6f, -kr * 0.92f, 3.2f, kr * 0.5f, 1.5f);
    g.setColour (Col::orange);
    g.fillPath (ptr, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
}

void DustLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool,
                                    int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
    g.setColour (Col::lcdBg);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.5f);

    // LCD-Pixelraster
    g.setColour (Col::lcdText.withAlpha (0.05f));
    for (int yy = 2; yy < h - 2; yy += 3)
        g.drawHorizontalLine (yy, 3.0f, (float) w - 3.0f);

    // Pfeil
    juce::Path arrow;
    const float ax = (float) w - 18.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax, ay - 3.0f, ax + 8.0f, ay - 3.0f, ax + 4.0f, ay + 3.0f);
    g.setColour (Col::lcdText);
    g.fillPath (arrow);
}

juce::Font DustLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 17.0f, juce::Font::bold));
}

void DustLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (10, 1, box.getWidth() - 34, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void DustLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                            bool isHighlighted, bool isDown)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    const auto id = b.getComponentID();

    if (id == "throw" || id == "freeze")
    {
        // runde Performance-Taster
        const float d = juce::jmin (r.getWidth(), r.getHeight());
        auto c = r.withSizeKeepingCentre (d, d);
        const bool lit = b.getToggleState() || (id == "throw" && isDown);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillEllipse (c.translated (1.5f, 3.0f));
        auto base = id == "throw" ? (lit ? Col::orange.brighter (0.2f) : Col::orange.darker (0.55f))
                                  : (lit ? juce::Colour (0xff4fb3e8) : juce::Colour (0xff23485e));
        if (isHighlighted && ! lit) base = base.brighter (0.15f);
        juce::ColourGradient grad (base.brighter (0.35f), c.getCentreX(), c.getY(),
                                   base.darker (0.35f),   c.getCentreX(), c.getBottom(), false);
        g.setGradientFill (grad);
        g.fillEllipse (c);
        if (lit)
        {
            g.setColour (base.withAlpha (0.35f));
            g.drawEllipse (c.expanded (3.0f), 4.0f);
        }
        g.setColour (Col::ink);
        g.drawEllipse (c, 2.0f);
        return;
    }

    if (id == "head")
    {
        // Tonkopf: Metallblock, leuchtet orange wenn aktiv
        const bool on = b.getToggleState();
        juce::ColourGradient grad (on ? Col::orange.brighter (0.25f) : juce::Colour (0xff8d8678),
                                   r.getCentreX(), r.getY(),
                                   on ? Col::orange.darker (0.3f)   : juce::Colour (0xff4a463f),
                                   r.getCentreX(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (r, 3.0f);
        g.setColour (Col::ink);
        g.drawRoundedRectangle (r, 3.0f, 1.4f);
        g.setColour (Col::ink.withAlpha (0.6f));
        g.fillRect (r.getCentreX() - 1.0f, r.getY(), 2.0f, 4.0f);   // Kopfspalt
        if (isHighlighted)
        {
            g.setColour (juce::Colours::white.withAlpha (0.12f));
            g.fillRoundedRectangle (r, 3.0f);
        }
        return;
    }

    const bool on = b.getToggleState();
    g.setColour (on ? Col::orange : Col::knob);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawRoundedRectangle (r, 4.0f, 1.2f);
    if (isHighlighted && ! on)
    {
        g.setColour (Col::orange.withAlpha (0.25f));
        g.fillRoundedRectangle (r, 4.0f);
    }
}

void DustLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    const auto id = b.getComponentID();
    juce::Colour c = b.getToggleState() ? Col::ink : Col::plate.withAlpha (0.85f);
    float size = 12.5f;
    if (id == "throw" || id == "freeze") { c = Col::plate; size = 12.5f; }
    if (id == "head") { c = b.getToggleState() ? Col::ink : Col::plate; size = 15.0f; }
    g.setColour (c);
    g.setFont (juce::Font (juce::FontOptions (size, juce::Font::bold)));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

//==============================================================================
LoopheadAudioProcessorEditor::LoopheadAudioProcessorEditor (LoopheadAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);

    addKnob (time, "time", "HEAD SPACING", 90);
    addKnob (feedback, "feedback", "FEEDBACK");
    addKnob (drive,    "drive",    "DRIVE");
    addKnob (color,    "color",    "COLOR");
    addKnob (wobble,   "wobble",   "WOBBLE");
    addKnob (age,      "age",      "AGE");
    addKnob (spring,   "spring",   "SPRING");
    addKnob (duck,     "duck",     "DUCK");
    addKnob (width,    "width",    "WIDTH");
    addKnob (mix,      "mix",      "MIX");
    addKnob (output,   "output",   "OUTPUT");

    for (int h = 0; h < 4; ++h)
    {
        addToggle (headBtn[h], "head" + juce::String (h + 1), headAtt[h], "head");
        headBtn[h].setTooltip ("Wiedergabekopf " + juce::String (h + 1) + " an/aus");
    }
    addToggle (revBtn, "reverse", revAtt);
    revBtn.setTooltip ("Kopf 4 spielt rückwärts");
    addToggle (syncBtn, "sync", syncAtt);
    syncBtn.setTooltip ("Kopfabstand zum Songtempo synchronisieren");
    addToggle (freezeBtn, "freeze", freezeAtt, "freeze");
    freezeBtn.setTooltip ("Endlosschleife: Eingang trennen, Band läuft ohne Verlust weiter");

    // THROW: Taster, nur gedrückt aktiv -> Echo schwingt sich auf
    throwBtn.setComponentID ("throw");
    throwBtn.setTooltip ("Gedrückt halten: Feedback über 100 % -> Eigenschwingung");
    throwBtn.onStateChange = [this]
    {
        if (auto* param = proc.apvts.getParameter ("throw"))
        {
            const bool down = throwBtn.isDown();
            if ((param->getValue() > 0.5f) != down)
            {
                param->beginChangeGesture();
                param->setValueNotifyingHost (down ? 1.0f : 0.0f);
                param->endChangeGesture();
            }
        }
    };
    addAndMakeVisible (throwBtn);

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (juce::String (i + 1).paddedLeft ('0', 2) + "  " + proc.getProgramName (i).toUpperCase(), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedId() - 1;
        if (idx >= 0)
            proc.setCurrentProgram (idx);
    };
    addAndMakeVisible (presetBox);

    setSize (960, 540);
    timerCallback();
    startTimerHz (30);
}

LoopheadAudioProcessorEditor::~LoopheadAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void LoopheadAudioProcessorEditor::addKnob (Knob& k, const juce::String& paramId, const juce::String& name, int textW)
{
    k.slider.setColour (juce::Slider::textBoxTextColourId,       Col::ink);
    k.slider.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    k.slider.setColour (juce::Slider::textBoxBackgroundColourId, Col::plateDark.withAlpha (0.35f));
    k.slider.setColour (juce::Slider::textBoxHighlightColourId,  Col::orange.withAlpha (0.4f));
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, textW, 20);
    k.slider.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
    addAndMakeVisible (k.slider);

    k.label.setText (name, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
    addAndMakeVisible (k.label);

    if (paramId != "time")
        k.attachment = std::make_unique<SliderAtt> (proc.apvts, paramId, k.slider);
}

void LoopheadAudioProcessorEditor::addToggle (juce::TextButton& b, const juce::String& paramId,
                                              std::unique_ptr<ButtonAtt>& att, const juce::String& componentId)
{
    b.setClickingTogglesState (true);
    if (componentId.isNotEmpty())
        b.setComponentID (componentId);
    addAndMakeVisible (b);
    att = std::make_unique<ButtonAtt> (proc.apvts, paramId, b);
}

void LoopheadAudioProcessorEditor::attachTimeKnob (bool sync)
{
    // Großer Knopf steuert je nach SYNC die Zeit in ms oder die Notenwerte
    time.attachment.reset();
    time.attachment = std::make_unique<SliderAtt> (proc.apvts, sync ? "division" : "time", time.slider);
    time.label.setText (sync ? "HEAD SPACING (SYNC)" : "HEAD SPACING", juce::dontSendNotification);
}

void LoopheadAudioProcessorEditor::timerCallback()
{
    const int sync = proc.apvts.getRawParameterValue ("sync")->load() > 0.5f ? 1 : 0;
    if (sync != timeIsSync)
    {
        timeIsSync = sync;
        attachTimeKnob (sync == 1);
    }

    const int prog = proc.getCurrentProgram() + 1;
    if (presetBox.getSelectedId() != prog)
        presetBox.setSelectedId (prog, juce::dontSendNotification);

    // Bandlauf-Animation: kürzerer Kopfabstand = schnelleres Band
    const float baseMs = juce::jmax (10.0f, proc.getBaseMs());
    tapeOffset += proc.getTapeSpeed() * 2.2f * (120.0f / baseMs);
    if (tapeOffset > 1.0e6f) tapeOffset = 0.0f;

    for (int h = 0; h < 4; ++h)
    {
        const float lv = juce::jlimit (0.0f, 1.0f, proc.getHeadLevel (h) * 3.0f);
        glow[h] = lv > glow[h] ? lv : glow[h] * 0.85f;
    }

    repaint (sectionA);
}

//==============================================================================
void LoopheadAudioProcessorEditor::drawTapePath (juce::Graphics& g)
{
    auto ta = tapeArea.toFloat();

    // Gehäuse-Fenster
    g.setColour (Col::ink);
    g.fillRoundedRectangle (ta, 6.0f);

    // Bandschleife
    g.setColour (juce::Colour (0xff3b2a1e));
    g.strokePath (tapeLoop, juce::PathStrokeType (7.0f));

    // laufende Markierungen auf dem Band
    const float len = tapeLoop.getLength();
    if (len > 0.0f)
    {
        g.setColour (Col::plate.withAlpha (0.45f));
        const float spacing = 22.0f;
        const float start = std::fmod (tapeOffset, spacing);
        for (float d = start; d < len; d += spacing)
        {
            const auto pt = tapeLoop.getPointAlongPath (d);
            g.fillEllipse (pt.x - 1.5f, pt.y - 1.5f, 3.0f, 3.0f);
        }
    }

    // Rollen links/rechts
    auto bounds = tapeLoop.getBounds();
    const float r = bounds.getHeight() * 0.5f;
    for (float cx : { bounds.getX() + r, bounds.getRight() - r })
    {
        const float rr = r * 0.62f;
        g.setColour (juce::Colour (0xff6f6a60));
        g.fillEllipse (cx - rr, bounds.getCentreY() - rr, rr * 2, rr * 2);
        g.setColour (Col::ink);
        g.drawEllipse (cx - rr, bounds.getCentreY() - rr, rr * 2, rr * 2, 1.5f);
        g.setColour (Col::plate.withAlpha (0.8f));
        g.fillEllipse (cx - 4, bounds.getCentreY() - 4, 8, 8);
        const float a = tapeOffset / rr;
        g.setColour (Col::ink);
        g.drawLine (cx, bounds.getCentreY(), cx + rr * 0.9f * std::cos (a), bounds.getCentreY() + rr * 0.9f * std::sin (a), 2.0f);
    }

    // Lösch- und Aufnahmekopf (fest)
    auto staticHead = [&g] (juce::Rectangle<float> hr, const juce::String& name, juce::Colour c)
    {
        g.setColour (c);
        g.fillRoundedRectangle (hr, 3.0f);
        g.setColour (Col::ink);
        g.drawRoundedRectangle (hr, 3.0f, 1.4f);
        g.setColour (Col::plate);
        g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
        g.drawText (name, hr, juce::Justification::centred);
    };
    const auto hb0 = headBtn[0].getBounds().toFloat();
    const float headStep = headBtn[1].getBounds().getX() - headBtn[0].getBounds().getX();
    staticHead (hb0.translated (-2.0f * headStep, 0), "ERASE", juce::Colour (0xff5d5a54));
    staticHead (hb0.translated (-1.0f * headStep, 0), "REC",   juce::Colour (0xffa0372a));

    // Glühen über aktiven Wiedergabeköpfen
    for (int h = 0; h < 4; ++h)
    {
        if (glow[h] < 0.02f) continue;
        auto hb = headBtn[h].getBounds().toFloat();
        const float gr = 18.0f + 14.0f * glow[h];
        juce::ColourGradient gg (Col::orange.withAlpha (0.75f * glow[h]), hb.getCentreX(), hb.getY(),
                                 Col::orange.withAlpha (0.0f), hb.getCentreX() + gr, hb.getY(), true);
        g.setGradientFill (gg);
        g.fillEllipse (hb.getCentreX() - gr, hb.getY() - gr, gr * 2, gr * 2);
    }

    g.setColour (Col::plate.withAlpha (0.55f));
    g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    g.drawText ("PLAYBACK HEADS", headBtn[0].getBounds().getUnion (headBtn[2].getBounds()).translated (0, 32).withHeight (14),
                juce::Justification::centred);
}

void LoopheadAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto full = getLocalBounds().toFloat();
    g.fillAll (Col::body);

    auto plateR = full.reduced (12.0f);
    juce::ColourGradient grad (Col::plate.brighter (0.06f), 0, plateR.getY(),
                               Col::plate.darker (0.06f),   0, plateR.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (plateR, 6.0f);

    // Kopfleiste
    auto header = plateR.removeFromTop (88.0f);
    g.setColour (Col::ink);
    g.fillRoundedRectangle (header, 6.0f);
    g.fillRect (header.withTop (header.getBottom() - 8.0f));
    g.setColour (Col::orange);
    g.fillRect (header.getX(), header.getBottom() - 4.0f, header.getWidth(), 4.0f);

    g.setColour (Col::plate);
    g.setFont (juce::Font (juce::FontOptions (34.0f, juce::Font::bold)));
    g.drawText ("LOOP", (int) header.getX() + 26, (int) header.getY() + 14, 110, 40, juce::Justification::left);
    g.setColour (Col::orange);
    g.drawText ("HEAD", (int) header.getX() + 112, (int) header.getY() + 14, 120, 40, juce::Justification::left);
    g.setColour (Col::plate.withAlpha (0.7f));
    g.setFont (juce::Font (juce::FontOptions (11.5f)));
    g.drawText ("MULTI-HEAD TAPE ECHO  /  REVERSE  /  SPRING", (int) header.getX() + 28, (int) header.getY() + 54,
                360, 16, juce::Justification::left);

    auto drawSection = [&g] (juce::Rectangle<int> r, const juce::String& title)
    {
        auto rf = r.toFloat();
        auto t = juce::Rectangle<float> (rf.getX() + 16.0f, rf.getY() - 9.0f,
                                         (float) title.length() * 9.0f + 16.0f, 18.0f);
        {
            juce::Graphics::ScopedSaveState s (g);
            g.excludeClipRegion (t.toNearestInt());
            g.setColour (Col::ink.withAlpha (0.55f));
            g.drawRoundedRectangle (rf, 5.0f, 1.2f);
        }
        g.setColour (Col::ink);
        g.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::bold)));
        g.drawText (title, t, juce::Justification::centred);
    };
    drawSection (sectionA, "TAPE PATH");
    drawSection (loopSec,  "LOOP");
    drawSection (outSec,   "OUTPUT");

    drawTapePath (g);

    // Beschriftung der Performance-Taster
    g.setColour (Col::ink.withAlpha (0.65f));
    g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    g.drawText ("HALTEN", throwBtn.getBounds().translated (0, throwBtn.getHeight() - 2).withHeight (14), juce::Justification::centred);
    g.drawText ("AN / AUS", freezeBtn.getBounds().translated (0, freezeBtn.getHeight() - 2).withHeight (14), juce::Justification::centred);

    auto screw = [&g] (float cx, float cy)
    {
        g.setColour (Col::plateDark.darker (0.3f));
        g.fillEllipse (cx - 6, cy - 6, 12, 12);
        g.setColour (Col::ink.withAlpha (0.7f));
        g.drawLine (cx - 4, cy - 4, cx + 4, cy + 4, 1.4f);
    };
    const auto pb = getLocalBounds().toFloat().reduced (12.0f);
    screw (pb.getX() + 14, pb.getBottom() - 14);
    screw (pb.getRight() - 14, pb.getBottom() - 14);

    g.setColour (Col::ink.withAlpha (0.5f));
    g.setFont (juce::Font (juce::FontOptions (10.5f)));
    g.drawText ("G AUDIO  /  VINTAGE SERIES", getLocalBounds().withTrimmedBottom (18).removeFromBottom (14),
                juce::Justification::centred);
}

void LoopheadAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    auto header = area.removeFromTop (88);
    presetBox.setBounds (header.removeFromRight (290).reduced (16, 25));

    area.removeFromTop (22);
    area.removeFromBottom (28);
    area.reduce (18, 0);

    sectionA = area.removeFromTop (200);
    area.removeFromTop (22);
    auto row = area.removeFromTop (juce::jmin (area.getHeight(), 156));
    loopSec = row.removeFromLeft (row.getWidth() / 2 - 8);
    row.removeFromLeft (16);
    outSec = row;

    // --- Sektion A: Bandweg | Zeit | Performance ---------------------------
    auto a = sectionA.reduced (14, 18);
    tapeArea = a.removeFromLeft (520);
    a.removeFromLeft (10);
    auto timeCol = a.removeFromLeft (190);
    auto perf = a;

    // Bandschleife (Stadion-Form)
    auto loopR = tapeArea.toFloat().reduced (26.0f, 18.0f).withTrimmedBottom (34.0f);
    tapeLoop.clear();
    tapeLoop.addRoundedRectangle (loopR, loopR.getHeight() * 0.5f);

    // Köpfe sitzen auf der unteren Bandkante: ERASE, REC, 1-4
    const float headW = 44.0f, headH = 30.0f;
    const float x0 = loopR.getX() + loopR.getHeight() * 0.5f + 8.0f;
    const float x1 = loopR.getRight() - loopR.getHeight() * 0.5f - 8.0f - headW;
    const float step = (x1 - x0) / 5.0f;
    for (int h = 0; h < 4; ++h)
        headBtn[h].setBounds ((int) (x0 + step * (float) (h + 2)), (int) (loopR.getBottom() - headH * 0.35f),
                              (int) headW, (int) headH);
    revBtn.setBounds (headBtn[3].getBounds().withSizeKeepingCentre (58, 20).translated (0, (int) headH + 4));

    time.label.setBounds (timeCol.removeFromTop (16));
    syncBtn.setBounds (timeCol.removeFromBottom (24).withSizeKeepingCentre (80, 24));
    timeCol.removeFromBottom (4);
    time.slider.setBounds (timeCol);

    auto halfW = perf.getWidth() / 2;
    throwBtn.setBounds (perf.removeFromLeft (halfW).withSizeKeepingCentre (70, 70).translated (0, -8));
    freezeBtn.setBounds (perf.withSizeKeepingCentre (70, 70).translated (0, -8));

    // --- Knopfreihen --------------------------------------------------------
    auto layoutRow = [] (juce::Rectangle<int> r, std::initializer_list<Knob*> knobs)
    {
        r = r.reduced (6, 14);
        const int w = r.getWidth() / (int) knobs.size();
        for (auto* k : knobs)
        {
            auto cell = r.removeFromLeft (w);
            k->label.setBounds (cell.removeFromTop (16));
            k->slider.setBounds (cell.reduced (2, 0));
        }
    };
    layoutRow (loopSec, { &feedback, &drive, &color, &wobble, &age });
    layoutRow (outSec,  { &spring, &duck, &width, &mix, &output });
}
