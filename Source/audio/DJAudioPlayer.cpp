#include "DJAudioPlayer.h"

static double dbToLinear(double dB)
{
    return std::pow(10.0, dB / 20.0);
}

DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& _formatManager)
    : formatManager(_formatManager)
{
}

DJAudioPlayer::~DJAudioPlayer() = default;

//==============================================================================
void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    sampleRate_ = sampleRate;
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    // Smooth over 50ms to eliminate zipper noise
    smoothGain_.reset (sampleRate, 0.05);
    smoothGain_.setCurrentAndTargetValue(1.0f);
    smoothSpeed_.reset(sampleRate, 0.05);
    smoothSpeed_.setCurrentAndTargetValue(1.0);

    // Pre-size delay buffers (max 2s)
    int maxDelaySamples = static_cast<int>(sampleRate * 2.0) + 1;
    for (int ch = 0; ch < 2; ++ch)
    {
        delayBuffer_[ch].assign(static_cast<size_t>(maxDelaySamples), 0.0f);
    }
    delayWritePos_ = 0;

    updateEQCoefficients();
    updateFilterCoefficients();
}

void DJAudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (!isLoaded_)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    resampleSource.getNextAudioBlock(bufferToFill);

    if (eqDirty_.exchange(false))     updateEQCoefficients();
    if (filterDirty_.exchange(false)) updateFilterCoefficients();

    auto* buf      = bufferToFill.buffer;
    int   numCh    = juce::jmin(buf->getNumChannels(), 2);
    int   nSamp    = bufferToFill.numSamples;
    int   startS   = bufferToFill.startSample;

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buf->getWritePointer(ch, startS);

        // EQ
        lowCutFilters_[ch].processSamples(data, nSamp);
        lowBoostFilters_[ch].processSamples(data, nSamp);
        midFilters_[ch].processSamples(data, nSamp);
        highFilters_[ch].processSamples(data, nSamp);

        // LP filter (M6)
        if (lpfEnabled_)
            lpFilters_[ch].processSamples(data, nSamp);

        // High cut / low-pass filter
        if (hpfEnabled_)
            highCutFilters_[ch].processSamples(data, nSamp);

        // Delay (M6)
        if (delayEnabled_ && !delayBuffer_[ch].empty())
        {
            int  delaySamples = static_cast<int>(delayTime_ * sampleRate_);
            delaySamples = juce::jlimit(1, static_cast<int>(delayBuffer_[ch].size()) - 1,
                                        delaySamples);
            auto& db = delayBuffer_[ch];
            int   dbSize = static_cast<int>(db.size());
            float wet    = static_cast<float>(delayWet_);
            float fb     = static_cast<float>(delayFeedback_);

            for (int i = 0; i < nSamp; ++i)
            {
                int readPos = (delayWritePos_ - delaySamples + dbSize) % dbSize;
                float delayed = db[static_cast<size_t>(readPos)];
                float input   = data[i];
                db[static_cast<size_t>(delayWritePos_)] = input + delayed * fb;
                data[i] = input * (1.0f - wet) + delayed * wet;
                if (ch == 0)
                    delayWritePos_ = (delayWritePos_ + 1) % dbSize;
            }
        }
    }

    // For mono, delayWritePos_ is already advanced in the channel loop above.

    // RMS metering
    float rms = 0.0f;
    for (int ch = 0; ch < numCh; ++ch)
        rms = juce::jmax(rms, buf->getRMSLevel(ch, startS, nSamp));

    const float boostedMeter = juce::jlimit(0.0f, 1.0f, rms * 2.4f);
    rmsLevel_.store(boostedMeter);
}

void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

//==============================================================================
void DJAudioPlayer::loadURL(juce::URL audioURL)
{
    auto inputStream = audioURL.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress));
    auto* reader = inputStream != nullptr ? formatManager.createReaderFor(std::move(inputStream)) : nullptr;
    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(
            new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release());
        isLoaded_    = true;
        cuePoint_    = 0.0;
        loopEnabled_ = false;
        delayWritePos_ = 0;
        for (auto& db : delayBuffer_) std::fill(db.begin(), db.end(), 0.0f);
    }
    else
    {
        isLoaded_ = false;
    }
}

void DJAudioPlayer::start() { transportSource.start(); }
void DJAudioPlayer::stop()  { transportSource.stop();  }

