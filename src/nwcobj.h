
#pragma once

#include <vector>
#include "wx/file.h"

wxString LoadStringNULTerminated(wxFile& in);
wxString LoadStringSpaceTerminated(wxFile& in);

class CObj;
class CStaff;
class CNWCFile;

typedef enum
{
	FILELOAD_ALL,
	FILELOAD_INFO,
} FILELOAD;

typedef enum
{
	NWC_Version120	= 0x0114,
	NWC_Version130	= 0x011E,
	NWC_Version150	= 0x0132,
	NWC_Version155	= 0x0137,
	NWC_Version170	= 0x0146,
	NWC_Version175	= 0x014B,
	NWC_Version200	= 0x0200,
} NWC_Version;

inline bool	IsValidVersion(short nVersion)
{
	return //(nVersion==NWC_Version120) ||
		   (nVersion==NWC_Version130) ||
		   (nVersion==NWC_Version150) || (nVersion==NWC_Version155) ||
		   (nVersion==NWC_Version170) || (nVersion==NWC_Version175) ||
		   (nVersion==NWC_Version200);
}

typedef enum
{
	AC_SHARP,
	AC_FLAT,
	AC_NATURAL,
	AC_SHARP_SHARP,
	AC_FLAT_FLAT,
	AC_NORMAL,
} ACCIDENTAL;

// barline style
typedef enum
{
	BL_SINGLE,
	BL_DOUBLE,
	BL_SECTION_OPEN,
	BL_SECTION_CLOSE,
	BL_LOCAL_OPEN,
	BL_LOCAL_CLOSE,
	BL_MASTER_OPEN,
	BL_MASTER_CLOSE,

	// for ending bar line
	BL_HIDDEN,
} BL_STYLE;

typedef enum
{
	DT_DOT		= 0x01,
	DT_DOTDOT	= 0x02,
	DT_TRIPLET	= 0x0C,

	DT_TRI_START= 0x04,
	DT_TRI_CONT	= 0x08,
	DT_TRI_STOP	= 0x0C,
} DURATIONTYPE;

typedef enum
{
	FS_CODA,
	FS_SEGNO,
	FS_FINE,
	FS_TOCODA,
	FS_DACAPO,
	FS_DCALCODA,
	FS_DCALFINE,
	FS_DALSEGNO,
	FS_DSALCODA,
	FS_DSALFINE,
} FLOWSTYLE;

typedef enum
{
	NA_ACCENT			= 0x00001,
	NA_GRACE			= 0x00002,
	NA_STACCATO			= 0x00004,
	NA_TENUTO			= 0x00008,

	NA_BEAM_BEG			= 0x00100,
	NA_BEAM_END			= 0x00200,
	NA_BEAM_MID			= 0x00300,
	NA_BEAM_MASK		= 0x00300,

	NA_SLUR_BEG			= 0x00400,
	NA_SLUR_END			= 0x00800,
	NA_SLUR_MID			= 0x00C00,
	NA_SLUR_MASK		= 0x00C00,

	NA_SLUR_DIR_DEF		= 0x00000,
	NA_SLUR_DIR_UP		= 0x01000,
	NA_SLUR_DIR_DOWN	= 0x02000,
	NA_SLUR_DIR_MASK	= 0x03000,
	NA_SLUR_DIR_SHIFT	= 0x01000,

	NA_STEM_DEF			= 0x00000,
	NA_STEM_UP			= 0x04000,
	NA_STEM_DOWN		= 0x08000,
	NA_STEM_MASK		= 0x0C000,
	NA_STEM_SHIFT		= 0x04000,

	NA_TIE_BEG			= 0x10000,
	NA_TIE_END			= 0x20000,

	NA_TIE_DIR_DEF		= 0x00000,
	NA_TIE_DIR_UP		= 0x40000,
	NA_TIE_DIR_DOWN		= 0x80000,
	NA_TIE_DIR_MASK		= 0xC0000,
	NA_TIE_DIR_SHIFT	= 0x40000,
} NOTEATTR;

typedef enum
{
	TIMESIG_STANDARD,
	TIMESIG_COMMON_TIME,
	TIMESIG_ALLA_BREVE,
} TIMESIGSTYLE;

