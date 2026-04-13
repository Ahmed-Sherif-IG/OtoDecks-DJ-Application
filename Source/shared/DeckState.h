#pragma once

struct DeckState
{
    bool   isPlaying     = false;
    bool   isLoaded      = false;
    double gain          = 1.0;
    double speed         = 1.0;
    double bpm           = 0.0;
    double cuePoint      = 0.0;
    bool   loopActive    = false;
    double loopStartRel  = 0.0;
    double loopEndRel    = 0.0;
};
