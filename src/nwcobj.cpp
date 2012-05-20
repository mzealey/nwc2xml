// nwcobj.cpp : Defines the classes for NWC objects.
//

#include "Precompile.h"
#include "nwcobj.h"
#include "nwcfile.h"

wxMBConv* g_pMBConv = &wxConvLocal;
bool g_bDumpOffset = true;

wxString LoadStringNULTerminated(wxFile& in)
{
	wxString str;
	char ch;
	TByteArray barr;
	while( in.Read(&ch, 1) && ch != 0 )
	{
		barr.Add(ch);
	}
	barr.Add(0);
#ifdef UNICODE
	wxWCharBuffer wcb;
	wcb = g_pMBConv->cMB2WX((const char*)&barr[0]);
	if ( (LPCTSTR)wcb == NULL || wcslen(wcb) == 0 )
	{
		wcb = wxConvISO8859_1.cMB2WX((const char*)&barr[0]);
	}

	str = wcb;
#else
	wxWCharBuffer wcb = g_pMBConv->cMB2WX((const char*)&barr[0]);
	wxCharBuffer cb = wxConvUTF8.cWC2WX(wcb);
	str = cb;
#endif

	return str;
}

wxString LoadStringSpaceTerminated(wxFile& in)
{
	wxString str;
	char ch;
	TByteArray barr;
	while( in.Read(&ch, 1) && ch != 0 && !isspace((unsigned char)ch) )
	{
		barr.Add(ch);
	}
	barr.Add(0);
#ifdef UNICODE
	wxWCharBuffer wcb = g_pMBConv->cMB2WX((const char*)&barr[0]);
	str = wcb;
#else
	wxWCharBuffer wcb = g_pMBConv->cMB2WX((const char*)&barr[0]);
	wxCharBuffer cb = wxConvUTF8.cWX2MB(wcb);
	str = cb;
#endif

	return str;
}

void SaveStringNULTerminated(wxFile& out, const wxString& str)
{
#ifdef UNICODE
	wxCharBuffer cb = g_pMBConv->cWX2MB(str);
	out.Write(cb, strlen(cb)+1);
#else
	wxWCharBuffer wcb = wxConvUTF8.cMB2WX(str);
	wxCharBuffer cb = g_pMBConv->cWX2MB(wcb);
	out.Write(cb, strlen(cb)+1);
#endif
}

bool CObj::Load( wxFile& file )
{
	bool bResult = true;
	if ( GetLoadedVersion() >= NWC_Version170 )
		bResult = ReadBytes(file, mVisible);
	else
	{
		mVisible = 0;
	}

	return bResult;
}

bool CObj::Dump(FILE* WXUNUSED(fp))
{
	return true;
}

void CObj::DumpBinary(FILE* fp, BYTE* pData, size_t nCount, bool bPrependComma)
{
	for ( size_t i=0 ; i<nCount ; i++ )
	{
		if ( i == 0 && bPrependComma == false )
			wxFprintf(fp, _T(" 0x%02x"), pData[i]);
		else
			wxFprintf(fp, _T(",0x%02x"), pData[i]);
	}
}

NWC_Version	CObj::GetLoadedVersion()
{
	wxASSERT( m_pParent != NULL );
	return (NWC_Version)m_pParent->m_pParent->nVersion;
}

bool CClefObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	bResult = ReadLEShort(file, mClefType) &&
			  ReadLEShort(file, mOctaveShift);
	if ( bResult == false )
		return false;

	return bResult;
}

bool CClefObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("clef: "));
	wxFprintf(fp, _T("Color(%d),ShowPrint(%d)"), GetColor(), GetShowPrinted());
	wxFprintf(fp, _T(",Clef(%d),OctaveShift(%d)"), mClefType, mOctaveShift);
	wxFprintf(fp, _T("\n"));
	return true;
}

void CKeySigObj::GetChromAlter(CHROMALTER& ca) const
{
	if ( mSharp )
	{
		for ( int i=0 ; i<7 ; i++ )
		{
			ca.system[i] = ( (mSharp >> i) & 0x01 ) ? 1 : 0;
		}
	}
	else if ( mFlat )
	{
		for ( int i=0 ; i<7 ; i++ )
		{
			ca.system[i] = ( (mFlat >> i) & 0x01 ) ? -1 : 0;
		}
	}

	ca.ResetMeasure();
}

// convert pitch(CDEFGAB) to nwc pitch : see Obj_KeySig
#define	_NP(ch)	(1 << (ch - 'A'))

struct pitches2keysig
{
	LPCTSTR	szKeySig;
	UINT	pitch;
};

// http://en.wikipedia.org/wiki/Key_signature

// static BYTE s_StandardSharpKeySigPitch[] = { 0x00, 0x20, 0x24, 0x64, 0x6C, 0x6D, 0x7D, 0x7F };
// static BYTE s_StandardFlatKeySigPitch[] = { 0x00, 0x02, 0x12, 0x13, 0x1B, 0x5B, 0x5F, 0x7F };

static pitches2keysig s_StandardSharpKeySig[] =
{
	_T("C"),	0,
	_T("G"),	_NP('F'),
	_T("D"),	_NP('F')|_NP('C'),
	_T("A"),	_NP('F')|_NP('C')|_NP('G'),
	_T("E"),	_NP('F')|_NP('C')|_NP('G')|_NP('D'),
	_T("B"),	_NP('F')|_NP('C')|_NP('G')|_NP('D')|_NP('A'),
	_T("F#"),	_NP('F')|_NP('C')|_NP('G')|_NP('D')|_NP('A')|_NP('E'),
	_T("Db"),	_NP('F')|_NP('C')|_NP('G')|_NP('D')|_NP('A')|_NP('E')|_NP('B'),
};

static pitches2keysig s_StandardFlatKeySig[] =
{
	_T("C"),	0,
	_T("F"),	_NP('B'),
	_T("Bb"),	_NP('B')|_NP('E'),
	_T("Eb"),	_NP('B')|_NP('E')|_NP('A'),
	_T("Ab"),	_NP('B')|_NP('E')|_NP('A')|_NP('D'),
	_T("Db"),	_NP('B')|_NP('E')|_NP('A')|_NP('D')|_NP('G'),
	_T("Gb"),	_NP('B')|_NP('E')|_NP('A')|_NP('D')|_NP('G')|_NP('C'),
	_T("B"),	_NP('B')|_NP('E')|_NP('A')|_NP('D')|_NP('G')|_NP('C')|_NP('F'),
};

bool	CKeySigObj::SetFifth(int nFifth)
{
	if ( nFifth > 0 )
	{
		if ( nFifth < _countof(s_StandardSharpKeySig) )
		{
			mSharp = s_StandardSharpKeySig[nFifth].pitch;
			mFlat  = 0;
			return true;
		}
	}
	else if ( nFifth < 0 )
	{
		if ( -nFifth < _countof(s_StandardFlatKeySig) )
		{
			mFlat  = s_StandardFlatKeySig[-nFifth].pitch;
			mSharp = 0;
			return true;
		}
	}

	return false;
}

bool	CKeySigObj::GetStdName(wxString& strName) const
{
	if ( mFlat == 0 )
	{
		for ( int nFifth=0; nFifth< _countof(s_StandardSharpKeySig) ; nFifth++ )
		{
			if ( mSharp == s_StandardSharpKeySig[nFifth].pitch )
			{
				strName = s_StandardSharpKeySig[nFifth].szKeySig;
				return true;
			}
		}
	}
	else if ( mSharp == 0 )
	{
		for ( int nFifth=0; nFifth< _countof(s_StandardFlatKeySig) ; nFifth++ )
		{
			if ( mFlat == s_StandardFlatKeySig[nFifth].pitch )
			{
				strName = s_StandardFlatKeySig[nFifth].szKeySig;
				return true;
			}
		}
	}
	else
	{

	}

	return false;
}

bool CKeySigObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	bResult = ReadBytes(file, mFlat) &&
			  ReadBytes(file, mData2) &&
			  ReadBytes(file, mSharp) &&
			  ReadBytes(file, mData3);

	return bResult;
}

