#pragma once

#include "Vector2.h"

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
	void findBottomPoint();
	void initIndices();
	void sort();
	void preprocess();

	bool toLeft(const Vector2f &vecOrigin,
				const Vector2f &vecTarget,
				const Vector2f &targetPoint) const
	{
		if (vecTarget.x == targetPoint.x && vecTarget.y == targetPoint.y)
		{
			return false;
		}
		return cross(vecTarget - vecOrigin, targetPoint - vecOrigin) >= 0;
	}
	
private:
	std::vector<Vector2f> mPoints;
	std::vector<size_t> mHullIndices;

	size_t mStackTop;
	size_t mProcessPos;
};