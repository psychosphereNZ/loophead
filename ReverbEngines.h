#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <cmath>

//==============================================================================
// Grundbausteine
//==============================================================================
struct DelayLine
{
    std::vector<float> buf;
    int mask = 0, w = 0;

    void init (int maxLen)
    {
        int size = 1;
        while (size < maxLen + 4) size <<= 1;
        buf.assign ((size_t) size, 0.0f);
        mask = size - 1;
        w = 0;
    }
    void clear() { std::fill (buf.begin(), buf.end(), 0.0f); }
    void push (float x) { buf[(size_t) w] = x; w = (w + 1) & mask; }

    // d = 0 -> zuletzt geschriebenes Sample
    float tap (int d) const { return buf[(size_t) ((w - 1 - d) & mask)]; }

    float tapFrac (float d) const
    {
        const int i = (int) d;
        const float f = d - (float) i;
        return tap (i) + f * (tap (i + 1) - tap (i));
    }
};

// Schroeder-Allpass: w[n] = x + g*w[n-M],  y = -g*w[n] + w[n-M]
struct AllpassDelay
{
    DelayLine line;
    void init (int maxLen) { line.init (maxLen); }
    void clear() { line.clear(); }

    float process (float x, float len, float g)
    {
        const float wm = line.tapFrac (juce::jmax (0.0f, len - 1.0f));
        const float wn = x + g * wm;
        line.push (wn);
        return -g * wn + wm;
    }
    float tap (int d) const { return line.tap (d); }
};

struct OnePoleLP
{
    float z = 0.0f;
    float process (float x, float c) { z += c * (x - z); return z; }
    void clear() { z = 0.0f; }
    static float coef (float hz, double fs)
    {
        return 1.0f - std::exp (-juce::MathConstants<float>::twoPi * hz / (float) fs);
    }
};

//==============================================================================
// FEDERHALL
// Drei parallele "Federn". Jede Feder ist eine Rückkopplungsschleife aus
// einer Kaskade von Allpässen 1. Ordnung (Dispersion -> typisches "Drip"/Chirp),
// einer leicht modulierten Verzögerung und einem Tiefpass.
//==============================================================================
class SpringTank
{
public:
    static constexpr int numSprings = 3;
    static constexpr int numStages  = 90;

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        for (auto& s : springs)
        {
            s.delay.init ((int) (0.09 * fs));
            std::fill (std::begin (s.ap), std::end (s.ap), 0.0f);
            s.lp.clear();
            s.last = 0.0f;
        }
        for (int i = 0; i < numSprings; ++i)
            springs[i].lfoPhase = (float) i * 2.1f;
    }

    void clear() { prepare (fs); }

    // tension 0..1, drip 0..1, decaySec, toneHz
    void setParams (float tension, float drip, float decaySec, float toneHz)
    {
        const float scale = 1.45f - tension * 0.8f;               // höhere Spannung = kürzere Laufzeit
        const float base[numSprings] = { 0.0331f, 0.0389f, 0.0457f };
        apCoef = -(0.42f + drip * 0.42f);                          // stärker negativ = mehr Dispersion
        toneC  = OnePoleLP::coef (toneHz, fs);

        for (int i = 0; i < numSprings; ++i)
        {
            auto& s = springs[i];
            s.len = (float) (base[i] * scale * fs);
            // Gruppenlaufzeit der Allpass-Kaskade bei tiefen Frequenzen grob einrechnen
            const float apDelay = numStages * (1.0f - apCoef) / (1.0f + apCoef);
            const float loopSec = (s.len + apDelay * 0.5f) / (float) fs;
            s.fb = juce::jmin (0.985f, std::pow (10.0f, -3.0f * loopSec / juce::jmax (0.1f, decaySec)));
        }
    }

    void process (float in, float& outL, float& outR)
    {
        float o[numSprings];
        const float lfoInc = juce::MathConstants<float>::twoPi * 0.45f / (float) fs;

        for (int i = 0; i < numSprings; ++i)
        {
            auto& s = springs[i];
            float x = in * 0.6f + s.fb * s.last;

            // Dispersion: Kaskade von Allpässen 1. Ordnung (transponierte Form)
            const float a = apCoef;
            for (int k = 0; k < numStages; ++k)
            {
                const float y = a * x + s.ap[k];
                s.ap[k] = x - a * y;
                x = y;
            }

            s.delay.push (x);
            s.lfoPhase += lfoInc * (1.0f + 0.13f * (float) i);
            if (s.lfoPhase > juce::MathConstants<float>::twoPi) s.lfoPhase -= juce::MathConstants<float>::twoPi;
            const float mod = 1.0f + 0.004f * std::sin (s.lfoPhase);
            float y = s.delay.tapFrac (s.len * mod);

            y = s.lp.process (y, toneC);
            s.last = std::tanh (y);      // weiche Begrenzung wie bei echten Federn/Röhrentreibern
            o[i] = s.last;
        }

        outL = (o[0] + 0.6f * o[1]) * 2.2f;
        outR = (o[2] + 0.6f * o[1]) * 2.2f;
    }

