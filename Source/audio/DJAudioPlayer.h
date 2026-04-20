#pragma once

#include <JuceHeader.h>
#include "../shared/DeckState.h"
#include <array>

class DJAudioPlayer : public juce::AudioSource
{
public:
    DJAudioPlayer(juce::AudioFormatManager& formatManager);
    ~DJAudioPlayer() override;

    // AudioSource
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Transport
    void loadURL(juce::URL audioURL);
    void start();
    void stop();

    // Gain / speed / position
    void   setGain(double gain);
    void   setTrimGain(double trim);
    void   setCrossfaderGain(double gain);
    void   setSpeed(double ratio);
    void   setPosition(double posInSecs);
    void   setPositionRelative(double pos);

    double getPositionRelative() const;
    double getCurrentPositionSeconds() const;
    double getTotalLength() const;

    // Loop (M2)
    void setLoopIn();
    void setLoopOut();
    void clearLoop();
    bool isLoopEnabled() const;
    void checkAndLoopIfNeeded();
    // Set loop of exactly durationSeconds starting at current position
    void setLoopFromCurrentPosition(double durationSeconds);

    // Whole-track repeat
    void setTrackLoopEnabled(bool enabled);
    bool isTrackLoopEnabled() const;

    // Cue (M2)
    void   setCuePoint();
    void   jumpToCue();
    double getCuePoint() const;

    // Hotcues: 8 slots per deck
    static constexpr int kNumHotcues = 8;
    void   setHotcue(int index);           // set hotcue[index] = current position
    void   jumpToHotcue(int index);        // seek to hotcue[index]
    void   clearHotcue(int index);         // clear hotcue[index]
    double getHotcue(int index) const;     // returns -1 if not set
    bool   isHotcueSet(int index) const;

    // EQ (M3) — dB, range [-12, +12]
    void setEQLow (double dB);
    void setEQMid (double dB);
    void setEQHigh(double dB);
    void setEQLowKill (bool enabled);
    void setEQMidKill (bool enabled);
    void setEQHighKill(bool enabled);
    bool isEQLowKilled()  const;
    bool isEQMidKilled()  const;
    bool isEQHighKilled() const;

    // Level metering (M3)
    float getRMSLevel() const;

    // BPM (M5)
    double getBPM() const;
    void   setBPM(double bpm);

    // Effects (M6)
    void setLowPassEnabled (bool enabled);
    void setHighPassEnabled(bool enabled);
    bool isLowPassEnabled()  const;
    bool isHighPassEnabled() const;

    void setDelayEnabled(bool enabled);
    bool isDelayEnabled() const;
    void setDelayTime(double seconds);
    void setDelayFeedback(double feedback);   // [0, 0.9]
    void setDelayWetDry(double wet);          // [0, 1]

    // State
    DeckState getState() const;

private:
    void applyEffectiveGain();
    void updateEQCoefficients();
    void updateFilterCoefficients();

    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource  transportSource;
    juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };

    // Gain
    double userGain_       = 1.0;
    double trimGain_       = 1.0;
    double crossfaderGain_ = 1.0;
    double currentSpeed_   = 1.0;
    bool   isLoaded_       = false;

    // M6: smooth gain / speed changes to avoid zipper noise
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothGain_;
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Linear> smoothSpeed_;

    // Loop
    double loopStart_   = 0.0;
    double loopEnd_     = 0.0;
    bool   loopEnabled_ = false;
    bool   trackLoopEnabled_ = false;

    // Cue
    double cuePoint_ = 0.0;

    // EQ
    double eqLowDb_  = 0.0;
    double eqMidDb_  = 0.0;
    double eqHighDb_ = 0.0;
    bool   eqLowKill_  = false;
    bool   eqMidKill_  = false;
    bool   eqHighKill_ = false;
    double sampleRate_ = 44100.0;
    std::atomic<bool> eqDirty_{ false };

    juce::IIRFilter lowCutFilters_[2];
    juce::IIRFilter lowBoostFilters_[2];
    juce::IIRFilter midFilters_[2];
    juce::IIRFilter highFilters_[2];

    // M6: LP / HP filters
    bool   lpfEnabled_ = false;
    bool   hpfEnabled_ = false;
    std::atomic<bool> filterDirty_{ false };
    juce::IIRFilter lpFilters_[2];
    juce::IIRFilter hpFilters_[2];
    juce::IIRFilter highCutFilters_[2];

    // M6: delay
    bool   delayEnabled_   = false;
    double delayTime_      = 0.3;
    double delayFeedback_  = 0.4;
    double delayWet_       = 0.3;
    std::vector<float> delayBuffer_[2];
    int    delayWritePos_  = 0;

    // Metering
    std::atomic<float> rmsLevel_{ 0.0f };

    // BPM
    std::atomic<double> bpm_{ 0.0 };

    // Hotcues
    std::array<double, kNumHotcues> hotcues_{ -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DJAudioPlayer)
};
