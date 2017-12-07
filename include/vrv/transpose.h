#pragma once
#include "doc.h"
#include "accid.h"
#include "interval.h"

/// Based on transpose.cpp from MuseScore
namespace vrv {
class Transpose {
public:
    enum class TransposeDirection : char {
        NONE = -1,
        UP,
        DOWN,
        CLOSEST
    };

public:
    void SetDoc(Doc *m_doc);
    bool transposeFifths(int newFifths);
    bool transposeInterval(Interval interval, StaffDef staffDef);
    // returns chromatic pitch transformation to fit inside part comf or pro range
    int GetPartTransposition(Interval exitingInterval, StaffDef staffDef,
                             int comfHigh, int comfLow, int proHigh, int proLow, bool multiStaff);
    static int GetFirstKeySigFifths(Doc *m_doc);
private:
    Interval keydiff2Interval(int oldFifths, int newFifths,
                                              TransposeDirection dir = TransposeDirection::CLOSEST);
    static void transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats, int &step,
                             int &alter);
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
