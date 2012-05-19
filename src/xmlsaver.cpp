// XMLSaver.cpp: implementation of the XMLSaver class.
//
//////////////////////////////////////////////////////////////////////

#include "Precompile.h"
#include "XMLSaver.h"
#include "XMLWriter.h"
#include <math.h>

#ifdef DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

inline	int		XMLPosFromNWCPos(int nPos)
{
	/*
	 *	NWC		XML
	 *	-2		0
	 *	-1		-10
	 *	0		-20
	 */
	return -( nPos * 10 + 20 );
}

wxString Str2XML( const wxString& str )
{
	wxString s = str;
	s.Replace(_T("&"), _T("&amp;"));
	s.Replace(_T("<"), _T("&lt;"));
	s.Replace(_T(">"), _T("&gt;"));
	return s;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XMLSaver::XMLSaver()
{
	m_nVersion = MXML_10;
}

XMLSaver::~XMLSaver()
{
}

inline	void	XMLSaver::SaveClef(CXMLWriter& writer, const CClefObj& obj)
{
	writer.WriteKeyStart(_T("attributes"));
	writer.WriteKeyStart(_T("clef"));

		wxChar chSign[2] = {'G', 0};
		int nLine = 2;
		int nClefOctaveChange = 0;
		switch ( obj.mClefType )
		{
		case	0 :	//Treble
			chSign[0] = 'G'; nLine = 2;
			m_CA.clefShift = 0;
			break;
		case	1 :	//Bass
			chSign[0] = 'F'; nLine = 4;
			m_CA.clefShift = 12;
			break;
		case	2 :	//Alto
			chSign[0] = 'C'; nLine = 3;
			m_CA.clefShift = 0;
			break;
		case	3 :	//Tenor
			chSign[0] = 'G'; nLine = 2;
			nClefOctaveChange = -1;
			m_CA.clefShift = 0;
			break;
		}

		writer.WriteString(_T("sign"), chSign);
		writer.WriteInteger(_T("line"), nLine);
		if ( nClefOctaveChange != 0 )
			writer.WriteInteger(_T("clef-octave-change"), nClefOctaveChange);

	writer.WriteKeyEnd();
	writer.WriteKeyEnd();
}

inline int bitcount(BYTE btData)
{
#if 1
	// Sparse bitcount
	unsigned nCount;
	for (nCount = 0; btData ; nCount++)
		btData &= btData-1;
	return nCount;
#else
	// Iterated bitcount
	int nCount = 0;
	while( btData )
	{
		if ( btData & 0x01 )
			nCount++;

		btData >>= 1;
	}

	return nCount;
#endif
}

inline	void	XMLSaver::SaveKeySig(CXMLWriter& writer, const CKeySigObj& obj)
{
	writer.WriteKeyStart(_T("attributes"));
	writer.WriteKeyStart(_T("key"));

		obj.GetChromAlter(m_CA);

		int nCount = 0;
		if ( obj.mSharp )
			nCount = bitcount(obj.mSharp);
		else if ( obj.mFlat )
			nCount = -bitcount(obj.mFlat);

		writer.WriteInteger(_T("fifths"), nCount);

	writer.WriteKeyEnd();
	writer.WriteKeyEnd();
}

bool FindEnding(CObj *const * pNext, CObj *const * pLast, wxString& strNumber)
{
	wxString str;
	for ( ; pNext != pLast ; pNext++ )
	{
		CObj* pObj = *pNext;
		if ( pObj->mObjType == Obj_BarLine )
			return false;

		if ( pObj->mObjType == Obj_Ending )
		{
			CEndingObj* pEObj = (CEndingObj*)pObj;
			int nNumber = 1;
			UINT nEnding = pEObj->mStyle;
			while ( nEnding )
			{
				if ( nEnding & 0x01 )
				{
					if ( strNumber.Length() )
						str.Printf(_T(", %d"), nNumber);
					else
						str.Printf(_T("%d"), nNumber);

					strNumber += str;
				}

				nEnding >>= 1;
				nNumber ++;
			}

			return true;
		}
	}

	return false;
}

inline	void	XMLSaver::SaveBarLine(CXMLWriter& writer, const CBarLineObj& obj, int& nMeasurement, BOOL bNewMeasure,
									  CObj *const * pNext, CObj *const * pLast)
{
	BL_STYLE blStyle = obj.GetStyle();

	if ( blStyle == BL_SECTION_CLOSE || blStyle == BL_LOCAL_CLOSE || blStyle == BL_MASTER_CLOSE )
	{
		writer.WriteKeyStart(_T("barline"));
		writer.WriteAttrString(_T("location"), _T("right"));
		if ( blStyle == BL_SECTION_CLOSE || blStyle == BL_MASTER_CLOSE )
			writer.WriteString(_T("bar-style"), _T("light-heavy"));
		else if ( blStyle == BL_LOCAL_CLOSE )
			writer.WriteString(_T("bar-style"), _T("light-light"));

		if ( m_strEnding.Length() && (blStyle == BL_LOCAL_CLOSE || blStyle == BL_MASTER_CLOSE) )
		{
			writer.WriteKeyStart(_T("ending"));
			writer.WriteAttrString(_T("type"), _T("stop"));
			writer.WriteAttrString(_T("number"), m_strEnding);
			writer.WriteKeyEnd();
			m_strEnding.Empty();
		}

		if ( blStyle == BL_LOCAL_CLOSE )
		{
			writer.WriteKeyStart(_T("repeat"));
			writer.WriteAttrString(_T("direction"), _T("backward"));
			if ( obj.mLocalRepeatCount == 2 )
				;
			else
				writer.WriteAttrInteger(_T("times"), obj.mLocalRepeatCount);
			writer.WriteKeyEnd();
		}
		else if ( blStyle == BL_MASTER_CLOSE )
		{
			writer.WriteKeyStart(_T("repeat"));
			writer.WriteAttrString(_T("direction"), _T("backward"));
			writer.WriteKeyEnd();
		}
		writer.WriteKeyEnd();
	}

	if ( bNewMeasure )
	{
		writer.WriteKeyEnd();

		nMeasurement++;
		m_CA.ResetMeasure();

		writer.WriteKeyStart(_T("measure"));
		writer.WriteAttrInteger(_T("number"), nMeasurement);
	}

	wxString strNumber;
	BOOL bNumber = FindEnding(pNext, pLast, strNumber);

	if ( bNumber || blStyle == BL_DOUBLE || blStyle == BL_SECTION_OPEN || blStyle == BL_LOCAL_OPEN || blStyle == BL_MASTER_OPEN )
	{
		writer.WriteKeyStart(_T("barline"));
		writer.WriteAttrString(_T("location"), _T("left"));
		if ( blStyle == BL_SECTION_OPEN || blStyle == BL_MASTER_OPEN )
			writer.WriteString(_T("bar-style"), _T("heavy-light"));
		else if ( blStyle == BL_DOUBLE || blStyle == BL_LOCAL_OPEN )
			writer.WriteString(_T("bar-style"), _T("light-light"));

		if ( bNumber )
		{
			writer.WriteKeyStart(_T("ending"));
			writer.WriteAttrString(_T("type"), _T("start"));
			writer.WriteAttrString(_T("number"), strNumber);
			writer.WriteKeyEnd();
			m_strEnding = strNumber;
		}

		if ( blStyle == BL_LOCAL_OPEN || blStyle == BL_MASTER_OPEN )
		{
			writer.WriteKeyStart(_T("repeat"));
			writer.WriteAttrString(_T("direction"), _T("forward"));
			writer.WriteKeyEnd();
		}
		writer.WriteKeyEnd();
	}
}

inline	void	XMLSaver::SaveEndingBarLine(CXMLWriter& writer, const CStaff& staff)
{
	BYTE btStyle = staff.mStaffInfo.btEndingBar;
	BL_STYLE blStyle = BL_SINGLE;

	// index of [Section Close, Master Repeat Close, Single, Double, Open(hidden)]
	switch ( btStyle )
	{
	case	0 :	blStyle = BL_SECTION_CLOSE;	break;
	case	1 :	blStyle = BL_MASTER_CLOSE;	break;
	case	2 :	blStyle = BL_SINGLE;		break;
	case	3 :	blStyle = BL_DOUBLE;		break;
	case	4 :	blStyle = BL_HIDDEN;		break;
	}

	if ( m_strEnding.Length() || blStyle != BL_SINGLE )
	{
		writer.WriteKeyStart(_T("barline"));
		writer.WriteAttrString(_T("location"), _T("right"));
		if ( blStyle == BL_SECTION_CLOSE || blStyle == BL_MASTER_CLOSE )
			writer.WriteString(_T("bar-style"), _T("light-heavy"));
		else if ( blStyle == BL_DOUBLE )
			writer.WriteString(_T("bar-style"), _T("light-light"));
		else if ( blStyle == BL_HIDDEN )
			writer.WriteString(_T("bar-style"), _T("none"));

		if ( m_strEnding.Length() )
		{
			writer.WriteKeyStart(_T("ending"));
			writer.WriteAttrString(_T("type"), _T("stop"));
			writer.WriteAttrString(_T("number"), m_strEnding);
			writer.WriteKeyEnd();
			m_strEnding.Empty();
		}

		if ( blStyle == BL_MASTER_CLOSE )
		{
			writer.WriteKeyStart(_T("repeat"));
			writer.WriteAttrString(_T("direction"), _T("backward"));
			writer.WriteKeyEnd();
		}
		writer.WriteKeyEnd();
	}
}

inline	void	XMLSaver::SaveEnding(CXMLWriter& WXUNUSED(writer), const CEndingObj& WXUNUSED(obj))
{
	// handled by SaveBarLine
}

inline	void	XMLSaver::SaveInstrument(CXMLWriter& WXUNUSED(writer), const CInstrumentObj& WXUNUSED(obj))
{
#if 0
	writer.WriteKeyStart("direction");
	writer.WriteKeyStart("sound");

		writer.WriteKeyStart("midi-instrument");
		writer.WriteAttrInteger("relative-y", XMLPosFromNWCPos(obj.mPos));
		writer.WriteString(obj.GetStyleAsString(FALSE));
		writer.WriteKeyEnd();

	writer.WriteKeyEnd();
	writer.WriteKeyEnd();
#endif
}

inline	void	XMLSaver::SaveTimeSig(CXMLWriter& writer, const CTimeSigObj& obj)
{
	writer.WriteKeyStart(_T("attributes"));
		writer.WriteKeyStart(_T("time"));
		if ( obj.mStyle == 1 )
			writer.WriteAttrString(_T("symbol"), _T("common"));
		else if ( obj.mStyle == 2 )
			writer.WriteAttrString(_T("symbol"), _T("cut"));

			writer.WriteInteger(_T("beats"), obj.mBit_Measure);
			writer.WriteInteger(_T("beat-type"), 1 << obj.mBits);

		writer.WriteKeyEnd();
	writer.WriteKeyEnd();
}

inline	void	XMLSaver::SaveTempo(CXMLWriter& writer, const CTempoObj& obj)
{
	writer.WriteKeyStart(_T("sound"));
	writer.WriteAttrInteger(_T("tempo"), obj.GetTempoByQuarter());
	writer.WriteKeyEnd();
}

inline	void	XMLSaver::SaveDynamic(CXMLWriter& writer, const CDynamicObj& obj)
{
	writer.WriteKeyStart(_T("direction"));

		writer.WriteKeyStart(_T("direction-type"));

			writer.WriteKeyStart(_T("dynamics"));
			writer.WriteAttrInteger(_T("relative-y"), XMLPosFromNWCPos(obj.mPos));
				writer.WriteKeyStartEnd(obj.GetStyleAsString(FALSE));
			writer.WriteKeyEnd();

		writer.WriteKeyEnd();

	writer.WriteKeyEnd();
}

const wxChar* GetNoteType(UINT nDuration)
{
	static wxChar *s_Types[] = {_T("whole"), _T("half"), _T("quarter"), _T("eighth"), _T("16th"), _T("32nd"), _T("64th") };
	if ( nDuration < _countof(s_Types) )
		return s_Types[nDuration];
	return _T("unknown");
}

inline	bool	CanHaveLyric(NOTEATTR na)
{
	switch ( na & NA_SLUR_MASK )
	{
	case	NA_SLUR_END :
	case	NA_SLUR_MID :
		return false;
	}

	if ( na & NA_TIE_END )
		return false;

	return true;
}

inline	void	XMLSaver::SaveNote(CXMLWriter& writer, const CNoteObj& obj,
								   long nDivision,
								   const CLyricArray& strLyrics, int& nLyricIndex, bool bChord)
{
	writer.WriteKeyStart(_T("note"));

		if ( bChord )
			writer.WriteKeyStartEnd(_T("chord"));

		wxChar chStep[2] = { 0 };
		int nOctave;
		int nAlter;
		obj.GetOctaveStep(m_CA, nOctave, chStep[0], nAlter);
		writer.WriteKeyStart(_T("pitch"));

			writer.WriteString(_T("step"), chStep);
			if ( nAlter )
				writer.WriteInteger(_T("alter"), nAlter);
			writer.WriteInteger(_T("octave"), nOctave);

		writer.WriteKeyEnd();

		writer.WriteInteger(_T("duration"), obj.GetDuration(nDivision));

		DURATIONTYPE dt = obj.GetDurationType();
		NOTEATTR na = obj.GetNoteAttributes();

		if ( na & NA_TIE_END )
		{
			writer.WriteKeyStart(_T("tie"));
			writer.WriteAttrString(_T("type"), _T("stop"));
			writer.WriteKeyEnd();
		}
		else if ( na & NA_TIE_BEG )
		{
			if ( na & NA_TIE_DIR_MASK )
			{
				wxChar* szDir = ( (na & NA_TIE_DIR_MASK) == NA_TIE_DIR_DOWN ) ? _T("under") : _T("over");
				writer.WriteKeyStart(_T("tie"));
				writer.WriteAttrString(_T("type"), _T("start"));

					writer.WriteString(_T("orientation"), szDir);

				writer.WriteKeyEnd();
			}
			else
			{
				writer.WriteKeyStart(_T("tie"));
				writer.WriteAttrString(_T("type"), _T("start"));
				writer.WriteKeyEnd();
			}
		}

		if ( na & NA_GRACE )
			writer.WriteKeyStartEnd(_T("grace"));

		if ( m_nVoice )
			writer.WriteInteger(_T("voice"), m_nVoice);

		writer.WriteString(_T("type"), GetNoteType(obj.GetDuration()));

		if ( dt & DT_DOTDOT )
		{
			writer.WriteKeyStartEnd(_T("dot"));
			writer.WriteKeyStartEnd(_T("dot"));
		}
		else if ( dt & DT_DOT )
			writer.WriteKeyStartEnd(_T("dot"));

		{
			ACCIDENTAL ac = obj.GetAccidental();
			wxChar* szType = NULL;
			switch ( ac )
			{
			case	AC_SHARP : szType = _T("sharp"); break;
			case	AC_FLAT : szType = _T("flat"); break;
			case	AC_NATURAL : szType = _T("natural"); break;
			case	AC_SHARP_SHARP : szType = _T("sharp-sharp"); break;
			case	AC_FLAT_FLAT : szType = _T("flat-flat"); break;
			}

			if ( szType )
				writer.WriteString(_T("accidental"), szType);
		}

		if ( dt & DT_TRIPLET )
		{
			writer.WriteKeyStart(_T("time-modification"));
				writer.WriteInteger(_T("actual-notes"), 3);
				writer.WriteInteger(_T("normal-notes"), 2);
			writer.WriteKeyEnd();
		}

		BOOL bStemUp = FALSE;
		switch ( na & NA_STEM_MASK )
		{
		case	NA_STEM_UP :
			writer.WriteString(_T("stem"), _T("up"));
			bStemUp = TRUE;
			break;
		case	NA_STEM_DOWN :
			writer.WriteString(_T("stem"), _T("down"));
			bStemUp = FALSE;
			break;
		default :
			bStemUp = obj.mPos <= 0;
			break;
		}

		if ( na & NA_BEAM_MASK )
		{
			wxChar* szType = NULL;
			switch ( na & NA_BEAM_MASK )
			{
			case	NA_BEAM_BEG : szType = _T("begin"); break;
			case	NA_BEAM_END : szType = _T("end"); break;
			case	NA_BEAM_MID : szType = _T("continue"); break;
			}

			writer.WriteKeyStart(_T("beam"));
			writer.WriteAttrInteger(_T("number"), 1);
			writer.WriteString(szType);
			writer.WriteKeyEnd();
		}

		BOOL bNotations = (na & (NA_TIE_BEG|NA_TIE_END|NA_SLUR_MASK) ) != 0;
		BOOL bArticulation = ( na & (NA_ACCENT|NA_STACCATO|NA_TENUTO) ) != 0;
		BOOL bTriplet = ( dt & DT_TRIPLET ) != 0;

		if ( bNotations || bArticulation || bTriplet )
		{
			writer.WriteKeyStart(_T("notations"));

			if ( na & NA_TIE_END )
			{
				writer.WriteKeyStart(_T("tied"));
				writer.WriteAttrString(_T("type"), _T("stop"));
				writer.WriteKeyEnd();
			}
			else if ( na & NA_TIE_BEG )
			{
				writer.WriteKeyStart(_T("tied"));
				writer.WriteAttrString(_T("type"), _T("start"));
				writer.WriteKeyEnd();
			}

			if ( na & NA_SLUR_MASK )
			{
				wxChar* szType = NULL;
				switch ( na & NA_SLUR_MASK )
				{
				case	NA_SLUR_BEG : szType = _T("start"); break;
				case	NA_SLUR_END : szType = _T("stop"); break;
				case	NA_SLUR_MID : szType = _T("continue"); break;
				}

				writer.WriteKeyStart(_T("slur"));
				writer.WriteAttrString(_T("type"), szType);
				writer.WriteAttrInteger(_T("number"), m_nSlurNo);
				if ( (na & NA_SLUR_MASK) == NA_SLUR_BEG && (na & NA_SLUR_DIR_MASK) )
				{
					wxChar* szDir = ( (na & NA_SLUR_DIR_MASK) == NA_SLUR_DIR_DOWN ) ? _T("below") : _T("above");
					writer.WriteAttrString(_T("placement"), szDir);
				}
				writer.WriteKeyEnd();

				if ( (na & NA_SLUR_MASK) == NA_SLUR_END )
				{
					m_nSlurNo++;
				}
			}

			if ( bTriplet )
			{
				if (  ( dt & DT_TRIPLET ) == DT_TRI_START )
				{
					writer.WriteKeyStart(_T("tuplet"));
					writer.WriteAttrString(_T("type"), _T("start"));
					writer.WriteAttrString(_T("placement"), bStemUp ? _T("above") : _T("below"));
					writer.WriteKeyEnd();
				}
				else if ( ( dt & DT_TRIPLET ) == DT_TRI_STOP )
				{
					writer.WriteKeyStart(_T("tuplet"));
					writer.WriteAttrString(_T("type"), _T("stop"));
					writer.WriteAttrString(_T("placement"), bStemUp ? _T("above") : _T("below"));
					writer.WriteKeyEnd();
				}
			}

			if ( bArticulation )
			{
				writer.WriteKeyStart(_T("articulations"));
				if ( na & NA_ACCENT )
				{
					writer.WriteKeyStart(_T("accent"));
					writer.WriteAttrString(_T("placement"), bStemUp ? _T("below") : _T("above"));
					writer.WriteKeyEnd();
				}
				if ( na & NA_STACCATO )
				{
					writer.WriteKeyStart(_T("staccato"));
					writer.WriteAttrString(_T("placement"), bStemUp ? _T("below") : _T("above"));
					writer.WriteKeyEnd();
				}
				if ( na & NA_TENUTO )
				{
					writer.WriteKeyStart(_T("tenuto"));
					writer.WriteAttrString(_T("placement"), bStemUp ? _T("below") : _T("above"));
					writer.WriteKeyEnd();
				}
				writer.WriteKeyEnd();
			}

			writer.WriteKeyEnd();
		}

		if ( strLyrics.GetCount() && CanHaveLyric(na) )
		{
			for ( size_t i=0 ; i<strLyrics.GetCount() ; i++ )
			{
				const TVector<wxString>& strLyric = strLyrics[i];
				if ( (size_t)nLyricIndex >= strLyric.GetCount() )
					continue;

				writer.WriteKeyStart(_T("lyric"));
				writer.WriteAttrInteger(_T("number"), (int)(i+1));
				wxString str = strLyric[nLyricIndex];
				str.Trim(false);
				str.Trim(true);
				wxString strNext;
				if( (size_t)(nLyricIndex+1) < strLyric.GetCount() )
					strNext=strLyric[nLyricIndex+1];
				strNext.Trim(false);
				strNext.Trim(true);
				LPCTSTR szSyllable = NULL;
				if( str.Length() && str[0] == '-' )
				{
					// middle or end syllable
					if( strNext.Length() && strNext[0] == '-' )
					{
						szSyllable = _T("middle");
					}
					else
					{
						szSyllable = _T("end");
					}
					str = str.Mid(1);
				}
				else
				{
					// single or begin
					if( strNext.Length() && strNext[0] == '-' )
					{
						szSyllable = _T("begin");
					}
					else
					{
						szSyllable = _T("single");
					}
				}
				writer.WriteString(_T("syllabic"), szSyllable);
				writer.WriteString(_T("text"), str);
				writer.WriteKeyEnd();
			}

			if ( nLyricIndex >= 0 )
				nLyricIndex++;
		}

	writer.WriteKeyEnd();
}

inline	void	XMLSaver::UpdateNoteAcc(const CNoteObj& obj)
{
	wxChar chStep[2] = { 0 };
	int nOctave;
	int nAlter;
	obj.GetOctaveStep(m_CA, nOctave, chStep[0], nAlter);
}

inline	void	XMLSaver::SaveRest(CXMLWriter& writer, const CRestObj& obj, long nDivision, bool bChord)
{
	writer.WriteKeyStart(_T("note"));

		if ( bChord )
			writer.WriteKeyStartEnd(_T("chord"));

		wxChar chStep[2] = {0};
		int nOctave;
		BOOL bNotDefPos = obj.GetOctaveStep(m_CA, nOctave, chStep[0]);
		if ( bNotDefPos )
		{
			writer.WriteKeyStart(_T("rest"));
				writer.WriteString(_T("display-step"), chStep);
				writer.WriteInteger(_T("display-octave"), nOctave);
			writer.WriteKeyEnd();
		}
		else
		{
			writer.WriteKeyStartEnd(_T("rest"));
		}

		writer.WriteInteger(_T("duration"), obj.GetDuration(nDivision));
		if ( m_nVoice )
			writer.WriteInteger(_T("voice"), m_nVoice);
		writer.WriteString(_T("type"), GetNoteType(obj.GetDuration()));
		DURATIONTYPE dt = obj.GetDurationType();
		if ( dt & DT_DOTDOT )
		{
			writer.WriteKeyStartEnd(_T("dot"));
			writer.WriteKeyStartEnd(_T("dot"));
		}
		else if ( dt & DT_DOT )
			writer.WriteKeyStartEnd(_T("dot"));

	writer.WriteKeyEnd();
}

inline	void	XMLSaver::SaveNoteCM(CXMLWriter& writer, const CNoteCMObj& obj,
									 long nDivision,
									 const CLyricArray& strLyrics, int& nLyricIndex)
{
	UINT nStemMask = obj.GetStemMask();
	BOOL bLyricSaved = FALSE;
	BOOL bSaveNote = FALSE;
	for ( size_t i=0 ; i<obj.mObjArray.GetCount() ; i++ )
	{
		CObj* pObj = obj.mObjArray[i];
		bSaveNote = FALSE;

		switch( pObj->mObjType )
		{
		case	Obj_Note :
			if ( m_nVoice == 0 || nStemMask != 0x03 )
			{
				bSaveNote = TRUE;
			}
			else if ( ((CNoteObj*)pObj)->IsStemUp() )
			{
				if ( m_nVoice == 1 )
					bSaveNote = TRUE;
			}
			else
			{
				if ( m_nVoice == 2 )
					bSaveNote = TRUE;
			}

			if ( bSaveNote == FALSE )
			{
				UpdateNoteAcc(*(CNoteObj*)pObj);
				continue;
			}

			if ( bLyricSaved == FALSE )
			{
				SaveNote(writer, *(CNoteObj*)pObj, nDivision, strLyrics, nLyricIndex);
				bLyricSaved = TRUE;
			}
			else
			{
				int nLyricIndex1 = -1;	// ignore lyric
				SaveNote(writer, *(CNoteObj*)pObj, nDivision, strLyrics, nLyricIndex1, true);
			}
			break;
		case	Obj_Rest :
			SaveRest(writer, *(CRestObj*)pObj, nDivision);
			break;
		}
	}
}

inline	void	XMLSaver::SavePedal(CXMLWriter& writer, const CPedalObj& obj)
{
	PEDALSTYLE ps = obj.GetPedalStyle();
	wxChar* szStyle = NULL;
	switch ( ps )
	{
	case	PS_RELEASE :	szStyle = _T("stop");	break;
	case	PS_DOWN :		szStyle = _T("start");	break;
	}

	if ( szStyle == NULL )
		return;

	writer.WriteKeyStart(_T("direction"));
	writer.WriteKeyStart(_T("direction-type"));

		writer.WriteKeyStart(_T("pedal"));
		writer.WriteAttrInteger(_T("relative-y"), XMLPosFromNWCPos(obj.mPos));
		writer.WriteString(szStyle);
		writer.WriteKeyEnd();

	writer.WriteKeyEnd();
	writer.WriteKeyEnd();
}

inline	void	XMLSaver::SaveFlowDir(CXMLWriter& writer, const CFlowDirObj& obj)
{
#if	0
	FLOWSTYLE fs = obj.GetFlowStyle();
	switch ( fs )
	{
	case	FS_CODA :
	case	FS_SEGNO :
	case	FS_FINE :
	case	FS_TOCODA :
	case	FS_DACAPO :
	case	FS_DALSEGNO :
		break;

	default :
		return;
	}

	writer.WriteKeyStart(_T("direction"));
		if ( fs == FS_CODA )//|| fs == FS_SEGNO )
		{
			writer.WriteKeyStart(_T("direction-type"));
			if ( fs == FS_CODA )
				writer.WriteKeyStartEnd(_T("coda"));
			else if ( fs == FS_SEGNO )
				writer.WriteKeyStartEnd(_T("segno"));

			writer.WriteKeyEnd();
		}

		writer.WriteKeyStart(_T("sound"));
		switch ( fs )
		{
		case	FS_CODA :
			writer.WriteAttrString(_T("coda"), _T(""));
			break;
		case	FS_SEGNO :
			writer.WriteAttrString(_T("segno"), _T(""));
			break;
		case	FS_FINE :
			writer.WriteAttrString(_T("fine"), _T(""));
			break;
		case	FS_TOCODA :
			writer.WriteAttrString(_T("tocoda"), _T(""));
			break;
		case	FS_DACAPO :
			writer.WriteAttrString(_T("dacapo"), _T(""));
			break;
		case	FS_DALSEGNO :
			writer.WriteAttrString(_T("dalsegno"), _T(""));
			break;
		}
		writer.WriteKeyEnd();

	writer.WriteKeyEnd();
#endif
}

inline	void	XMLSaver::SaveMPC(CXMLWriter& WXUNUSED(writer), const CMPCObj& WXUNUSED(obj))
{
}

inline	void	XMLSaver::SaveTempVar(CXMLWriter& WXUNUSED(writer), const CTempVarObj& WXUNUSED(obj))
{
	// process fermata
}

inline	void	XMLSaver::SaveDynVar(CXMLWriter& WXUNUSED(writer), const CDynVarObj& WXUNUSED(obj))
{
}

inline	void	XMLSaver::SavePerform(CXMLWriter& WXUNUSED(writer), const CPerformObj& WXUNUSED(obj))
{
}

inline	void	XMLSaver::SaveText(CXMLWriter& writer, const CTextObj& obj, const wxChar* szLang)
{
	writer.WriteKeyStart(_T("direction"));
	//writer.WriteAttrString("placement", "above");
		writer.WriteKeyStart(_T("direction-type"));

			writer.WriteKeyStart(_T("words"));
			if ( szLang )
				writer.WriteAttrString(_T("xml:lang"), szLang);
			writer.WriteAttrInteger(_T("relative-y"), XMLPosFromNWCPos(obj.mPos));
			writer.WriteString(obj.mText);
			//writer.TextPrint("<words xml:lang="de" font-weight="bold" relative-y="45" relative-x="-41">No. 1</words>\n");
			writer.WriteKeyEnd();

		writer.WriteKeyEnd();

	writer.WriteKeyEnd();
}

inline	void	XMLSaver::SaveRestCM(CXMLWriter& writer, const CRestCMObj& obj, long nDivision)
{
	SaveRest(writer, obj, nDivision);

	CLyricArray strLyrics;
	int nLyricIndex = -1;	// ignore lyric

	for ( size_t i=0 ; i<obj.mObjArray.GetCount() ; i++ )
	{
		CObj* pObj = obj.mObjArray[i];

		switch( pObj->mObjType )
		{
		case	Obj_Note :
			{
				SaveNote(writer, *(CNoteObj*)pObj, nDivision, strLyrics, nLyricIndex, true);
			}
			break;
		case	Obj_Rest :
			SaveRest(writer, *(CRestObj*)pObj, nDivision);
			break;
		}
	}
}

inline	void XMLSaver::CheckMultiVoice(const CStaff& staff, int nIndex, int& nStartIndex)
{
	UINT  nStemMask = 0;
	for ( size_t i=nIndex ; i<staff.mObjArray.GetCount() ; i++ )
	{
		CObj* pObj = staff.mObjArray[i];
		nStemMask = 0;
		switch( pObj->mObjType )
		{
		default :
			continue;

		case	Obj_Ending :
		case	Obj_BarLine :
			return;

		case	Obj_NoteCM :
			{
				CNoteCMObj& obj = *(CNoteCMObj*)pObj;
				nStemMask = obj.GetStemMask();
			}
			break;

		case	Obj_RestCM :
			{
				CRestCMObj& obj = *(CRestCMObj*)pObj;
				nStemMask = obj.GetStemMask();
			}
			break;
		}

		if ( nStemMask == 0x03 )
		{
			nStartIndex = nIndex;
			break;
		}
	}
}

inline	void XMLSaver::SaveForward(CXMLWriter& writer, int nDuration)
{
	writer.WriteKeyStart(_T("forward"));
	writer.WriteInteger(_T("duration"), nDuration);
	writer.WriteInteger(_T("voice"), m_nVoice);
	writer.WriteKeyEnd();
}

inline	void XMLSaver::SaveMultiVoice(CXMLWriter& writer, const CStaff& staff,
									  long nDivision, int nStartIndex, int nEndIndex, int nMeasureDuration)
{
	writer.WriteKeyStart(_T("backup"));
	writer.WriteInteger(_T("duration"), nMeasureDuration);
	writer.WriteKeyEnd();

	m_CA.ResetMeasure();

	int nDuration = 0;
	int nStemMask = 0;
	TVector<CObj*>*	pObjArray;
	m_nVoice = 2;
	BOOL bContinue = TRUE;
	for ( size_t i=nStartIndex ; bContinue && i<staff.mObjArray.GetCount() ; i++ )
	{
		CObj* pObj = staff.mObjArray[i];
		pObjArray = NULL;
		switch( pObj->mObjType )
		{
		default :
			continue;

		case	Obj_Ending :
		case	Obj_BarLine :
			bContinue = FALSE;	// break for
			break;

		case	Obj_Note :
			if ( ((CNoteObj*)pObj)->IsStemUp() )
			{
				//nDuration += ((CNoteObj*)pObj)->GetDuration(nDivision);
				UpdateNoteAcc(*(CNoteObj*)pObj);
			}
			else
			{
				if ( nDuration )
				{
					SaveForward(writer, nDuration);
					nDuration = 0;
				}

				int nLyricIndex = -1;
				SaveNote(writer, *(CNoteObj*)pObj, nDivision, staff.m_strLyrics, nLyricIndex);
			}
			break;
		case	Obj_Rest :
			nDuration += ((CRestObj*)pObj)->GetDuration(nDivision);
			break;

		case	Obj_NoteCM :
			{
				CNoteCMObj& obj = *(CNoteCMObj*)pObj;
				pObjArray = &obj.mObjArray;
			}
			break;

		case	Obj_RestCM :
			{
				CRestCMObj& obj = *(CRestCMObj*)pObj;
				pObjArray = &obj.mObjArray;
			}
			break;
		}

		if ( pObjArray )
		{
			nStemMask = 0;
			for ( size_t j=0 ; j<(*pObjArray).GetCount() ; j++ )
			{
				CObj* pObj1 = (*pObjArray)[j];

				if ( pObj1->mObjType == Obj_Note )
				{
					if ( ((CNoteObj*)pObj1)->IsStemUp() )
						nStemMask |= 0x01;
					else
						nStemMask |= 0x02;
				}
			}

			if ( nStemMask != 0x03 )
			{
				CObj* pObj1 = (*pObjArray)[0];
				switch( pObj1->mObjType )
				{
				case	Obj_Note :
					UpdateNoteAcc(*(CNoteObj*)pObj1);
					nDuration += ((CNoteObj*)pObj1)->GetDuration(nDivision);
					break;
				case	Obj_Rest :
					nDuration += ((CRestObj*)pObj1)->GetDuration(nDivision);
					break;
				}
			}
			else
			{
				for ( size_t j=0 ; j<(*pObjArray).GetCount() ; j++ )
				{
					CObj* pObj1 = (*pObjArray)[j];

					if ( pObj1->mObjType == Obj_Note )
					{
						if ( ((CNoteObj*)pObj1)->IsStemUp() == FALSE )
						{
							if ( nDuration )
							{
								SaveForward(writer, nDuration);
								nDuration = 0;
							}

							int nLyricIndex = -1;
							SaveNote(writer, *(CNoteObj*)pObj1, nDivision, staff.m_strLyrics, nLyricIndex);
						}
						else
						{
							UpdateNoteAcc(*(CNoteObj*)pObj1);
						}
					}
				}
			}
		}
	}

	if ( nDuration )
	{
		SaveForward(writer, nDuration);
		nDuration = 0;
	}
}

inline	int XMLSaver::GetFirstVoiceDuration(TVector<CObj*>& objArray, int nDivision) const
{
	UINT nStemMask = CNoteCMObj::GetStemMask(objArray);
	for ( size_t j=0 ; j<objArray.GetCount() ; j++ )
	{
		CObj* pObj = objArray[j];

		if ( pObj->mObjType == Obj_Note )
		{
			if ( nStemMask != 0x03 || ((CNoteObj*)pObj)->IsStemUp() )
				return ((CNoteObj*)pObj)->GetDuration(nDivision);
		}
		else if ( pObj->mObjType == Obj_RestCM )
		{
			return ((CRestObj*)pObj)->GetDuration(nDivision);
		}
	}

	return 0;
}

inline	void XMLSaver::Save(CXMLWriter& writer, const CStaff& staff, int nMeasurement, const wxChar* szLang)
{
	writer.WriteKeyStart(_T("measure"));
	writer.WriteAttrInteger(_T("number"), nMeasurement);

	writer.WriteKeyStart(_T("attributes"));
		long nDivisions = staff.GetDivisions();
		writer.WriteInteger(_T("divisions"), nDivisions);
	writer.WriteKeyEnd();

	m_strEnding.Empty();
	int nLyricIndex = 0;
	BOOL bCreateNewMeasure = FALSE;
	int nStartIndex = -1;
	m_nVoice = 0;
	m_nSlurNo = 1;
	int nMeasureDuration = 0;
	BOOL bSaveNote = FALSE;
	for ( size_t i=0 ; i<staff.mObjArray.GetCount() ; i++ )
	{
		CObj* pObj = staff.mObjArray[i];
		switch( pObj->mObjType )
		{
		case	Obj_Clef :		SaveClef(writer, *(CClefObj*)pObj); break;
		case	Obj_KeySig :	SaveKeySig(writer, *(CKeySigObj*)pObj); break;
		case	Obj_BarLine :
			if ( nStartIndex != -1 )
				SaveMultiVoice(writer, staff, nDivisions, nStartIndex, (int)i, nMeasureDuration);

			SaveBarLine(writer, *(CBarLineObj*)pObj, nMeasurement, bCreateNewMeasure,
						&staff.mObjArray[0] + i+1, &staff.mObjArray[0] + staff.mObjArray.GetCount());
			bCreateNewMeasure = FALSE;
			nStartIndex = -1;
			nMeasureDuration = 0;
			m_nSlurNo = 1;
			break;
		case	Obj_Ending :	SaveEnding(writer, *(CEndingObj*)pObj); break;
		case	Obj_Instrument :SaveInstrument(writer, *(CInstrumentObj*)pObj); break;
		case	Obj_TimeSig :	SaveTimeSig(writer, *(CTimeSigObj*)pObj); break;
		case	Obj_Tempo :		SaveTempo(writer, *(CTempoObj*)pObj); break;
		case	Obj_Dynamic :	SaveDynamic(writer, *(CDynamicObj*)pObj); break;
		case	Obj_Note :
			if ( bCreateNewMeasure == FALSE )
			{
				CheckMultiVoice(staff, (int)i, nStartIndex);
				m_nVoice = nStartIndex == -1 ? 0 : 1;
			}

			bCreateNewMeasure = TRUE;

			bSaveNote = FALSE;
			if ( m_nVoice == 0 )
			{
				bSaveNote = TRUE;
			}
			else if ( ((CNoteObj*)pObj)->IsStemUp() )
			{
				if ( m_nVoice == 1 )
					bSaveNote = TRUE;
			}

			if ( bSaveNote )
			{
				SaveNote(writer, *(CNoteObj*)pObj, nDivisions, staff.m_strLyrics, nLyricIndex);
				if ( nStartIndex != -1 )
					nMeasureDuration += ((CNoteObj*)pObj)->GetDuration(nDivisions);
			}
			else
			{
				UpdateNoteAcc(*(CNoteObj*)pObj);
			}
			break;
		case	Obj_Rest :
			if ( bCreateNewMeasure == FALSE )
			{
				CheckMultiVoice(staff, (int)i, nStartIndex);
				m_nVoice = nStartIndex == -1 ? 0 : 1;
			}

			bCreateNewMeasure = TRUE;
			SaveRest(writer, *(CRestObj*)pObj, nDivisions);
			if ( nStartIndex != -1 )
				nMeasureDuration += ((CRestObj*)pObj)->GetDuration(nDivisions);
			break;
		case	Obj_NoteCM :
			if ( bCreateNewMeasure == FALSE )
			{
				CheckMultiVoice(staff, (int)i, nStartIndex);
				m_nVoice = nStartIndex == -1 ? 0 : 1;
			}

			bCreateNewMeasure = TRUE;
			SaveNoteCM(writer, *(CNoteCMObj*)pObj, nDivisions, staff.m_strLyrics, nLyricIndex);
			if ( nStartIndex != -1 )
				nMeasureDuration += GetFirstVoiceDuration( ((CNoteCMObj*)pObj)->mObjArray, nDivisions );
			break;
		case	Obj_Pedal :		SavePedal(writer, *(CPedalObj*)pObj); break;
		case	Obj_FlowDir :	SaveFlowDir(writer, *(CFlowDirObj*)pObj); break;
		case	Obj_MPC :		SaveMPC(writer, *(CMPCObj*)pObj); break;
		case	Obj_TempVar :	SaveTempVar(writer, *(CTempVarObj*)pObj); break;
		case	Obj_DynVar :	SaveDynVar(writer, *(CDynVarObj*)pObj); break;
		case	Obj_Perform :	SavePerform(writer, *(CPerformObj*)pObj); break;
		case	Obj_Text :		SaveText(writer, *(CTextObj*)pObj, szLang); break;
		case	Obj_RestCM :
			if ( bCreateNewMeasure == FALSE )
			{
				CheckMultiVoice(staff, (int)i, nStartIndex);
				m_nVoice = nStartIndex == -1 ? 0 : 1;
			}

			bCreateNewMeasure = TRUE;
			SaveRestCM(writer, *(CRestCMObj*)pObj, nDivisions);
			if ( nStartIndex != -1 )
				nMeasureDuration += GetFirstVoiceDuration( ((CRestCMObj*)pObj)->mObjArray, nDivisions );
			break;
		}
	}

	if ( nStartIndex != -1 )
		SaveMultiVoice(writer, staff, nDivisions, nStartIndex, (int)(staff.mObjArray.GetCount()-1), nMeasureDuration);
	
	SaveEndingBarLine(writer, staff);

	writer.WriteKeyEnd();
}

static wxChar *s_szGMPatchNames[] = {
	_T("Acoustic Grand Piano"),		// 1
	_T("Bright Acoustic Piano"),
	_T("Electric Grand Piano"),
	_T("Honky-tonk Piano"),
	_T("Electric Piano 1"),
	_T("Electric Piano 2"),
	_T("Harpsichord"),
	_T("Clavi"),
	_T("Celesta"),
	_T("Glockenspiel"),
	_T("Music Box"),
	_T("Vibraphone"),
	_T("Marimba"),
	_T("Xylophone"),
	_T("Tubular Bells"),
	_T("Dulcimer"),				// 16
	_T("Drawbar Organ"),
	_T("Percussive Organ"),
	_T("Rock Organ"),
	_T("Church Organ"),
	_T("Reed Organ"),
	_T("Accordion"),
	_T("Harmonica"),
	_T("Tango Accordion"),
	_T("Acoustic Guitar (nylon)"),
	_T("Acoustic Guitar (steel)"),
	_T("Electric Guitar (jazz)"),
	_T("Electric Guitar (clean)"),
	_T("Electric Guitar (muted)"),
	_T("Overdriven Guitar"),
	_T("Distortion Guitar"),
	_T("Guitar harmonics"),		// 32
	_T("Acoustic Bass"),
	_T("Electric Bass (finger)"),
	_T("Electric Bass (pick)"),
	_T("Fretless Bass"),
	_T("Slap Bass 1"),
	_T("Slap Bass 2"),
	_T("Synth Bass 1"),
	_T("Synth Bass 2"),
	_T("Violin"),
	_T("Viola"),
	_T("Cello"),
	_T("Contrabass"),
	_T("Tremolo Strings"),
	_T("Pizzicato Strings"),
	_T("Orchestral Harp"),
	_T("Timpani"),					// 48
	_T("String Ensemble 1"),
	_T("String Ensemble 2"),
	_T("SynthStrings 1"),
	_T("SynthStrings 2"),
	_T("Choir Aahs"),
	_T("Voice Oohs"),
	_T("Synth Voice"),
	_T("Orchestra Hit"),
	_T("Trumpet"),
	_T("Trombone"),
	_T("Tuba"),
	_T("Muted Trumpet"),
	_T("French Horn"),
	_T("Brass Section"),
	_T("SynthBrass 1"),
	_T("SynthBrass 2"),			// 64
	_T("Soprano Sax"),
	_T("Alto Sax"),
	_T("Tenor Sax"),
	_T("Baritone Sax"),
	_T("Oboe"),
	_T("English Horn"),
	_T("Bassoon"),
	_T("Clarinet"),
	_T("Piccolo"),
	_T("Flute"),
	_T("Recorder"),
	_T("Pan Flute"),
	_T("Blown Bottle"),
	_T("Shakuhachi"),
	_T("Whistle"),
	_T("Ocarina"),					// 80
	_T("Lead 1 (square)"),
	_T("Lead 2 (sawtooth)"),
	_T("Lead 3 (calliope)"),
	_T("Lead 4 (chiff)"),
	_T("Lead 5 (charang)"),
	_T("Lead 6 (voice)"),
	_T("Lead 7 (fifths)"),
	_T("Lead 8 (bass + lead)"),
	_T("Pad 1 (new age)"),
	_T("Pad 2 (warm)"),
	_T("Pad 3 (polysynth)"),
	_T("Pad 4 (choir)"),
	_T("Pad 5 (bowed)"),
	_T("Pad 6 (metallic)"),
	_T("Pad 7 (halo)"),
	_T("Pad 8 (sweep)"),			// 96
	_T("FX 1 (rain)"),
	_T("FX 2 (soundtrack)"),
	_T("FX 3 (crystal)"),
	_T("FX 4 (atmosphere)"),
	_T("FX 5 (brightness)"),
	_T("FX 6 (goblins)"),
	_T("FX 7 (echoes)"),
	_T("FX 8 (sci-fi)"),
	_T("Sitar"),
	_T("Banjo"),
	_T("Shamisen"),
	_T("Koto"),
	_T("Kalimba"),
	_T("Bag pipe"),
	_T("Fiddle"),
	_T("Shanai"),					// 112
	_T("Tinkle Bell"),
	_T("Agogo"),
	_T("Steel Drums"),
	_T("Woodblock"),
	_T("Taiko Drum"),
	_T("Melodic Tom"),
	_T("Synth Drum"),
	_T("Reverse Cymbal"),
	_T("Guitar Fret Noise"),
	_T("Breath Noise"),
	_T("Seashore"),
	_T("Bird Tweet"),
	_T("Telephone Ring"),
	_T("Helicopter"),
	_T("Applause"),
	_T("Gunshot"),					// 128
};

const wxChar* GetGMPatchName(int nPatch)
{
	if ( 0 < nPatch && nPatch < _countof(s_szGMPatchNames) )
		return s_szGMPatchNames[nPatch-1];
	return _T("");
}

BOOL	XMLSaver::Save(LPCTSTR szNWCFile, LPCTSTR szFile, CNWCFile* pNWCObj, MXML_VERSION nVersion)
{
	m_nVersion = nVersion;

	CXMLWriter writer(szFile);
	if ( FALSE == writer.IsOk() )
		return FALSE;

	wxString str;
	str.Printf(_T("-//Recordare//DTD MusicXML %d.%d Partwise//EN"),
				 HIBYTE(nVersion), LOBYTE(nVersion));

	writer.WriteDocTypePublic(_T("score-partwise"), str,
							  _T("http://www.musicxml.org/dtds/partwise.dtd"));

	writer.WriteKeyStart(_T("score-partwise"));

	if ( pNWCObj->strTitle.Length() )
	{
		writer.WriteKeyStart(_T("work"));
			writer.WriteString(_T("work-title"), pNWCObj->strTitle);
		writer.WriteKeyEnd();
	}

	{
		writer.WriteKeyStart(_T("identification"));
			if ( pNWCObj->strAuthor.Length() )
			{
				writer.WriteKeyStart(_T("creator"));
				writer.WriteAttrString(_T("type"), _T("transcriber"));
				writer.WriteString(pNWCObj->strAuthor);
				writer.WriteKeyEnd();
			}
			if ( pNWCObj->strCopyright1.Length() )
				writer.WriteString(_T("rights"), pNWCObj->strCopyright1);
			if ( pNWCObj->strCopyright2.Length() )
				writer.WriteString(_T("rights"), pNWCObj->strCopyright2);

			WIN32_FILE_ATTRIBUTE_DATA wfad;
			GetFileAttributesEx(szNWCFile, GetFileExInfoStandard, &wfad);
			SYSTEMTIME st, lt;
			FileTimeToSystemTime(&wfad.ftLastWriteTime, &st);
			SystemTimeToTzSpecificLocalTime(NULL, &st, &lt);

			writer.WriteKeyStart(_T("encoding"));
				writer.WriteString(_T("software"), _T("Noteworthy Composer"));
				str.Printf(_T("%04d-%02d-%02d"), lt.wYear, lt.wMonth, lt.wDay);
				writer.WriteString(_T("encoding-date"), str);
			writer.WriteKeyEnd();

		writer.WriteKeyEnd();
	}

	writer.WriteKeyStart(_T("part-list"));

	size_t i;
	for ( i=0 ; i<pNWCObj->mStaffs.GetCount() ; i++ )
	{
		CStaff& staff = *pNWCObj->mStaffs[i];

		writer.WriteKeyStart(_T("score-part"));
		str.Printf(_T("P%d"), i+1);
		writer.WriteAttrString(_T("id"), str);

			writer.WriteString(_T("part-name"), staff.strName);

			writer.WriteKeyStart(_T("score-instrument"));
			str.Printf(_T("P%d-I1"), i+1);
			writer.WriteAttrString(_T("id"), str);
			writer.WriteString(_T("instrument-name"), GetGMPatchName(staff.mStaffInfo.btPatchName));
			writer.WriteKeyEnd();

			writer.WriteKeyStart(_T("midi-instrument"));
			str.Printf(_T("P%d-I1"), i+1);
			writer.WriteAttrString(_T("id"), str);
			writer.WriteInteger(_T("midi-channel"), staff.mStaffInfo.btChannel+1);	// midi-channel is 1-based
			writer.WriteInteger(_T("midi-program"), staff.mStaffInfo.btPatchName+1);// midi-program is 1-based
			writer.WriteKeyEnd();

		writer.WriteKeyEnd();
	}

	writer.WriteKeyEnd();

	int nMeasurement = pNWCObj->nMeasureStart;
	for ( i=0 ; i<pNWCObj->mStaffs.GetCount() ; i++ )
	{
		CStaff& staff = *pNWCObj->mStaffs[i];

		writer.WriteKeyStart(_T("part"));
		str.Printf(_T("P%d"), i+1);
		writer.WriteAttrString(_T("id"), str);

			Save(writer, staff, nMeasurement);

		writer.WriteKeyEnd();
	}

	writer.WriteKeyEnd();

	return TRUE;
}
