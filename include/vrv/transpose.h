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

			bool operator!=(const Interval& a) const { return diatonic != a.diatonic || chromatic != a.chromatic; }
			bool operator==(const Interval& a) const { return diatonic == a.diatonic && chromatic == a.chromatic; }
		private:
			signed char diatonic;
			signed char chromatic;
		};

	public:
		Transpose(Doc *doc);
		bool transpose(int newFifths);
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

		Doc *m_doc;
	};
}