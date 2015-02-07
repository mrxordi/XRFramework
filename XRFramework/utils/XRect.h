#pragma once
#include <windows.h>
#include "wx/gdicmn.h"

/////////////////////////////////////////////////////////////////////////////
// Classes declared in this file

class XSize;
class XRect;
class XPoint;

/////////////////////////////////////////////////////////////////////////////
// CSize - An extent, similar to Windows SIZE structure.

class XSize :
	public tagSIZE
{
public:

	// Constructors
	// construct an uninitialized size
	XSize() throw();
	// create from two integers
	XSize(
		_In_ int initCX,
		_In_ int initCY) throw();
	// create from another size
	XSize(_In_ SIZE initSize) throw();
	// create from a point
	XSize(_In_ POINT initPt) throw();
	// create from a DWORD: cx = LOWORD(dw) cy = HIWORD(dw)
	XSize(_In_ DWORD dwSize) throw();

	// Operations
	BOOL operator==(_In_ SIZE size) const throw();
	BOOL operator!=(_In_ SIZE size) const throw();
	void operator+=(_In_ SIZE size) throw();
	void operator-=(_In_ SIZE size) throw();
	void SetSize(_In_ int CX, _In_ int CY) throw();

	// Operators returning CSize values
	XSize operator+(_In_ SIZE size) const throw();
	XSize operator-(_In_ SIZE size) const throw();
	XSize operator-() const throw();

	// Operators returning CPoint values
	XPoint operator+(_In_ POINT point) const throw();
	XPoint operator-(_In_ POINT point) const throw();

	// Operators returning CRect values
	XRect operator+(_In_ const RECT* lpRect) const throw();
	XRect operator-(_In_ const RECT* lpRect) const throw();
	operator wxSize() const throw() {
		return wxSize(cx, cy);
	}
};

/////////////////////////////////////////////////////////////////////////////
// CPoint - A 2-D point, similar to Windows POINT structure.

class XPoint :
	public tagPOINT
{
public:
	// Constructors

	// create an uninitialized point
	XPoint() throw();
	// create from two integers
	XPoint(
		_In_ int initX,
		_In_ int initY) throw();
	// create from another point
	XPoint(_In_ POINT initPt) throw();
	// create from a size
	XPoint(_In_ SIZE initSize) throw();
	// create from an LPARAM: x = LOWORD(dw) y = HIWORD(dw)
	XPoint(_In_ LPARAM dwPoint) throw();


	// Operations

	// translate the point
	void Offset(
		_In_ int xOffset,
		_In_ int yOffset) throw();
	void Offset(_In_ POINT point) throw();
	void Offset(_In_ SIZE size) throw();
	void SetPoint(
		_In_ int X,
		_In_ int Y) throw();

	BOOL operator==(_In_ POINT point) const throw();
	BOOL operator!=(_In_ POINT point) const throw();
	void operator+=(_In_ SIZE size) throw();
	void operator-=(_In_ SIZE size) throw();
	void operator+=(_In_ POINT point) throw();
	void operator-=(_In_ POINT point) throw();

	// Operators returning CPoint values
	XPoint operator+(_In_ SIZE size) const throw();
	XPoint operator-(_In_ SIZE size) const throw();
	XPoint operator-() const throw();
	XPoint operator+(_In_ POINT point) const throw();

	// Operators returning CSize values
	XSize operator-(_In_ POINT point) const throw();

	// Operators returning CRect values
	XRect operator+(_In_ const RECT* lpRect) const throw();
	XRect operator-(_In_ const RECT* lpRect) const throw();

	operator wxPoint() const throw();
};

