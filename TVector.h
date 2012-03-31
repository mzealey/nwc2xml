// TVector.h: interface for the TVector template class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(__TVector_H__)
#define __TVector_H__

#include <algorithm>
#include <vector>

// template TVector
template<class T, class _Ax = std::allocator<T> >
class TVector
{
protected:
	typedef std::vector<T, _Ax>	VECTOR;
	VECTOR	m_vector;

public:
	typedef typename std::vector<T, _Ax>::iterator iterator;
	typedef typename std::vector<T, _Ax>::const_iterator const_iterator;
	typedef typename std::vector<T, _Ax>::reverse_iterator reverse_iterator;
	typedef typename std::vector<T, _Ax>::const_reverse_iterator const_reverse_iterator;
	typedef typename std::vector<T, _Ax>::size_type size_type;

	bool operator == (const TVector& rhs) const
	{
		if ( GetCount() != rhs.GetCount() )
			return false;

		const_iterator ti = begin();
		const_iterator ri = rhs.begin();
		while ( ti != end() )
		{
			if ( *ti != *ri )
				return false;

			ti++;
			ri++;
		}

		return true;
	}

	bool operator != (const TVector& rhs) const
	{
		if ( GetCount() != rhs.GetCount() )
			return true;

		const_iterator ti = begin();
		const_iterator ri = rhs.begin();
		while ( ti != end() )
		{
			if ( *ti != *ri )
				return true;

			ti++;
			ri++;
		}

		return false;
	}

	size_t GetCount(void) const
	{
		return m_vector.size();
	}

	void SetCount(size_t _N, const T& _X = T())
	{
		m_vector.resize(_N, _X);
	}

	T&	operator[] (size_t nIndex)
	{
		return m_vector[nIndex];
	}

	const T&	operator[] (size_t nIndex) const
	{
		return m_vector[nIndex];
	}

	T&	GetAt (size_t nIndex)
	{
		return m_vector[nIndex];
	}

	const T&	GetAt (size_t nIndex) const
	{
		return m_vector[nIndex];
	}

	T&	ElementAt (size_t nIndex)
	{
		return m_vector[nIndex];
	}

	const T&	ElementAt (size_t nIndex) const
	{
		return m_vector[nIndex];
	}

	void	SetAt (size_t nIndex, const T& _X)
	{
		m_vector[nIndex] = _X;
	}

	iterator InsertAt(size_t nIndex, const T& _X)
	{
		return m_vector.insert(begin() + nIndex, _X);
	}

	iterator InsertAt(iterator _P, const T& _X)
	{
		return m_vector.insert(_P, _X);
	}

	iterator Add(const T& _X)
	{
		return m_vector.insert(end(), _X);
	}

	iterator RemoveAt(size_t nIndex, int nCount=1)
	{
		return m_vector.erase(m_vector.begin() + nIndex, m_vector.begin() + nIndex + nCount);
	}

	iterator RemoveAt(iterator _P)
	{
		return m_vector.erase(_P);
	}

	void RemoveHead()
	{
		RemoveAt(0);
	}

	void RemoveTail()
	{
		RemoveAt(GetCount()-1);
	}

	void Remove(const T& _V)
	{
		size_t nIndex = Find( _V );
		if ( nIndex != -1 )
		{
			m_vector.erase(m_vector.begin() + nIndex);
		}
	}

	void RemoveAll(void)
	{
		m_vector.clear();
	}

	// iterators
	void	AddHead(T _X)
	{
		m_vector.insert(begin(), _X);
	}

	void	AddTail(T _X)
	{
		m_vector.push_back(_X);
	}

	T	GetHead(void)
	{
		return m_vector.front();
	}

	const T	GetHead(void) const
	{
		return m_vector.front();
	}

	T	GetTail(void)
	{
		return m_vector.back();
	}

	const T	GetTail(void) const
	{
		return m_vector.back();
	}

	int Find(const T& _V) const
	{
		size_t nIndex = 0;
		const_iterator _L = end();
		for (const_iterator _F = begin(); _F != _L; )
		{
			if (*_F == _V)
				return nIndex;

			++_F;
			nIndex++;
		}

		return -1;
	}

