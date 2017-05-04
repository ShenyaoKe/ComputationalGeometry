#include "Delaunay.h"
#include "Utils.h"

Delaunay::Delaunay(std::vector<Vector2f>& inPoints)
	: mPoints(std::move(inPoints))
{
	if (mPoints.size() > 2)
	{
		// Pick 3 points to form the first triangle
		// Find left most pt, add pt beyond
		init();

		// Initialize bucketing
		initBucket();

		// for pt in bucket
		//     insert pt in bucket
		//     re-bucketing
		//
		//     update frontier
		traversalPts();
	}
}

Delaunay::~Delaunay()
{
}

void linkHalfEdge(HDS_HalfEdge& prev, HDS_HalfEdge& next)
{
	prev.next_offset = next.index - prev.index;
	next.prev_offset = -prev.next_offset;
}

void linkEdgeLoop(HDS_HalfEdge& he1, HDS_HalfEdge& he2, HDS_HalfEdge& he3)
{
	linkHalfEdge(he1, he2);
	linkHalfEdge(he2, he3);
	linkHalfEdge(he3, he1);
}

void linkVertex(HDS_HalfEdge& he1, size_t ptId)
{
	he1.vid = ptId;
}
void linkFace(HDS_HalfEdge& he1, HDS_HalfEdge& he2, HDS_HalfEdge& he3, HDS_Face& face)
{
	// Link HE-F
	he1.fid = he2.fid = he3.fid = face.index;
	face.heid = he1.index;
}
// Given an original edge, two new HE
void constructFace(
	HDS_HalfEdge& oriHE, HDS_HalfEdge& newHE1, HDS_HalfEdge& newHE2, HDS_Face& face, size_t newPtId)
{
	// Link V-HE
	newHE1.vid = oriHE.next()->vid;
	newHE2.vid = newPtId;

	// Link HE-HE
	linkEdgeLoop(oriHE, newHE1, newHE2);

	// Link HE-F
	linkFace(oriHE, newHE1, newHE2, face);
}
//////////////////////////////////////////////////////////////////////////

void Delaunay::init()
{
	std::vector<HDS_HalfEdge>& hes = mMesh.halfedges;
	std::vector<HDS_Face>& faces = mMesh.faces;
	//mMesh.verts.reserve(mPoints.size() + 2);
	mMesh.halfedges.reserve(3 * mPoints.size());

	size_t leftMostPtId = 0;
	const Vector2f* pt = &mPoints.front();

	// find left most point
	for (size_t i = 1; i < mPoints.size(); i++)
	{
		const Vector2f* curPt = &mPoints[i];

		if ((curPt->x < pt->x) ||
			(curPt->x == pt->x && curPt->y < pt->y))
		{
			leftMostPtId = i;
			pt = curPt;
		}
	}
	for (size_t i = 0; i < mPoints.size(); i++)
	{
		if (i == leftMostPtId)
		{
			continue;
		}
		mUnusedPtIds.push(i);
	}

	//mMesh.verts.resize(3);
	hes.resize(3);
	mMesh.faces.resize(1);
	// Insert P-1 P-2 as v0 v1
	hes[0].vid = pid_neg2;
	hes[1].vid = pid_neg1;
	hes[2].vid = leftMostPtId;

	hes[0].fid = hes[1].fid = hes[2].fid = 0;
	mMesh.faces[0].heid = 0;

	linkEdgeLoop(hes[0], hes[1], hes[2]);
}

void Delaunay::initBucket()
{
	size_t ptCount = mPoints.size();
	//mBucket[0].reserve(ptCount - 1);
	for (size_t i = 0; i < ptCount; i++)
	{
		if (i != mMesh.halfedges[2].vid)
		{
			mBucket[0].push_back(i);
		}
	}
	mPtToBucket.resize(ptCount, 0);
}

