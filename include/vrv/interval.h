#pragma once

// Based on interval.as
namespace vrv {
    class Interval {
    public:
        Interval();
        Interval(int a, int b);

        void flip();

        bool SetDiatonic(int d);
        signed char GetDiatonic() { return diatonic; }

        bool SetChromatic(int c);
        signed char GetChromatic() { return chromatic; }

        Interval NormalizeTritone();
        vrv::Interval Normalize();
        int GetFifths();
        Interval Abs();

        static Interval FromPitches(int pitch);

        bool operator!=(const Interval &a) const { return diatonic != a.diatonic || chromatic != a.chromatic; }
        bool operator==(const Interval &a) const { return diatonic == a.diatonic && chromatic == a.chromatic; }
    private:
        signed char diatonic;
        signed char chromatic;

        int IntervalClass();
        int StepClass();

        static int LEAST_FIFTHS_STEPS[];
        static int LEAST_FIFTHS[];
    };
}