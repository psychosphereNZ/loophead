#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Kopfabstand-Basis bei Tempo-Sync (in Vierteln). Kopf n spielt bei n * Basis.
    const float divBeats[] = { 0.125f, 1.0f / 6.0f, 0.25f, 1.0f / 3.0f, 0.375f, 0.5f, 2.0f / 3.0f, 0.75f, 1.0f };

    struct Preset
    {
        const char* name;
        bool sync; int div; float timeMs;
        bool h[4]; bool rev;
        float fb, drive, color, wobble, age, spring, duck, width, mix, out;
    };

    const Preset presets[] =
    {
        //  name              sync div  time    heads                    rev    fb    drive  color  wob   age   spr   duck  width mix   out
        { "Space Station",    false, 5, 110.f, { false, true, false, true }, false, 0.55f, 0.35f,  0.0f, 0.30f, 0.25f, 0.30f, 0.0f, 0.6f, 0.35f, 0.f },
        { "Dub Siren",        true,  7, 150.f, { false, false, true, false }, false, 0.72f, 0.45f, -0.3f, 0.35f, 0.30f, 0.35f, 0.3f, 0.7f, 0.40f, 0.f },
        { "Slapback 50s",     false, 5,  95.f, { true, false, false, false }, false, 0.10f, 0.40f, -0.2f, 0.15f, 0.20f, 0.10f, 0.0f, 0.0f, 0.30f, 0.f },
        { "Triplet Bounce",   true,  3, 150.f, { true, false, true, true },  false, 0.45f, 0.30f,  0.1f, 0.20f, 0.10f, 0.15f, 0.4f, 0.9f, 0.30f, 0.f },
        { "Backwards Dream",  true,  5, 150.f, { false, true, false, true }, true,  0.50f, 0.25f,  0.2f, 0.40f, 0.15f, 0.40f, 0.2f, 0.8f, 0.40f, 0.f },
        { "Runaway",          false, 5, 180.f, { true, true, true, true },   false, 0.95f, 0.60f, -0.4f, 0.50f, 0.40f, 0.25f, 0.0f, 0.7f, 0.45f, -3.f },
        { "Vocal Throw",      true,  5, 150.f, { false, false, false, true }, false, 0.35f, 0.20f,  0.3f, 0.10f, 0.05f, 0.15f, 0.6f, 0.5f, 0.30f, 0.f },
        { "Clean Digital",    true,  5, 150.f, { false, false, false, true }, false, 0.40f, 0.00f,  0.6f, 0.00f, 0.00f, 0.00f, 0.0f, 0.0f, 0.30f, 0.f },
    };
    constexpr int numPresets = (int) (sizeof (presets) / sizeof (presets[0]));

    juce::String pct (float v) { return juce::String (juce::roundToInt (v * 100.f)) + " %"; }
}

const juce::StringArray& LoopheadAudioProcessor::divisionNames()
{
    static const juce::StringArray names { "1/32", "1/16T", "1/16", "1/8T", "1/16D", "1/8", "1/4T", "1/8D", "1/4" };
    return names;
}

//==============================================================================
LoopheadAudioProcessor::LoopheadAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    pTime = apvts.getRawParameterValue ("time");      pSync = apvts.getRawParameterValue ("sync");
    pDiv = apvts.getRawParameterValue ("division");   pRev = apvts.getRawParameterValue ("reverse");
    for (int h = 0; h < numHeads; ++h)
        pHead[h] = apvts.getRawParameterValue ("head" + juce::String (h + 1));
    pFeedback = apvts.getRawParameterValue ("feedback"); pDrive = apvts.getRawParameterValue ("drive");
    pColor = apvts.getRawParameterValue ("color");       pWobble = apvts.getRawParameterValue ("wobble");
    pAge = apvts.getRawParameterValue ("age");           pSpring = apvts.getRawParameterValue ("spring");
    pDuck = apvts.getRawParameterValue ("duck");         pWidth = apvts.getRawParameterValue ("width");
    pMix = apvts.getRawParameterValue ("mix");           pOutput = apvts.getRawParameterValue ("output");
    pThrow = apvts.getRawParameterValue ("throw");       pFreeze = apvts.getRawParameterValue ("freeze");
}

