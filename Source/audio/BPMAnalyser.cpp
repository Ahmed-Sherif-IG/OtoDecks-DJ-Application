#include "BPMAnalyser.h"
#include <numeric>
#include <cmath>

BPMAnalyser::BPMAnalyser() : juce::Thread("BPMAnalyser") {}

BPMAnalyser::~BPMAnalyser()
{
    cancel();
}

//==============================================================================
void BPMAnalyser::analyse(const juce::File& file,
                           juce::AudioFormatManager& formatManager)
{
    cancel();
    file_          = file;
    formatManager_ = &formatManager;
    startThread(juce::Thread::Priority::background);
}

void BPMAnalyser::cancel()
{
    signalThreadShouldExit();
    stopThread(2000);
}

//==============================================================================
void BPMAnalyser::run()
{
    if (formatManager_ == nullptr || !file_.existsAsFile())
    {
        juce::MessageManager::callAsync([this]() { if (onResult) onResult(0.0); });
        return;
    }

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager_->createReaderFor(file_));

    if (reader == nullptr)
    {
        juce::MessageManager::callAsync([this]() { if (onResult) onResult(0.0); });
        return;
    }

    const double sampleRate   = reader->sampleRate;
    const int    analysisLen  = static_cast<int>(juce::jmin(
                                    (double)reader->lengthInSamples,
                                    sampleRate * 60.0));   // analyse first 60s max
    const int    blockSize    = 512;

    juce::AudioBuffer<float> buffer(1, blockSize);
    std::vector<float> monoSamples;
    monoSamples.reserve(static_cast<size_t>(analysisLen));

    for (int pos = 0; pos < analysisLen && !threadShouldExit(); pos += blockSize)
    {
        int toRead = juce::jmin(blockSize, analysisLen - pos);
        buffer.clear();
        reader->read(&buffer, 0, toRead, static_cast<juce::int64>(pos), true, true);

        auto* data = buffer.getReadPointer(0);
        for (int i = 0; i < toRead; ++i)
            monoSamples.push_back(data[i]);
    }

    if (threadShouldExit()) return;

    double bpm = estimateBPM(monoSamples, sampleRate);

    juce::MessageManager::callAsync([this, bpm]()
    {
        if (onResult) onResult(bpm);
    });
}

//==============================================================================
double BPMAnalyser::estimateBPM(const std::vector<float>& samples,
                                 double sampleRate)
{
    if (samples.empty()) return 0.0;

    // Energy-based onset detection:
    // 1. Compute RMS energy in short windows
    // 2. Detect sudden energy increases (onsets)
    // 3. Estimate inter-onset interval → BPM

    const int windowSize = static_cast<int>(sampleRate * 0.010); // 10ms windows
    if (windowSize <= 0) return 0.0;

    std::vector<float> energy;
    energy.reserve(samples.size() / static_cast<size_t>(windowSize));

    for (size_t i = 0; i + static_cast<size_t>(windowSize) < samples.size();
         i += static_cast<size_t>(windowSize))
    {
        float sum = 0.0f;
        for (int k = 0; k < windowSize; ++k)
            sum += samples[i + static_cast<size_t>(k)] * samples[i + static_cast<size_t>(k)];
        energy.push_back(std::sqrt(sum / static_cast<float>(windowSize)));
    }

    if (energy.size() < 8) return 0.0;

    // Onset detection: local energy increase above running average
    const int avgWindow = 20;
    std::vector<double> onsetTimes;

    for (size_t i = static_cast<size_t>(avgWindow); i < energy.size(); ++i)
    {
        float avg = 0.0f;
        for (int k = 0; k < avgWindow; ++k)
            avg += energy[i - static_cast<size_t>(k) - 1];
        avg /= static_cast<float>(avgWindow);

        if (energy[i] > avg * 1.5f && energy[i] > 0.01f)
        {
            double t = (static_cast<double>(i) * windowSize) / sampleRate;
            // Enforce minimum gap between onsets (avoid double-triggers)
            if (onsetTimes.empty() || (t - onsetTimes.back()) > 0.15)
                onsetTimes.push_back(t);
        }
    }

    if (onsetTimes.size() < 4) return 0.0;

    // Collect inter-onset intervals
    std::vector<double> intervals;
    intervals.reserve(onsetTimes.size() - 1);
    for (size_t i = 1; i < onsetTimes.size(); ++i)
        intervals.push_back(onsetTimes[i] - onsetTimes[i - 1]);

    // Median interval
    std::sort(intervals.begin(), intervals.end());
    double median = intervals[intervals.size() / 2];

    if (median <= 0.0) return 0.0;

    double rawBPM = 60.0 / median;

    // Fold into [60, 180] BPM range
    while (rawBPM < 60.0)  rawBPM *= 2.0;
    while (rawBPM > 180.0) rawBPM /= 2.0;

    return std::round(rawBPM * 10.0) / 10.0;  // one decimal place
}