void Delaunay::reBucketPoint(size_t vid, size_t fid)
{
	mBucket[fid].push_back(vid);
	mPtToBucket[vid] = fid;
}

void Delaunay::reBucketFlip(const HDS_HalfEdge& he, const HDS_HalfEdge& hef)
{
	// TODO
	std::vector<size_t>& bucket1 = mBucket[he.fid];
	std::vector<size_t>& bucket2 = mBucket[hef.fid];
	//std::cout << "Original buckets: 1: " << bucket1.size() << ", 2: " << bucket2.size() << std::endl;

	std::vector<size_t> dirtyPts;
	dirtyPts.insert(dirtyPts.end(), bucket1.begin(), bucket1.end());
	dirtyPts.insert(dirtyPts.end(), bucket2.begin(), bucket2.end());
	bucket1.clear();
	bucket2.clear();

	for (size_t dirtyPid : dirtyPts)
	{
		if (toLeft(he.vid, hef.vid, dirtyPid))
		{
			reBucketPoint(dirtyPid, he.fid);
		}
		else
		{
			reBucketPoint(dirtyPid, hef.fid);
		}
	}
}

void Delaunay::reBucketSplit(const size_t fid, const HDS_HalfEdge& splitHE)
{
	const HDS_HalfEdge& splitHEF = *splitHE.flip();

	if (splitHE.fid != fid && splitHEF.fid != fid)
	{
		return;
	}

	std::vector<size_t>& curBucket = mBucket[fid];
	auto dirtyPts = curBucket;
	curBucket.clear();

	for (auto dirtyPid : dirtyPts)
	{
		if (toLeft(splitHE.vid, splitHEF.vid, dirtyPid))
		{
			reBucketPoint(dirtyPid, splitHE.fid);
		}
		else
		{
			reBucketPoint(dirtyPid, splitHEF.fid);
		}
	}
}

size_t Delaunay::onEdge(size_t ptId, size_t fid)
{
	const HDS_Face& face = mMesh.faces[fid];
	const HDS_HalfEdge* he = mMesh.heFromFace(fid);
	const HDS_HalfEdge* curHE = he;
	//size_t vid[3]{ he1.vid, he1.next()->vid, he1.prev()->vid };

	do
	{
		size_t curVid = curHE->vid;
		size_t nextVid = curHE->next()->vid;
		if (curVid < pid_neg2 && nextVid < pid_neg2)
		{
			// check cross product == 0?
			float area = cross(mPoints[nextVid] - mPoints[curVid], mPoints[ptId] - mPoints[curVid]);
			if (area < std::numeric_limits<float>::epsilon() &&
				area > -std::numeric_limits<float>::epsilon())
			{
				return curHE->index;
			}
		}
		curHE = curHE->next();
	} while (curHE != he);
	return cInvalidHDS;
}

bool Delaunay::toLeft(size_t oriVid, size_t targVid, size_t curVid)
{
	// Origin Point must NOT be infinite points
	if (targVid == pid_neg2)
	{
		return mPoints[curVid].x > mPoints[oriVid].x;
	}
	if (targVid == pid_neg1)
	{
		return mPoints[curVid].x < mPoints[oriVid].x;
	}

	return cross(mPoints[targVid] - mPoints[oriVid], mPoints[curVid] - mPoints[oriVid]) > 0;
}

bool Delaunay::toRight(size_t oriVid, size_t targVid, size_t curVid)
{
	return !toLeft(oriVid, targVid, curVid);
}

bool Delaunay::inCircle(size_t triP0, size_t triP1, size_t triP2, size_t targ)
{
	// P0 is the point we inserted from previous step
	// P1 P2 cannot be negative at the same time, since we stop at boundary edge
	if (targ >= pid_neg2)
	{
		return false;
	}
	if (triP1 >= pid_neg2)
	{
		return toRight(triP0, triP2, targ);
	}
	if (triP2 >= pid_neg2)
	{
		return toLeft(triP0, triP1, targ);
	}

	return Utils::inCircle(mPoints[triP0], mPoints[triP1], mPoints[triP2], mPoints[targ]);
}