typedef enum
{
	PS_RELEASE,
	PS_DOWN,
} PEDALSTYLE;

typedef enum
{
	FONTID_STAFF_ITALIC,
	FONTID_STAFF_BOLD,
	FONTID_STAFF_LYRIC,
	FONTID_PAGE_TITLE_TEXT,
	FONTID_PAGE_TEXT,
	FONTID_PAGE_SMALL_TEXT,
	FONTID_USER1,
	FONTID_USER2,
	FONTID_USER3,
	FONTID_USER4,
	FONTID_USER5,
	FONTID_USER6,
} ;

typedef enum
{
	// All position is based on A octave

	// From 1.70, CCCC CSSS : Color & Show On Printed Page option is added right after objtype

	Obj_Clef	= 0x00, // 0x00, 0x00, 0x00, 0x00
						//	 |			 +-----> // index of [None, Octave Up, Octave Down]
						//	 +-----> // index of [Treble, Bass, Alto, Tenor]
	Obj_KeySig	= 0x01,	// 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						//   |		   +-> Sharp turn on bit(0GFE DCBA)
						//   +-> Flat turn on bit(0GFE DCBA)
	Obj_BarLine	= 0x02,	// 0x01, 0x02
						//   |		+-> Local Repeat Count (default:2)
						//   +-> 0x80:(Force System Break), 0x7F:Style(0=Single, 1=Double, 2/3=Section Open/Close, 4,5=Local Repeat Open/Close, 6,7=Master Repeat Open/Close)
	Obj_Ending	= 0x03,	// 0x00, 0x00
						//   +-> Style(nth special ending, 0x01:1st, 0x02:2nd, 0x04, 3rd, ...)
	Obj_Instrument=0x04,// 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00
						//	 |		|	 |			 |			 +-> // Patch Name
						//	 |		|	 |			 +------> 0x3F80:Controller 0 mask, 0x007F:Controller 32 mask
						//	 |		|	 +-> 0x01:Select a patch bank, 0x06:Justification(index of [Left, Center, Right])
						//	 |		+-> 0x01:Preserve Width, 0x18:Alignment(index of [best, before, after, at next])
						//	 +-> Staff Position(minus)

						// 2.0 version
						//													+----------------+ name +-----------+			+-> dynamic velocity
						//													|									|			|
						// 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x5b, 0x23, 0x31, 0x32, 0x38, 0x5d, 0x00, 0x00, 0x0a, 0x1e, 0x2d, 0x3c, 0x4b, 0x5c, 0x6c, 0x7f

	Obj_TimeSig	= 0x05,	// 0x04, 0x00, 0x02, 0x00, 0x00, 0x00
						//   |			|			+------> Style(index of [Standard Signature, Common Time, Alla Breve])
						//   |			+------> Value of Beats(index of [1,2,4,8,16,32])
						//   +-----> Bits Per Measure
	Obj_Tempo	= 0x06,	// 0xF3, 0xFF, 0x78, 0x00, 0x02, 0x00(String)
						//   |		|	|			 +-> Base(index of [8th, dotted 8th, quarter, dotted quarter, half, dotted half])
						//   |		|	+-------> 120:Value
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_Dynamic	= 0x07,	// 0xF3, 0xFF, 0x03, 0x7F, 0x00, 0xFF, 0xFF
						//	 |		|	|		|			+-----> MIDI Vol
						//	 |		|	|		+-----> Note Velocity
						//   |		|	+-> 0x1F:Style(index of [ppp, pp, p, mp, mf, f, ff, fff]), 0x20:Overide Note Velocity, 0x40:Override MIDI Vol
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_Note	= 0x08,	// 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x0D, 0x00, 0x00, 
						//   |		|	|		   |		|	 |				   +-> 0x10: Mute, 0x20: No Leger Lines, 0x40: next byte contains stem length(version 2.0, default 7), 0x80: use stem for articulations
						//   |		|	|		   |		|	 |		+-> 0x08 : Tie Direction(0:Up, 1:Down), 0x07(index of [#, b, n, ##(?), bb(?), normal])
						//   |		|	|		   |		|	 +-> Position
						//   |		|	|		   |		+-> 0x80:Slur Direction On, 0x40:Tie Direction On, 0x20:grace, 0x04:Tenuto, 0x03:slur(index of [begin, end, middle])
						//   |		|	|		   +-> 0x20:accent, 0x10:tied begin, 0x08:tied end, 0x04: . (half of note), 0x02:Staccato, 0x01:..
						//   |		|	+-------> 0x0040:Slur Direction(0:Up, 1:Down), 0x0180:Lyric Syllable(index of [Default, Always, Never]), 0x30:Stem(index of [Default, Up, Down]), 0x03:Beam(1:begin, 2:middle, 3:end), 0x0C:Triplet(1:begin, 2:middle, 3:end), 
						//	 |		+-> 0x0F:Extra Note Spacing
						//   +-> 0xF0:Extra Accidental Spacing, 0x0F:Duration(0=Whole, 1=Half, 2=Quarter, 3=8th, 4=16th, ...)
	Obj_Rest	= 0x09,	// 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						//   |			|			|		|	 +------> Vertical Offset
						//	 |			|			|		+-> 0x80:Slur Direction On, 0x40:Tie Direction On, 0x20:grace, 0x02:slur end, 0x01:slur begin
						//	 |			|			+-> 0x04:Dot, 0x01:DotDot
						//	 |			+-------> 0x0040:Slur Direction(0:Up, 1:Down), 0x0180:Lyric Syllable(index of [Default, Always, Never])
						//   +-> 0x0F:Duration(0=Whole, 1=Half, 2=Quarter, 3=8th, 4=16th, ...)
	Obj_NoteCM	= 0x0A,	// chord member starting from note
						// 0x02 0x00 0x00 0x00 0x00 0x00 0x04 0x45 0x05 0x02 0x00
						//										|		  +----+-> note count
						//										+-> 0x10: Mute, 0x20: No Leger Lines, 0x40: next byte contains stem length(version 2.0, default 7), 0x80: use stem for articulations
	Obj_Pedal	= 0x0B,	// 0xF3, 0x01, 0x03
						//   |		|	+-> Style(index of [Pedal Release, Pedal Down])
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_FlowDir	= 0x0C,	// 0xF3, 0x01, 0x03, 0x00
						//   |		|	+-> Style(index of [Coda, Segno, Fine, To Coda, Da capo, D.C. al Coda, D.C. al Fine, Dal Segno, D.S. al Coda, D.S. al Fine])
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_MPC		= 0x0D,	// 0xF3, 0x01, 0xFF(x32)
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_TempVar	= 0x0E,	// 0xF3, 0x01, 0x02, 0x03
						//   |		|	|		+-> Delay
						//   |		|	+-> Style(index of [Breath Mark, Fermata, Accelerando, Allargrando, Rallentando, Ritardando, Ritenuto, Rubato, Stringendo])
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_DynVar	= 0x0F,	// 0xF3, 0x01, 0x03
						//   |		|	+-> Style(index of [cresc, decresc, dim, rfz, sfz])
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_Perform	= 0x10,	// 0xF3, 0x01, 0x03
						//   |		|	+-> 0x1F:Style(index of [Ad Libitum, Animato, Cantabile, Con brio, Dolce, Espressivo, Grazioso, Legato, Maestoso, Marcato, Meno Mosso,]
						//	 |		|							[Poco a poco, Pi?mosso, Semplice, Simile, Solo, Sostenuto, Sotto Voce, Staccato, Subito, Tenuto, Tutti, Volta Subito])
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_Text	= 0x11,	// 0xF3, 0xF6, 0x00, String
						//   |		|	+-> Display Font(index of [Staff Italic, Staff Bold, Staff Lyric, Page Title Text, Page Text, Page Small Text, User 1, ...])
						//	 |		+-> 0x01:Preserve Width, 0x06:Justification(index of [Left, Center, Right]), 0x18:Alignment(index of [best, before, after, at next])
						//   +------> (-13) : Position
	Obj_RestCM	= 0x12,	//	// chord member starting from rest
} OBJTYPE;