class XRect :
	public tagRECT
{
	// Constructors
public:
	// uninitialized rectangle
	XRect() throw();
	// from left, top, right, and bottom
	XRect(
		_In_ int l,
		_In_ int t,
		_In_ int r,
		_In_ int b) throw();
	// copy constructor
	XRect(_In_ const RECT& srcRect) throw();

	// from a pointer to another rect
	XRect(_In_ LPCRECT lpSrcRect) throw();
	// from a point and size
	XRect(
		_In_ POINT point,
		_In_ SIZE size) throw();
	// from two points
	XRect(
		_In_ POINT topLeft,
		_In_ POINT bottomRight) throw();

	// Attributes (in addition to RECT members)

	// retrieves the width
	int Width() const throw();
	// returns the height
	int Height() const throw();
	// returns the size
	XSize Size() const throw();
	// reference to the top-left point
	XPoint& TopLeft() throw();
	// reference to the bottom-right point
	XPoint& BottomRight() throw();
	// const reference to the top-left point
	const XPoint& TopLeft() const throw();
	// const reference to the bottom-right point
	const XPoint& BottomRight() const throw();
	// the geometric center point of the rectangle
	XPoint CenterPoint() const throw();
	// swap the left and right
	void SwapLeftRight() throw();
	static void WINAPI SwapLeftRight(_Inout_ LPRECT lpRect) throw();

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT() throw();
	operator LPCRECT() const throw();

	// returns TRUE if rectangle has no area
	BOOL IsRectEmpty() const throw();
	// returns TRUE if rectangle is at (0,0) and has no area
	BOOL IsRectNull() const throw();
	// returns TRUE if point is within rectangle
	BOOL PtInRect(_In_ POINT point) const throw();

	// Operations

	// set rectangle from left, top, right, and bottom
	void SetRect(
		_In_ int x1,
		_In_ int y1,
		_In_ int x2,
		_In_ int y2) throw();
	void SetRect(
		_In_ POINT topLeft,
		_In_ POINT bottomRight) throw();
	// empty the rectangle
	void SetRectEmpty() throw();
	// copy from another rectangle
	void CopyRect(_In_ LPCRECT lpSrcRect) throw();
	// TRUE if exactly the same as another rectangle
	BOOL EqualRect(_In_ LPCRECT lpRect) const throw();

	// Inflate rectangle's width and height by
	// x units to the left and right ends of the rectangle
	// and y units to the top and bottom.
	void InflateRect(
		_In_ int x,
		_In_ int y) throw();
	// Inflate rectangle's width and height by
	// size.cx units to the left and right ends of the rectangle
	// and size.cy units to the top and bottom.
	void InflateRect(_In_ SIZE size) throw();
	// Inflate rectangle's width and height by moving individual sides.
	// Left side is moved to the left, right side is moved to the right,
	// top is moved up and bottom is moved down.
	void InflateRect(_In_ LPCRECT lpRect) throw();
	void InflateRect(
		_In_ int l,
		_In_ int t,
		_In_ int r,
		_In_ int b) throw();

	// deflate the rectangle's width and height without
	// moving its top or left
	void DeflateRect(
		_In_ int x,
		_In_ int y) throw();
	void DeflateRect(_In_ SIZE size) throw();
	void DeflateRect(_In_ LPCRECT lpRect) throw();
	void DeflateRect(
		_In_ int l,
		_In_ int t,
		_In_ int r,
		_In_ int b) throw();

	// translate the rectangle by moving its top and left
	void OffsetRect(
		_In_ int x,
		_In_ int y) throw();
	void OffsetRect(_In_ SIZE size) throw();
	void OffsetRect(_In_ POINT point) throw();
	void NormalizeRect() throw();

	// absolute position of rectangle
	void MoveToY(_In_ int y) throw();
	void MoveToX(_In_ int x) throw();
	void MoveToXY(
		_In_ int x,
		_In_ int y) throw();
	void MoveToXY(_In_ POINT point) throw();

	// set this rectangle to intersection of two others
	BOOL IntersectRect(
		_In_ LPCRECT lpRect1,
		_In_ LPCRECT lpRect2) throw();

	// set this rectangle to bounding union of two others
	BOOL UnionRect(
		_In_ LPCRECT lpRect1,
		_In_ LPCRECT lpRect2) throw();

	// set this rectangle to minimum of two others
	BOOL SubtractRect(
		_In_ LPCRECT lpRectSrc1,
		_In_ LPCRECT lpRectSrc2) throw();

	// Additional Operations
	void operator=(_In_ const RECT& srcRect) throw();
	BOOL operator==(_In_ const RECT& rect) const throw();
	BOOL operator!=(_In_ const RECT& rect) const throw();
	void operator+=(_In_ POINT point) throw();
	void operator+=(_In_ SIZE size) throw();
	void operator+=(_In_ LPCRECT lpRect) throw();
	void operator-=(_In_ POINT point) throw();
	void operator-=(_In_ SIZE size) throw();
	void operator-=(_In_ LPCRECT lpRect) throw();
	void operator&=(_In_ const RECT& rect) throw();
	void operator|=(_In_ const RECT& rect) throw();

	// Operators returning CRect values
	XRect operator+(_In_ POINT point) const throw();
	XRect operator-(_In_ POINT point) const throw();
	XRect operator+(_In_ LPCRECT lpRect) const throw();
	XRect operator+(_In_ SIZE size) const throw();
	XRect operator-(_In_ SIZE size) const throw();
	XRect operator-(_In_ LPCRECT lpRect) const throw();
	XRect operator&(_In_ const RECT& rect2) const throw();
	XRect operator|(_In_ const RECT& rect2) const throw();
	XRect MulDiv(
		_In_ int nMultiplier,
		_In_ int nDivisor) const throw();
};

