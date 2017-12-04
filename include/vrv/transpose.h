#pragma once
#include "doc.h"
#include "accid.h"
/// Based on transpose.cpp from MuseScore
namespace vrv {
    class Transpose
    {
    public:
        class Interval {
        public:
            Interval();
            Interval(int a, int b);

            void flip();

            bool SetDiatonic(int d);
            signed char GetDiatonic() { return diatonic; }

            bool SetChromatic(int c);
            signed char GetChromatic() { return chromatic; }

            Interval *NormalizeTritone();

            static Interval *FromPitches(int pitch);

            bool operator!=(const Interval& a) const { return diatonic != a.diatonic || chromatic != a.chromatic; }
            bool operator==(const Interval& a) const { return diatonic == a.diatonic && chromatic == a.chromatic; }
        private:
            signed char diatonic;
            signed char chromatic; 
            
            int IntervalClass();
            int StepClass();

            static int LEAST_FIFTHS_STEPS[];
        };

        enum class TransposeDirection : char {
            NONE = -1, UP, DOWN, CLOSEST
        };

    public:
        Transpose(Doc *doc);
        bool transposeFifths(int newFifths);
        bool transposeInterval(Interval interval, StaffDef staffDef);
        // returns chromatic pitch transformation to fit inside part comf or pro range
        int GetPartTransposition(vrv::Transpose::Interval exitingInterval, StaffDef staffDef,
                                  int comfHigh, int comfLow, int proHigh, int proLow, bool multiStaff);
        static int GetFirstKeySigFifths(Doc * m_doc);
    private:
        vrv::Transpose::Interval keydiff2Interval(int oldFifths, int newFifths, TransposeDirection dir = TransposeDirection::CLOSEST);
        static void transposeTpc(int tpc, vrv::Transpose::Interval interval, bool useDoubleSharpsFlats, int & step, int & alter);
        static int PitchFromPname(data_PITCHNAME pname);
        static data_ACCIDENTAL_IMPLICIT AlterToAccId(int value);
        static int AccIdToAlter(Accid *accid);
        static int tpc2step(int tpc);
        static int step2tpc(int step, int alter);
        static int tpc2pitch(int tpc);
        bool transposeNotes(Interval interval, ArrayOfObjects notes);

        int chromaticHistory;
        Doc *m_doc;
    };
}