// store key alteration for each step(A-G)
class CHROMALTER
{
public:
	short	clefShift;
	char	measure[8];
	char	system[8];

	CHROMALTER()
	{
		clefShift = 0;
		memset(measure, 0, sizeof(measure));
		memset(system, 0, sizeof(system));
	}

	void	ResetMeasure()
	{
		memcpy(measure, system, sizeof(measure));
	}
};

typedef TVector< TVector<wxString> >	CLyricArray;

extern wxMBConv* g_pMBConv;
extern bool g_bDumpOffset;

class CObj
{
public:
	CStaff*	m_pParent;

	short	mObjType;
#pragma pack(push, 1)
	BYTE	mVisible;
#pragma pack(pop)

	CObj(short objType, CStaff* pParent=NULL)
	{ mObjType = objType; m_pParent = pParent; mVisible = 0; }
	virtual ~CObj()
	{ }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	static	void DumpBinary(FILE* fp, BYTE* pData, size_t nCount, bool bPrependComma=true);

	NWC_Version	GetLoadedVersion();

	BYTE	GetColor()
	{
		return mVisible >> 3;
	}

	BYTE	GetShowPrinted()
	{
		return mVisible & 0x07;
	}
};

// zero fill everything after the vtbl pointer
#define COBJ_ZERO_INIT_OBJECT(base_class) \
	memset(((base_class*)this)+1, 0, sizeof(*this) - sizeof(class base_class));