// CRect
inline XRect::XRect() throw()
{
	left = 0;
	top = 0;
	right = 0;
	bottom = 0;
}

inline XRect::XRect(
	_In_ int l,
	_In_ int t,
	_In_ int r,
	_In_ int b) throw()
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline XRect::XRect(_In_ const RECT& srcRect) throw()
{
	::CopyRect(this, &srcRect);
}

inline XRect::XRect(_In_ LPCRECT lpSrcRect) throw()
{
	::CopyRect(this, lpSrcRect);
}

inline XRect::XRect(
	_In_ POINT point,
	_In_ SIZE size) throw()
{
	right = (left = point.x) + size.cx;
	bottom = (top = point.y) + size.cy;
}

inline XRect::XRect(
	_In_ POINT topLeft,
	_In_ POINT bottomRight) throw()
{
	left = topLeft.x;
	top = topLeft.y;
	right = bottomRight.x;
	bottom = bottomRight.y;
}

inline int XRect::Width() const throw()
{
	return right - left;
}

inline int XRect::Height() const throw()
{
	return bottom - top;
}

inline XSize XRect::Size() const throw()
{
	return XSize(right - left, bottom - top);
}

inline XPoint& XRect::TopLeft() throw()
{
	return *((XPoint*)this);
}

inline XPoint& XRect::BottomRight() throw()
{
	return *((XPoint*)this + 1);
}

inline const XPoint& XRect::TopLeft() const throw()
{
	return *((XPoint*)this);
}

inline const XPoint& XRect::BottomRight() const throw()
{
	return *((XPoint*)this + 1);
}

inline XPoint XRect::CenterPoint() const throw()
{
	return XPoint((left + right) / 2, (top + bottom) / 2);
}

inline void XRect::SwapLeftRight() throw()
{
	SwapLeftRight(LPRECT(this));
}

inline void WINAPI XRect::SwapLeftRight(_Inout_ LPRECT lpRect) throw()
{
	LONG temp = lpRect->left;
	lpRect->left = lpRect->right;
	lpRect->right = temp;
}

inline XRect::operator LPRECT() throw()
{
	return this;
}

inline XRect::operator LPCRECT() const throw()
{
	return this;
}

inline BOOL XRect::IsRectEmpty() const throw()
{
	return ::IsRectEmpty(this);
}

inline BOOL XRect::IsRectNull() const throw()
{
	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}

inline BOOL XRect::PtInRect(_In_ POINT point) const throw()
{
	return ::PtInRect(this, point);
}

inline void XRect::SetRect(
	_In_ int x1,
	_In_ int y1,
	_In_ int x2,
	_In_ int y2) throw()
{
	::SetRect(this, x1, y1, x2, y2);
}