juce::AudioProcessorValueTreeState::ParameterLayout LoopheadAudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using A = juce::AudioParameterFloatAttributes;
    using B = juce::AudioParameterBool;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    const auto& d = presets[0];

    auto pctAttr = A().withStringFromValueFunction ([] (float v, int) { return pct (v); });
    auto unit = [] { return juce::NormalisableRange<float> (0.f, 1.f, 0.001f); };

    l.add (std::make_unique<P> (juce::ParameterID { "time", 1 }, "Head Spacing",
                                juce::NormalisableRange<float> (20.f, 350.f, 0.1f, 0.6f), d.timeMs,
                                A().withStringFromValueFunction ([] (float v, int) { return juce::String (v, 0) + " ms"; })));
    l.add (std::make_unique<B> (juce::ParameterID { "sync", 1 }, "Tempo Sync", d.sync));
    l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "division", 1 }, "Division",
                                                         divisionNames(), d.div));
    for (int h = 0; h < numHeads; ++h)
        l.add (std::make_unique<B> (juce::ParameterID { "head" + juce::String (h + 1), 1 },
                                    "Head " + juce::String (h + 1), d.h[h]));
    l.add (std::make_unique<B> (juce::ParameterID { "reverse", 1 }, "Reverse Head 4", d.rev));
    l.add (std::make_unique<P> (juce::ParameterID { "feedback", 1 }, "Feedback",
                                juce::NormalisableRange<float> (0.f, 1.1f, 0.001f), d.fb, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "drive", 1 }, "Drive", unit(), d.drive, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "color", 1 }, "Color",
                                juce::NormalisableRange<float> (-1.f, 1.f, 0.001f), d.color,
                                A().withStringFromValueFunction ([] (float v, int)
                                {
                                    const int p = juce::roundToInt (v * 100.f);
                                    return p == 0 ? juce::String ("neutral") : (p > 0 ? "hell +" : "dunkel ") + juce::String (std::abs (p));
                                })));
    l.add (std::make_unique<P> (juce::ParameterID { "wobble", 1 }, "Wobble", unit(), d.wobble, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "age", 1 }, "Age", unit(), d.age, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "spring", 1 }, "Spring", unit(), d.spring, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "duck", 1 }, "Duck", unit(), d.duck, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "width", 1 }, "Width", unit(), d.width, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "mix", 1 }, "Mix", unit(), d.mix, pctAttr));
    l.add (std::make_unique<P> (juce::ParameterID { "output", 1 }, "Output",
                                juce::NormalisableRange<float> (-24.f, 12.f, 0.1f), d.out,
                                A().withStringFromValueFunction ([] (float v, int)
                                { return (v > 0 ? "+" : "") + juce::String (v, 1) + " dB"; })));
    l.add (std::make_unique<B> (juce::ParameterID { "throw", 1 }, "Throw", false));
    l.add (std::make_unique<B> (juce::ParameterID { "freeze", 1 }, "Freeze", false));
    return l;
}

//==============================================================================
bool LoopheadAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void LoopheadAudioProcessor::prepareToPlay (double sampleRate, int)
{
    fs = sampleRate;

    // Platz für 4 Köpfe à max. 1 s + Rückwärts-Kopf (bis 2x Kopf 4)
    int size = 1;
    while (size < (int) (9.0 * fs)) size <<= 1;
    tape.assign ((size_t) size, 0.0f);
    mask = size - 1;
    writePos = 0;
    maxDelay = (float) (8.6 * fs);

    for (auto* b : { &loopHP, &loopLP, &ageLP, &hissHP })
        b->reset();
    hissHP.highpass (fs, 2000.0, 0.5);

    spring.prepare (fs);
    spring.setParams (0.55f, 0.75f, 2.2f, 4200.0f);

    baseCur = targetBaseSamples();
    revPhase = 0.0f;
    fbCur = pFeedback->load();
    duckEnv = 0.0f;
    dropGain = dropTarget = 1.0f;
    dropHold = 0;

    mixSm.reset (fs, 0.05);    mixSm.setCurrentAndTargetValue (pMix->load());
    outSm.reset (fs, 0.05);    outSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    springSm.reset (fs, 0.05); springSm.setCurrentAndTargetValue (pSpring->load());
}