class CClefObj : public CObj
{
public:
#pragma pack(push, 1)
	short	mClefType;
	short	mOctaveShift;
#pragma pack(pop)

	CClefObj(CStaff* pParent=NULL) : CObj(Obj_Clef, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);
};

class CKeySigObj : public CObj
{
public:
#pragma pack(push, 1)
	BYTE	mFlat;
	BYTE	mData2[1];
	BYTE	mSharp;
	BYTE	mData3[7];
#pragma pack(pop)

	CKeySigObj(CStaff* pParent=NULL) : CObj(Obj_KeySig, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	void	GetChromAlter(CHROMALTER& ca) const;

	bool	SetFifth(int nFifth);

	bool	GetStdName(wxString& strName) const;
};

class CBarLineObj : public CObj
{
public:
#pragma pack(push, 1)
	BYTE	mStyle;
	BYTE	mLocalRepeatCount;
#pragma pack(pop)

	CBarLineObj(CStaff* pParent=NULL) : CObj(Obj_BarLine, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	bool		SystemBreak() const
	{
		return (mStyle & 0xF0) != 0;
	}

	BL_STYLE	GetStyle() const
	{
		return (BL_STYLE) (mStyle & 0x7F);
	}
};

class CEndingObj : public CObj
{
public:
#pragma pack(push, 1)
	BYTE	mStyle;
	BYTE	mData2[1];
#pragma pack(pop)

	CEndingObj(CStaff* pParent=NULL) : CObj(Obj_Ending, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);
};

class CInstrumentObj : public CObj
{
public:
#pragma pack(push, 1)
	BYTE	mData1[8];
#pragma pack(pop)
	wxString	mName;
	BYTE	mData2[1];
	BYTE	mVelocity[8];

	CInstrumentObj(CStaff* pParent=NULL) : CObj(Obj_Instrument, pParent)
	{
		memset(&mData1, 0, sizeof(mData1));
		memset(&mData2, 0, sizeof(mData2));
		memset(&mVelocity, 0, sizeof(mVelocity));
	}

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);
};

class CTimeSigObj : public CObj
{
public:
#pragma pack(push, 1)
	short	mBit_Measure;
	short	mBits;
	short	mStyle;
#pragma pack(pop)

	CTimeSigObj(CStaff* pParent=NULL) : CObj(Obj_TimeSig, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	wxString	GetAsString() const;
};

class CTempoObj : public CObj
{
public:
#pragma pack(push, 1)
	char	mPos;
	BYTE	mPlacement;
	short	mTempoValue;
	BYTE	mBase;
#pragma pack(pop)

	wxString	mText;

	CTempoObj(CStaff* pParent=NULL) : CObj(Obj_Tempo, pParent)
	{ memset(&mPos, 0, offsetof(CTempoObj, mText) - offsetof(CTempoObj, mPos)); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	wxString GetBaseAsString();

	long	GetTempoByQuarter() const;
};

class CDynamicObj : public CObj
{
public:
#pragma pack(push, 1)
	char	mPos;
	BYTE	mPlacement;
	BYTE	mStyle;
	short	mNoteVelocity;
	short	mMIDIVolume;
#pragma pack(pop)