void Delaunay::flipEdge(HDS_HalfEdge& he, HDS_HalfEdge& hef)
{
	HDS_HalfEdge& he1 = *he.next();
	HDS_HalfEdge& he2 = *he.prev();
	HDS_HalfEdge& he3 = *hef.next();
	HDS_HalfEdge& he4 = *hef.prev();

	he.vid = he2.vid;
	hef.vid = he4.vid;
	linkEdgeLoop(he, he4, he1);
	linkFace(he, he4, he1, *mMesh.faceFromHe(he.index));

	linkEdgeLoop(hef, he2, he3);
	linkFace(hef, he2, he3, *mMesh.faceFromHe(hef.index));
}

void Delaunay::insertIntoFace(size_t ptId, size_t fid)
{
	std::vector<HDS_HalfEdge>& hes = mMesh.halfedges;
	std::vector<HDS_Face>& faces = mMesh.faces;
	size_t newHeId = hes.size();
	size_t newFaceId = faces.size();

	hes.resize(newHeId + 6);
	faces.resize(newFaceId + 2);

	HDS_HalfEdge* newHE = &hes[newHeId];
	HDS_HalfEdge* oriHe0 = mMesh.heFromFace(fid);
	HDS_HalfEdge* oriHe1 = oriHe0->next();
	HDS_HalfEdge* oriHe2 = oriHe1->next();

	//std::array<HDS_Face*, 3> splitFaces = { &faces[fid], &faces[newFaceId], &faces[newFaceId + 1] };
	//splitFaces[0]->heid = newHeId;
	// Link face loop
	constructFace(*oriHe0, newHE[1], newHE[0], faces[fid], ptId);
	constructFace(*oriHe1, newHE[3], newHE[2], faces[newFaceId], ptId);
	constructFace(*oriHe2, newHE[5], newHE[4], faces[newFaceId + 1], ptId);

	newHE[1].setFlip(newHE + 2);
	newHE[3].setFlip(newHE + 4);
	newHE[5].setFlip(newHE);

	//////////////////////////////////////////////////////////////////////////
	// update bucket
	auto& curBucket = mBucket[mPtToBucket[ptId]];
	auto dirtyPts = curBucket;
	curBucket.clear();
	for (auto dirtyPtId : dirtyPts)
	{
		if (dirtyPtId == ptId)
		{
			continue;
		}
		if (toLeft(ptId, oriHe0->vid, dirtyPtId))
		{
			if (toLeft(ptId, oriHe1->vid, dirtyPtId))
			{
				// in f1
				reBucketPoint(dirtyPtId, newFaceId);
			}
			else
			{
				// in f0
				reBucketPoint(dirtyPtId, fid);
			}
		}
		else
		{
			if (toLeft(ptId, oriHe2->vid, dirtyPtId))
			{
				// in f2
				reBucketPoint(dirtyPtId, newFaceId + 1);
			}
			else
			{
				//in f1
				reBucketPoint(dirtyPtId, newFaceId);
			}
		}
	}

	//if (sCurPtId != mPoints.size() - 1)
	swapTest({ oriHe0, oriHe1, oriHe2 }, ptId);
}