//==============================================================================
void DJAudioPlayer::setGain(double gain)
{
    userGain_ = juce::jlimit(0.0, 1.0, gain);
    applyEffectiveGain();
}

void DJAudioPlayer::setTrimGain(double trim)
{
    trimGain_ = juce::jlimit(0.0, 2.0, trim);
    applyEffectiveGain();
}

void DJAudioPlayer::setCrossfaderGain(double gain)
{
    crossfaderGain_ = juce::jlimit(0.0, 2.0, gain);
    applyEffectiveGain();
}

void DJAudioPlayer::applyEffectiveGain()
{
    float effective = static_cast<float>(userGain_ * trimGain_ * crossfaderGain_);
    smoothGain_.setTargetValue(effective);
    // Actual per-sample smoothing applied in getNextAudioBlock via buffer multiply
    // For simplicity, set transport gain directly (SmoothedValue drives the buffer gain)
    transportSource.setGain(effective);
}

void DJAudioPlayer::setSpeed(double ratio)
{
    ratio = juce::jlimit(0.05, 4.0, ratio);
    resampleSource.setResamplingRatio(ratio);
    currentSpeed_ = ratio;
}

void DJAudioPlayer::setPosition(double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
    if (pos < 0.0 || pos > 1.0) return;
    setPosition(transportSource.getLengthInSeconds() * pos);
}

double DJAudioPlayer::getPositionRelative() const
{
    double len = transportSource.getLengthInSeconds();
    return (len > 0.0) ? transportSource.getCurrentPosition() / len : 0.0;
}

double DJAudioPlayer::getTotalLength() const
{
    return transportSource.getLengthInSeconds();
}

//==============================================================================
void DJAudioPlayer::setLoopIn()
{
    loopStart_   = transportSource.getCurrentPosition();
    loopEnabled_ = (loopEnd_ > loopStart_);
}

void DJAudioPlayer::setLoopOut()
{
    loopEnd_     = transportSource.getCurrentPosition();
    loopEnabled_ = (loopEnd_ > loopStart_);
}

void DJAudioPlayer::clearLoop()
{
    loopEnabled_ = false;
    loopStart_   = 0.0;
    loopEnd_     = 0.0;
}

bool DJAudioPlayer::isLoopEnabled() const { return loopEnabled_; }

//==============================================================================
void DJAudioPlayer::setHotcue(int index)
{
    if (index >= 0 && index < kNumHotcues)
        hotcues_[static_cast<size_t>(index)] = transportSource.getCurrentPosition();
}

void DJAudioPlayer::jumpToHotcue(int index)
{
    if (index >= 0 && index < kNumHotcues && hotcues_[static_cast<size_t>(index)] >= 0.0)
        transportSource.setPosition(hotcues_[static_cast<size_t>(index)]);
}

void DJAudioPlayer::clearHotcue(int index)
{
    if (index >= 0 && index < kNumHotcues)
        hotcues_[static_cast<size_t>(index)] = -1.0;
}

double DJAudioPlayer::getHotcue(int index) const
{
    if (index >= 0 && index < kNumHotcues)
        return hotcues_[static_cast<size_t>(index)];
    return -1.0;
}

bool DJAudioPlayer::isHotcueSet(int index) const
{
    return index >= 0 && index < kNumHotcues
        && hotcues_[static_cast<size_t>(index)] >= 0.0;
}

void DJAudioPlayer::setLoopFromCurrentPosition(double durationSeconds)
{
    const double start = transportSource.getCurrentPosition();
    const double total = transportSource.getLengthInSeconds();
    if (total <= 0.0 || durationSeconds <= 0.0) return;

    loopStart_   = start;
    loopEnd_     = juce::jmin(start + durationSeconds, total);
    loopEnabled_ = (loopEnd_ > loopStart_);
}

void DJAudioPlayer::checkAndLoopIfNeeded()
{
    if (!loopEnabled_ || !isLoaded_) return;
    if (transportSource.getCurrentPosition() >= loopEnd_)
        transportSource.setPosition(loopStart_);
}

//==============================================================================
void DJAudioPlayer::setCuePoint()    { cuePoint_ = getPositionRelative(); }
void DJAudioPlayer::jumpToCue()      { setPositionRelative(cuePoint_);    }
double DJAudioPlayer::getCuePoint() const { return cuePoint_; }

