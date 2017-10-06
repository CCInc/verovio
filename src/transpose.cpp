#include "transpose.h"

#include <assert.h>

#include "staff.h"
#include "note.h"
#include "measure.h"
#include "layer.h"

namespace vrv {
	vrv::Transpose::Interval::Interval()
		: diatonic(0), chromatic(0)
	{
	}

	vrv::Transpose::Interval::Interval(int a, int b)
		: diatonic(a), chromatic(b)
	{
	}

	bool vrv::Transpose::Interval::SetDiatonic(int d)
	{
		diatonic = d;
		return true;
	}

	bool vrv::Transpose::Interval::SetChromatic(int c)
	{
		chromatic = c;
		return true;
	}

	void vrv::Transpose::Interval::Interval::flip()
	{
		diatonic = -diatonic;
		chromatic = -chromatic;
	}

	//---------------------------------------------------------
	//   keydiff2Interval
	//    keysig -   -7(Cb) - +7(C#)
	//---------------------------------------------------------
	vrv::Transpose::Interval vrv::Transpose::keydiff2Interval(int oldFifths, int newFifths)
	{
		static int stepTable[15] = {
			// C  G  D  A  E  B Fis
			0, 4, 1, 5, 2, 6, 3,
		};

		int cofSteps;     // circle of fifth steps
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


		if (chromatic > 6) {
			chromatic = chromatic - 12;
			diatonic = diatonic - 7;
			if (diatonic == -7)
				diatonic = 0;
			if (chromatic == -12)
				chromatic = 0;
		}

		return vrv::Transpose::Interval(diatonic, chromatic);
	}