void Delaunay::insertAtEdge(size_t ptId, size_t edgeId)
{
	// TODO
	std::vector<HDS_HalfEdge>& hes = mMesh.halfedges;
	std::vector<HDS_Face>& faces = mMesh.faces;
	size_t newHeId = hes.size();
	size_t newFaceId = faces.size();

	hes.resize(newHeId + 6);
	faces.resize(newFaceId + 2);

	HDS_HalfEdge* he = &hes[edgeId];
	HDS_HalfEdge* hef = he->flip();
	HDS_HalfEdge* oriHe0 = he->next();
	HDS_HalfEdge* oriHe1 = oriHe0->next();
	HDS_HalfEdge* oriHe2 = hef->next();
	HDS_HalfEdge* oriHe3 = oriHe2->next();

	HDS_HalfEdge* newHE = &hes[newHeId];

	// Link face loop
	constructFace(*oriHe0, newHE[0], *he, faces[he->fid], ptId);
	constructFace(*oriHe1, newHE[2], newHE[1], faces[newFaceId], ptId);
	constructFace(*oriHe2, newHE[4], newHE[3], faces[newFaceId + 1], ptId);
	constructFace(*oriHe3, *hef, newHE[5], faces[hef->fid], ptId);

	newHE[0].setFlip(newHE + 1);
	newHE[2].setFlip(newHE + 3);
	newHE[4].setFlip(newHE + 5);

	// re-bucket
	reBucketSplit(he->fid, newHE[1]);
	reBucketSplit(hef->fid, newHE[5]);
}

void Delaunay::swapTest(std::vector<HDS_HalfEdge*> dirtyEdges, size_t insertedPtId)
{
	while (!dirtyEdges.empty())
	{
		HDS_HalfEdge* curHE = dirtyEdges.back();
		dirtyEdges.pop_back();

		if (curHE->isBoundary())
		{
			continue;
		}
		HDS_HalfEdge* curHEF = curHE->flip();
		size_t flipPid = curHEF->prev()->vid;

		if (inCircle(/*curHE->prev()->vid*/insertedPtId, curHE->vid, curHE->next()->vid, flipPid))
		{
			dirtyEdges.push_back(curHEF->prev());
			dirtyEdges.push_back(curHEF->next());

			flipEdge(*curHE, *curHEF);
			reBucketFlip(*curHE, *curHEF);
		}
	}
}

void Delaunay::traversalPts()
{
	size_t skipPtId = mMesh.halfedges[2].vid;
	for (size_t pointId = 0; pointId < mPoints.size(); pointId++)
	{
		//sCurPtId = pointId;
		if (pointId == mPoints.size() - 1)
		{
			cout << "hello world!\n";
		}
		if (pointId == skipPtId)
		{
			continue;
		}

		size_t faceId = mPtToBucket[pointId];
		size_t edgeId = onEdge(pointId, faceId);
		if (edgeId == cInvalidHDS)
		{
			// Insert into face
			insertIntoFace(pointId, faceId);
		}
		else
		{
			// Insert on edge
			insertAtEdge(pointId, edgeId);
		}

	}
}

bool Delaunay::keepInsertion()
{
	if (mUnusedPtIds.empty())
	{
		return false;
	}
	size_t pointId = mUnusedPtIds.top();

	size_t faceId = mPtToBucket[pointId];
	size_t edgeId = onEdge(pointId, faceId);
	if (edgeId == cInvalidHDS)
	{
		// Insert into face
		insertIntoFace(pointId, faceId);
	}
	else
	{
		// Insert on edge
		insertAtEdge(pointId, edgeId);
	}

	mUnusedPtIds.pop();
	return true;
}

void Delaunay::extractTriangles(std::vector<uint32_t>& outIndices) const
{
	for (auto& f : mMesh.faces)
	{
		const HDS_HalfEdge* he = mMesh.heFromFace(f.index);
		const HDS_HalfEdge* curHE = he;

		std::array<uint32_t, 3> vids;
		size_t localVidIdx = 0;

		bool invalidFace = false;
		do
		{
			if (curHE->vid >= pid_neg2)
			{
				vids[localVidIdx++] = mPoints.size() + (curHE->vid - pid_neg2);
			}
			else
			{
				vids[localVidIdx++] = curHE->vid;
			}
			curHE = curHE->next();
		} while (curHE != he);
		if (invalidFace)
		{
			continue;
		}
		outIndices.insert(outIndices.end(), vids.begin(), vids.end());
	}
}

