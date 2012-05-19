
#pragma once

#include "nwcobj.h"

struct NWCInfo
{
	wxString	strUser;
#pragma pack(push, 1)
	union
	{
		short	nVersion;

		struct
		{
			char	nVerMinor;
			char	nVerMajor;
		};
	};
#pragma pack(pop)
	bool	bUnregistered;
	wxString strTitle;
	wxString strAuthor;
	wxString strLyricist;	// from 2.0
	wxString strCopyright1;
	wxString strCopyright2;
	wxString strComment;
	UINT	nStaffCount;
	wxString strKeySignature;
	wxString strTimeSignature;
	wxString strLyric;
};

class CNWCFile
{
public:
	wxString	strUser;
	wxString	strUnknown1;

#pragma pack(push, 1)
	union
	{
		short	nVersion;

		struct
		{
			char	nVerMinor;
			char	nVerMajor;
		};
	};
	char	btUnknown2[4];
#pragma pack(pop)

	wxString strTitle;
	wxString strAuthor;
	wxString strLyricist;	// from 2.0
	wxString strCopyright1;
	wxString strCopyright2;
	wxString strComment;

#pragma pack(push, 1)
	BYTE	chExtendLastSystem;		// 'Y' or 'N'
	BYTE	chIncreaseNoteSpacing;	// 'Y' or 'N'
	BYTE	btUnknown3[5];
	BYTE	btMeasureNumbers;		// index of [None, Plain, Circled, Boxed]
	BYTE	btUnknown4[1];			// 0x00
	short	nMeasureStart;
#pragma pack(pop)

	wxString strMarginTop;
	wxString strMarginInside;
	wxString strMarginOutside;
	wxString strMarginBottom;

#pragma pack(push, 1)
	BYTE	bMirrorMargin;			// 0 or 1
	BYTE	btUnknown5[2];			// btUnknown5[0] = 0x07, 0x08 ? has notation typeface
	BYTE	nGroupVisibility[32];	// 0x01:first group, 0x02:second group
	BYTE	bAllowLayering;			// 0x00 or 0xFF
	wxString	strNotationTypeface;
	short	nStaffHeight;
#pragma pack(pop)

	// 10 or 12 fontinfo
	TVector<CFontInfo>
			mFontInfos;

#pragma pack(push, 1)
	BYTE	btTitlePageInfo;
	BYTE	btStaffLabels;
	short	nStartPageNo;
	short	nStaffCount;
#pragma pack(pop)

	BYTE	btJustifyPrintedSystemVertically;	// version 2.0

	TVector<CStaff*>
			mStaffs;

public:
	CNWCFile();
	~CNWCFile();

	void	Init();

	DWORD	Load(wxFile& in, FILE* out, FILELOAD fl=FILELOAD_ALL);

protected:
	DWORD	LoadCompressed(wxFile& in, FILE* out, FILELOAD fl);
} ;
