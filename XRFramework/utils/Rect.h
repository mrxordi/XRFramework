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
	XRect(float xx, float yy, float xx2, float yy2)
//		: x(xx), y(yy), x2(ww), y2(hh)
	{
		if (yy2 < yy) { y = yy2; y2 = yy; } else { y = yy; y2 = yy2; }
		if (xx2 < xx) { x = xx2; x2 = xx; } else { x = xx; x2 = xx2; }
	}
	XRect(const XPoint& topLeft, const XPoint& bottomRight);
	XRect(const XPoint& pt, const XSize& size)
		: x(pt.m_x), y(pt.m_y), x2(pt.m_x+size.m_w), y2(pt.m_y+size.m_h)
	{ }
	XRect(const XSize& size)
		: x(0), y(0), x2(size.m_w), y2(size.m_h)
	{ }

	float GetWidth() const { return x2 - x; }
	void SetWidth(float w) { x2 = x+w; }

	float GetHeight() const { return y2 - y; }
	void SetHeight(float h) { y2 = y+h; }

	XPoint GetPosition() const { return XPoint(x, y); }
	void SetPosition(const XPoint &p) {
		float w = GetWidth(), h = GetHeight();
		x = p.m_x;
		y = p.m_y;
		x2 = x + w;
		y2 = x + h;
	}
	void SetPosition(float xx, float yy) {
		float w = GetWidth(), h = GetHeight();
		x = xx;
		y = yy;
		x2 = x + w;
		y2 = x + h;
	}

	XSize GetSize() const { return XSize(GetWidth(), GetHeight()); }
	void SetSize(const XSize &s) { SetWidth(s.m_w); SetHeight(s.m_h); }

	bool IsEmpty() const { return ((x == x2) || (y == y2)); }

	float GetLeft()   const { return x; }
	float GetTop()    const { return y; }
	float GetBottom() const { return y2; }
	float GetRight()  const { return x2; }

	void SetLeft(float left) { if (left > x2) { x2 = x; } x = left; }
	void SetRight(float right) { if (right < x) { x = x2; } x2 = right; }
	void SetTop(float top) { if (top > y2) { y2 = y; } y = top; }
	void SetBottom(float bottom) { if (bottom < y) { y = y2; } y2 = bottom; }

	XPoint GetTopLeft() const { return GetPosition(); }
	void SetTopLeft(const XPoint &p) { SetPosition(p); }

	XPoint GetBottomRight() const { return XPoint(x2, y2); }
	void SetBottomRight(const XPoint &p) { SetRight(p.m_x); SetBottom(p.m_y); }

	XPoint GetTopRight() const { return XPoint(GetTop(), GetRight()); }
	void SetTopRight(const XPoint &p) { SetRight(p.m_x); SetTop(p.m_y); }

	XPoint GetBottomLeft() const { return XPoint(GetLeft(), GetBottom()); }
	void SetBottomLeft(const XPoint &p) { SetLeft(p.m_x); SetBottom(p.m_y); }

	// operations with rect
/*
	XRect& Inflate(float dx, float dy);
	XRect& Inflate(const XSize& d) { return Inflate(d.m_w, d.m_h); }
	XRect& Inflate(float d) { return Inflate(d, d); }
	XRect Inflate(float dx, float dy) const
	{
		XRect r = *this;
		r.Inflate(dx, dy);
		return r;
	}
*/

/*
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
	bool Intersects(const XRect& rect) const;*/

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
