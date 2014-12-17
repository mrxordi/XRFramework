#pragma once

typedef class tagXPoint {
public:
	tagXPoint() : m_x(0), m_y(0){};
	tagXPoint(float x, float y) : m_x(x), m_y(y){};

	float m_x, m_y;
} XPoint, *pXPoint;

typedef class tagXSize {
public:
	tagXSize() : m_w(0), m_h(0){};
	tagXSize(float x, float y) : m_w(x), m_h(y){};

	float m_w, m_h;
} XSize, *pXSize;


class XRect
{
public:
	XRect()
		: x(0), y(0), x2(0), y2(0)
	{ }
	XRect(float xx, float yy, float ww, float hh)
		: x(xx), y(yy), x2(ww), y2(hh)
	{ }
	XRect(const XPoint& topLeft, const XPoint& bottomRight);
	XRect(const XPoint& pt, const XSize& size)
		: x(pt.m_x), y(pt.m_y), x2(size.m_w), y2(size.m_h)
	{ }
	XRect(const XSize& size)
		: x(0), y(0), x2(size.m_w), y2(size.m_h)
	{ }

	// default copy ctor and assignment operators ok

	float GetX() const { return x; }
	void SetX(float xx) { x = xx; }

	float GetY() const { return y; }
	void SetY(float yy) { y = yy; }

	float GetWidth() const { return x2; }
	void SetWidth(float w) { x2 = w; }

	float GetHeight() const { return y2; }
	void SetHeight(float h) { y2 = h; }

	XPoint GetPosition() const { XPoint a;  a.m_x = x, a.m_y = y; return a; }
	void SetPosition(const XPoint &p) { x = p.m_x; y = p.m_y; }

	XSize GetSize() const { return XSize(x2, y2); }
	void SetSize(const XSize &s) { x2 = s.m_w; y2 = s.m_h; }

	bool IsEmpty() const { return (x2 <= 0) || (y2 <= 0); }

	float GetLeft()   const { return x; }
	float GetTop()    const { return y; }
	float GetBottom() const { return y + y2 - 1; }
	float GetRight()  const { return x + x2 - 1; }

	void SetLeft(float left) { x = left; }
	void SetRight(float right) { x2 = right - x + 1; }
	void SetTop(float top) { y = top; }
	void SetBottom(float bottom) { y2 = bottom - y + 1; }

	XPoint GetTopLeft() const { return GetPosition(); }
	XPoint GetLeftTop() const { return GetTopLeft(); }
	void SetTopLeft(const XPoint &p) { SetPosition(p); }
	void SetLeftTop(const XPoint &p) { SetTopLeft(p); }

	XPoint GetBottomRight() const { return XPoint(GetRight(), GetBottom()); }
	XPoint GetRightBottom() const { return GetBottomRight(); }
	void SetBottomRight(const XPoint &p) { SetRight(x); SetBottom(y); }
	void SetRightBottom(const XPoint &p) { SetBottomRight(p); }

	XPoint GetTopRight() const { return XPoint(GetRight(), GetTop()); }
	XPoint GetRightTop() const { return GetTopRight(); }
	void SetTopRight(const XPoint &p) { SetRight(p.m_x); SetTop(p.m_y); }
	void SetRightTop(const XPoint &p) { SetTopRight(p); }

	XPoint GetBottomLeft() const { return XPoint(GetLeft(), GetBottom()); }
	XPoint GetLeftBottom() const { return GetBottomLeft(); }
	void SetBottomLeft(const XPoint &p) { SetLeft(x); SetBottom(y); }
	void SetLeftBottom(const XPoint &p) { SetBottomLeft(p); }

	// operations with rect
	XRect& Inflate(float dx, float dy);
	XRect& Inflate(const XSize& d) { return Inflate(d.m_w, d.m_h); }
	XRect& Inflate(float d) { return Inflate(d, d); }
	XRect Inflate(float dx, float dy) const
	{
		XRect r = *this;
		r.Inflate(dx, dy);
		return r;
	}

	XRect& Deflate(float dx, float dy) { return Inflate(-dx, -dy); }
	XRect& Deflate(const XSize& d) { return Inflate(-d.m_w, -d.m_h); }
	XRect& Deflate(float d) { return Inflate(-d); }
	XRect Deflate(float dx, float dy) const
	{
		XRect r = *this;
		r.Deflate(dx, dy);
		return r;
	}

	void Offset(float dx, float dy) { x += dx; y += dy; }
	void Offset(const XPoint& pt) { Offset(pt.m_x, pt.m_y); }

	XRect& Intersect(const XRect& rect);
	XRect Intersect(const XRect& rect) const
	{
		XRect r = *this;
		r.Intersect(rect);
		return r;
	}

	XRect& Union(const XRect& rect);
	XRect Union(const XRect& rect) const
	{
		XRect r = *this;
		r.Union(rect);
		return r;
	}

	// return true if the XPoint is (not strcitly) inside the rect
	bool Contains(float x, float y) const;
	bool Contains(const XPoint& pt) const { return Contains(pt.m_x, pt.m_y); }
	// return true if the rectangle 'rect' is (not strictly) inside this rect
	bool Contains(const XRect& rect) const;

	// return true if the rectangles have a non empty intersection
	bool Intersects(const XRect& rect) const;

	// like Union() but don't ignore empty rectangles
	XRect& operator+=(const XRect& rect);

	// intersections of two rectrangles not testing for empty rectangles
	XRect& operator*=(const XRect& rect);


public:
	float x, y, x2, y2;
};

// compare rectangles
inline bool operator==(const XRect& r1, const XRect& r2)
{
	return (r1.x == r2.x) && (r1.y == r2.y) &&
		(r1.x2 == r2.x2) && (r1.y2 == r2.y2);
}

inline bool operator!=(const XRect& r1, const XRect& r2)
{
	return !(r1 == r2);
}

// like Union() but don't treat empty rectangles specially
XRect operator+(const XRect& r1, const XRect& r2);

// intersections of two rectangles
XRect operator*(const XRect& r1, const XRect& r2);