float LoopheadAudioProcessor::targetBaseSamples()
{
    float ms = pTime->load();
    if (pSync->load() > 0.5f)
    {
        double bpm = 120.0;
        if (auto* ph = getPlayHead())
            if (auto pos = ph->getPosition())
                if (auto b = pos->getBpm())
                    bpm = juce::jlimit (30.0, 300.0, *b);
        const int div = juce::jlimit (0, 8, (int) pDiv->load());
        ms = (float) (divBeats[div] * 60000.0 / bpm);
    }
    ms = juce::jlimit (10.0f, 1000.0f, ms);
    baseMsDisplay.store (ms);
    return ms * 0.001f * (float) fs;
}

// kubische Hermite-Interpolation
float LoopheadAudioProcessor::readTape (float d) const
{
    d = juce::jlimit (2.0f, maxDelay, d);
    const float pos = (float) writePos - d;
    const int i = (int) std::floor (pos);
    const float f = pos - (float) i;
    const float xm1 = tape[(size_t) ((i - 1) & mask)], x0 = tape[(size_t) (i & mask)];
    const float x1  = tape[(size_t) ((i + 1) & mask)], x2 = tape[(size_t) ((i + 2) & mask)];
    const float c1 = 0.5f * (x1 - xm1);
    const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
    const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
    return ((c3 * f + c2) * f + c1) * f + x0;
}

void LoopheadAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (buffer.getNumChannels(), getTotalNumOutputChannels());
    const int n = buffer.getNumSamples();
    if (numCh == 0 || n == 0)
        return;

    // --- Parameter pro Block ------------------------------------------------
    const float baseTarget = targetBaseSamples();
    bool on[numHeads];
    int numOn = 0;
    for (int h = 0; h < numHeads; ++h) { on[h] = pHead[h]->load() > 0.5f; numOn += on[h] ? 1 : 0; }
    const float fbNorm = 1.0f / (float) juce::jmax (1, numOn);
    const bool reverse = pRev->load() > 0.5f;
    const bool freeze  = pFreeze->load() > 0.5f;
    const bool throwOn = pThrow->load() > 0.5f;

    const float fbTarget = throwOn ? juce::jmax (pFeedback->load(), 1.4f) : pFeedback->load();
    const float drive = pDrive->load(), color = pColor->load(), wobble = pWobble->load(), age = pAge->load();
    const float duckAmt = pDuck->load(), width = pWidth->load();
    const float k = 1.0f + drive * 4.0f;

    loopHP.highpass (fs, 110.0 * std::pow (2.0, -color * 0.8), 0.7);
    loopLP.lowpass  (fs, 3200.0 * std::pow (2.0, color * 1.7), 0.6);
    ageLP.lowpass   (fs, 16000.0 * (1.0 - age * 0.75), 0.6);

    mixSm.setTargetValue (pMix->load());
    outSm.setTargetValue (juce::Decibels::decibelsToGain (pOutput->load()));
    springSm.setTargetValue (pSpring->load());

    const float inertiaC = 1.0f - std::exp (-1.0f / (0.3f * (float) fs));    // Motor-Trägheit
    const float fbC      = 1.0f - std::exp (-1.0f / (0.12f * (float) fs));
    const float duckAtt  = 1.0f - std::exp (-1.0f / (0.005f * (float) fs));
    const float duckRel  = 1.0f - std::exp (-1.0f / (0.25f * (float) fs));
    const float wowDepth = std::pow (wobble, 1.5f) * 0.0035f * (float) fs;
    const float flDepth  = std::pow (wobble, 2.0f) * 0.00015f * (float) fs;
    const double twoPi = juce::MathConstants<double>::twoPi;
    const double wowInc = twoPi * 0.5 / fs, wowInc2 = twoPi * 0.21 / fs, flInc = twoPi * 9.0 / fs;
    const float hissAmp = age * age * 0.012f;
    const float dropProb = age * age * 1.5f / (float) fs;
    const float dropSm = 1.0f - std::exp (-1.0f / (0.015f * (float) fs));

    // Panorama der Köpfe: 1 links, 2 rechts, 3 halblinks, 4 halbrechts
    const float pans[numHeads] = { -1.0f, 1.0f, -0.5f, 0.5f };
    float gL[numHeads], gR[numHeads];
    for (int h = 0; h < numHeads; ++h)
    {
        const float p = pans[h] * width;
        gL[h] = std::sqrt (0.5f * (1.0f - p));
        gR[h] = std::sqrt (0.5f * (1.0f + p));
    }

    float lvl[numHeads] = {};
    float speedAcc = 0.0f;

    auto* L = buffer.getWritePointer (0);
    auto* R = numCh > 1 ? buffer.getWritePointer (1) : nullptr;

    for (int i = 0; i < n; ++i)
    {
        const float dryL = L[i], dryR = R != nullptr ? R[i] : L[i];
        const float in = 0.5f * (dryL + dryR);

        // Motor-Trägheit: Kopfabstand folgt langsam -> Tonhöhen-Bends beim Drehen
        const float prevBase = baseCur;
        baseCur += (baseTarget - baseCur) * inertiaC;
        speedAcc += 1.0f - (baseCur - prevBase);   // relative Bandgeschwindigkeit
        fbCur += (fbTarget - fbCur) * fbC;

        // Wow & Flutter
        wowPhase += wowInc;   if (wowPhase  > twoPi) wowPhase  -= twoPi;
        wowPhase2 += wowInc2; if (wowPhase2 > twoPi) wowPhase2 -= twoPi;
        flPhase += flInc;     if (flPhase   > twoPi) flPhase   -= twoPi;
        if (rng.nextFloat() < 0.0004f) driftTarget = rng.nextFloat();
        drift += (driftTarget - drift) * 0.00005f;
        const float mod = wowDepth * (0.6f * (0.5f + 0.5f * (float) std::sin (wowPhase))
                                      + 0.25f * (0.5f + 0.5f * (float) std::sin (wowPhase2)) + 0.3f * drift)
                        + flDepth * (1.0f + (float) std::sin (flPhase));

        // --- Wiedergabeköpfe ---------------------------------------------------
        float head[numHeads];
        for (int h = 0; h < numHeads; ++h)
            head[h] = readTape (baseCur * (float) (h + 1) + mod);

        if (reverse)
        {
            // Rückwärts-Kopf: zwei überlappende Grains, die das Band rückwärts lesen
            const float C = juce::jmax (64.0f, baseCur * 4.0f);
            revPhase += 1.0f;
            if (revPhase >= C) revPhase -= C;
            float pB = revPhase + 0.5f * C;
            if (pB >= C) pB -= C;
            const float wA = std::sin (juce::MathConstants<float>::pi * revPhase / C);
            const float wB = std::sin (juce::MathConstants<float>::pi * pB / C);
            head[3] = readTape (2.0f * revPhase + 2.0f + mod) * wA * wA
                    + readTape (2.0f * pB + 2.0f + mod) * wB * wB;
        }

        float fbSum = 0.0f, wetL = 0.0f, wetR = 0.0f;
        for (int h = 0; h < numHeads; ++h)
        {
            if (! on[h]) continue;
            fbSum += head[h];
            wetL += head[h] * gL[h];
            wetR += head[h] * gR[h];
            lvl[h] = juce::jmax (lvl[h], std::abs (head[h]));
        }

        // --- Aufnahme ----------------------------------------------------------
        float rec;
        if (freeze)
        {
            // Endlosschleife: Eingang getrennt, Kopf 4 wird verlustfrei zurückgespielt
            rec = readTape (baseCur * 4.0f);
        }
        else
        {
            float x = in + fbSum * fbNorm * fbCur;
            x = loopHP.process (x, 0);
            x = loopLP.process (x, 0);
            x = ageLP.process (x, 0);
            rec = std::tanh (x * k) / k;     // Sättigung begrenzt auch die Eigenschwingung
        }
        tape[(size_t) writePos] = rec;
        writePos = (writePos + 1) & mask;

        // --- Ausgang: Federhall, Rauschen, Dropouts, Ducking ---------------------
        const float sAmt = springSm.getNextValue();
        if (sAmt > 0.001f)
        {
            float sl, sr;
            spring.process (0.5f * (wetL + wetR), sl, sr);
            wetL += sl * sAmt * 0.8f;
            wetR += sr * sAmt * 0.8f;
        }
        if (hissAmp > 0.0f)
        {
            wetL += hissHP.process (rng.nextFloat() * 2.0f - 1.0f, 0) * hissAmp;
            wetR += hissHP.process (rng.nextFloat() * 2.0f - 1.0f, 1) * hissAmp;
        }

        if (dropHold > 0) { if (--dropHold == 0) dropTarget = 1.0f; }
        else if (dropProb > 0.0f && rng.nextFloat() < dropProb)
        {
            dropTarget = 1.0f - age * (0.3f + 0.5f * rng.nextFloat());
            dropHold = (int) ((0.02f + 0.08f * rng.nextFloat()) * (float) fs);
        }
        dropGain += (dropTarget - dropGain) * dropSm;

        const float lev = std::abs (in);
        duckEnv += (lev - duckEnv) * (lev > duckEnv ? duckAtt : duckRel);
        const float duckGain = 1.0f - duckAmt * juce::jmin (1.0f, duckEnv * 4.0f) * 0.9f;

        // Mix: Dry bleibt bis 50 % voll, Wet ab 50 % voll
        const float m = mixSm.getNextValue();
        const float dG = juce::jmin (1.0f, 2.0f * (1.0f - m));
        const float wG = juce::jmin (1.0f, 2.0f * m) * duckGain * dropGain;
        const float og = outSm.getNextValue();

        L[i] = (dryL * dG + wetL * wG) * og;
        if (R != nullptr) R[i] = (dryR * dG + wetR * wG) * og;
    }

    for (int h = 0; h < numHeads; ++h)
        headLevel[(size_t) h].store (on[h] ? lvl[h] : 0.0f);
    tapeSpeed.store (juce::jlimit (0.0f, 4.0f, speedAcc / (float) n));
}