bool CKeySigObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("key sig: "));

	wxString strName;
	if ( GetStdName(strName) )
	{
		wxFprintf(fp, _T("%s"), strName.c_str());
		DumpBinary(fp, mData2, sizeof(mData2));
	}
	else
	{
		wxFprintf(fp, _T("flat(0x%02x)"), mFlat);
		DumpBinary(fp, mData2, sizeof(mData2));
		wxFprintf(fp, _T(",sharp(0x%02x)"), mSharp);
	}
	DumpBinary(fp, mData3, sizeof(mData3));
	wxFprintf(fp, _T("\n"));
	return true;
}

bool CBarLineObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	bResult = ReadBytes(file, mStyle) &&
			  ReadBytes(file, mLocalRepeatCount);

	return bResult;
}

bool CBarLineObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("bar: "));

	UINT nStyle = mStyle & 0x7F;
	wxString strStyle;
	switch(nStyle)
	{
	case	0 :
		strStyle = _T("Single"); break;
	case	1 :
		strStyle = _T("Double"); break;
	case	2 :
		strStyle = _T("Section Open"); break;
	case	3 :
		strStyle = _T("Section Close"); break;
	case	4 :
		strStyle = _T("Local Repeat Open"); break;
	case	5 :
		strStyle = _T("Local Repeat Close"); break;
	case	6 :
		strStyle = _T("Master Repeat Open"); break;
	case	7 :
		strStyle = _T("Master Repeat Close"); break;
	default :
		strStyle = _T("invalid"); break;
	}

	wxFprintf(fp, _T("style(%s)"), strStyle.c_str());
	if ( nStyle == 5 )
		wxFprintf(fp, _T(",local repeat(%d)"), mLocalRepeatCount);
	if ( mStyle & 0x80 )
		wxFprintf(fp, _T(",force system break"));
	wxFprintf(fp, _T("\n"));
	return true;
}

bool CEndingObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	bResult = ReadBytes(file, mStyle) &&
			  ReadBytes(file, mData2);

	return bResult;
}

bool CEndingObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("ending: "));

	wxString strStyle;
	switch(mStyle)
	{
	default :
		strStyle = _T("unknown"); break;
	case	1 :
		strStyle = _T("1st ending"); break;
	case	2 :
		strStyle = _T("2nd ending"); break;
	case	3 :
		strStyle = _T("3rd ending"); break;
	case	4 :
	case	5 :
	case	6 :
	case	7 :
		strStyle.Printf(_T("%dth ending"), mStyle); break;
	case	8 :
		strStyle = _T("Default Ending"); break;
	}

	wxFprintf(fp, _T("style(%s)"), strStyle.c_str());
	DumpBinary(fp, mData2, sizeof(mData2));
	wxFprintf(fp, _T("\n"));
	return true;
}

bool CInstrumentObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( GetLoadedVersion() < NWC_Version170 )
	{
		mData1[6] = mData1[7] = 0;
		bResult = file.Read(mData1, 6) == 6;
	}
	else if ( GetLoadedVersion() < NWC_Version200 )
	{
		bResult = ReadBytes(file, mData1);
	}
	else
	{
		bResult = ReadBytes(file, mData1);
		mName = LoadStringNULTerminated(file);
		bResult = ReadBytes(file, mData2);
		bResult = ReadBytes(file, mVelocity);
	}

	return bResult;
}

bool CInstrumentObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("instrument: "));
	DumpBinary(fp, mData1, sizeof(mData1), false);
	wxFprintf(fp, _T("\n"));
	return true;
}

bool CTimeSigObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	bResult = ReadLEShort(file, mBit_Measure) &&
			  ReadLEShort(file, mBits) &&
			  ReadLEShort(file, mStyle);

	return bResult;
}

bool CTimeSigObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("time sig: "));
	wxFprintf(fp, _T("%d/%d"), mBit_Measure, 1 << mBits );
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString	CTimeSigObj::GetAsString() const
{
	wxString str;
	str.Printf(_T("%d/%d"), mBit_Measure, 1 << mBits);
	return str;
}

bool CTempoObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	bResult = ReadBytes(file, mPos) &&
			  ReadBytes(file, mPlacement) &&
			  ReadLEShort(file, mTempoValue) &&
			  ReadBytes(file, mBase);
	if ( GetLoadedVersion() < NWC_Version170 )
	{
		// old version has mBase as short & one NUL 
		short bTemp;
		bResult = ReadLEShort(file, bTemp);
	}

	mText = LoadStringNULTerminated(file);

	return bResult;
}

bool CTempoObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("tempo: "));
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	wxFprintf(fp, _T(",Placement=0x%0x"), mPlacement);
	wxFprintf(fp, _T(",%s=%d"), GetBaseAsString().c_str(), mTempoValue);
	wxFprintf(fp, _T(",String(%s)"), mText.c_str());
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString CTempoObj::GetBaseAsString()
{
	wxString str;
	switch( mBase / 2 )
	{
	default:
	case	0 :	str = _T("8"); break;	// U+266A, U+1D160
	case	1 :	str = _T("4"); break;	// U+2669, U+1D15F
	case	2 :	str = _T("2"); break;	// U+1D15E
//	case	3 :	str = _T("1"); break;	// U+1D15D
	}

	if ( mBase % 2 )
	{
		str += '.';
	}

	return str;
}

long	CTempoObj::GetTempoByQuarter() const
{
	long nTempo;
	switch( mBase )
	{
	case	0 : nTempo = mTempoValue / 2; break;
	case	1 : nTempo = MulDiv(mTempoValue, 3, 4); break;
	default:
	case	2 : nTempo = mTempoValue; break;
	case	3 : nTempo = MulDiv(mTempoValue, 3, 2); break;
	case	4 : nTempo = mTempoValue * 2; break;
	case	5 : nTempo = mTempoValue * 3; break;
	}

	return nTempo;
}

bool CDynamicObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( GetLoadedVersion() >= NWC_Version170 )
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mStyle) &&
				  ReadLEShort(file, mNoteVelocity) &&
				  ReadLEShort(file, mMIDIVolume);
	}
	else
	{
		mStyle = 0;
		bResult = ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mPos) &&
				  ReadLEShort(file, mNoteVelocity) &&
				  ReadLEShort(file, mMIDIVolume);
		mStyle = mPlacement & 0x07;
		mPlacement = mPlacement & (~0x07);	// 0x10 : Preserve Width
	}

	return bResult;
}

bool CDynamicObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("dynamic: "));
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	wxFprintf(fp, _T(",Placement=0x%0x"), mPlacement);
	wxFprintf(fp, _T(",%s"), GetStyleAsString(true).c_str());
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString CDynamicObj::GetStyleAsString(bool bExtended) const
{
	static wxChar *s_Styles[] = { _T("ppp"), _T("pp"), _T("p"), _T("mp"), _T("mf"), _T("f"), _T("ff"), _T("fff") };
	wxString str;
	UINT nStyle = (mStyle & 0x1F);
	if ( nStyle < _countof(s_Styles) )
	{
		str = s_Styles[nStyle];
	}
	else
	{
		str = _T("unknown");
	}

	if ( bExtended )
	{
		bool bNoteVel = (mStyle & 0x20) != 0;

		bool bMIDIVol = (mStyle & 0x40) != 0;

		if ( bNoteVel || bMIDIVol )
		{
			wxString strVel(_T("-")), strVol(_T("-"));
			if ( bNoteVel )
				strVel.Printf(_T("%d"), mNoteVelocity);
			if ( bMIDIVol )
				strVol.Printf(_T("%d"), mMIDIVolume);

			wxString strTmp;
			strTmp.Printf(_T("(%s,%s)"), strVel.c_str(), strVol.c_str());
			str += strTmp;
		}
	}

	return str;
}

bool CNoteObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( m_pParent->m_pParent->nVersion <= NWC_Version170 )
	{
		bResult = ReadBytes(file, mDuration) &&
				  ReadBytes(file, mData2) &&
				  ReadBytes(file, mAttribute1) &&
				  ReadBytes(file, mPos) &&
				  ReadBytes(file, mAttribute2) &&
				  ReadBytes(file, mData3);
	}
	else
	{
		bResult = ReadBytes(file, mDuration) &&
				  ReadBytes(file, mData2) &&
				  ReadBytes(file, mAttribute1) &&
				  ReadBytes(file, mPos) &&
				  ReadBytes(file, mAttribute2);
	}

	mStemLength = 7;
	if ( m_pParent->m_pParent->nVersion >= NWC_Version200 && (mAttribute2[0] & 0x40) != 0 )
	{
		bResult = ReadBytes(file, mStemLength);
	}

	return bResult;
}