	void Reverse()
	{
		if ( GetCount() < 2 )
			return;

		size_t nSize_1 = GetCount()-1;
		size_t nSize_2 = GetCount()/2;
		T v;
		for ( size_t i=0 ; i<nSize_2 ; i++ )
		{
			v = m_vector[i];
			m_vector[i] = m_vector[nSize_1-i];
			m_vector[nSize_1-i] = v;
		}
	}

	iterator begin()
	{
		return m_vector.begin();
	}

	const_iterator begin() const
	{
		return m_vector.begin();
	}

	iterator end()
	{
		return m_vector.end();
	}

	const_iterator end() const
	{
		return m_vector.end();
	}

	reverse_iterator rbegin()
	{
		return m_vector.rbegin();
	}

	const_reverse_iterator rbegin() const
	{
		return m_vector.rbegin();
	}

	reverse_iterator rend()
	{
		return m_vector.rend();
	}

	const_reverse_iterator rend() const
	{
		return m_vector.rend();
	}
} ;


// template RVector : same as TVector except it has reference counting
template<class T, class _Ax = std::allocator<T> >
class RVector : public TVector<T, _Ax>
{
public:
	RVector() : TVector<T, _Ax>()
	{
	}

	RVector(const RVector<T>& rhs) : TVector<T, _Ax>()
	{
		m_vector = rhs.m_vector;

		int nSize = rhs.GetCount();
		for ( int i=0 ; i<nSize ; i++ )
		{
			T& _X = m_vector[i];
			(*_X).AddRef();
		}
	}

	~RVector()
	{
		RemoveAll();
	}

	const RVector<T>& operator = ( const RVector<T>& rhs )
	{
		RemoveAll();

		m_vector = rhs.m_vector;

		iterator _L = end();
		for (iterator _F = begin(); _F != _L; _F++ )
		{
			T& _X = *_F;
			(*_X).AddRef();
		}

		return *this;
	}

	void	SetAt (size_t nIndex, const T& _X)
	{
		m_vector[nIndex]->Release();
		(*_X).AddRef();
		m_vector[nIndex] = _X;
	}

	iterator InsertAt(size_t nIndex, const T& _X)
	{
		(*_X).AddRef();
		return m_vector.insert(begin() + nIndex, _X);
	}

	iterator InsertAt(iterator _P, const T& _X)
	{
		(*_X).AddRef();
		return m_vector.insert(_P, _X);
	}

	iterator RemoveAt(size_t nIndex)
	{
		ASSERT ( 0 <= nIndex && nIndex < (int)GetCount() );

		T& _X = m_vector[nIndex];
		(*_X).Release();

		return m_vector.erase(m_vector.begin() + nIndex, m_vector.begin() + nIndex + 1);
	}

	iterator RemoveAt(size_t nIndex, int nCount)
	{
		int nMax = min((int)GetCount(), nIndex+nCount);
		for ( int i=nIndex ; i<nMax ; i++ )
		{
			T& _X = m_vector[i];
			(*_X).Release();
		}

		return m_vector.erase(m_vector.begin() + nIndex, m_vector.begin() + nIndex + nCount);
	}

	iterator RemoveAt(iterator _P)
	{
		T& _X = *_P;
		(*_X).Release();
		return m_vector.erase(_P);
	}

	void RemoveHead()
	{
		RemoveAt(0);
	}

	void RemoveTail()
	{
		RemoveAt(GetCount()-1);
	}

	int Remove(const T& _V)
	{
		size_t nIndex = Find( _V );
		if ( nIndex != -1 )
		{
			T& _X = m_vector[nIndex];
			(*_X).Release();
			m_vector.erase(m_vector.begin() + nIndex);
		}

		return nIndex;
	}

	void RemoveAll(void)
	{
		int nSize = GetCount();
		for ( int i=0 ; i<nSize ; i++ )
		{
			T& _X = m_vector[i];
			(*_X).Release();
		}

		m_vector.clear();
	}

	// iterators
	void	AddHead(const T& _X)
	{
		(*_X).AddRef();
		m_vector.insert(begin(), _X);
	}

	void	AddTail(const T& _X)
	{
		(*_X).AddRef();
		m_vector.push_back(_X);
	}
} ;

#endif	//__TVector_H__
