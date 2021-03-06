#include "DelaunayTriangulation.h"
#include "Utils.h"

DelaunayTriangulation::DelaunayTriangulation(std::vector<Vector2f>& inPoints)
	: mPoints(std::move(inPoints))
{
	if (mPoints.size() > 2)
	{
		// Pick 3 points to form the first triangle
		// Find left most pt, add pt beyond
		initFirstTriangle();

		// Initialize bucketing
		initBucket();

		// for pt in bucket
		//     insert pt in bucket
		//     re-bucketing
		//
		//     update frontier
		traversalPts();

		finalizeHDS();
	}
}

DelaunayTriangulation::~DelaunayTriangulation()
{
}

void DelaunayTriangulation::initFirstTriangle()
{
	std::vector<HDS::HalfEdge>& hes = mMesh.halfedges;

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

	hes.resize(3);
	mMesh.faces.resize(1);
	// Insert P-1 P-2 as v0 v1
	hes[0].vid = cVertexIdNegTwo;
	hes[1].vid = cVertexIdNegOne;
	hes[2].vid = leftMostPtId;

	hes[0].fid = hes[1].fid = hes[2].fid = 0;
	mMesh.faces[0].heid = 0;

	linkEdgeLoop(hes[0], hes[1], hes[2]);
}

void DelaunayTriangulation::initBucket()
{
	size_t ptCount = mPoints.size();

	for (size_t i = 0; i < ptCount; i++)
	{
		if (i != mMesh.halfedges[2].vid)
		{
			mBucket[0].push_back(i);
		}
	}
	mVertexToBucketMap.resize(ptCount, 0);
}

void DelaunayTriangulation::reBucketPoint(size_t vid, size_t fid)
{
	mBucket[fid].push_back(vid);
	mVertexToBucketMap[vid] = fid;
}

void DelaunayTriangulation::reBucketFlip(const HDS::HalfEdge& he)
{
	const HDS::HalfEdge& hef = *he.flip();
	size_t fid0 = he.fid;
	size_t fid1 = hef.fid;
	size_t vid0 = he.vid;
	size_t vid1 = hef.vid;

	std::vector<size_t>& bucket1 = mBucket[fid0];
	std::vector<size_t>& bucket2 = mBucket[fid1];

	std::vector<size_t> dirtyPts;
	dirtyPts.insert(dirtyPts.end(), bucket1.begin(), bucket1.end());
	dirtyPts.insert(dirtyPts.end(), bucket2.begin(), bucket2.end());
	bucket1.clear();
	bucket2.clear();

	//                     *
	//                   / |
	//                 /   |
	//               /  f0 |
	//             /       |
	//            <---he---*v0
	//         v1 *---hef-->
	//             \       |
	//               \  f1 |
	//                 \   |
	//                   \ |
	//                     *
	for (size_t dirtyPid : dirtyPts)
	{
		if (toLeft(vid0, vid1, dirtyPid))
		{
			reBucketPoint(dirtyPid, fid0);
		}
		else
		{
			reBucketPoint(dirtyPid, fid1);
		}
	}
}