bool CNoteObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("note: "));
	wxFprintf(fp, _T("%2s"), GetDurationAsString().c_str());
	DumpBinary(fp, mData2, sizeof(mData2)+sizeof(mAttribute1));
	wxFprintf(fp, _T(",%s"), GetPitchAsString().c_str());
	DumpBinary(fp, mAttribute2, sizeof(mAttribute2));
	DumpBinary(fp, mData3, sizeof(mData3));
	if ( m_pParent->m_pParent->nVersion >= NWC_Version200 && (mAttribute2[0] & 0x40) != 0 )
		wxFprintf(fp, _T(",stemlength(%d)"), mStemLength);
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString CNoteObj::GetDurationAsString() const
{
	wxString str;
	str.Printf(_T("%d"), 1 << GetDuration());

	if ( mAttribute1[0] & 0x01 )
	{
		str += _T("..");
	}
	else if ( mAttribute1[0] & 0x04 )
	{
		str += '.';
	}

	return str;
}

long	CNoteObj::GetDuration(long nDivision) const
{
	int nDuration = nDivision / (1 << GetDuration());
	if ( mAttribute1[0] & 0x01 )
	{
		nDuration = nDuration + nDuration / 4;
	}
	else if ( mAttribute1[0] & 0x04 )
	{
		nDuration = nDuration + nDuration / 2;
	}

	// triplet
	if ( mData2[1] & 0x0C )
	{
		nDuration = nDuration * 2 / 3;
	}

	return nDuration * 4;
}

void	CNoteObj::SetDuration(BYTE nDura, long nDivision)
{
}

DURATIONTYPE	CNoteObj::GetDurationType() const
{
	int nDuration = 0;
	if ( mAttribute1[0] & 0x01 )
	{
		nDuration |= DT_DOTDOT;
	}
	else if ( mAttribute1[0] & 0x04 )
	{
		nDuration |= DT_DOT;
	}

	// triplet
	if ( mData2[1] & 0x0C )
	{
		nDuration |= (mData2[1] & 0x0C);
	}

	return (DURATIONTYPE)nDuration;
}

long	CNoteObj::GetDivision() const
{
	long nDivision = 1 << GetDuration();
	if ( mAttribute1[0] & 0x01 )
	{
		nDivision <<= 2;
	}
	else if ( mAttribute1[0] & 0x04 )
	{
		nDivision <<= 1;
	}

	// triplet
	if ( mData2[1] & 0x0C )
	{
		if ( nDivision % 2 )
			nDivision = nDivision * 3;
		else
			nDivision = nDivision / 2 * 3;
	}

	return nDivision;
}

wxString CNoteObj::GetPitchAsString() const
{
	wxString str;
	str.Printf(_T("%d"), mPos);
	BYTE nAdjust = (mAttribute2[0] & 0x07);
	switch ( nAdjust )
	{
	case	0 :	str = _T('#') + str; break;
	case	1 :	str = _T('b') + str; break;
	case	2 :	str = _T('n') + str; break;
	case	3 :	str = _T("##") + str; break;
	case	4 :	str = _T("bb") + str; break;
	default :
	case	5 :
		break;
	}

	wxString s;
	s.Printf(_T("Pos(%2s)"), str.c_str());
	return s;
}

bool		CNoteObj::IsStemUp() const
{
	NOTEATTR na = GetNoteAttributes();
	bool bStemUp = false;
	switch ( na & NA_STEM_MASK )
	{
	case	NA_STEM_UP :
		bStemUp = true;
		break;
	case	NA_STEM_DOWN :
		bStemUp = false;
		break;
	default :
		bStemUp = mPos > 0;
		break;
	}

	return bStemUp;
}

NOTEATTR	CNoteObj::GetNoteAttributes() const
{
	long nAttr = 0;
	if ( mAttribute1[1] & 0x20 )
		nAttr |= NA_GRACE;
	if ( mAttribute1[1] & 0x04 )
		nAttr |= NA_TENUTO;

	nAttr |= ( mAttribute1[1] & 0x03 ) * NA_SLUR_BEG;

	if ( mAttribute1[0] & 0x20 )
		nAttr |= NA_ACCENT;

	if ( mAttribute1[0] & 0x10 )
		nAttr |= NA_TIE_BEG;
	if ( mAttribute1[0] & 0x08 )
		nAttr |= NA_TIE_END;

	if ( mAttribute1[0] & 0x02 )
		nAttr |= NA_STACCATO;

	nAttr |= ( mData2[1] & 0x03 ) * NA_BEAM_BEG;

	if ( mAttribute1[1] & 0x80 )
	{
		if ( mData2[1] & 0x40 )
			nAttr |= NA_SLUR_DIR_DOWN;
		else
			nAttr |= NA_SLUR_DIR_UP;
	}

	if ( mAttribute1[1] & 0x40 )
	{
		if ( mAttribute2[0] & 0x08 )
			nAttr |= NA_TIE_DIR_DOWN;
		else
			nAttr |= NA_TIE_DIR_UP;
	}

	nAttr |= ( (mData2[1] & 0x30) >> 4 ) * NA_STEM_SHIFT;

	return (NOTEATTR)nAttr;
}

void	CNoteObj::GetOctaveStep(CHROMALTER& ca, int& nOctave, wxChar& chStep, int& nAlter) const
{
/*
 *	pos	oct	step
 *	-1	5	'C'
 *	0	4	'B'
 *	1	4	'A'
 *	2	4	'G'
 *	...
 *	6	4	'C'
 *	7	3	'B'
 */
	int nPos = ca.clefShift + mPos;

	nOctave = (4*7 - nPos + 6) / 7;
	int nStep = (4*7 - nPos + 1) % 7;
	chStep = nStep + 'A';

	ACCIDENTAL ac = GetAccidental();
	if ( AC_SHARP <= ac && ac <= AC_FLAT_FLAT )
	{
		int nAlters[] = { 1, -1, 0, 2, -2 };
		nAlter = nAlters[ac];
		ca.measure[nStep] = nAlter;
	}
	else
	{
		nAlter = ca.measure[nStep];
	}
}

void CNoteObj::SetOctaveStep(CHROMALTER& ca, int nOctave, wxChar chStep, int nAlter)
{
	int nStep = chStep - 'A';
	int nPos = ( 4 - nOctave ) * 7 + (8 - nStep) % 7;
	mPos = nPos - ca.clefShift;
}

ACCIDENTAL	CNoteObj::GetAccidental() const
{
	return (ACCIDENTAL)( mAttribute2[0] & 0x07 );
}


bool CRestObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( m_pParent->m_pParent->nVersion <= NWC_Version150 )
	{
		bResult = ReadBytes(file, mDuration) &&
				  file.Read(&mData2, 5) == 5;
		mOffset = mData2[4];
	}
	else
	{
		bResult = ReadBytes(file, mDuration) &&
				  ReadBytes(file, mData2) &&
				  ReadLEShort(file, mOffset);
	}

	return bResult;
}

bool CRestObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("rest: "));
	DumpData(fp);
	wxFprintf(fp, _T("\n"));
	return true;
}

void CRestObj::DumpData(FILE* fp)
{
	wxFprintf(fp, _T("%2s"), GetDurationAsString().c_str());
	DumpBinary(fp, mData2, sizeof(mData2));
	wxFprintf(fp, _T(",offset(%d)"), mOffset);
}

wxString CRestObj::GetDurationAsString() const
{
	wxString str;
	str.Printf(_T("%d"), 1 << GetDuration());

	if ( mData2[3] & 0x01 )
	{
		str += _T("..");
	}
	else if ( mData2[3] & 0x04 )
	{
		str += '.';
	}

	return str;
}

long	CRestObj::GetDuration(long nDivision) const
{
	int nDuration = nDivision / (1 << GetDuration());
	if ( mData2[3] & 0x01 )
	{
		nDuration = nDuration + nDuration / 4;
	}
	else if ( mData2[3] & 0x04 )
	{
		nDuration = nDuration + nDuration / 2;
	}

	return nDuration * 4;
}

