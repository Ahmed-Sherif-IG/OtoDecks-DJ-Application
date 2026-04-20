#include "BPMAnalyser.h"
#include <algorithm>
#include <cmath>
#include <numeric>

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
                                    sampleRate * 60.0));
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

    const int windowSize = static_cast<int>(sampleRate * 0.020);
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

    std::vector<float> flux;
    flux.reserve(energy.size());
    flux.push_back(0.0f);
    for (size_t i = 1; i < energy.size(); ++i)
        flux.push_back(juce::jmax(0.0f, energy[i] - energy[i - 1]));

    const int avgWindow = 24;
    std::vector<double> onsetTimes;

    for (size_t i = static_cast<size_t>(avgWindow); i < flux.size(); ++i)
    {
        double avg = 0.0;
        for (int k = 0; k < avgWindow; ++k)
            avg += flux[i - static_cast<size_t>(k) - 1];
        avg /= static_cast<double>(avgWindow);

        double variance = 0.0;
        for (int k = 0; k < avgWindow; ++k)
        {
            const double diff = flux[i - static_cast<size_t>(k) - 1] - avg;
            variance += diff * diff;
        }
        const double threshold = avg + std::sqrt(variance / static_cast<double>(avgWindow)) * 1.15;

        if (flux[i] > threshold && flux[i] > 0.004f)
        {
            double t = (static_cast<double>(i) * windowSize) / sampleRate;
            if (onsetTimes.empty() || (t - onsetTimes.back()) > 0.18)
                onsetTimes.push_back(t);
        }
    }

    if (onsetTimes.size() < 4) return 0.0;

    std::vector<double> bpmScores(181, 0.0);
    for (size_t i = 0; i < onsetTimes.size(); ++i)
    {
        for (size_t j = i + 1; j < onsetTimes.size() && j < i + 8; ++j)
        {
            const double interval = onsetTimes[j] - onsetTimes[i];
            if (interval < 0.25 || interval > 2.0)
                continue;

            double bpm = 60.0 / interval;
            while (bpm < 60.0)  bpm *= 2.0;
            while (bpm > 180.0) bpm /= 2.0;

            const int bin = juce::jlimit(60, 180, static_cast<int>(std::round(bpm)));
            const double weight = 1.0 / static_cast<double>(j - i);
            bpmScores[static_cast<size_t>(bin)] += weight;
            if (bin > 60)  bpmScores[static_cast<size_t>(bin - 1)] += weight * 0.35;
            if (bin < 180) bpmScores[static_cast<size_t>(bin + 1)] += weight * 0.35;
        }
    }

    int bestBin = 0;
    double bestScore = 0.0;
    for (int bpm = 60; bpm <= 180; ++bpm)
    {
        if (bpmScores[static_cast<size_t>(bpm)] > bestScore)
        {
            bestScore = bpmScores[static_cast<size_t>(bpm)];
            bestBin = bpm;
        }
    }

    if (bestBin == 0 || bestScore <= 0.0)
        return 0.0;

    double weightedSum = 0.0;
    double totalWeight = 0.0;
    for (int bpm = juce::jmax(60, bestBin - 2); bpm <= juce::jmin(180, bestBin + 2); ++bpm)
    {
        const double weight = bpmScores[static_cast<size_t>(bpm)];
        weightedSum += static_cast<double>(bpm) * weight;
        totalWeight += weight;
    }

    return totalWeight > 0.0
        ? std::round((weightedSum / totalWeight) * 10.0) / 10.0
        : static_cast<double>(bestBin);
}