void DelaunayTriangulation::reBucketStarSplit(
	const HDS::HalfEdge& he0, const HDS::HalfEdge& he1, const HDS::HalfEdge& he2)
{
	size_t vId = he1.vid;
	size_t vid0 = he0.next()->vid;
	size_t vid1 = he1.next()->vid;
	size_t vid2 = he2.next()->vid;

	size_t fid0 = he0.fid;
	size_t fid1 = he1.fid;
	size_t fid2 = he2.fid;

	// Clear buckets
	std::vector<size_t>& bucket0 = mBucket[fid0];
	std::vector<size_t>& bucket1 = mBucket[fid1];
	std::vector<size_t>& bucket2 = mBucket[fid1];

	std::vector<size_t> dirtyPts;
	dirtyPts.insert(dirtyPts.end(), bucket0.begin(), bucket0.end());
	dirtyPts.insert(dirtyPts.end(), bucket1.begin(), bucket1.end());
	dirtyPts.insert(dirtyPts.end(), bucket2.begin(), bucket2.end());
	bucket0.clear();
	bucket1.clear();
	bucket2.clear();

	//              * v2
	//            / ^ \
	//           /  |  \
	//          /  he2  \
	//         /    |    \
	//        /f2   *v  f1\
	//       /    _/ \_    \
	//      /   he0    he1  \
	//     / |/_    f0   _\| \
	// v0 *-------------------* v1
	for (auto dirtyPtId : dirtyPts)
	{
		if (dirtyPtId == vId)
		{
			continue;
		}
		if (toLeft(vId, vid0, dirtyPtId))
		{
			if (toLeft(vId, vid1, dirtyPtId))
			{
				// in f1
				reBucketPoint(dirtyPtId,fid1);
			}
			else
			{
				// in f0
				reBucketPoint(dirtyPtId, fid0);
			}
		}
		else
		{
			if (toLeft(vId, vid2, dirtyPtId))
			{
				// in f2
				reBucketPoint(dirtyPtId, fid2);
			}
			else
			{
				//in f1
				reBucketPoint(dirtyPtId, fid1);
			}
		}
	}
}

size_t DelaunayTriangulation::onEdge(size_t ptId, size_t fid)
{
	const HDS::HalfEdge* he = mMesh.heFromFace(fid);
	const HDS::HalfEdge* curHE = he;

	do
	{
		size_t curVid = curHE->vid;
		size_t nextVid = curHE->next()->vid;
		if (curVid < cVertexIdNegTwo && nextVid < cVertexIdNegTwo)
		{
			// check cross product == 0
			float area = cross(mPoints[nextVid] - mPoints[curVid], mPoints[ptId] - mPoints[curVid]);
			if (area < std::numeric_limits<float>::epsilon() &&
				area > -std::numeric_limits<float>::epsilon())
			{
				return curHE->index;
			}
		}
		curHE = curHE->next();
	} while (curHE != he);
	return cInvalidIndex;
}

bool DelaunayTriangulation::toLeft(size_t vId0, size_t vId1, size_t pId)
{
	if (vId1 == cVertexIdNegTwo)
	{
		return mPoints[pId].x > mPoints[vId0].x;
	}
	if (vId1 == cVertexIdNegOne)
	{
		return mPoints[pId].x < mPoints[vId0].x;
	}

	return cross(mPoints[vId1] - mPoints[vId0], mPoints[pId] - mPoints[vId0]) > 0;
}

bool DelaunayTriangulation::toRight(size_t oriVid, size_t targVid, size_t curVid)
{
	return !toLeft(oriVid, targVid, curVid);
}

bool DelaunayTriangulation::inCircle(size_t triP0, size_t triP1, size_t triP2, size_t targ)
{
	// P0 is the point we inserted from previous step
	// P1 P2 cannot be negative at the same time, since we stop at boundary edge
	if (targ >= cVertexIdNegTwo)
	{
		return false;
	}
	if (triP1 >= cVertexIdNegTwo)
	{
		return toRight(triP0, triP2, targ);
	}
	if (triP2 >= cVertexIdNegTwo)
	{
		return toLeft(triP0, triP1, targ);
	}

	return Utils::inCircle(mPoints[triP0], mPoints[triP1], mPoints[triP2], mPoints[targ]);
}

void DelaunayTriangulation::flipEdge(HDS::HalfEdge& he, HDS::HalfEdge& hef)
{
	HDS::HalfEdge& he1 = *he.next();
	HDS::HalfEdge& he2 = *he.prev();
	HDS::HalfEdge& he3 = *hef.next();
	HDS::HalfEdge& he4 = *hef.prev();

	he.vid = he2.vid;
	hef.vid = he4.vid;
	linkEdgeLoop(he, he4, he1);
	linkFace(he, he4, he1, *mMesh.faceFromHe(he.index));

	linkEdgeLoop(hef, he2, he3);
	linkFace(hef, he2, he3, *mMesh.faceFromHe(hef.index));
}

