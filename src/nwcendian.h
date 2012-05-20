
#ifndef	__NWCENDIAN_H__
#define	__NWCENDIAN_H__

#pragma once

// endian functions & macro : these do not support Middle-endian

#define	SWAP_BYTE(a, b)	{ tmp = a; a = b; b = tmp; }

#ifdef MACOS
#include <machine/endian.h>
#define	__BYTE_ORDER	BYTE_ORDER
#define	__LITTLE_ENDIAN	LITTLE_ENDIAN
#define	__BIG_ENDIAN	BIG_ENDIAN
#endif

#ifdef LINUX
#include <endian.h>
#endif

#ifndef __BYTE_ORDER

#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321

#if (defined(LITTLEENDIAN) && (LITTLEENDIAN!=0)) || defined(_M_IX86)
#define	__BYTE_ORDER	__LITTLE_ENDIAN
#else
#define	__BYTE_ORDER	__BIG_ENDIAN
#endif

#endif

inline unsigned short SwapEndianS(unsigned short s)
{
	char* ch = (char*)&s, tmp;
	SWAP_BYTE(ch[0], ch[1]);
	return s;
}

inline unsigned long SwapEndianL(unsigned long l)
{
	char* ch = (char*)&l, tmp;
	SWAP_BYTE(ch[1], ch[2]);
	SWAP_BYTE(ch[0], ch[3]);
	return l;
}

#if (__BYTE_ORDER==__BIG_ENDIAN)
#define	BE2HostL(l)		(l)
#define	BE2HostS(s)		(s)
#define	Host2BEL(l)		(l)
#define	Host2BES(s)		(s)
#else
#define	BE2HostL(l)		SwapEndianL(l)
#define	BE2HostS(s)		SwapEndianS(s)
#define	Host2BEL(l)		SwapEndianL(l)
#define	Host2BES(s)		SwapEndianS(s)
#endif

#if (__BYTE_ORDER==__LITTLE_ENDIAN)
#define	LE2HostL(l)		(l)
#define	LE2HostS(s)		(s)
#define	Host2LEL(l)		(l)
#define	Host2LES(s)		(s)
#else
#define	LE2HostL(l)		SwapEndianL(l)
#define	LE2HostS(s)		SwapEndianS(s)
#define	Host2LEL(l)		SwapEndianL(l)
#define	Host2LES(s)		SwapEndianS(s)
#endif

#endif /* __NWCENDIAN_H__ */
