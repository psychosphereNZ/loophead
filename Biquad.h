#pragma once

#include <juce_core/juce_core.h>
#include <cmath>
#include <vector>

//==============================================================================
// Biquad (RBJ-Kochbuch), Transposed Direct Form II, Stereo, ohne Allokation
//==============================================================================
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1[2] {}, z2[2] {};

    void reset() { z1[0] = z1[1] = z2[0] = z2[1] = 0.0f; }

    float process (float x, int ch)
    {
        const float y = b0 * x + z1[ch];
        z1[ch] = b1 * x - a1 * y + z2[ch];
        z2[ch] = b2 * x - a2 * y;
        return y;
    }

    void setRaw (double nb0, double nb1, double nb2, double na0, double na1, double na2)
    {
        b0 = (float) (nb0 / na0); b1 = (float) (nb1 / na0); b2 = (float) (nb2 / na0);
        a1 = (float) (na1 / na0); a2 = (float) (na2 / na0);
    }

    static double w (double f, double fs) { return juce::MathConstants<double>::twoPi * juce::jlimit (5.0, fs * 0.49, f) / fs; }

    void lowpass (double fs, double f, double q)
    {
        const double w0 = w (f, fs), cs = std::cos (w0), al = std::sin (w0) / (2.0 * q);
        setRaw ((1 - cs) / 2, 1 - cs, (1 - cs) / 2, 1 + al, -2 * cs, 1 - al);
    }
    void highpass (double fs, double f, double q)
    {
        const double w0 = w (f, fs), cs = std::cos (w0), al = std::sin (w0) / (2.0 * q);
        setRaw ((1 + cs) / 2, -(1 + cs), (1 + cs) / 2, 1 + al, -2 * cs, 1 - al);
    }
    void peak (double fs, double f, double q, double gainDb)
    {
        const double w0 = w (f, fs), cs = std::cos (w0), al = std::sin (w0) / (2.0 * q);
        const double A = std::pow (10.0, gainDb / 40.0);
        setRaw (1 + al * A, -2 * cs, 1 - al * A, 1 + al / A, -2 * cs, 1 - al / A);
    }
    void highShelf (double fs, double f, double gainDb)
    {
        const double w0 = w (f, fs), cs = std::cos (w0), al = std::sin (w0) / (2.0 * 0.707);
        const double A = std::pow (10.0, gainDb / 40.0), sq = 2.0 * std::sqrt (A) * al;
        setRaw (A * ((A + 1) + (A - 1) * cs + sq), -2 * A * ((A - 1) + (A + 1) * cs), A * ((A + 1) + (A - 1) * cs - sq),
                (A + 1) - (A - 1) * cs + sq, 2 * ((A - 1) - (A + 1) * cs), (A + 1) - (A - 1) * cs - sq);
    }
};