void DelaunayTriangulation::insertIntoFace(size_t vId, size_t fid)
{
	HDS::Face* newFace = nullptr;
	HDS::HalfEdge* newHE = nullptr;
	mMesh.insertNewVertexInFace(newHE, newFace, vId, fid);

	// update bucket
	reBucketStarSplit(newHE[0], newHE[2], newHE[4]);

	// Legalize edges
	std::vector<HDS::HalfEdge*> frontierEdges{
		newHE[0].next(), newHE[2].next(), newHE[4].next()
	};
	legalizeEdge(frontierEdges, vId);
}

void DelaunayTriangulation::insertAtEdge(size_t vId, size_t heId)
{
	HDS::Face* newFace = nullptr;
	HDS::HalfEdge* newHE = nullptr;
	mMesh.insertNewVertexOnEdge(newHE, newFace, vId, heId);

	// re-bucket
	reBucketFlip(newHE[1]);
	reBucketFlip(newHE[5]);

	// Legalize edges
	std::vector<HDS::HalfEdge*> frontierEdges{
		newHE[0].prev(), newHE[1].next(), newHE[3].next(), newHE[5].next() };
	legalizeEdge(frontierEdges, vId);
}

void DelaunayTriangulation::legalizeEdge(std::vector<HDS::HalfEdge*>& frontierEdges,
										 size_t insertedPtId)
{
	while (!frontierEdges.empty())
	{
		HDS::HalfEdge* curHE = frontierEdges.back();
		frontierEdges.pop_back();

		if (curHE->isBoundary())
		{
			continue;
		}
		HDS::HalfEdge* curHEF = curHE->flip();
		size_t flipPid = curHEF->prev()->vid;

		// If the vertex P on the opposite triangle lies in Circle(A, B, C)
		// Flip edge BC, and re-bucket points in two triangles
		//           A             A
		//          / \           /|\
		//         B---C  ---->  B | C
		//          \ /           \|/
		//           P             P
		if (inCircle(insertedPtId, curHE->vid, curHEF->vid, flipPid))
		{
			frontierEdges.push_back(curHEF->prev());
			frontierEdges.push_back(curHEF->next());

			flipEdge(*curHE, *curHEF);
			reBucketFlip(*curHE/*, *curHEF*/);
		}
	}
}

void DelaunayTriangulation::traversalPts()
{
	size_t skipPtId = mMesh.halfedges[2].vid;
	for (size_t pointId = 0; pointId < mPoints.size(); pointId++)
	{
		if (pointId == skipPtId)
		{
			continue;
		}

		size_t faceId = mVertexToBucketMap[pointId];
		size_t edgeId = onEdge(pointId, faceId);
		if (edgeId == cInvalidIndex)
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

void DelaunayTriangulation::finalizeHDS()
{
	for (HDS::HalfEdge& he : mMesh.halfedges)
	{
		if (he.vid >= cVertexIdNegTwo && !he.isInvalid())
		{
			HDS::HalfEdge* curHE = &he;
			mMesh.faces[he.fid].setToInvalid();
			do
			{
				HDS::HalfEdge* nextHE = curHE->next();
				curHE->setToInvalid();
				curHE->breakFlip();
				curHE = nextHE;
			} while (curHE != &he);
		}
	}
}

/*
bool Delaunay::keepInsertion()
{
	if (mUnusedPtIds.empty())
	{
		return false;
	}
	size_t pointId = mUnusedPtIds.top();

	size_t faceId = mVertexToBucketMap[pointId];
	size_t edgeId = onEdge(pointId, faceId);
	if (edgeId == cInvalidIndex)
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
}*/

void DelaunayTriangulation::extractTriangleIndices(std::vector<uint32_t>& outIndices) const
{
	for (auto& f : mMesh.faces)
	{
		if (f.isInvalid())
		{
			continue;
		}

		const HDS::HalfEdge* he = mMesh.heFromFace(f.index);
		const HDS::HalfEdge* curHE = he;

		std::array<uint32_t, 3> vids;
		size_t localVidIdx = 0;

		do
		{
			vids[localVidIdx++] = static_cast<uint32_t>(curHE->vid);
			curHE = curHE->next();
		} while (curHE != he);

		outIndices.insert(outIndices.end(), vids.begin(), vids.end());
	}
}