void	CRestObj::SetDuration(BYTE nDura, long nDivision)
{
}

DURATIONTYPE	CRestObj::GetDurationType() const
{
	int nDuration = 0;
	if ( mData2[3] & 0x01 )
	{
		nDuration |= DT_DOTDOT;
	}
	else if ( mData2[3] & 0x04 )
	{
		nDuration = DT_DOT;
	}

	return (DURATIONTYPE)nDuration;
}

long	CRestObj::GetDivision() const
{
	long nDivision = 1 << GetDuration();
	if ( mData2[3] & 0x01 )
	{
		nDivision <<= 2;
	}
	else if ( mData2[3] & 0x04 )
	{
		nDivision <<= 1;
	}
	return nDivision;
}

bool	CRestObj::GetOctaveStep(const CHROMALTER& ca, int& nOctave, wxChar& chStep) const
{
	int nPos = ca.clefShift + mOffset;

	nOctave = (4*7 - nPos + 6) / 7;
	int nStep = (4*7 - nPos + 1) % 7;
	chStep = nStep + 'A';

	return mOffset != 0;
}

bool CNoteCMObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( m_pParent->m_pParent->nVersion <= NWC_Version170 )
		bResult = ReadBytes(file, mData1);
	else
		bResult = file.Read(&mData1, 8) == 8;

	mStemLength = 7;
	if ( m_pParent->m_pParent->nVersion >= NWC_Version200 && (mData1[7] & 0x40) != 0 )
	{
		bResult = ReadBytes(file, mStemLength);
	}

	bResult = ReadLEShort(file, mCount);

	return bResult;
}

UINT CNoteCMObj::GetStemMask(const TVector<CObj*>& objArray)
{
	UINT nStemMask = 0;
	for ( size_t j=0 ; j<objArray.GetCount() ; j++ )
	{
		CObj* pObj = objArray[j];

		if ( pObj->mObjType == Obj_Note )
		{
			if ( ((CNoteObj*)pObj)->IsStemUp() )
				nStemMask |= 0x01;
			else
				nStemMask |= 0x02;
		}
	}

	return nStemMask;
}

// load mCount note or rest
bool CNoteCMObj::LoadChildren(wxFile& in, FILE* out, CStaff* pStaff)
{
	UINT nObjCount = mCount;

	for ( UINT i=0; i<nObjCount; i++ )
	{
		CObj* pObj = CreateNLoadObject(in, out, pStaff);
		if ( pObj )
		{
			mObjArray.Add(pObj);
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool CNoteCMObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("NoteCM: "));
	size_t nCount = ( m_pParent->m_pParent->nVersion <= NWC_Version170 ) ? sizeof(mData1) : 8;
	DumpBinary(fp, mData1, nCount, false);
	if ( m_pParent->m_pParent->nVersion >= NWC_Version200 && (mData1[7] & 0x40) != 0 )
		wxFprintf(fp, _T(",stemlength(%d)"), mStemLength);
	wxFprintf(fp, _T(",Count(%d)"), mCount);
	wxFprintf(fp, _T("\n"));
	return true;
}

bool CPedalObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	NWC_Version nVersion = GetLoadedVersion();
	if ( nVersion >= NWC_Version170 )
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mStyle);
	}
	else if ( nVersion <= NWC_Version155 )
	{
		char unknown = 0;
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, unknown) &&
				  ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mStyle);
	}
	else
	{
		mPlacement = 0;
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mStyle);
	}

	return bResult;
}

bool CPedalObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("Pedal: "));
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	wxFprintf(fp, _T(",Placement=0x%0x"), mPlacement);
	wxFprintf(fp, _T(",%s"), GetStyleAsString().c_str());
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString CPedalObj::GetStyleAsString()
{
	static wxChar *s_Styles[] = { _T("Pedal Release"), _T("Pedal Down") };
	wxString str;
	UINT nStyle = mStyle;
	if ( nStyle < _countof(s_Styles) )
	{
		str = s_Styles[nStyle];
	}
	else
	{
		str = _T("unknown");
	}

	return str;
}

PEDALSTYLE	CPedalObj::GetPedalStyle() const
{
	return (PEDALSTYLE)mStyle;
}

bool CFlowDirObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( GetLoadedVersion() >= NWC_Version170 )
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  ReadLEShort(file, mStyle);
	}
	else
	{
		mPos = -8;
		mPlacement = 0x01;
		bResult = ReadLEShort(file, mStyle);
	}

	return bResult;
}

bool CFlowDirObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("FlowDir: "));
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	wxFprintf(fp, _T(",Placement=0x%0x"), mPlacement);
	wxFprintf(fp, _T(",%s"), GetStyleAsString().c_str());
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString CFlowDirObj::GetStyleAsString()
{
	static wxChar *s_Styles[] = { _T("Coda"), _T("Segno"), _T("Fine"), _T("To Coda"), _T("Da capo"), _T("D.C. al Coda"), _T("D.C. al Fine"), _T("Dal Segno"), _T("D.S. al Coda"), _T("D.S. al Fine") };
	wxString str;
	UINT nStyle = mStyle;
	if ( nStyle < _countof(s_Styles) )
	{
		str = s_Styles[nStyle];
	}
	else
	{
		str = _T("unknown");
	}

	return str;
}

FLOWSTYLE	CFlowDirObj::GetFlowStyle() const
{
	return (FLOWSTYLE)mStyle;
}

bool CMPCObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( m_pParent->m_pParent->nVersion <= NWC_Version155 )
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  file.Read(&mData1, 0x1E) == 0x1E;
	}
	else
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mData1);
	}

	return bResult;
}

bool CMPCObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("MPC: "));
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	wxFprintf(fp, _T(",Placement=0x%0x"), mPlacement);
	DumpBinary(fp, mData1, sizeof(mData1));
	wxFprintf(fp, _T("\n"));
	return true;
}

bool CTempVarObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( GetLoadedVersion() >= NWC_Version170 )
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mStyle) &&
				  ReadBytes(file, mDelay);
	}
	else
	{
		mPlacement = 0;
		bResult = ReadBytes(file, mStyle) &&
				  ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mDelay);
		mStyle = mStyle & 0x0F;
	}

	return bResult;
}

bool CTempVarObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("TempVar: "));
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	wxFprintf(fp, _T(",Placement=0x%0x"), mPlacement);
	wxFprintf(fp, _T(",%s"), GetStyleAsString().c_str());
	wxFprintf(fp, _T(",Delay=%d"), mDelay);
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString CTempVarObj::GetStyleAsString()
{
	static wxChar *s_Styles[] = { _T("Breath Mark"), _T(" Fermata"), _T(" Accelerando"), _T(" Allargrando"), _T(" Rallentando"), _T(" Ritardando"), _T(" Ritenuto"), _T(" Rubato"), _T(" Stringendo") };
	wxString str;
	UINT nStyle = mStyle;
	if ( nStyle < _countof(s_Styles) )
	{
		str = s_Styles[nStyle];
	}
	else
	{
		str = _T("unknown");
	}

	return str;
}

bool CDynVarObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( GetLoadedVersion() >= NWC_Version170 )
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mStyle);
	}
	else
	{
		mPlacement = 0;
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mStyle);
	}

	return bResult;
}

bool CDynVarObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("DynVar: "));
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	wxFprintf(fp, _T(",Placement=0x%0x"), mPlacement);
	wxFprintf(fp, _T(",%s"), GetStyleAsString().c_str());
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString CDynVarObj::GetStyleAsString()
{
	static wxChar *s_Styles[] = { _T("cresc"), _T("decresc"), _T("dim"), _T("rfz"), _T("sfz") };
	wxString str;
	UINT nStyle = mStyle;
	if ( nStyle < _countof(s_Styles) )
	{
		str = s_Styles[nStyle];
	}
	else
	{
		str = _T("unknown");
	}

	return str;
}

bool CPerformObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( GetLoadedVersion() >= NWC_Version170 )
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mPlacement) &&
				  ReadBytes(file, mStyle);
	}
	else
	{
		mPlacement = 0;
		bResult = ReadBytes(file, mStyle) &&
				  ReadBytes(file, mPos);
	}

	return bResult;
}

