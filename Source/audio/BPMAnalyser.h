#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>

/**
 * Background-thread energy-based BPM analyser.
 *
 * Usage:
 *   BPMAnalyser analyser;
 *   analyser.onResult = [](double bpm){ ... };
 *   analyser.analyse(audioFile, formatManager);
 *
 * The result callback is invoked on the message thread when analysis completes.
 * Call cancel() before replacing or destroying an in-progress analysis.
 */
class BPMAnalyser : private juce::Thread
{
public:
    BPMAnalyser();
    ~BPMAnalyser() override;

    // Starts analysis on a background thread. Safe to call repeatedly;
    // cancels any in-progress run first.
    void analyse(const juce::File& file,
                 juce::AudioFormatManager& formatManager);

    void cancel();

    // Invoked on the message thread with the estimated BPM (0 = detection failed)
    std::function<void(double bpm)> onResult;

private:
    void run() override;

    static double estimateBPM(const std::vector<float>& samples,
                               double sampleRate);

    juce::File               file_;
    juce::AudioFormatManager* formatManager_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BPMAnalyser)
};