inline void XRect::SetRect(
	_In_ POINT topLeft,
	_In_ POINT bottomRight) throw()
{
	::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
}

inline void XRect::SetRectEmpty() throw()
{
	::SetRectEmpty(this);
}

inline void XRect::CopyRect(_In_ LPCRECT lpSrcRect) throw()
{
	::CopyRect(this, lpSrcRect);
}

inline BOOL XRect::EqualRect(_In_ LPCRECT lpRect) const throw()
{
	return ::EqualRect(this, lpRect);
}

inline void XRect::InflateRect(
	_In_ int x,
	_In_ int y) throw()
{
	::InflateRect(this, x, y);
}

inline void XRect::InflateRect(_In_ SIZE size) throw()
{
	::InflateRect(this, size.cx, size.cy);
}

inline void XRect::DeflateRect(
	_In_ int x,
	_In_ int y) throw()
{
	::InflateRect(this, -x, -y);
}

inline void XRect::DeflateRect(_In_ SIZE size) throw()
{
	::InflateRect(this, -size.cx, -size.cy);
}

inline void XRect::OffsetRect(
	_In_ int x,
	_In_ int y) throw()
{
	::OffsetRect(this, x, y);
}

inline void XRect::OffsetRect(_In_ POINT point) throw()
{
	::OffsetRect(this, point.x, point.y);
}

inline void XRect::OffsetRect(_In_ SIZE size) throw()
{
	::OffsetRect(this, size.cx, size.cy);
}

inline void XRect::MoveToY(_In_ int y) throw()
{
	bottom = Height() + y;
	top = y;
}

inline void XRect::MoveToX(_In_ int x) throw()
{
	right = Width() + x;
	left = x;
}

inline void XRect::MoveToXY(
	_In_ int x,
	_In_ int y) throw()
{
	MoveToX(x);
	MoveToY(y);
}

inline void XRect::MoveToXY(_In_ POINT pt) throw()
{
	MoveToX(pt.x);
	MoveToY(pt.y);
}

inline BOOL XRect::IntersectRect(
	_In_ LPCRECT lpRect1,
	_In_ LPCRECT lpRect2) throw()
{
	return ::IntersectRect(this, lpRect1, lpRect2);
}

inline BOOL XRect::UnionRect(
	_In_ LPCRECT lpRect1,
	_In_ LPCRECT lpRect2) throw()
{
	return ::UnionRect(this, lpRect1, lpRect2);
}

inline void XRect::operator=(_In_ const RECT& srcRect) throw()
{
	::CopyRect(this, &srcRect);
}

inline BOOL XRect::operator==(_In_ const RECT& rect) const throw()
{
	return ::EqualRect(this, &rect);
}

inline BOOL XRect::operator!=(_In_ const RECT& rect) const throw()
{
	return !::EqualRect(this, &rect);
}

inline void XRect::operator+=(_In_ POINT point) throw()
{
	::OffsetRect(this, point.x, point.y);
}

inline void XRect::operator+=(_In_ SIZE size) throw()
{
	::OffsetRect(this, size.cx, size.cy);
}

inline void XRect::operator+=(_In_ LPCRECT lpRect) throw()
{
	InflateRect(lpRect);
}

inline void XRect::operator-=(_In_ POINT point) throw()
{
	::OffsetRect(this, -point.x, -point.y);
}

inline void XRect::operator-=(_In_ SIZE size) throw()
{
	::OffsetRect(this, -size.cx, -size.cy);
}

inline void XRect::operator-=(_In_ LPCRECT lpRect) throw()
{
	DeflateRect(lpRect);
}

inline void XRect::operator&=(_In_ const RECT& rect) throw()
{
	::IntersectRect(this, this, &rect);
}

inline void XRect::operator|=(_In_ const RECT& rect) throw()
{
	::UnionRect(this, this, &rect);
}

inline XRect XRect::operator+(_In_ POINT pt) const throw()
{
	XRect rect(*this);
	::OffsetRect(&rect, pt.x, pt.y);
	return rect;
}