	CDynamicObj(CStaff* pParent=NULL) : CObj(Obj_Dynamic, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	wxString GetStyleAsString(bool bExtended) const;
};

class CNoteObj : public CObj
{
public:
#pragma pack(push, 1)
	BYTE	mDuration;
	BYTE	mData2[3];
	BYTE	mAttribute1[2];
	char	mPos;
	BYTE	mAttribute2[1];
	BYTE	mData3[2];
	BYTE	mStemLength;
#pragma pack(pop)

	CNoteObj(CStaff* pParent=NULL) : CObj(Obj_Note, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	BYTE	GetDuration() const { return (0x0F&mDuration); }
	wxString GetDurationAsString() const;
	long	GetDuration(long nDivision) const;
	void	SetDuration(BYTE nDura, long nDivision);

	DURATIONTYPE GetDurationType() const;
	long		GetDivision() const;
	bool		IsStemUp() const;
	NOTEATTR	GetNoteAttributes() const;

	void		GetOctaveStep(CHROMALTER& ca, int& nOctave, wxChar& chStep, int& nAlter) const;
	void		SetOctaveStep(CHROMALTER& ca, int nOctave, wxChar chStep, int nAlter);
	ACCIDENTAL	GetAccidental() const;

	wxString GetPitchAsString() const;
};

class CRestObj : public CObj
{
public:
#pragma pack(push, 1)
	BYTE	mDuration;
	BYTE	mData2[5];
	short	mOffset;
#pragma pack(pop)

