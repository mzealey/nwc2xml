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

#endif // !defined(__Precompile_H__)