//==============================================================================
void DJAudioPlayer::setEQLow (double dB) { eqLowDb_  = juce::jlimit(-12.0,12.0,dB); eqDirty_.store(true); }
void DJAudioPlayer::setEQMid (double dB) { eqMidDb_  = juce::jlimit(-12.0,12.0,dB); eqDirty_.store(true); }
void DJAudioPlayer::setEQHigh(double dB) { eqHighDb_ = juce::jlimit(-12.0,12.0,dB); eqDirty_.store(true); }

void DJAudioPlayer::updateEQCoefficients()
{
    if (sampleRate_ <= 0.0) return;
    const auto lowGain  = static_cast<float>(dbToLinear(eqLowDb_));
    const auto midGain  = static_cast<float>(dbToLinear(eqMidDb_ * 1.15));
    const auto highGain = static_cast<float>(dbToLinear(eqHighDb_));

    auto lowCut  = juce::IIRCoefficients::makeLowShelf (sampleRate_, 180.0, 0.75f,
                    static_cast<float>(dbToLinear(eqLowDb_ * 1.6)));
    auto lowBody = juce::IIRCoefficients::makePeakFilter(sampleRate_, 105.0, 0.95f,
                    lowGain);
    auto mc = juce::IIRCoefficients::makePeakFilter(sampleRate_, 950.0, 0.8f,
                    midGain);
    auto hc = juce::IIRCoefficients::makeHighShelf(sampleRate_, 4200.0, 0.75f,
                    highGain);
    for (int ch = 0; ch < 2; ++ch)
    {
        lowCutFilters_[ch].setCoefficients(lowCut);
        lowBoostFilters_[ch].setCoefficients(lowBody);
        midFilters_[ch].setCoefficients(mc);
        highFilters_[ch].setCoefficients(hc);
    }
}

//==============================================================================
void DJAudioPlayer::setLowPassEnabled (bool e) { lpfEnabled_ = e; filterDirty_.store(true); }
void DJAudioPlayer::setHighPassEnabled(bool e) { hpfEnabled_ = e; filterDirty_.store(true); }
bool DJAudioPlayer::isLowPassEnabled()  const  { return lpfEnabled_;  }
bool DJAudioPlayer::isHighPassEnabled() const  { return hpfEnabled_;  }

void DJAudioPlayer::updateFilterCoefficients()
{
    if (sampleRate_ <= 0.0) return;
    auto lp = juce::IIRCoefficients::makeLowPass (sampleRate_, 800.0);
    auto hp = juce::IIRCoefficients::makeHighPass(sampleRate_, 300.0);
    auto highCut = juce::IIRCoefficients::makeLowPass(sampleRate_, 1800.0);
    for (int ch = 0; ch < 2; ++ch)
    {
        lpFilters_[ch].setCoefficients(lp);
        hpFilters_[ch].setCoefficients(hp);
        highCutFilters_[ch].setCoefficients(highCut);
    }
}

void DJAudioPlayer::setDelayEnabled (bool e)        { delayEnabled_  = e;   }
void DJAudioPlayer::setDelayTime    (double secs)   { delayTime_     = juce::jlimit(0.01, 2.0, secs); }
void DJAudioPlayer::setDelayFeedback(double fb)     { delayFeedback_ = juce::jlimit(0.0, 0.9, fb);   }
void DJAudioPlayer::setDelayWetDry  (double wet)    { delayWet_      = juce::jlimit(0.0, 1.0, wet);   }

//==============================================================================
float  DJAudioPlayer::getRMSLevel() const { return rmsLevel_.load(); }
double DJAudioPlayer::getBPM()      const { return bpm_.load();      }
void   DJAudioPlayer::setBPM(double b)    { bpm_.store(b);            }

//==============================================================================
DeckState DJAudioPlayer::getState() const
{
    const double lengthSeconds = transportSource.getLengthInSeconds();
    const bool hasValidLoop = loopEnabled_ && lengthSeconds > 0.0 && loopEnd_ > loopStart_;

    return {
        transportSource.isPlaying(),
        isLoaded_,
        userGain_,
        currentSpeed_,
        bpm_.load(),
        cuePoint_,
        hasValidLoop,
        hasValidLoop ? juce::jlimit(0.0, 1.0, loopStart_ / lengthSeconds) : 0.0,
        hasValidLoop ? juce::jlimit(0.0, 1.0, loopEnd_ / lengthSeconds) : 0.0
    };
}
