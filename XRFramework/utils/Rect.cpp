#include "stdafxf.h"
#include "Rect.h"
#define xMin(a, b) (a > b) ? b : a
#define xMax(a, b) (a > b) ? a : b

XRect::XRect(const XPoint& point1, const XPoint& point2)
{
	if (point2.m_x > point1.m_x) {
		x = point2.m_x; x2 = point1.m_x;
	} else {
		x = point1.m_x; x2 = point2.m_x;
	}
	if (point2.m_y > point1.m_y) {
		y = point2.m_y; y2 = point1.m_y;
	} else {
		y = point1.m_y; y2 = point2.m_y;
	}
}

XRect& XRect::operator+=(const XRect& rect)
{
	*this = *this + rect;
	return *this;
}


XRect& XRect::operator*=(const XRect& rect)
{
	*this = *this * rect;
	return *this;
}


XRect operator+(const XRect& r1, const XRect& r2)
{
	float x1 = xMin(r1.x, r2.x);
	float y1 = xMin(r1.y, r2.y);
	float y2 = xMax(r1.y + r1.y2, r2.y2 + r2.y);
	float x2 = xMax(r1.x + r1.x2, r2.x2 + r2.x);
	return XRect(x1, y1, x2 - x1, y2 - y1);
}

XRect operator*(const XRect& r1, const XRect& r2)
{
	float x1 = xMax(r1.x, r2.x);
	float y1 = xMax(r1.y, r2.y);
	float y2 = xMin(r1.y + r1.y2, r2.y2 + r2.y);
	float x2 = xMin(r1.x + r1.x2, r2.x2 + r2.x);
	return XRect(x1, y1, x2 - x1, y2 - y1);
}