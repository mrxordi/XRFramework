#include "stdafxf.h"
#include "Rect.h"
#define xMin(a, b) (a > b) ? b : a
#define xMax(a, b) (a > b) ? a : b

XRect::XRect(const XPoint& point1, const XPoint& point2)
{
	x = point1.m_x;
	y = point1.m_y;
	width = point2.m_x - point1.m_x;
	height = point2.m_y - point1.m_y;

	if (width < 0)
	{
		width = -width;
		x = point2.m_x;
	}
	width++;

	if (height < 0)
	{
		height = -height;
		y = point2.m_y;
	}
	height++;
}

XRect& XRect::Union(const XRect& rect)
{
	// ignore empty rectangles: union with an empty rectangle shouldn't extend
	// this one to (0, 0)
	if (!width || !height)
	{
		*this = rect;
	}
	else if (rect.width && rect.height)
	{
		float x1 = xMin(x, rect.x);
		float y1 = xMin(y, rect.y);
		float y2 = xMax(y + height, rect.height + rect.y);
		float x2 = xMax(x + width, rect.width + rect.x);

		x = x1;
		y = y1;
		width = x2 - x1;
		height = y2 - y1;
	}
	//else: we're not empty and rect is empty

	return *this;
}

XRect& XRect::Inflate(float dx, float dy)
{
	if (-2 * dx > width)
	{
		// Don't allow deflate to eat more width than we have,
		// a well-defined rectangle cannot have negative width.
		x += width / 2;
		width = 0;
	}
	else
	{
		// The inflate is valid.
		x -= dx;
		width += 2 * dx;
	}

	if (-2 * dy > height)
	{
		// Don't allow deflate to eat more height than we have,
		// a well-defined rectangle cannot have negative height.
		y += height / 2;
		height = 0;
	}
	else
	{
		// The inflate is valid.
		y -= dy;
		height += 2 * dy;
	}

	return *this;
}

bool XRect::Contains(float cx, float cy) const
{
	return ((cx >= x) && (cy >= y)
		&& ((cy - y) < height)
		&& ((cx - x) < width)
		);
}

bool XRect::Contains(const XRect& rect) const
{
	return Contains(rect.GetTopLeft()) && Contains(rect.GetBottomRight());
}

XRect& XRect::Intersect(const XRect& rect)
{
	float x2 = GetRight(),
		y2 = GetBottom();

	if (x < rect.x)
		x = rect.x;
	if (y < rect.y)
		y = rect.y;
	if (x2 > rect.GetRight())
		x2 = rect.GetRight();
	if (y2 > rect.GetBottom())
		y2 = rect.GetBottom();

	width = x2 - x + 1;
	height = y2 - y + 1;

	if (width <= 0 || height <= 0)
	{
		width =
			height = 0;
	}

	return *this;
}

bool XRect::Intersects(const XRect& rect) const
{
	XRect r = Intersect(rect);

	// if there is no intersection, both width and height are 0
	return r.width != 0;
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
	float y2 = xMax(r1.y + r1.height, r2.height + r2.y);
	float x2 = xMax(r1.x + r1.width, r2.width + r2.x);
	return XRect(x1, y1, x2 - x1, y2 - y1);
}

XRect operator*(const XRect& r1, const XRect& r2)
{
	float x1 = xMax(r1.x, r2.x);
	float y1 = xMax(r1.y, r2.y);
	float y2 = xMin(r1.y + r1.height, r2.height + r2.y);
	float x2 = xMin(r1.x + r1.width, r2.width + r2.x);
	return XRect(x1, y1, x2 - x1, y2 - y1);
}