	bool vrv::Transpose::transpose(Doc *m_doc, int newFifths)
	{
		// Transpose by key

		// Find the first key signature from the pitched staves
		int oldFifths = 0;

		Measure firstMeasure = m_doc->FindChildByType(MEASURE);
		ArrayOfObjects firstMeasureStaffs = m_doc->FindAllChildByType(STAFF);

		ArrayOfObjects::iterator iter;
		for (iter = firstMeasureStaffs.begin(); iter != firstMeasureStaffs.end(); iter++) {
			Staff *staff = dynamic_cast<Staff *>(*iter);
			assert(staff);

			StaffDef *staffDef = staff->m_drawingStaffDef;
			assert(staffDef);

			Clef *clef = staffDef->GetCurrentClef();
			if (clef->GetShape() != CLEFSHAPE_perc && clef->GetShape() != CLEFSHAPE_TAB)
			{
				KeySig *keySig = staffDef->GetKeySigCopy();
				int keySigLog = keySig->ConvertToKeySigLog();
				delete keySig;

				oldFifths = keySigLog - KEYSIGNATURE_0;
				break;
			}
		}

		Interval interval = keydiff2Interval(oldFifths, newFifths);

		ArrayOfObjects staffs = m_doc->FindAllChildByType(STAFF);
		ArrayOfObjects::iterator iter2;
		for (iter2 = staffs.begin(); iter2 != staffs.end(); iter2++) {
			Staff *staff = dynamic_cast<Staff *>(*iter2);
			assert(staff);

			StaffDef *staffDef = staff->m_drawingStaffDef;
			assert(staffDef);

			Clef *clef = staffDef->GetCurrentClef();
			if (!clef || clef->GetShape() == CLEFSHAPE_perc) continue;


			//ArrayOfObjects staffs = m_doc->FindAllChildByType(STAFF);
			//for (element = staff->GetFirst(); element; element = staff->GetNext()) {
			//	if (element->Is(NOTE)) {
			//		Note *note = dynamic_cast<Note *>(element);

			//		int pitch = note->GetPname() + interval.GetChromatic();
			//		int oct = note->GetOct();

			//		note->AdjustPname(pitch, oct);
			//		note->SetPname(static_cast<data_PITCHNAME>(pitch));
			//		note->SetOct(oct);
			//	}
			//	else if (element->Is(CHORD)) {
			//		
			//	}
			//	else if (element->Is(KEYSIG)) {

			//	}
			//}

			//ArrayOfObjects notes = staff->FindAllChildByType(NOTE);

			//for (iter = notes.begin(); iter != notes.end(); iter++) {
			//	Note *note = dynamic_cast<Note *>(*iter);

			//	int pitch = note->GetPname() + interval.GetChromatic();
			//	int oct = note->GetOct();

			//	note->AdjustPname(pitch, oct);
			//	note->SetPname(static_cast<data_PITCHNAME>(pitch));
			//	note->SetOct(oct);
			//}

			ArrayOfObjects notes = staff->FindAllChildByType(NOTE);
			ArrayOfObjects::iterator notesIter;
			for (notesIter = notes.begin(); notesIter != notes.end(); notesIter++) {
				Note *note = dynamic_cast<Note *>(*notesIter);

				int steps = note->GetPname();
				int oct = note->GetOct();

				//int currentPitch = PitchFromPname(note->GetPname()) + AccIdToAlter(note->GetDrawingAccid());
				//int newPitch = currentPitch + interval.GetChromatic();

				//int currentStep = note->GetPname();
				//int newStep = currentStep + interval.GetDiatonic();

				//note->AdjustPname(newStep, oct);
				//int alteration = newPitch - PitchFromPname(static_cast<data_PITCHNAME>(newStep));

				//while (alteration > 2) {
				//	newStep = ((newStep + 1) % 7);
				//	alteration = newPitch - PitchFromPname(static_cast<data_PITCHNAME>(newStep));
				//};
				//while (alteration < -(2)) {
				//	newStep = (((newStep + 7) - 1) % 7);
				//	alteration = newPitch - PitchFromPname(static_cast<data_PITCHNAME>(newStep));
				//};

				//Accid *accid = note->GetDrawingAccid();
				//int midiBase;
				//data_PITCHNAME pname = note->GetPname();

				//int origSteps = note->GetPname();
				//int alter = AccIdToAlter(note->GetDrawingAccid());
				//transposeTpc(origSteps, alter, interval, false);
				int currentStep = note->GetPname();
				int tpc = step2tpc(currentStep - 1, AccIdToAlter(note->GetDrawingAccid()));
				int newStep = currentStep - 1, newAlter = AccIdToAlter(note->GetDrawingAccid()), newOct = 0;
				transposeTpc(tpc, interval, false, newStep, newAlter, newOct);

				note->SetPname(static_cast<data_PITCHNAME>(newStep + 1));

				Accid *accid = note->GetDrawingAccid();
				if (accid)
				{
					note->DeleteChild(accid);
				}
				if (newAlter != 0)
				{
					accid = new Accid();
					accid->SetAccidGes(AlterToAccId(newAlter));
					note->AddChild(accid);
				}

				note->SetOct(oct + newOct);
			}

			//m_doc->CollectScoreDefs();
			//StaffDef *upcomingStaffDef = m_doc->m_scoreDef.GetStaffDef(staffDef->GetN());
			//assert(upcomingStaffDef);
			//upcomingStaffDef->SetKeySig(KEYSIGNATURE_6s);
			//upcomingStaffDef->SetCurrentKeySig(new KeySig());
			//m_doc->m_scoreDef.SetRedrawFlags(false, true, false, false, false);
			//m_doc->m_scoreDef.ReplaceDrawingValues(upcomingStaffDef);

			if (staffDef->HasKeySig())
			{
				data_KEYSIGNATURE keySig = staffDef->GetKeySig();

				int keySigLog = newFifths + KEYSIGNATURE_0;

				//if (keySigLog < KEYSIGNATURE_7f) {
				//	keySigLog = keySigLog + 7;
				//}
				//else if (keySigLog > KEYSIGNATURE_7s) {
				//	keySigLog = keySigLog - 7;
				//}

				StaffDef *updatedStaffDef = m_doc->m_scoreDef.GetStaffDef(staffDef->GetN());
				updatedStaffDef->SetKeySig(static_cast<data_KEYSIGNATURE>(keySigLog));
			}
		}
		m_doc->UnCastOffDoc();
		m_doc->CastOffDoc();
		return true;
	}

