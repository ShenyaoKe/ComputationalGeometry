#include "ConvexHull.h"

ConvexHull::ConvexHull(std::vector<Vector2f> &points)
	: mPoints(std::move(points))
	, mHullIndices(mPoints.size(), 0)
{
	if (!mPoints.empty())
	{

		preprocess();
	}
}

ConvexHull::~ConvexHull()
{
}

void ConvexHull::extractLineSegmentIndices(std::vector<uint32_t> &indices) const
{
	if (mStackTop == 0)
	{
		indices.clear();
		return;
	}

	indices.resize(mStackTop + 1);
	for (size_t i = 0; i <= mStackTop; i++)
	{
		indices[i] = static_cast<uint32_t>(mHullIndices[i]);
	}
}

void ConvexHull::findBottomPoint()
{
	const Vector2f* minP = &mPoints.front();

	for (size_t i = 1; i < mPoints.size(); i++)
	{
		if ((mPoints[i].y == minP->y && mPoints[i].x < minP->x)
			|| (mPoints[i].y < minP->y))
		{
			minP = &mPoints[i];
			mHullIndices[0] = i;
		}
	}
}

void ConvexHull::initIndices()
{
	for (size_t i = 0, idx = 1; i < mPoints.size(); i++)
	{
		if (i != mHullIndices.front())
		{
			mHullIndices[idx++] = i;
		}
	}
	mStackTop = 1;
	mProcessPos = 2;
}

void ConvexHull::sort()
{
	std::sort(mHullIndices.begin() + 1, mHullIndices.end(),
			  [&](size_t i, size_t j) -> bool
	{
		const Vector2f v0 = mPoints[i] - mPoints[mHullIndices[0]];
		const Vector2f v1 = mPoints[j] - mPoints[mHullIndices[0]];
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
	});
}

void ConvexHull::preprocess()
{
	findBottomPoint();

	initIndices();

	sort();
}

void ConvexHull::process()
{
	while (advance())
	{
	}
}

bool ConvexHull::advance()
{
	if (mProcessPos >= mPoints.size())
	{
		return false;
	}

	if (toLeft(mPoints[mHullIndices[mStackTop - 1]],
			   mPoints[mHullIndices[mStackTop]],
			   mPoints[mHullIndices[mProcessPos]]))
	{
		mHullIndices[++mStackTop] = mHullIndices[mProcessPos++];
	}
	else
	{
		mStackTop--;
	}

	return true;
}