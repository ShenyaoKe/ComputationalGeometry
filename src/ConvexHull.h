#pragma once

#include "common.h"
#include "Vector2.h"

/*
static int ptCount = 0;
//static Vector2f* pts = nullptr;
static Vector2f* bottomPt = nullptr;*/

class ConvexHull
{
public:
	ConvexHull(std::vector<Vector2f> &points);
	~ConvexHull();

	void process();
	bool advance();
	void extractLineSegmentIndices(std::vector<uint32_t> &indices) const;
	const Vector2f* data() const { return mPoints.data(); }
private:
	void init();
	void findBottomPoint();
	void preprocess();

	bool toLeft(const Vector2f &vecOrigin,
				const Vector2f &vecTarget,
				const Vector2f &targetPoint)
	{
		const Vector2f v0 = vecTarget - vecOrigin;
		const Vector2f v1 = targetPoint - vecOrigin;
		float res = cross(v0, v1);
		if (res > 0)
		{
			return true;
		}
		else if (res < 0)
		{
			return false;
		}
		// Res == 0
		else
		{
			return v1.lengthSquared() > v0.lengthSquared();
		}
		/*else if (targetPoint.y > vecTarget.y)
		{
			return true;
		}
		else if (targetPoint.y == vecTarget.y)
		{
			std::cout << "horizontally same!\n";
			return abs(targetPoint.x - vecOrigin.x) >
				abs(vecTarget.x - vecOrigin.x);
		}
		else
		{
			return false;
		}*/
	}
	
private:
	std::vector<Vector2f> mPoints;
	std::vector<size_t> mHullIndices;
	//std::vector<size_t> mOutIndices;

	size_t mStackTop;
	size_t mProcessPos;
};
