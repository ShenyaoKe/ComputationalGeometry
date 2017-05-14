#include "ConvexHull.h"

ConvexHull::ConvexHull(std::vector<Vector2f> &points)
	: mPoints(std::move(points))
{
	//process();
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
	indices.resize(mStackTop << 1);

	for (size_t i = 0; i < mStackTop; i++)
	{
		indices[i << 1] = static_cast<uint32_t>(mHullIndices[i]);
		indices[(i << 1) + 1] = static_cast<uint32_t>(mHullIndices[i + 1]);
	}
	//*(indices.end() - 2) = static_cast<uint32_t>(mHullIndices.back());
	//*(indices.end() - 1) = static_cast<uint32_t>(mHullIndices.front());
}

void ConvexHull::init()
{
	size_t pointCount = mPoints.size();
	mHullIndices.resize(pointCount, 0);
	if (pointCount > 3)
	{
		mStackTop = 1;
		mProcessPos = 2;
	}
	else
	{
		mStackTop = mProcessPos = pointCount - 1;
	}
}

void ConvexHull::findBottomPoint()
{
	const Vector2f* minP = &mPoints[0];

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

void ConvexHull::preprocess()
{
	size_t pointCount = mPoints.size();
	
	init();
	findBottomPoint();
	size_t bottomId = mHullIndices[0];
	//mOutIndices.push_back(bottomId);

	for (size_t i = 0, idx = 1; i < pointCount; i++)
	{
		if (i != bottomId)
		{
			mHullIndices[idx++] = i;
		}
	}

	const Vector2f& bottomPt = mPoints[bottomId];
	std::sort(mHullIndices.begin() + 1,
			  mHullIndices.end(),
			  [&](size_t i, size_t j) {
				return !ConvexHull::toLeft(mPoints[i], bottomPt, mPoints[j]);
	});
}

void ConvexHull::process()
{
	//////////////////////////////////////////////////////////////////////////
	/* Finish pre-process                                                   */
	//////////////////////////////////////////////////////////////////////////
	/*auto stackTop = mHullIndices.begin() + 1;
	auto curPos = stackTop + 1;*/
	while (advance())
	{
	}
}

bool ConvexHull::advance()
{
	if (mProcessPos >= mHullIndices.size())
	{
		return false;
	}
	if (mStackTop == 0)
	{
		std::cout << "mStackTop == 0!\n";
		mHullIndices[++mStackTop] = mHullIndices[mProcessPos++];
	}

	//std::cout << "Prev StackTop: " << mStackTop;
	if (ConvexHull::toLeft(mPoints[mHullIndices[mStackTop - 1]],
						   mPoints[mHullIndices[mStackTop]],
						   mPoints[mHullIndices[mProcessPos]]))
	{
		mHullIndices[++mStackTop] = mHullIndices[mProcessPos++];
	}
	else
	{
		mStackTop--;
	}
	//std::cout << " Cur StackTop: " << mStackTop << " Cur ProcessPos: " << mProcessPos << std::endl;

	return true;
}