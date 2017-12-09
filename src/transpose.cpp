#include "transpose.h"

#include <assert.h>
#include <math.h>

#include "staff.h"
#include "note.h"
#include "measure.h"
#include "layer.h"
#include <attcomparison.h>

namespace vrv {

void Transpose::SetDoc(Doc *doc)
{
    m_doc = doc;
    chromaticHistory = GetFirstKeySigFifths(m_doc);
}

//---------------------------------------------------------
//   keydiff2Interval
//    keysig -   -7(Cb) - +7(C#)
//---------------------------------------------------------
vrv::Interval vrv::Transpose::keydiff2Interval(int oldFifths, int newFifths, TransposeDirection dir)
{
    static int stepTable[15] = {
        // C  G  D  A  E  B Fis
        0, 4, 1, 5, 2, 6, 3,
    };

    int cofSteps; // circle of fifth steps
    int diatonic;
    if (newFifths > oldFifths)
        cofSteps = int(newFifths) - int(oldFifths);
    else
        cofSteps = 12 - (int(oldFifths) - int(newFifths));
    diatonic = stepTable[(int(newFifths) + 7) % 7] - stepTable[(int(oldFifths) + 7) % 7];
    if (diatonic < 0)
        diatonic += 7;
    diatonic %= 7;
    int chromatic = (cofSteps * 7) % 12;

    if (dir == TransposeDirection::CLOSEST) {
        if (chromaticHistory + chromatic > 6) {
            dir = TransposeDirection::DOWN;
        }
        else if (chromaticHistory + chromatic < -6) {
            dir = TransposeDirection::UP;
        }
    }

    if (dir == TransposeDirection::DOWN) {
        chromatic = chromatic - 12;
        diatonic = diatonic - 7;
        if (diatonic == -7)
            diatonic = 0;
        if (chromatic == -12)
            chromatic = 0;
    }
    if (dir == TransposeDirection::UP) {
        chromatic = chromatic + 12;
        diatonic = diatonic + 7;
        if (diatonic == 7)
            diatonic = 0;
        if (chromatic == 12)
            chromatic = 0;
    }
    if (abs(chromaticHistory + chromatic) > 6)
        assert(abs(chromaticHistory + chromatic) <= 6);

    chromaticHistory += chromatic;

    return vrv::Interval(diatonic, chromatic);
}

int vrv::Transpose::GetFirstKeySigFifths(Doc *m_doc)
{
    int keySigLog = KEYSIGNATURE_0;

    if (m_doc->m_scoreDef.HasKeySig())
    {
        keySigLog = m_doc->m_scoreDef.GetKeySig();
    }
    else
    {
        std::vector<int> staffs = m_doc->m_scoreDef.GetStaffNs();
        std::vector<int>::iterator iter;
        for (iter = staffs.begin(); iter != staffs.end(); iter++) {
            StaffDef *staffDef = m_doc->m_scoreDef.GetStaffDef(*iter);
            assert(staffDef);

            if (staffDef->HasKeySig()) {
                keySigLog = staffDef->GetKeySig();

                Interval transpositionInterval = staffDef->GetTransInterval();
                int fifths = keySigLog - KEYSIGNATURE_0;
                // add the transposition to inverse the transposition
                fifths += transpositionInterval.GetFifths();

                // We have to undo the enharmonic distance thing we did earlier
                // NOTE that this will never happen in practice, since we always set the ScoreDef keySig before changing the staffDef one
                if (staffDef->HasEnharmonicInt())
                {
                    int enharmonicDistance = staffDef->GetEnharmonicInt();
                    while (enharmonicDistance > 0)
                    {
                        fifths += 12;
                        enharmonicDistance--;
                    }
                    while (enharmonicDistance < 0)
                    {
                        fifths -= 12;
                        enharmonicDistance++;
                    }
                }

                keySigLog = fifths + KEYSIGNATURE_0;
                keySigLog = Transpose::GetAbsoluteKeySig(keySigLog);
                break;
            }
        }
    }
    // Need to subtract KEYSIGNATURE_0 to convert the enum int to actual key sig fifths
    return keySigLog - KEYSIGNATURE_0;
}

data_KEYSIGNATURE vrv::Transpose::GetAbsoluteKeySig(int keySigLog)
{
    //keySigLog = (keySigLog + 14) % 15 + 1;
    return static_cast<data_KEYSIGNATURE>(keySigLog);
}

void vrv::Transpose::ChangeStaffDefKeySig(StaffDef *staffDef, int origFifths)
{
    int fifths = origFifths;
    int oldKeySigLog = origFifths + KEYSIGNATURE_0;
    Interval transpositionInterval = staffDef->GetTransInterval();

    // First, set the scoreDef key sig to what the keysig actually should be
    m_doc->m_scoreDef.SetKeySig(static_cast<data_KEYSIGNATURE>(oldKeySigLog));

    int enharmonicDistance = 0;
    fifths = fifths - transpositionInterval.GetFifths();
    while (fifths > 7)
    {
        fifths -= 12;
        enharmonicDistance++;
    }
    while(fifths < -7)
    {
        fifths += 12;
        enharmonicDistance--;
    }
    if ((fifths <= 5 || fifths >= 5) && origFifths != fifths)
    {
        /// TODO: Implement this part, which deals with changing key sigs
        // LINES 675-677 in MEasure.as
    }
    staffDef->SetEnharmonicInt(enharmonicDistance);

    int keySigLog = fifths + KEYSIGNATURE_0;
    data_KEYSIGNATURE partKeySig = Transpose::GetAbsoluteKeySig(keySigLog);
    // we already added the transposition interval
    //data_KEYSIGNATURE partKeySig = Transpose::GetAbsoluteKeySig(keySigLog + transpositionInterval.GetFifths());
    staffDef->SetKeySig(partKeySig);
}

void vrv::Transpose::ChangeKeySignature(int newFifths)
{
    ArrayOfObjects staffs = m_doc->FindAllChildByType(STAFF);
    ArrayOfObjects::iterator iter2;
    for (iter2 = staffs.begin(); iter2 != staffs.end(); iter2++) {
        Staff *staff = dynamic_cast<Staff *>(*iter2);
        assert(staff);

        StaffDef *staffDef = staff->m_drawingStaffDef;
        assert(staffDef);

        // skip perc. clefs
        Clef *clef = staffDef->GetCurrentClef();
        if (!clef || clef->GetShape() == CLEFSHAPE_perc) continue;

        StaffDef *updatedStaffDef = m_doc->m_scoreDef.GetStaffDef(staffDef->GetN());
        ChangeStaffDefKeySig(updatedStaffDef, newFifths);
    }

    int keySigLog = newFifths + KEYSIGNATURE_0;
    m_doc->m_scoreDef.SetKeySig(static_cast<data_KEYSIGNATURE>(keySigLog));
}

bool vrv::Transpose::transposeFifths(int newFifths)
{
    // Transpose by key

    // Find the first key signature from the pitched staves
    int oldFifths = GetFirstKeySigFifths(m_doc);
    Interval interval = keydiff2Interval(oldFifths, newFifths);

    ArrayOfObjects staffs = m_doc->FindAllChildByType(STAFF);
    ArrayOfObjects::iterator iter2;
    for (iter2 = staffs.begin(); iter2 != staffs.end(); iter2++) {
        Staff *staff = dynamic_cast<Staff *>(*iter2);
        assert(staff);

        StaffDef *staffDef = staff->m_drawingStaffDef;
        assert(staffDef);

        // skip perc. clefs
        Clef *clef = staffDef->GetCurrentClef();
        if (!clef || clef->GetShape() == CLEFSHAPE_perc) continue;

        ArrayOfObjects notes = staff->FindAllChildByType(NOTE);
        transposeNotes(interval, notes);
    }

    ChangeKeySignature(newFifths);

    m_doc->UnCastOffDoc();
    m_doc->CastOffDoc();
    return true;
}

bool vrv::Transpose::transposeInterval(vrv::Interval interval, StaffDef staffDef)
{
    Clef *clef = staffDef.GetCurrentClef();
    if (!clef || clef->GetShape() == CLEFSHAPE_perc) return false;

    AttCommonNComparison comparisonStaffDef(STAFF, staffDef.GetN());
    ArrayOfObjects staffs;
    m_doc->FindAllChildByAttComparison(&staffs, &comparisonStaffDef);

    ArrayOfObjects::iterator iter2;
    for (iter2 = staffs.begin(); iter2 != staffs.end(); iter2++) {
        Staff *staff = dynamic_cast<Staff *>(*iter2);
        assert(staff);

        // skip perc. clefs
        Clef *clef = staffDef.GetCurrentClef();
        if (!clef || clef->GetShape() == CLEFSHAPE_perc) continue;

        ArrayOfObjects notes = staff->FindAllChildByType(NOTE);
        transposeNotes(interval, notes);
    }

    m_doc->UnCastOffDoc();
    m_doc->CastOffDoc();
    return true;
}

int vrv::Transpose::GetPartTransposition(vrv::Interval transposeInterval, StaffDef staffDef, int comfHigh,
                                         int comfLow, int proHigh, int proLow, bool multiStaff)
{
    Clef *clef = staffDef.GetCurrentClef();
    if (!clef || clef->GetShape() == CLEFSHAPE_perc) return 0;

    AttCommonNComparison comparisonStaffDef(STAFF, staffDef.GetN());
    ArrayOfObjects staffs;
    m_doc->FindAllChildByAttComparison(&staffs, &comparisonStaffDef);

    int noteHigh = 0;
    int noteLow = 1000;
    int noteCount = 0;
    int noteSum = 0;

    ArrayOfObjects::iterator iter2;
    for (iter2 = staffs.begin(); iter2 != staffs.end(); iter2++) {
        Staff *staff = dynamic_cast<Staff *>(*iter2);
        assert(staff);

        // skip perc. clefs
        Clef *clef = staffDef.GetCurrentClef();
        if (!clef || clef->GetShape() == CLEFSHAPE_perc) continue;
        int clefLocOffset = clef->GetClefLocOffset();

        ArrayOfObjects notes = staff->FindAllChildByType(NOTE);
        ArrayOfObjects::iterator noteIter;
        for (noteIter = notes.begin(); noteIter != notes.end(); noteIter++) {
            Note *note = dynamic_cast<Note *>(*noteIter);

            data_PITCHNAME steps = note->GetPname();
            int oct = note->GetOct();
            //offset by transposition amount because it will be transposed
            int pitchNumber = (PitchFromPname(steps) + AccIdToAlter(note->GetDrawingAccid())) + (oct * 12) +
                transposeInterval.GetChromatic();
            int noteLoc = PitchInterface::CalcLoc(note->GetPname(), note->GetOct(), clefLocOffset);

            noteCount++;
            noteSum += pitchNumber;
            if (pitchNumber > noteHigh)
                noteHigh = pitchNumber;
            if (pitchNumber < noteLow)
                noteLow = pitchNumber;
        }
    }

    if (noteCount < 1) return 0;

    int noteAvg = round((double)noteSum / (double)noteCount);

    int octaveTransposition = 0;
    if (!multiStaff) {
        int averageComf = (comfLow + comfHigh) / 2;
        while (true) {
            // if the average is between the average range of the instrument, break
            if (noteAvg + octaveTransposition >= averageComf - 6 && noteAvg + octaveTransposition <= averageComf + 6)
                break;

            if (noteAvg + octaveTransposition < averageComf)
                octaveTransposition += 12;
            else if (noteAvg + octaveTransposition > averageComf)
                octaveTransposition -= 12;
            else
                break;
        }

        // if it's outside of the comfortbale range, see if it can be fitted in the professional range
        if ((noteHigh + octaveTransposition) > comfHigh || (noteLow + octaveTransposition) < comfLow) {
            int proOctTransp = 0;
            int proComf = (proHigh + proLow) / 2;
            while (true) {
                // if the average is between the average range of the instrument, break
                if (noteAvg + proOctTransp >= proComf - 6 && noteAvg + proOctTransp <= proComf + 6)
                    break;

                if (noteAvg + proOctTransp < proComf)
                    proOctTransp += 12;
                else if (noteAvg + proOctTransp > proComf)
                    proOctTransp -= 12;
                else
                    break;
            }

            if (noteHigh + proOctTransp <= comfHigh && noteLow + proOctTransp >= comfLow)
                octaveTransposition = proOctTransp;
        }
    }
    else {
        while (true) {
            // if the average is between the average range of the instrument, break
            if (noteLow + octaveTransposition >= comfLow && noteHigh + octaveTransposition <= comfHigh)
                break;

            int distanceFromHigh = (noteHigh + octaveTransposition) - comfHigh;
            int distanceFromLow = comfLow - (noteHigh + octaveTransposition);
            if (distanceFromHigh > distanceFromLow) {
                octaveTransposition -= 12;
            }
            else if (distanceFromHigh < distanceFromLow) {
                octaveTransposition += 12;
            }
            else
                break;

            if (abs(distanceFromHigh - distanceFromLow) < 12 && abs(noteHigh - noteLow) > abs(comfHigh - comfLow))
                break;
        }
    }

    return octaveTransposition;
}

bool vrv::Transpose::transposeNotes(Interval interval, ArrayOfObjects notes)
{
    ArrayOfObjects::iterator notesIter;
    for (notesIter = notes.begin(); notesIter != notes.end(); notesIter++) {
        Note *note = dynamic_cast<Note *>(*notesIter);

        data_PITCHNAME steps = note->GetPname();
        int oct = note->GetOct();
        int pitchNumber = (PitchFromPname(steps) + AccIdToAlter(note->GetDrawingAccid())) + (oct * 12);
        pitchNumber += interval.GetChromatic();

        int currentStep = note->GetPname();
        int tpc = step2tpc(currentStep - 1, AccIdToAlter(note->GetDrawingAccid()));
        int newStep = currentStep - 1, newAlter = AccIdToAlter(note->GetDrawingAccid());
        transposeTpc(tpc, interval, false, newStep, newAlter);

        data_PITCHNAME newPname = static_cast<data_PITCHNAME>((newStep % 7) + 1);

        int newOct = 0;
        int newPitchNumber = (PitchFromPname(newPname) + newAlter);
        while (newPitchNumber < 0) {
            newPitchNumber += 12;
            newOct++;
        }
        if (newPitchNumber % 12 != pitchNumber % 12)
            assert(newPitchNumber % 12 == pitchNumber % 12);

        while (newPitchNumber != pitchNumber) {
            newPitchNumber += 12;
            newOct++;
        }

        note->SetPname(newPname);

        Accid *accid = note->GetDrawingAccid();
        if (accid) {
            note->DeleteChild(accid);
        }
        if (newAlter != 0) {
            accid = new Accid();
            accid->SetAccidGes(AlterToAccId(newAlter));
            note->AddChild(accid);
        }

        note->SetOct(newOct);
    }

    return true;
}

int vrv::Transpose::PitchFromPname(data_PITCHNAME pname)
{
    int midiBase = 0;
    switch (pname) {
        case PITCHNAME_c: midiBase = 0;
            break;
        case PITCHNAME_d: midiBase = 2;
            break;
        case PITCHNAME_e: midiBase = 4;
            break;
        case PITCHNAME_f: midiBase = 5;
            break;
        case PITCHNAME_g: midiBase = 7;
            break;
        case PITCHNAME_a: midiBase = 9;
            break;
        case PITCHNAME_b: midiBase = 11;
            break;
        case PITCHNAME_NONE: break;
    }
    return midiBase;
}

int vrv::Transpose::AccIdToAlter(Accid *accid)
{
    int midiBase = 0;
    // Check for accidentals
    if (accid && accid->HasAccidGes()) {
        data_ACCIDENTAL_IMPLICIT accImp = accid->GetAccidGes();
        switch (accImp) {
            case ACCIDENTAL_IMPLICIT_s: midiBase += 1;
                break;
            case ACCIDENTAL_IMPLICIT_f: midiBase -= 1;
                break;
            case ACCIDENTAL_IMPLICIT_ss: midiBase += 2;
                break;
            case ACCIDENTAL_IMPLICIT_ff: midiBase -= 2;
                break;
            default: break;
        }
    }
    else if (accid) {
        data_ACCIDENTAL_EXPLICIT accExp = accid->GetAccid();
        switch (accExp) {
            case ACCIDENTAL_EXPLICIT_s: midiBase += 1;
                break;
            case ACCIDENTAL_EXPLICIT_f: midiBase -= 1;
                break;
            case ACCIDENTAL_EXPLICIT_ss: midiBase += 2;
                break;
            case ACCIDENTAL_EXPLICIT_x: midiBase += 2;
                break;
            case ACCIDENTAL_EXPLICIT_ff: midiBase -= 2;
                break;
            case ACCIDENTAL_EXPLICIT_xs: midiBase += 3;
                break;
            case ACCIDENTAL_EXPLICIT_ts: midiBase += 3;
                break;
            case ACCIDENTAL_EXPLICIT_tf: midiBase -= 3;
                break;
            case ACCIDENTAL_EXPLICIT_nf: midiBase -= 1;
                break;
            case ACCIDENTAL_EXPLICIT_ns: midiBase += 1;
                break;
            default: break;
        }
    }
    return midiBase;
}

vrv::data_ACCIDENTAL_IMPLICIT vrv::Transpose::AlterToAccId(int value)
{
    if (value == -2)
        return ACCIDENTAL_IMPLICIT_ff;
    else if (value == -1)
        return ACCIDENTAL_IMPLICIT_f;
    else if (value == 1)
        return ACCIDENTAL_IMPLICIT_s;
    else if (value == 2)
        return ACCIDENTAL_IMPLICIT_ss;

    return ACCIDENTAL_IMPLICIT_NONE;
}

int Transpose::tpc2step(int tpc)
{
    const int TPC_DELTA_SEMITONE = 7; // the delta in tpc value to go 1 semitone up or down
    const int TPC_DELTA_ENHARMONIC = 12; // the delta in tpc value to reach the next (or prev) enharmonic spelling
    //const int   TPC_FIRST_STEP          = 3;  // the step of the first valid tpc (= F = step 3)
    const int PITCH_DELTA_OCTAVE = 12; // the delta in pitch value to go 1 octave up or down
    const int STEP_DELTA_OCTAVE = 7; // the number of steps in an octave
    //const int   STEP_DELTA_TPC          = 4;  // the number of steps in a tpc step (= a fifth = 4 steps)
    static const int steps[STEP_DELTA_OCTAVE] = {3, 0, 4, 1, 5, 2, 6};
    return steps[(tpc - -1) % STEP_DELTA_OCTAVE];
}

int Transpose::step2tpc(int step, int alter)
{
    //    TPC - tonal pitch classes
    //    "line of fifth's" LOF

    static const int spellings[] = {
        //     bb  b   -   #  ##
        0, 7, 14, 21, 28, // C
        2, 9, 16, 23, 30, // D
        4, 11, 18, 25, 32, // E
        -1, 6, 13, 20, 27, // F
        1, 8, 15, 22, 29, // G
        3, 10, 17, 24, 31, // A
        5, 12, 19, 26, 33, // B
    };

    int i = step * 5 + alter + 2;
    return spellings[i];
}

int Transpose::tpc2pitch(int tpc)
{
    static int pitches[] = {
        //step:     F   C   G   D   A   E   B
        3, -2, 5, 0, 7, 2, 9, // bb
        4, -1, 6, 1, 8, 3, 10, // b
        5, 0, 7, 2, 9, 4, 11, // -
        6, 1, 8, 3, 10, 5, 12, // #
        7, 2, 9, 4, 11, 6, 13 // ##
    };
    return pitches[tpc + 1];
}

void vrv::Transpose::transposeTpc(int tpc, vrv::Interval interval, bool useDoubleSharpsFlats, int &step,
                                  int &alter)
{
    int minAlter;
    int maxAlter;
    if (useDoubleSharpsFlats) {
        minAlter = -2;
        maxAlter = 2;
    }
    else {
        minAlter = -1;
        maxAlter = 1;
    }
    int steps = interval.GetDiatonic();
    int semitones = interval.GetChromatic();

    // qDebug("transposeTpc tpc %d steps %d semitones %d", tpc, steps, semitones);
    if (semitones == 0 && steps == 0)
        return;

    int pitch = tpc2pitch(tpc);

    for (int k = 0; k < 10; ++k) {
        step = tpc2step(tpc) + steps;
        while (step < 0)
            step += 7;
        step %= 7;
        int p1 = tpc2pitch(step2tpc(step, 0));
        alter = semitones - (p1 - pitch);
        // alter  = p1 + semitones - pitch;

        //            if (alter < 0) {
        //                  alter *= -1;
        //                  alter = 12 - alter;
        //                  }
        while (alter < 0)
            alter += 12;

        alter %= 12;
        if (alter > 6)
            alter -= 12;
        if (alter > maxAlter)
            ++steps;
        else if (alter < minAlter)
            --steps;
        else
            break;
        //            qDebug("  again alter %d steps %d, step %d", alter, steps, step);
    }
    //      qDebug("  = step %d alter %d  tpc %d", step, alter, step2tpc(step, alter));
}
}