inline XRect XRect::operator-(_In_ POINT pt) const throw()
{
	XRect rect(*this);
	::OffsetRect(&rect, -pt.x, -pt.y);
	return rect;
}

inline XRect XRect::operator+(_In_ SIZE size) const throw()
{
	XRect rect(*this);
	::OffsetRect(&rect, size.cx, size.cy);
	return rect;
}

inline XRect XRect::operator-(_In_ SIZE size) const throw()
{
	XRect rect(*this);
	::OffsetRect(&rect, -size.cx, -size.cy);
	return rect;
}

inline XRect XRect::operator+(_In_ LPCRECT lpRect) const throw()
{
	XRect rect(this);
	rect.InflateRect(lpRect);
	return rect;
}

inline XRect XRect::operator-(_In_ LPCRECT lpRect) const throw()
{
	XRect rect(this);
	rect.DeflateRect(lpRect);
	return rect;
}

inline XRect XRect::operator&(_In_ const RECT& rect2) const throw()
{
	XRect rect;
	::IntersectRect(&rect, this, &rect2);
	return rect;
}

inline XRect XRect::operator|(_In_ const RECT& rect2) const throw()
{
	XRect rect;
	::UnionRect(&rect, this, &rect2);
	return rect;
}

inline BOOL XRect::SubtractRect(
	_In_ LPCRECT lpRectSrc1,
	_In_ LPCRECT lpRectSrc2) throw()
{
	return ::SubtractRect(this, lpRectSrc1, lpRectSrc2);
}

inline void XRect::NormalizeRect() throw()
{
	int nTemp;
	if (left > right)
	{
		nTemp = left;
		left = right;
		right = nTemp;
	}
	if (top > bottom)
	{
		nTemp = top;
		top = bottom;
		bottom = nTemp;
	}
}

inline void XRect::InflateRect(_In_ LPCRECT lpRect) throw()
{
	left -= lpRect->left;
	top -= lpRect->top;
	right += lpRect->right;
	bottom += lpRect->bottom;
}

inline void XRect::InflateRect(
	_In_ int l,
	_In_ int t,
	_In_ int r,
	_In_ int b) throw()
{
	left -= l;
	top -= t;
	right += r;
	bottom += b;
}

inline void XRect::DeflateRect(_In_ LPCRECT lpRect) throw()
{
	left += lpRect->left;
	top += lpRect->top;
	right -= lpRect->right;
	bottom -= lpRect->bottom;
}

inline void XRect::DeflateRect(
	_In_ int l,
	_In_ int t,
	_In_ int r,
	_In_ int b) throw()
{
	left += l;
	top += t;
	right -= r;
	bottom -= b;
}

inline XRect XRect::MulDiv(
	_In_ int nMultiplier,
	_In_ int nDivisor) const throw()
{
	return XRect(
		::MulDiv(left, nMultiplier, nDivisor),
		::MulDiv(top, nMultiplier, nDivisor),
		::MulDiv(right, nMultiplier, nDivisor),
		::MulDiv(bottom, nMultiplier, nDivisor));
}

// CSize
inline XSize::XSize() throw()
{
	cx = 0;
	cy = 0;
}

inline XSize::XSize(
	_In_ int initCX,
	_In_ int initCY) throw()
{
	cx = initCX;
	cy = initCY;
}

inline XSize::XSize(_In_ SIZE initSize) throw()
{
	*(SIZE*)this = initSize;
}

inline XSize::XSize(_In_ POINT initPt) throw()
{
	*(POINT*)this = initPt;
}

inline XSize::XSize(_In_ DWORD dwSize) throw()
{
	cx = (short)LOWORD(dwSize);
	cy = (short)HIWORD(dwSize);
}

inline BOOL XSize::operator==(_In_ SIZE size) const throw()
{
	return (cx == size.cx && cy == size.cy);
}

inline BOOL XSize::operator!=(_In_ SIZE size) const throw()
{
	return (cx != size.cx || cy != size.cy);
}

inline void XSize::operator+=(_In_ SIZE size) throw()
{
	cx += size.cx;
	cy += size.cy;
}