	CRestObj(short objType, CStaff* pParent=NULL) : CObj(objType, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	CRestObj(CStaff* pParent=NULL) : CObj(Obj_Rest, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	void DumpData(FILE* fp);

	BYTE	GetDuration() const { return (0x0F&mDuration); }
	wxString GetDurationAsString() const;
	long	GetDuration(long nDivision) const;
	void	SetDuration(BYTE nDura, long nDivision);

	DURATIONTYPE	GetDurationType() const;
	long	GetDivision() const;

	bool	GetOctaveStep(const CHROMALTER& ca, int& nOctave, wxChar& chStep) const;
};

class CNoteCMObj : public CObj
{
public:
#pragma pack(push, 1)
	BYTE	mData1[12];
	short	mCount;
#pragma pack(pop)
	BYTE	mStemLength;

	TVector<CObj*>	mObjArray;

	CNoteCMObj(CStaff* pParent=NULL) : CObj(Obj_NoteCM, pParent)
	{ memset(&mData1, 0, offsetof(CNoteCMObj, mObjArray) - offsetof(CNoteCMObj, mData1)); }

	virtual ~CNoteCMObj()
	{
		RemoveAll();
	}

	void RemoveAll()
	{
		size_t nCount = mObjArray.GetCount();
		for ( size_t i=0; i<nCount ; i++ )
		{
			CObj* pObj = mObjArray[i];
			delete pObj;
		}
		mObjArray.RemoveAll();
	}

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	static	UINT	GetStemMask(const TVector<CObj*>& objArray);

	UINT	GetStemMask() const
	{
		return GetStemMask(mObjArray);
	}

	// load mCount note or rest
	bool	LoadChildren(wxFile& in, FILE* out, CStaff* pStaff);
};

class CPedalObj : public CObj
{
public:
#pragma pack(push, 1)
	char	mPos;
	BYTE	mPlacement;
	BYTE	mStyle;
#pragma pack(pop)

	CPedalObj(CStaff* pParent=NULL) : CObj(Obj_Pedal, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	wxString		GetStyleAsString();
	PEDALSTYLE	GetPedalStyle() const;
};

class CFlowDirObj : public CObj
{
public:
#pragma pack(push, 1)
	char	mPos;
	BYTE	mPlacement;
	short	mStyle;
#pragma pack(pop)

	CFlowDirObj(CStaff* pParent=NULL) : CObj(Obj_FlowDir, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	wxString GetStyleAsString();
	FLOWSTYLE	GetFlowStyle() const;
};

class CMPCObj : public CObj
{
public:
#pragma pack(push, 1)
	char	mPos;
	BYTE	mPlacement;
	BYTE	mData1[32];
#pragma pack(pop)

	CMPCObj(CStaff* pParent=NULL) : CObj(Obj_MPC, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);
};

class CTempVarObj : public CObj
{
public:
#pragma pack(push, 1)
	char	mPos;
	BYTE	mPlacement;
	BYTE	mStyle;
	BYTE	mDelay;
#pragma pack(pop)

	CTempVarObj(CStaff* pParent=NULL) : CObj(Obj_TempVar, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	wxString GetStyleAsString();
};

class CDynVarObj : public CObj
{
public:
#pragma pack(push, 1)
	char	mPos;
	BYTE	mPlacement;
	BYTE	mStyle;
#pragma pack(pop)

	CDynVarObj(CStaff* pParent=NULL) : CObj(Obj_DynVar, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	wxString GetStyleAsString();
};

class CPerformObj : public CObj
{
public:
#pragma pack(push, 1)
	char	mPos;
	BYTE	mPlacement;
	BYTE	mStyle;
#pragma pack(pop)

	CPerformObj(CStaff* pParent=NULL) : CObj(Obj_Perform, pParent)
	{ COBJ_ZERO_INIT_OBJECT(CObj); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	wxString GetStyleAsString();
};

class CTextObj : public CObj
{
public:
	char	mPos;
	BYTE	mPreserveWidth;
	BYTE	mJustification;
	BYTE	mAlignment;
	BYTE	mFont;

protected:
	struct BF170
	{
		BYTE	bPreserveWidth : 1;
		BYTE	nJustification : 2;
		BYTE	nAlignment : 2;
		BYTE	nUnknown1 : 3;
	};
	BYTE	mData[1];

public:
	wxString	mText;

	CTextObj(CStaff* pParent=NULL) : CObj(Obj_Text, pParent)
	{ memset(&mPos, 0, offsetof(CTextObj, mText) - offsetof(CTextObj, mPos)); }

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);
};

class CRestCMObj : public CRestObj
{
public:
#pragma pack(push, 1)
	short	mCount;
#pragma pack(pop)

	TVector<CObj*>	mObjArray;

	CRestCMObj(CStaff* pParent=NULL) : CRestObj(Obj_RestCM, pParent)
	{ mCount = 0; }

	virtual ~CRestCMObj()
	{
		RemoveAll();
	}

	void RemoveAll()
	{
		size_t nCount = mObjArray.GetCount();
		for ( size_t i=0; i<nCount ; i++ )
		{
			CObj* pObj = mObjArray[i];
			delete pObj;
		}
		mObjArray.RemoveAll();
	}

	virtual bool Load(wxFile& file);
	virtual bool Dump(FILE* fp);

	UINT	GetStemMask() const
	{
		return CNoteCMObj::GetStemMask(mObjArray);
	}

	// load mCount note or rest
	bool	LoadChildren(wxFile& in, FILE* out, CStaff* pStaff);
};

CObj*	CreateNLoadObject(wxFile& file, FILE* out, CStaff* pStaff);

class CFontInfo
{
public:
	wxString strName;
	BYTE	btStyle;	// 0x01 : BOLD, 0x02 : ITALIC
	BYTE	btSize;
	BYTE	btReserved;
	BYTE	btCharset;	// LOGFONT.lfCharSet

	bool Load(wxFile& in)
	{
		strName = LoadStringNULTerminated(in);

		UINT nCount = 4;
		if ( nCount != (size_t)in.Read(&btStyle, nCount) )
			return FALSE;

		return TRUE;
	}

	void SetDefault()
	{
		strName = wxString("Times New Roman", wxConvLocal);
		btStyle = 0;
		btSize = 12;
		btReserved = 0;
		btCharset = ANSI_CHARSET;
	}

	bool Dump(FILE* fp, int nIndex);
} ;

class CStaff
{
public:
	CNWCFile*	m_pParent;

	wxString	strName;
	wxString	strLabel;		// 2.0
	wxString	strInstName;	// 2.0
	wxString	strGroup;

	struct staff130
	{
#pragma pack(push, 1)
		BYTE	btEndingBar;		// index of [Section Close, Master Repeat Close, Single, Double, Open(hidden)]
		BYTE	btMuted;
		BYTE	btReserved1;
		BYTE	btChannel;
		BYTE	btReserved2;
		BYTE	btPlaybackDevice;
		BYTE	btReserved3;
		BYTE	btSelectPatchBank;
		BYTE	btReserved4[3];
		BYTE	btPatchName;
//		BYTE	btReserved5;
		short	nStyle;				// index of [Standard, Upper Grand Staff, Lower Grand Staff, Orchestral]
		short	nVerticalSizeUpper;
		short	nVerticalSizeLower;
		short	nLayerWithNextStaff;
		//short	nTransposition;
		short	nPartVolume;
		short	nStereoPan;
		BYTE	btReserved6;
		BYTE	btColor;			// index of color
		short	nNumLyric;
		//short	nAlignment;			// index of [Bottom, Top]
		//short	nStaffOffset;

		// per lyric
		short	nLyricBlockSizeInByte;	// size of lyric block in byte from btReserved6
		short	nLyricSizeInByte;		// size of valid lyric in byte from btReserved6
		short	nReserved8;
#pragma pack(pop)
	};

	struct staff150
	{
#pragma pack(push, 1)
		BYTE	btEndingBar;		// index of [Section Close, Master Repeat Close, Single, Double, Open(hidden)]
		BYTE	btMuted;
		BYTE	btReserved1;
		BYTE	btChannel;
		BYTE	btReserved2;
		BYTE	btPlaybackDevice;
		BYTE	btReserved3;
		BYTE	btSelectPatchBank;
		BYTE	btReserved4[3];
		BYTE	btPatchName;
		//		BYTE	btReserved5;
		short	nStyle;				// index of [Standard, Upper Grand Staff, Lower Grand Staff, Orchestral]
		short	nVerticalSizeUpper;
		short	nVerticalSizeLower;
		short	nLayerWithNextStaff;
		short	nTransposition;
		short	nPartVolume;
		short	nStereoPan;
		BYTE	btReserved6;
		BYTE	btColor;			// index of color
		short	nNumLyric;
		short	nAlignment;			// index of [Bottom, Top]
		short	nStaffOffset;

		// per lyric
		short	nLyricBlockSizeInByte;	// size of lyric block in byte from btReserved6
		short	nLyricSizeInByte;		// size of valid lyric in byte from btReserved6
		short	nReserved8;
#pragma pack(pop)
	};

	struct staff155
	{
#pragma pack(push, 1)
		BYTE	btEndingBar;		// OFFSET:0x00, index of [Section Close, Master Repeat Close, Single, Double, Open(hidden)]
		BYTE	btMuted;
		BYTE	btReserved1;
		BYTE	btChannel;
		BYTE	btReserved2;
		BYTE	btPlaybackDevice;
		BYTE	btReserved3;
		BYTE	btSelectPatchBank;
		BYTE	btReserved4[3];
		BYTE	btPatchName;
		BYTE	btReserved5;
		short	nStyle;				// OFFSET:0x0D, index of [Standard, Upper Grand Staff, Lower Grand Staff, Orchestral]
		short	nVerticalSizeUpper;
		short	nVerticalSizeLower;
		short	nLayerWithNextStaff;
		short	nTransposition;
		short	nPartVolume;
		short	nStereoPan;
		BYTE	btReserved6;
		BYTE	btColor;			// index of color
		short	nNumLyric;

		// when lyric exists
		short	nAlignment;			// index of [Bottom, Top]
		short	nStaffOffset;

		// per lyric
		short	nLyricBlockSizeInByte;	// size of lyric block in byte from btReserved6
		short	nLyricSizeInByte;		// size of valid lyric in byte from btReserved6
		short	nReserved8;
#pragma pack(pop)
	};

	struct staff170
	{
#pragma pack(push, 1)
		BYTE	btEndingBar;		// index of [Section Close, Master Repeat Close, Single, Double, Open(hidden)]
		BYTE	btMuted;
		BYTE	btReserved1;
		BYTE	btChannel;
		BYTE	btReserved2;
		BYTE	btPlaybackDevice;
		BYTE	btReserved3;
		BYTE	btSelectPatchBank;
		BYTE	btReserved4[3];
		BYTE	btPatchName;
		BYTE	btReserved5;
		short	nStyle;				// index of [Standard, Upper Grand Staff, Lower Grand Staff, Orchestral]
		short	nVerticalSizeUpper;
		short	nVerticalSizeLower;
		short	nLayerWithNextStaff;
		short	nTransposition;
		short	nPartVolume;
		short	nStereoPan;
		BYTE	btColor;			// index of color
		short	btReserved6;
		short	nNumLyric;
		short	nAlignment;			// index of [Bottom, Top]
		short	nStaffOffset;

		// per lyric
		short	nLyricBlockSizeInByte;	// size of lyric block in byte from btReserved6
		short	nLyricSizeInByte;		// size of valid lyric in byte from btReserved6
		short	nReserved8;
#pragma pack(pop)
	};

	struct staff175
	{
#pragma pack(push, 1)
		BYTE	btEndingBar;		// index of [Section Close, Master Repeat Close, Single, Double, Open(hidden)]
		BYTE	btMuted;
		BYTE	btReserved1;
		BYTE	btChannel;
		BYTE	btReserved2;
		BYTE	btPlaybackDevice;
		BYTE	btReserved3;
		BYTE	btSelectPatchBank;
		BYTE	btReserved4[3];
		BYTE	btPatchName;
		BYTE	btReserved5;
		short	nStyle;				// index of [Standard, Upper Grand Staff, Lower Grand Staff, Orchestral]
		short	nVerticalSizeUpper;
		short	nVerticalSizeLower;
		BYTE	btLines;
		short	nLayerWithNextStaff;
		short	nTransposition;
		short	nPartVolume;
		short	nStereoPan;
		BYTE	btColor;			// index of color
		short	nAlignSyllable;
		short	nNumLyric;
		short	nAlignment;			// index of [Bottom, Top]
		short	nStaffOffset;

		// per lyric
		short	nLyricBlockSizeInByte;	// size of lyric block in byte from btReserved6
		short	nLyricSizeInByte;		// size of valid lyric in byte from btReserved6
		short	nReserved8;
#pragma pack(pop)
	};

	struct staff200
	{
#pragma pack(push, 1)
		BYTE	btEndingBar;		// index of [Section Close, Master Repeat Close, Single, Double, Open(hidden)]
		BYTE	btMuted;
		BYTE	btReserved1;
		BYTE	btChannel;
		BYTE	btReserved2;
		BYTE	btPlaybackDevice;
		BYTE	btReserved3;
		BYTE	btSelectPatchBank;
		BYTE	btReserved4[3];
		BYTE	btPatchName;
		BYTE	btReserved5;
		BYTE	btDefaultDynamicVelocity[8];
		short	nStyle;				// index of [Standard, Upper Grand Staff, Lower Grand Staff, Orchestral]
		short	nVerticalSizeUpper;
		short	nVerticalSizeLower;
		BYTE	btLines;
		short	nLayerWithNextStaff;
		short	nTransposition;
		short	nPartVolume;
		short	nStereoPan;
		BYTE	btColor;			// index of color
		short	nAlignSyllable;
		short	nNumLyric;
		short	nAlignment;			// index of [Bottom, Top]
		short	nStaffOffset;

		// per lyric
		short	nLyricBlockSizeInByte;	// size of lyric block in byte from btReserved6
		short	nLyricSizeInByte;		// size of valid lyric in byte from btReserved6
		short	nReserved8;
#pragma pack(pop)
	};

	staff200		mStaffInfo;
	TVector<CObj*>	mObjArray;
	TVector<wxString>	m_strLyric;
	CLyricArray		m_strLyrics;

public:
	CStaff(CNWCFile* pParent=NULL)
	{ m_pParent = pParent; }

	virtual ~CStaff()
	{
		RemoveAll();
	}
	void RemoveAll();

	void SetParent(CNWCFile* pParent)
	{ m_pParent = pParent; }

	void SetDefault();

	long GetDivisions() const;

	int  FindNthObjIndex(OBJTYPE objType, UINT nIndex=0);

	bool Load(wxFile& in, FILE* out, FILELOAD fl);

protected:
	bool LoadLyric(wxFile& in, FILE* out);
	bool LoadNotes(wxFile& in, FILE* out);
} ;