bool CPerformObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("Perform: "));
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	wxFprintf(fp, _T(",Placement=0x%0x"), mPlacement);
	wxFprintf(fp, _T(",%s"), GetStyleAsString().c_str());
	wxFprintf(fp, _T("\n"));
	return true;
}

wxString CPerformObj::GetStyleAsString()
{
	static wxChar *s_Styles[] = { _T("Ad Libitum"), _T("Animato"), _T(" Cantabile"), _T(" Con brio"), _T(" Dolce"), _T(" Espressivo"), _T(" Grazioso"),
		_T(" Legato"), _T(" Maestoso"), _T(" Marcato"), _T(" Meno Mosso"), _T("Poco a poco"), _T(" Pi?mosso"),
		_T(" Semplice"), _T(" Simile"), _T(" Solo"), _T(" Sostenuto"), _T(" Sotto Voce"), _T(" Staccato"), _T(" Subito"), _T(" Tenuto"), _T(" Tutti"), _T(" Volta Subito") };
	wxString str;
	UINT nStyle = mStyle;
	if ( nStyle < _countof(s_Styles) )
	{
		str = s_Styles[nStyle];
	}
	else
	{
		str = _T("unknown");
	}

	return str;
}

bool CTextObj::Load(wxFile& file)
{
	bool bResult = CObj::Load(file);

	if ( GetLoadedVersion() >= NWC_Version170 )
	{
		bResult = ReadBytes(file, mPos) &&
				  ReadBytes(file, mData) &&
				  ReadBytes(file, mFont);
		BF170* pBF170 = (BF170*)&mData;
		mPreserveWidth = pBF170->bPreserveWidth;
		mJustification = pBF170->nJustification;
		mAlignment = pBF170->nAlignment;
	}
	else
	{
		// 1.55
		bResult = ReadBytes(file, mFont) &&
				  ReadBytes(file, mPos);
		mPreserveWidth = mFont >> 4;
		mFont = mFont & 0x0F;
	}

	if ( bResult )
		mText = LoadStringNULTerminated(file);

	return bResult;
}

bool CTextObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("text: (%s),"), mText.c_str());
	wxFprintf(fp, _T("Pos(%d)"), mPos);
	if ( mPreserveWidth )
		wxFprintf(fp, _T(",PreserveWidth=%d"), mPreserveWidth);
	wxFprintf(fp, _T(",Justification=%d"), mJustification);
	wxFprintf(fp, _T(",Alignment=%d"), mAlignment);
	wxFprintf(fp, _T(",Font=%d"), mFont);
	wxFprintf(fp, _T("\n"));
	return true;
}

bool CRestCMObj::Load(wxFile& file)
{
	bool bResult = CRestObj::Load(file);

	bResult = ReadLEShort(file, mCount);

	return bResult;
}

