#include "interval.h"

#include "staff.h"
#include <math.h>

namespace vrv {
int vrv::Interval::LEAST_FIFTHS_STEPS[] = {0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6};
int vrv::Interval::LEAST_FIFTHS[] = { 0, -5, 2, -3, 4, -1, 6, 1, -4, 3, -2, 5 };

vrv::Interval::Interval()
    : diatonic(0)
    , chromatic(0)
{
}

vrv::Interval::Interval(int diatonic, int chromatic)
    : diatonic(diatonic)
    , chromatic(chromatic)
{
}

bool vrv::Interval::SetDiatonic(int d)
{
    diatonic = d;
    return true;
}

bool vrv::Interval::SetChromatic(int c)
{
    chromatic = c;
    return true;
}

void vrv::Interval::Interval::flip()
{
    diatonic = -diatonic;
    chromatic = -chromatic;
}

vrv::Interval vrv::Interval::Normalize()
{
    return Interval(StepClass(), IntervalClass());
}

vrv::Interval vrv::Interval::NormalizeTritone()
{
    if (IntervalClass() == 6 && StepClass() == 4) {
        return Interval(diatonic - 1, chromatic);
    }
    return *this;
}

int vrv::Interval::IntervalClass()
{
    int pitches = chromatic;
    while (pitches < 0) {
        pitches += 12;
    }
    while (pitches >= 12) {
        pitches -= 12;
    }
    return pitches;
}

int vrv::Interval::StepClass()
{
    int pitches = diatonic;
    while (pitches < 0) {
        pitches += 7;
    }
    while (pitches >= 7) {
        pitches -= 7;
    }
    return pitches;
}

vrv::Interval vrv::Interval::FromPitches(int pitch)
{
    int octaveNum = floor(abs(pitch) / double(12));
    int pitchNoOctave = abs(pitch) % 12;
    int fifths = LEAST_FIFTHS_STEPS[pitchNoOctave] + (octaveNum * 7);
    if (pitch < 0)
        fifths = -fifths;
    return Interval(fifths, pitch);
}

vrv::Interval vrv::Interval::Abs()
{
    return Interval(abs(GetDiatonic()), abs(GetChromatic()));
}

int vrv::Interval::GetFifths()
{
    Interval interval = this->Abs().Normalize();
    int fifths = LEAST_FIFTHS[interval.GetChromatic()];
    int steps = interval.GetDiatonic() - LEAST_FIFTHS_STEPS[interval.GetChromatic()];
    while (steps > 3)
    {
        steps -= 7;
    }
    while (steps < -3)
    {
        steps += 7;
    }
    fifths -= steps * 12;

    if (GetChromatic() >= 0 && GetDiatonic() >= 0)
        return fifths;
    else
        return -fifths;
}
}