	int vrv::Transpose::PitchFromPname(data_PITCHNAME pname)
	{
		int midiBase = 0;
		switch (pname) {
		case PITCHNAME_c: midiBase = 0; break;
		case PITCHNAME_d: midiBase = 2; break;
		case PITCHNAME_e: midiBase = 4; break;
		case PITCHNAME_f: midiBase = 5; break;
		case PITCHNAME_g: midiBase = 7; break;
		case PITCHNAME_a: midiBase = 9; break;
		case PITCHNAME_b: midiBase = 11; break;
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
			case ACCIDENTAL_IMPLICIT_s: midiBase += 1; break;
			case ACCIDENTAL_IMPLICIT_f: midiBase -= 1; break;
			case ACCIDENTAL_IMPLICIT_ss: midiBase += 2; break;
			case ACCIDENTAL_IMPLICIT_ff: midiBase -= 2; break;
			default: break;
			}
		}
		else if (accid) {
			data_ACCIDENTAL_EXPLICIT accExp = accid->GetAccid();
			switch (accExp) {
			case ACCIDENTAL_EXPLICIT_s: midiBase += 1; break;
			case ACCIDENTAL_EXPLICIT_f: midiBase -= 1; break;
			case ACCIDENTAL_EXPLICIT_ss: midiBase += 2; break;
			case ACCIDENTAL_EXPLICIT_x: midiBase += 2; break;
			case ACCIDENTAL_EXPLICIT_ff: midiBase -= 2; break;
			case ACCIDENTAL_EXPLICIT_xs: midiBase += 3; break;
			case ACCIDENTAL_EXPLICIT_ts: midiBase += 3; break;
			case ACCIDENTAL_EXPLICIT_tf: midiBase -= 3; break;
			case ACCIDENTAL_EXPLICIT_nf: midiBase -= 1; break;
			case ACCIDENTAL_EXPLICIT_ns: midiBase += 1; break;
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
		const int   TPC_DELTA_SEMITONE = 7;  // the delta in tpc value to go 1 semitone up or down
		const int   TPC_DELTA_ENHARMONIC = 12; // the delta in tpc value to reach the next (or prev) enharmonic spelling
											   //const int   TPC_FIRST_STEP          = 3;  // the step of the first valid tpc (= F = step 3)
		const int   PITCH_DELTA_OCTAVE = 12; // the delta in pitch value to go 1 octave up or down
		const int   STEP_DELTA_OCTAVE = 7;  // the number of steps in an octave
											//const int   STEP_DELTA_TPC          = 4;  // the number of steps in a tpc step (= a fifth = 4 steps)
		static const int steps[STEP_DELTA_OCTAVE] = { 3, 0, 4, 1, 5, 2, 6 };
		return steps[(tpc - -1) % STEP_DELTA_OCTAVE];
	}

	int Transpose::step2tpc(int step, int alter)
	{
		//    TPC - tonal pitch classes
		//    "line of fifth's" LOF

		static const int spellings[] = {
			//     bb  b   -   #  ##
			0,  7, 14, 21, 28,  // C
			2,  9, 16, 23, 30,  // D
			4, 11, 18, 25, 32,  // E
			-1,  6, 13, 20, 27,  // F
			1,  8, 15, 22, 29,  // G
			3, 10, 17, 24, 31,  // A
			5, 12, 19, 26, 33,  // B
		};

		int i = step * 5 + alter + 2;
		return spellings[i];
	}

	int Transpose::tpc2pitch(int tpc)
	{
		static int pitches[] = {
			//step:     F   C   G   D   A   E   B
			3, -2,  5,  0,  7,  2,  9,     // bb
			4, -1,  6,  1,  8,  3, 10,     // b
			5,  0,  7,  2,  9,  4, 11,     // -
			6,  1,  8,  3, 10,  5, 12,     // #
			7,  2,  9,  4, 11,  6, 13      // ##
		};
		return pitches[tpc + 1];
	}
	/*
	void vrv::Transpose::transposeTpc(int &origStep, int &accAlter, vrv::Transpose::Interval interval, bool useDoubleSharpsFlats)
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

		if (semitones == 0 && steps == 0)
			return;

		int step;
		int alter;
		int pitch = PitchFromPname(static_cast<data_PITCHNAME>(origStep)) + accAlter;
		//static int pitches[] = {
		//		0,2,4,5,7,9,11
		//};
		//int pitch = pitches[origStep - 1] + accAlter;
		for (int k = 0; k < 10; ++k) {



			step = (origStep - 1) + steps;
			while (step < 0)
				step += 7;
			step %= 7;
			int p1 = PitchFromPname(static_cast<data_PITCHNAME>(step));
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
		origStep = step;
		accAlter = alter;
	}
	*/
	void vrv::Transpose::transposeTpc(int tpc, vrv::Transpose::Interval interval, bool useDoubleSharpsFlats, int& step, int& alter, int& oct)
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
			{
				step += 7;
				oct--;
			}
			while (step > 7)
			{
				step -= 7;
				oct++;
			}
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