private:
    struct Spring
    {
        DelayLine delay;
        float ap[numStages] {};
        OnePoleLP lp;
        float last = 0.0f, len = 1000.0f, fb = 0.5f, lfoPhase = 0.0f;
    };

    Spring springs[numSprings];
    double fs = 44100.0;
    float apCoef = -0.6f, toneC = 0.3f;
};

//==============================================================================
// PLATTENHALL nach Dattorro (1997), skaliert auf die Host-Samplerate.
//==============================================================================
class PlateTank
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        r = (float) (fs / 29761.0);
        const float maxScale = 1.5f;
        auto len = [this, maxScale] (float n) { return (int) (n * r * maxScale) + 64; };

        in1.init (len (142)); in2.init (len (107)); in3.init (len (379)); in4.init (len (277));
        apL1.init (len (672 + 40)); apR1.init (len (908 + 40));
        dL1.init (len (4453)); dR1.init (len (4217));
        apL2.init (len (1800)); apR2.init (len (2656));
        dL2.init (len (3720)); dR2.init (len (3163));
        bw.clear(); dampL.clear(); dampR.clear();
        lastL = lastR = 0.0f;
        lfo = 0.0f;
    }

    void clear() { prepare (fs); }

    void setParams (float size, float mod, float decaySec, float toneHz)
    {
        scale  = 0.6f + size * 0.85f;
        modAmt = (4.0f + mod * 28.0f) * r;
        dampC  = OnePoleLP::coef (toneHz, fs);
        const float loopSec = 21589.0f * scale / 29761.0f;
        decay = juce::jlimit (0.0f, 0.97f, std::pow (10.0f, -3.0f * loopSec / (4.0f * 1.35f * juce::jmax (0.1f, decaySec))));
    }

    void process (float in, float& outL, float& outR)
    {
        auto L = [this] (float n) { return n * r * scale; };

        float x = bw.process (in, 0.9995f);
        x = in1.process (x, L (142), 0.75f);
        x = in2.process (x, L (107), 0.75f);
        x = in3.process (x, L (379), 0.625f);
        x = in4.process (x, L (277), 0.625f);

        lfo += juce::MathConstants<float>::twoPi * 0.9f / (float) fs;
        if (lfo > juce::MathConstants<float>::twoPi) lfo -= juce::MathConstants<float>::twoPi;
        const float mL = modAmt * std::sin (lfo);
        const float mR = modAmt * std::cos (lfo * 0.93f);

        // linke Tankhälfte
        float a = apL1.process (x + decay * lastR, L (672) + mL, -0.7f);
        dL1.push (a);
        float t = dampL.process (dL1.tap ((int) L (4453)), dampC) * decay;
        t = apL2.process (t, L (1800), 0.5f);
        dL2.push (t);
        const float newL = dL2.tap ((int) L (3720)) * decay;

        // rechte Tankhälfte
        float b = apR1.process (x + decay * lastL, L (908) + mR, -0.7f);
        dR1.push (b);
        float u = dampR.process (dR1.tap ((int) L (4217)), dampC) * decay;
        u = apR2.process (u, L (2656), 0.5f);
        dR2.push (u);
        const float newR = dR2.tap ((int) L (3163)) * decay;

        lastL = newL; lastR = newR;

        auto T = [this] (float n) { return (int) (n * r * scale); };
        outL = 0.6f * ( dR1.tap (T (266)) + dR1.tap (T (2974)) - apR2.tap (T (1913)) + dR2.tap (T (1996))
                      - dL1.tap (T (1990)) - apL2.tap (T (187)) - dL2.tap (T (1066)));
        outR = 0.6f * ( dL1.tap (T (353)) + dL1.tap (T (3627)) - apL2.tap (T (1228)) + dL2.tap (T (2673))
                      - dR1.tap (T (2111)) - apR2.tap (T (335)) - dR2.tap (T (121)));
    }

private:
    double fs = 44100.0;
    float r = 1.0f, scale = 1.0f, modAmt = 16.0f, dampC = 0.5f, decay = 0.5f;
    float lastL = 0.0f, lastR = 0.0f, lfo = 0.0f;

    OnePoleLP bw, dampL, dampR;
    AllpassDelay in1, in2, in3, in4, apL1, apR1, apL2, apR2;
    DelayLine dL1, dR1, dL2, dR2;
};
