// Precompile.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(__Precompile_H__)
#define __Precompile_H__

#if !defined(MACOS)
#pragma once
#endif

#define _CRT_SECURE_NO_DEPRECATE

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/wx.h"
#include "wx/file.h"
#include "wx/ffile.h"
#include "TVector.h"

typedef TVector<unsigned char> TByteArray;

#ifndef	_WIN32
typedef unsigned char BYTE;
typedef unsigned int  UINT;
typedef unsigned int  DWORD;
typedef const wxChar* LPCTSTR;

#define	HIBYTE(W)		(((W) >> 8) & 0xFF)
#define	LOBYTE(W)		((W) & 0xFF)

#define	ANSI_CHARSET		0

#define	ERROR_SUCCESS		0
#define	ERROR_INVALID_DATA	13
#define	ERROR_GEN_FAILURE	31

#define _countof(x)	(sizeof(x)/sizeof(x[0]))

inline long MulDiv(long a, long b, long c) { return a*b/c; }
#endif

#define	ERROR_INVALID_VERSION	100
#define	ERROR_INVALID_STAFF		101


#endif // !defined(__Precompile_H__)