// load mCount note or rest
bool CRestCMObj::LoadChildren(wxFile& in, FILE* out, CStaff* pStaff)
{
	UINT nObjCount = mCount;

	for ( UINT i=0; i<nObjCount; i++ )
	{
		CObj* pObj = CreateNLoadObject(in, out, pStaff);
		if ( pObj )
		{
			mObjArray.Add(pObj);
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool CRestCMObj::Dump(FILE* fp)
{
	wxFprintf(fp, _T("RestCM: "));
	DumpData(fp);
	wxFprintf(fp, _T(",Count(%d)"), mCount);
	wxFprintf(fp, _T("\n"));
	return true;
}

bool CFontInfo::Dump(FILE* fp, int nIndex)
{
#define	FI_BOLD		0x01
#define	FI_ITALIC	0x02

	wxString strStyle;
	switch ( btStyle & (FI_BOLD|FI_ITALIC) )
	{
	case	(FI_BOLD|FI_ITALIC) :
		strStyle = _T("Bold Italic");
		break;
	case	(FI_BOLD) :
		strStyle = _T("Bold");
		break;
	case	(FI_ITALIC) :
		strStyle = _T("Italic");
		break;
	default:
		strStyle = _T("Regular");
		break;
	}

	wxFprintf(fp, _T("font%d:%s, size=%d, style=%s\n"), nIndex, strName.c_str(), btSize, strStyle.c_str());
	return true;
}

#ifdef	__WXDEBUG__
#define	MyTrap()		wxTrap()
#else
inline void MyTrap()	{ }
#endif

CObj*	CreateNLoadObject(wxFile& in, FILE* out, CStaff* pStaff)
{
	long nPos = in.Tell();

	short objType;
	if ( ReadLEShort(in, objType) == false )
	{
		fprintf(stderr, "unexpected end of file\n");
		if ( wxIsDebuggerRunning() ) MyTrap();
		return NULL;
	}

	CObj* pObj = NULL;
	switch ( objType )
	{
	case	Obj_Clef :		pObj = new CClefObj(pStaff); break;
	case	Obj_KeySig :	pObj = new CKeySigObj(pStaff); break;
	case	Obj_BarLine :	pObj = new CBarLineObj(pStaff); break;
	case	Obj_Ending :	pObj = new CEndingObj(pStaff); break;
	case	Obj_Instrument:	pObj = new CInstrumentObj(pStaff); break;
	case	Obj_TimeSig :	pObj = new CTimeSigObj(pStaff); break;
	case	Obj_Tempo :		pObj = new CTempoObj(pStaff); break;
	case	Obj_Dynamic:	pObj = new CDynamicObj(pStaff); break;
	case	Obj_Note :		pObj = new CNoteObj(pStaff); break;
	case	Obj_Rest :		pObj = new CRestObj(pStaff); break;
	case	Obj_NoteCM :	pObj = new CNoteCMObj(pStaff); break;
	case	Obj_Pedal:		pObj = new CPedalObj(pStaff); break;
	case	Obj_FlowDir:	pObj = new CFlowDirObj(pStaff); break;
	case	Obj_MPC:		pObj = new CMPCObj(pStaff); break;
	case	Obj_TempVar:	pObj = new CTempVarObj(pStaff); break;
	case	Obj_DynVar:		pObj = new CDynVarObj(pStaff); break;
	case	Obj_Perform:	pObj = new CPerformObj(pStaff); break;
	case	Obj_Text :		pObj = new CTextObj(pStaff); break;
	case	Obj_RestCM :	pObj = new CRestCMObj(pStaff); break;
	}

	if ( pObj )
	{
		pObj->mObjType = objType;
		bool bResult = pObj->Load(in);
		if ( bResult )
		{
			if ( g_bDumpOffset )
				wxFprintf(out, _T("%08x:"), nPos);
			pObj->Dump(out);

			if ( Obj_NoteCM == pObj->mObjType )
			{
				((CNoteCMObj*)pObj)->LoadChildren(in, out, pStaff);
			}
			else if ( Obj_RestCM == pObj->mObjType )
			{
				((CRestCMObj*)pObj)->LoadChildren(in, out, pStaff);
			}
		}
		else
		{
			delete pObj;
			pObj = NULL;

			fprintf(stderr, "failed to load object type=0x%04x, at file offset=%08x\n", (unsigned short)objType, nPos);
			if ( wxIsDebuggerRunning() ) MyTrap();
			return NULL;
		}
	}
	else
	{
		fprintf(stderr, "unknown object type=0x%04x, at file offset=%08x\n", (unsigned short)objType, nPos);
		if ( wxIsDebuggerRunning() ) MyTrap();
		return NULL;
	}

	fflush(out);

	return pObj;
}

void CStaff::RemoveAll()
{
	size_t nCount = mObjArray.GetCount();
	for ( size_t i=0; i<nCount ; i++ )
	{
		CObj* pObj = mObjArray[i];
		delete pObj;
	}
	mObjArray.RemoveAll();
}

void CStaff::SetDefault()
{
	mStaffInfo.btEndingBar = 0;
	mStaffInfo.btMuted = 0;
	mStaffInfo.btReserved1 = 0;
	mStaffInfo.btChannel = 0;
	mStaffInfo.btReserved2 = 0;
	mStaffInfo.btPlaybackDevice = 0;
	mStaffInfo.btReserved3 = 0;
	mStaffInfo.btSelectPatchBank = 0;
	memset(mStaffInfo.btReserved4, 0, sizeof(mStaffInfo.btReserved4));
	mStaffInfo.btPatchName = 0;
	mStaffInfo.btReserved5 = 0;
	mStaffInfo.nStyle = 0;
	mStaffInfo.nVerticalSizeUpper = -12;
	mStaffInfo.nVerticalSizeLower = 12;
	mStaffInfo.btLines = 5;
	mStaffInfo.nLayerWithNextStaff = 0;
	mStaffInfo.nTransposition = 0;
	mStaffInfo.nPartVolume = 127;
	mStaffInfo.nStereoPan = 64;
	mStaffInfo.btColor = 0;
	//mStaffInfo.btReserved6 = 0;
}

template <class intcls> inline
intcls gcd(intcls x, intcls y)
{
	intcls t;
	while ( y != 0 )
	{
		t = x % y;
		x = y;
		y = t;
	}

	return x;
}

template <class intcls> inline
intcls lcd(intcls x, intcls y)
{
	return x * y / gcd(x, y);
}

long CStaff::GetDivisions() const
{
	long nDivisions = 1;
	size_t nCount = mObjArray.GetCount();
	for ( size_t i=0; i<nCount ; i++ )
	{
		CObj* pObj = mObjArray[i];
		switch ( pObj->mObjType )
		{
		case	Obj_Note :
			nDivisions = lcd(nDivisions, ((CNoteObj*)pObj)->GetDivision());
			break;
		case	Obj_Rest :
			nDivisions = lcd(nDivisions, ((CRestObj*)pObj)->GetDivision());
			break;
		}
	}

	return nDivisions;
}

int  CStaff::FindNthObjIndex(OBJTYPE objType, UINT nIndex)
{
	for ( size_t i=0 ; i<mObjArray.GetCount() ; i++ )
	{
		CObj* pObj = mObjArray[i];
		if ( objType == pObj->mObjType )
		{
			if ( nIndex == 0 )
				return (int)i;
			nIndex--;
		}
	}

	return -1;
}

bool CStaff::Load(wxFile& in, FILE* out, FILELOAD fl)
{
	long nPos = in.Tell();
	strName = LoadStringNULTerminated(in);
	wxFprintf(out, _T("name=%s\n"), strName.c_str());
	if ( m_pParent->nVersion >= NWC_Version200 )
	{
		strLabel = LoadStringNULTerminated(in);
		wxFprintf(out, _T("label=%s\n"), strLabel.c_str());
		strInstName = LoadStringNULTerminated(in);
		wxFprintf(out, _T("inst=%s\n"), strInstName.c_str());
	}
	strGroup = LoadStringNULTerminated(in);
	wxFprintf(out, _T("group=%s\n"), strGroup.c_str());

	nPos = in.Tell();

	memset(&mStaffInfo, 0, sizeof(mStaffInfo));

	bool bResult = false;
	if ( m_pParent->nVersion <= NWC_Version130 )
	{
		staff130 staff;
		memset(&staff, 0, sizeof(staff));
		bResult = ReadBytes(in, staff.btEndingBar) &&
				  ReadBytes(in, staff.btMuted) &&
				  ReadBytes(in, staff.btReserved1) &&
				  ReadBytes(in, staff.btChannel) &&
				  ReadBytes(in, staff.btReserved2) &&
				  ReadBytes(in, staff.btPlaybackDevice) &&
				  ReadBytes(in, staff.btReserved3) &&
				  ReadBytes(in, staff.btSelectPatchBank) &&
				  ReadBytes(in, staff.btReserved4) &&
				  ReadBytes(in, staff.btPatchName) &&
				  ReadLEShort(in, staff.nStyle) &&
				  ReadLEShort(in, staff.nVerticalSizeUpper) &&
				  ReadLEShort(in, staff.nVerticalSizeLower) &&
				  ReadLEShort(in, staff.nLayerWithNextStaff) &&
				  ReadLEShort(in, staff.nPartVolume) &&
				  ReadLEShort(in, staff.nStereoPan) &&
				  ReadBytes(in, staff.btReserved6) &&
				  ReadBytes(in, staff.btColor) &&
				  ReadLEShort(in, staff.nNumLyric);
		if ( !bResult )
			return false;

		mStaffInfo.btEndingBar = staff.btEndingBar;
		mStaffInfo.btMuted = staff.btMuted;
		//mStaffInfo.btReserved1 = staff.btReserved1;
		mStaffInfo.btChannel = staff.btChannel;
		//mStaffInfo.btReserved2 = staff.btReserved2;
		mStaffInfo.btPlaybackDevice = staff.btPlaybackDevice;
		//mStaffInfo.btReserved3 = staff.btReserved3;
		mStaffInfo.btSelectPatchBank = staff.btSelectPatchBank;
		//memcpy(mStaffInfo.btReserved4, staff.btReserved4, sizeof(mStaffInfo.btReserved4));
		mStaffInfo.btPatchName = staff.btPatchName;
		//mStaffInfo.btReserved5 = staff.btReserved5;
		mStaffInfo.nStyle = staff.nStyle;
		mStaffInfo.nVerticalSizeUpper = staff.nVerticalSizeUpper;
		mStaffInfo.nVerticalSizeLower = staff.nVerticalSizeLower;
		mStaffInfo.btLines = 5;
		mStaffInfo.nLayerWithNextStaff = staff.nLayerWithNextStaff;
		//mStaffInfo.nTransposition = staff.nTransposition;
		mStaffInfo.nPartVolume = staff.nPartVolume;
		mStaffInfo.nStereoPan = staff.nStereoPan;
		mStaffInfo.btColor = staff.btColor;
		//mStaffInfo.btReserved6 = staff.btReserved6;
		mStaffInfo.nNumLyric = staff.nNumLyric;
		mStaffInfo.nLyricBlockSizeInByte = staff.nLyricBlockSizeInByte;
		mStaffInfo.nLyricSizeInByte = staff.nLyricSizeInByte;
		mStaffInfo.nReserved8 = staff.nReserved8;
	}
	else if ( m_pParent->nVersion <= NWC_Version150 )
	{
		staff150 staff;
		memset(&staff, 0, sizeof(staff));
		bResult = ReadBytes(in, staff.btEndingBar) &&
			ReadBytes(in, staff.btMuted) &&
			ReadBytes(in, staff.btReserved1) &&
			ReadBytes(in, staff.btChannel) &&
			ReadBytes(in, staff.btReserved2) &&
			ReadBytes(in, staff.btPlaybackDevice) &&
			ReadBytes(in, staff.btReserved3) &&
			ReadBytes(in, staff.btSelectPatchBank) &&
			ReadBytes(in, staff.btReserved4) &&
			ReadBytes(in, staff.btPatchName) &&
			ReadLEShort(in, staff.nStyle) &&
			ReadLEShort(in, staff.nVerticalSizeUpper) &&
			ReadLEShort(in, staff.nVerticalSizeLower) &&
			ReadLEShort(in, staff.nLayerWithNextStaff) &&
			ReadLEShort(in, staff.nTransposition) &&
			ReadLEShort(in, staff.nPartVolume) &&
			ReadLEShort(in, staff.nStereoPan) &&
			ReadBytes(in, staff.btReserved6) &&
			ReadBytes(in, staff.btColor) &&
			ReadLEShort(in, staff.nNumLyric);
		if ( !bResult )
			return false;

		mStaffInfo.btEndingBar = staff.btEndingBar;
		mStaffInfo.btMuted = staff.btMuted;
		mStaffInfo.btReserved1 = staff.btReserved1;
		mStaffInfo.btChannel = staff.btChannel;
		mStaffInfo.btReserved2 = staff.btReserved2;
		mStaffInfo.btPlaybackDevice = staff.btPlaybackDevice;
		mStaffInfo.btReserved3 = staff.btReserved3;
		mStaffInfo.btSelectPatchBank = staff.btSelectPatchBank;
		memcpy(mStaffInfo.btReserved4, staff.btReserved4, sizeof(mStaffInfo.btReserved4));
		mStaffInfo.btPatchName = staff.btPatchName;
		//mStaffInfo.btReserved5 = staff.btReserved5;
		mStaffInfo.nStyle = staff.nStyle;
		mStaffInfo.nVerticalSizeUpper = staff.nVerticalSizeUpper;
		mStaffInfo.nVerticalSizeLower = staff.nVerticalSizeLower;
		mStaffInfo.btLines = 5;
		mStaffInfo.nLayerWithNextStaff = staff.nLayerWithNextStaff;
		mStaffInfo.nTransposition = staff.nTransposition;
		mStaffInfo.nPartVolume = staff.nPartVolume;
		mStaffInfo.nStereoPan = staff.nStereoPan;
		mStaffInfo.btColor = staff.btColor;
		//mStaffInfo.btReserved6 = staff.btReserved6;
		mStaffInfo.nNumLyric = staff.nNumLyric;

		nPos = in.Tell();

		if ( mStaffInfo.nNumLyric )
		{
			bResult = ReadLEShort(in, mStaffInfo.nAlignment) &&
					  ReadLEShort(in, mStaffInfo.nStaffOffset);
			if ( !bResult )
				return false;
		}
	}
	else if ( m_pParent->nVersion == NWC_Version155 )
	{
		staff155 staff;
		memset(&staff, 0, sizeof(staff));
		bResult = ReadBytes(in, staff.btEndingBar) &&
			ReadBytes(in, staff.btMuted) &&
			ReadBytes(in, staff.btReserved1) &&
			ReadBytes(in, staff.btChannel) &&
			ReadBytes(in, staff.btReserved2) &&
			ReadBytes(in, staff.btPlaybackDevice) &&
			ReadBytes(in, staff.btReserved3) &&
			ReadBytes(in, staff.btSelectPatchBank) &&
			ReadBytes(in, staff.btReserved4) &&
			ReadBytes(in, staff.btPatchName) &&
			ReadBytes(in, staff.btReserved5) &&
			ReadLEShort(in, staff.nStyle) &&
			ReadLEShort(in, staff.nVerticalSizeUpper) &&
			ReadLEShort(in, staff.nVerticalSizeLower) &&
			ReadLEShort(in, staff.nLayerWithNextStaff) &&
			ReadLEShort(in, staff.nTransposition) &&
			ReadLEShort(in, staff.nPartVolume) &&
			ReadLEShort(in, staff.nStereoPan) &&
			ReadBytes(in, staff.btReserved6) &&
			ReadBytes(in, staff.btColor) &&
			ReadLEShort(in, staff.nNumLyric);
		if ( !bResult )
			return false;

		mStaffInfo.btEndingBar = staff.btEndingBar;
		mStaffInfo.btMuted = staff.btMuted;
		mStaffInfo.btReserved1 = staff.btReserved1;
		mStaffInfo.btChannel = staff.btChannel;
		mStaffInfo.btReserved2 = staff.btReserved2;
		mStaffInfo.btPlaybackDevice = staff.btPlaybackDevice;
		mStaffInfo.btReserved3 = staff.btReserved3;
		mStaffInfo.btSelectPatchBank = staff.btSelectPatchBank;
		memcpy(mStaffInfo.btReserved4, staff.btReserved4, sizeof(mStaffInfo.btReserved4));
		mStaffInfo.btPatchName = staff.btPatchName;
		mStaffInfo.btReserved5 = staff.btReserved5;
		mStaffInfo.nStyle = staff.nStyle;
		mStaffInfo.nVerticalSizeUpper = staff.nVerticalSizeUpper;
		mStaffInfo.nVerticalSizeLower = staff.nVerticalSizeLower;
		mStaffInfo.btLines = 5;
		mStaffInfo.nLayerWithNextStaff = staff.nLayerWithNextStaff;
		mStaffInfo.nTransposition = staff.nTransposition;
		mStaffInfo.nPartVolume = staff.nPartVolume;
		mStaffInfo.nStereoPan = staff.nStereoPan;
		mStaffInfo.btColor = staff.btColor;
		//mStaffInfo.btReserved6 = staff.btReserved6;
		mStaffInfo.nNumLyric = staff.nNumLyric;

		// some file comes with 0xCDCD.
		if ( (unsigned short)mStaffInfo.nNumLyric == 0xCDCD )
			mStaffInfo.nNumLyric = 0;

		nPos = in.Tell();

		if ( mStaffInfo.nNumLyric )
		{
			bResult = ReadLEShort(in, mStaffInfo.nAlignment) &&
					  ReadLEShort(in, mStaffInfo.nStaffOffset);
			if ( !bResult )
				return false;
		}
	}
	else if ( m_pParent->nVersion == NWC_Version170 )
	{
		staff170 staff;
		memset(&staff, 0, sizeof(staff));
		bResult = ReadBytes(in, staff.btEndingBar) &&
			ReadBytes(in, staff.btMuted) &&
			ReadBytes(in, staff.btReserved1) &&
			ReadBytes(in, staff.btChannel) &&
			ReadBytes(in, staff.btReserved2) &&
			ReadBytes(in, staff.btPlaybackDevice) &&
			ReadBytes(in, staff.btReserved3) &&
			ReadBytes(in, staff.btSelectPatchBank) &&
			ReadBytes(in, staff.btReserved4) &&
			ReadBytes(in, staff.btPatchName) &&
			ReadBytes(in, staff.btReserved5) &&
			ReadLEShort(in, staff.nStyle) &&
			ReadLEShort(in, staff.nVerticalSizeUpper) &&
			ReadLEShort(in, staff.nVerticalSizeLower) &&
			ReadLEShort(in, staff.nLayerWithNextStaff) &&
			ReadLEShort(in, staff.nTransposition) &&
			ReadLEShort(in, staff.nPartVolume) &&
			ReadLEShort(in, staff.nStereoPan) &&
			ReadBytes(in, staff.btColor) &&
			ReadLEShort(in, staff.btReserved6) &&
			ReadLEShort(in, staff.nNumLyric);
		if ( !bResult )
			return false;

		mStaffInfo.btEndingBar = staff.btEndingBar;
		mStaffInfo.btMuted = staff.btMuted;
		mStaffInfo.btReserved1 = staff.btReserved1;
		mStaffInfo.btChannel = staff.btChannel;
		mStaffInfo.btReserved2 = staff.btReserved2;
		mStaffInfo.btPlaybackDevice = staff.btPlaybackDevice;
		mStaffInfo.btReserved3 = staff.btReserved3;
		mStaffInfo.btSelectPatchBank = staff.btSelectPatchBank;
		memcpy(mStaffInfo.btReserved4, staff.btReserved4, sizeof(mStaffInfo.btReserved4));
		mStaffInfo.btPatchName = staff.btPatchName;
		mStaffInfo.btReserved5 = staff.btReserved5;
		mStaffInfo.nStyle = staff.nStyle;
		mStaffInfo.nVerticalSizeUpper = staff.nVerticalSizeUpper;
		mStaffInfo.nVerticalSizeLower = staff.nVerticalSizeLower;
		mStaffInfo.btLines = 5;
		mStaffInfo.nLayerWithNextStaff = staff.nLayerWithNextStaff;
		mStaffInfo.nTransposition = staff.nTransposition;
		mStaffInfo.nPartVolume = staff.nPartVolume;
		mStaffInfo.nStereoPan = staff.nStereoPan;
		mStaffInfo.btColor = staff.btColor;
		//mStaffInfo.btReserved6 = staff.btReserved6;
		mStaffInfo.nNumLyric = staff.nNumLyric;

		nPos = in.Tell();

		if ( mStaffInfo.nNumLyric )
		{
			bResult = ReadLEShort(in, mStaffInfo.nAlignment) &&
					  ReadLEShort(in, mStaffInfo.nStaffOffset);
			if ( !bResult )
				return false;
		}
	}
	else if ( m_pParent->nVersion == NWC_Version175 )
	{
		staff175 staff;
		memset(&staff, 0, sizeof(staff));
		bResult = ReadBytes(in, staff.btEndingBar) &&
			ReadBytes(in, staff.btMuted) &&
			ReadBytes(in, staff.btReserved1) &&
			ReadBytes(in, staff.btChannel) &&
			ReadBytes(in, staff.btReserved2) &&
			ReadBytes(in, staff.btPlaybackDevice) &&
			ReadBytes(in, staff.btReserved3) &&
			ReadBytes(in, staff.btSelectPatchBank) &&
			ReadBytes(in, staff.btReserved4) &&
			ReadBytes(in, staff.btPatchName) &&
			ReadBytes(in, staff.btReserved5) &&
			ReadLEShort(in, staff.nStyle) &&
			ReadLEShort(in, staff.nVerticalSizeUpper) &&
			ReadLEShort(in, staff.nVerticalSizeLower) &&
			ReadBytes(in, staff.btLines) &&
			ReadLEShort(in, staff.nLayerWithNextStaff) &&
			ReadLEShort(in, staff.nTransposition) &&
			ReadLEShort(in, staff.nPartVolume) &&
			ReadLEShort(in, staff.nStereoPan) &&
			ReadBytes(in, staff.btColor) &&
			ReadLEShort(in, staff.nAlignSyllable) &&
			ReadLEShort(in, staff.nNumLyric);
		if ( !bResult )
			return false;

		mStaffInfo.btEndingBar = staff.btEndingBar;
		mStaffInfo.btMuted = staff.btMuted;
		mStaffInfo.btReserved1 = staff.btReserved1;
		mStaffInfo.btChannel = staff.btChannel;
		mStaffInfo.btReserved2 = staff.btReserved2;
		mStaffInfo.btPlaybackDevice = staff.btPlaybackDevice;
		mStaffInfo.btReserved3 = staff.btReserved3;
		mStaffInfo.btSelectPatchBank = staff.btSelectPatchBank;
		memcpy(mStaffInfo.btReserved4, staff.btReserved4, sizeof(mStaffInfo.btReserved4));
		mStaffInfo.btPatchName = staff.btPatchName;
		mStaffInfo.btReserved5 = staff.btReserved5;
		mStaffInfo.nStyle = staff.nStyle;
		mStaffInfo.nVerticalSizeUpper = staff.nVerticalSizeUpper;
		mStaffInfo.nVerticalSizeLower = staff.nVerticalSizeLower;
		mStaffInfo.btLines = 5;
		mStaffInfo.nLayerWithNextStaff = staff.nLayerWithNextStaff;
		mStaffInfo.nTransposition = staff.nTransposition;
		mStaffInfo.nPartVolume = staff.nPartVolume;
		mStaffInfo.nStereoPan = staff.nStereoPan;
		mStaffInfo.btColor = staff.btColor;
		mStaffInfo.nAlignSyllable = staff.nAlignSyllable;
		mStaffInfo.nNumLyric = staff.nNumLyric;
		mStaffInfo.nAlignment = staff.nAlignment;
		mStaffInfo.nStaffOffset = staff.nStaffOffset;

		nPos = in.Tell();

		if ( mStaffInfo.nNumLyric )
		{
			bResult = ReadLEShort(in, mStaffInfo.nAlignment) &&
					  ReadLEShort(in, mStaffInfo.nStaffOffset);
			if ( !bResult )
				return false;
		}
	}
	else
	{
		staff200& staff = mStaffInfo;
		bResult = ReadBytes(in, staff.btEndingBar) &&
			ReadBytes(in, staff.btMuted) &&
			ReadBytes(in, staff.btReserved1) &&
			ReadBytes(in, staff.btChannel) &&
			ReadBytes(in, staff.btReserved2) &&
			ReadBytes(in, staff.btPlaybackDevice) &&
			ReadBytes(in, staff.btReserved3) &&
			ReadBytes(in, staff.btSelectPatchBank) &&
			ReadBytes(in, staff.btReserved4) &&
			ReadBytes(in, staff.btPatchName) &&
			ReadBytes(in, staff.btReserved5) &&
			ReadBytes(in, staff.btDefaultDynamicVelocity) &&
			ReadLEShort(in, staff.nStyle) &&
			ReadLEShort(in, staff.nVerticalSizeUpper) &&
			ReadLEShort(in, staff.nVerticalSizeLower) &&
			ReadBytes(in, staff.btLines) &&
			ReadLEShort(in, staff.nLayerWithNextStaff) &&
			ReadLEShort(in, staff.nTransposition) &&
			ReadLEShort(in, staff.nPartVolume) &&
			ReadLEShort(in, staff.nStereoPan) &&
			ReadBytes(in, staff.btColor) &&
			ReadLEShort(in, staff.nAlignSyllable) &&
			ReadLEShort(in, staff.nNumLyric);
		if ( !bResult )
			return false;

		nPos = in.Tell();

		if ( mStaffInfo.nNumLyric )
		{
			bResult = ReadLEShort(in, mStaffInfo.nAlignment) &&
					  ReadLEShort(in, mStaffInfo.nStaffOffset);
			if ( !bResult )
				return false;
		}
	}

	if ( mStaffInfo.nNumLyric == 0 )
	{
		mStaffInfo.nAlignment = 0;
		mStaffInfo.nStaffOffset = 0;
	}

	wxASSERT ( 0 <= mStaffInfo.nNumLyric && mStaffInfo.nNumLyric <= 8 );

	if ( false == LoadLyric(in, out) )
		return false;

	if ( false == LoadNotes(in, out) )
		return false;

	return true;
}

bool CStaff::LoadLyric(wxFile& in, FILE* out)
{
	long nPos = in.Tell();
	wxFprintf(out, _T("#lyric = %d\n"), mStaffInfo.nNumLyric);
	if ( !(0 <= mStaffInfo.nNumLyric && mStaffInfo.nNumLyric <= 8 ) )
	{
		fprintf(stderr, "invalid nNumLyric=%d at file offset=%08x\n", mStaffInfo.nNumLyric, nPos);
		if ( wxIsDebuggerRunning() ) MyTrap();
		return false;
	}

	wxString str;
	m_strLyric.SetCount(mStaffInfo.nNumLyric);
	m_strLyrics.SetCount(mStaffInfo.nNumLyric);
	for ( int i=0 ; i<mStaffInfo.nNumLyric; i++ )
	{
		ReadLEShort(in, mStaffInfo.nLyricBlockSizeInByte);
		mStaffInfo.nLyricSizeInByte = 0;

		wxString& strThisLyric = m_strLyric[i];
		TVector<wxString>& strThisLyrics = m_strLyrics[i];

		if ( mStaffInfo.nLyricBlockSizeInByte )
		{
			ReadLEShort(in, mStaffInfo.nLyricSizeInByte);

			nPos = in.Tell();
			ReadLEShort(in, mStaffInfo.nReserved8);

			while ( true )
			{
				str = LoadStringNULTerminated(in);
				if ( str.IsEmpty() )
					break;

				strThisLyric += str;
				strThisLyrics.Add(str);
			}

			nPos = nPos + mStaffInfo.nLyricBlockSizeInByte;
			in.Seek(nPos, wxFromStart);
		}

		wxFprintf(out, _T("lyric%d : %s\n"), i, strThisLyric.c_str());
	}

	return true;
}

bool CStaff::LoadNotes(wxFile& in, FILE* out)
{
	long nPos = in.Tell();

	struct 
	{
		short	nUnknown1;
		short	nUnknown2;
		short	nNoObj;
	} StaffHeader;

	bool bResult = false;
	if ( mStaffInfo.nNumLyric )
	{
		bResult = ReadLEShort(in, StaffHeader.nUnknown1) &&
				  ReadLEShort(in, StaffHeader.nUnknown2) &&
				  ReadLEShort(in, StaffHeader.nNoObj);
	}
	else
	{
		StaffHeader.nUnknown1 = 0;
		bResult = ReadLEShort(in, StaffHeader.nUnknown2) &&
				  ReadLEShort(in, StaffHeader.nNoObj);
	}

	if ( bResult == false )
	{
		fprintf(stderr, "error in file, at file offset=%08x\n", nPos);
		if ( wxIsDebuggerRunning() ) MyTrap();
		return false;
	}

	short nObjCount = StaffHeader.nNoObj;
	if( m_pParent->nVersion > NWC_Version150 )
		nObjCount -= 2;

	if ( nObjCount < 0 )
	{
		fprintf(stderr, "invalid objcount=%d, at file offset=%08x\n", nObjCount, nPos);
		if ( wxIsDebuggerRunning() ) MyTrap();
		return false;
	}

	for ( int i=0; i<nObjCount; i++ )
	{
		nPos = in.Tell();

		CObj* pObj = CreateNLoadObject(in, out, this);
		if ( pObj )
		{
			mObjArray.Add(pObj);
		}
		else
		{
			return false;
		}
	}

	wxFprintf(out, _T("end of staff\n"));
	return true;
}