inline void XSize::operator-=(_In_ SIZE size) throw()
{
	cx -= size.cx;
	cy -= size.cy;
}

inline void XSize::SetSize(
	_In_ int CX,
	_In_ int CY) throw()
{
	cx = CX;
	cy = CY;
}

inline XSize XSize::operator+(_In_ SIZE size) const throw()
{
	return XSize(cx + size.cx, cy + size.cy);
}

inline XSize XSize::operator-(_In_ SIZE size) const throw()
{
	return XSize(cx - size.cx, cy - size.cy);
}

inline XSize XSize::operator-() const throw()
{
	return XSize(-cx, -cy);
}

inline XPoint XSize::operator+(_In_ POINT point) const throw()
{
	return XPoint(cx + point.x, cy + point.y);
}

inline XPoint XSize::operator-(_In_ POINT point) const throw()
{
	return XPoint(cx - point.x, cy - point.y);
}

inline XRect XSize::operator+(_In_ const RECT* lpRect) const throw()
{
	return XRect(lpRect) + *this;
}

inline XRect XSize::operator-(_In_ const RECT* lpRect) const throw()
{
	return XRect(lpRect) - *this;
}

// CPoint
inline XPoint::XPoint() throw()
{
	x = 0;
	y = 0;
}

inline XPoint::XPoint(
	_In_ int initX,
	_In_ int initY) throw()
{
	x = initX;
	y = initY;
}

inline XPoint::XPoint(_In_ POINT initPt) throw()
{
	*(POINT*)this = initPt;
}

inline XPoint::XPoint(_In_ SIZE initSize) throw()
{
	*(SIZE*)this = initSize;
}

inline XPoint::XPoint(_In_ LPARAM dwPoint) throw()
{
	x = (short)LOWORD(dwPoint);
	y = (short)HIWORD(dwPoint);
}

inline void XPoint::Offset(
	_In_ int xOffset,
	_In_ int yOffset) throw()
{
	x += xOffset;
	y += yOffset;
}

inline void XPoint::Offset(_In_ POINT point) throw()
{
	x += point.x;
	y += point.y;
}

inline void XPoint::Offset(_In_ SIZE size) throw()
{
	x += size.cx;
	y += size.cy;
}

inline void XPoint::SetPoint(
	_In_ int X,
	_In_ int Y) throw()
{
	x = X;
	y = Y;
}

inline BOOL XPoint::operator==(_In_ POINT point) const throw()
{
	return (x == point.x && y == point.y);
}

inline BOOL XPoint::operator!=(_In_ POINT point) const throw()
{
	return (x != point.x || y != point.y);
}

inline void XPoint::operator+=(_In_ SIZE size) throw()
{
	x += size.cx;
	y += size.cy;
}

inline void XPoint::operator-=(_In_ SIZE size) throw()
{
	x -= size.cx;
	y -= size.cy;
}

inline void XPoint::operator+=(_In_ POINT point) throw()
{
	x += point.x;
	y += point.y;
}

inline void XPoint::operator-=(_In_ POINT point) throw()
{
	x -= point.x;
	y -= point.y;
}

inline XPoint XPoint::operator+(_In_ SIZE size) const throw()
{
	return XPoint(x + size.cx, y + size.cy);
}

inline XPoint XPoint::operator-(_In_ SIZE size) const throw()
{
	return XPoint(x - size.cx, y - size.cy);
}

inline XPoint XPoint::operator-() const throw()
{
	return XPoint(-x, -y);
}

inline XPoint XPoint::operator+(_In_ POINT point) const throw()
{
	return XPoint(x + point.x, y + point.y);
}

inline XSize XPoint::operator-(_In_ POINT point) const throw()
{
	return XSize(x - point.x, y - point.y);
}

inline XRect XPoint::operator+(_In_ const RECT* lpRect) const throw()
{
	return XRect(lpRect) + *this;
}

inline XRect XPoint::operator-(_In_ const RECT* lpRect) const throw()
{
	return XRect(lpRect) - *this;
}

inline XPoint::operator wxPoint() const throw()
{
	return wxPoint(x, y);
}