//==============================================================================
int LoopheadAudioProcessor::getNumPrograms() { return numPresets; }

const juce::String LoopheadAudioProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, numPresets) ? juce::String (presets[index].name) : juce::String();
}

void LoopheadAudioProcessor::setCurrentProgram (int index)
{
    if (! juce::isPositiveAndBelow (index, numPresets))
        return;

    currentProgram = index;
    const auto& p = presets[index];
    auto set = [this] (const juce::String& id, float v)
    {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (v));
    };
    set ("sync", p.sync ? 1.f : 0.f);  set ("division", (float) p.div); set ("time", p.timeMs);
    for (int h = 0; h < numHeads; ++h)
        set ("head" + juce::String (h + 1), p.h[h] ? 1.f : 0.f);
    set ("reverse", p.rev ? 1.f : 0.f);
    set ("feedback", p.fb);  set ("drive", p.drive);   set ("color", p.color);
    set ("wobble", p.wobble); set ("age", p.age);      set ("spring", p.spring);
    set ("duck", p.duck);    set ("width", p.width);   set ("mix", p.mix);
    set ("output", p.out);   set ("freeze", 0.f);      set ("throw", 0.f);
}

//==============================================================================
void LoopheadAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void LoopheadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = juce::ValueTree::fromXml (*xml);
            currentProgram = juce::jlimit (0, numPresets - 1, (int) tree.getProperty ("program", 0));
            apvts.replaceState (tree);
        }
}

juce::AudioProcessorEditor* LoopheadAudioProcessor::createEditor()
{
    return new LoopheadAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LoopheadAudioProcessor();
}
