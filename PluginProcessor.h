#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Biquad.h"
#include "ReverbEngines.h"

//==============================================================================
// LOOPHEAD – Multi-Head Tape Echo
//
//   Eingang ─┬──────────────────────────────────────────────► Dry
//            └► Aufnahmekopf ─► [ Bandschleife ] ─► Kopf 1..4 ─┬► Pan/Width ─► Federhall ─► Ducking ─► Wet
//                     ▲            (Wow/Flutter,  (Kopf 4 opt.  │
//                     │             Motor-Trägheit) rückwärts)  │
//                     └── Sättigung ◄ Color ◄ Low-Cut ◄ Feedback ◄┘
//
// Besonderheiten: 4 schaltbare Köpfe, Reverse-Kopf, Motor-Trägheit (Pitch-Bends
// beim Zeit-Drehen), THROW (Eigenschwingung auf Knopfdruck), FREEZE (Endlosschleife),
// eingebauter Federhall, Ducking, Tempo-Sync.
//==============================================================================
class LoopheadAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int numHeads = 4;

    LoopheadAudioProcessor();
    ~LoopheadAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Für die Anzeige
    float getHeadLevel (int h) const { return headLevel[(size_t) h].load(); }
    float getTapeSpeed() const       { return tapeSpeed.load(); }   // 1 = Grundgeschwindigkeit
    float getBaseMs() const          { return baseMsDisplay.load(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    static const juce::StringArray& divisionNames();
    juce::AudioProcessorValueTreeState apvts;

private:
    float readTape (float delaySamples) const;
    float targetBaseSamples();

    double fs = 44100.0;
    int currentProgram = 0;

    // Bandschleife (mono)
    std::vector<float> tape;
    int mask = 0, writePos = 0;
    float maxDelay = 1.0f;

    float baseCur = 1000.0f;              // aktuelle Kopfabstand-Basis in Samples (mit Trägheit)
    float revPhase = 0.0f;
    float fbCur = 0.0f;

    Biquad loopHP, loopLP, ageLP, hissHP;
    SpringTank spring;
    juce::Random rng;

    double wowPhase = 0.0, wowPhase2 = 0.0, flPhase = 0.0;
    float drift = 0.0f, driftTarget = 0.0f;
    float duckEnv = 0.0f;
    float dropGain = 1.0f, dropTarget = 1.0f;
    int dropHold = 0;

    juce::SmoothedValue<float> mixSm, outSm, springSm;

    std::array<std::atomic<float>, numHeads> headLevel {};
    std::atomic<float> tapeSpeed { 1.0f }, baseMsDisplay { 100.0f };

    std::atomic<float>* pTime = nullptr;   std::atomic<float>* pSync = nullptr;
    std::atomic<float>* pDiv = nullptr;    std::atomic<float>* pHead[numHeads] {};
    std::atomic<float>* pRev = nullptr;    std::atomic<float>* pFeedback = nullptr;
    std::atomic<float>* pDrive = nullptr;  std::atomic<float>* pColor = nullptr;
    std::atomic<float>* pWobble = nullptr; std::atomic<float>* pAge = nullptr;
    std::atomic<float>* pSpring = nullptr; std::atomic<float>* pDuck = nullptr;
    std::atomic<float>* pWidth = nullptr;  std::atomic<float>* pMix = nullptr;
    std::atomic<float>* pOutput = nullptr; std::atomic<float>* pThrow = nullptr;
    std::atomic<float>* pFreeze = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopheadAudioProcessor)
};
