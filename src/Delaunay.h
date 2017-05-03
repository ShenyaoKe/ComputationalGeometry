#pragma once
#include "common.h"
#include "Vector2.h"
#include "HalfEdge.h"

const static size_t pid_neg1 = -1;
const static size_t pid_neg2 = -2;

class Delaunay
{
public:
	Delaunay(std::vector<Vector2f>& inPoints)
		: mPoints(std::move(inPoints))
	{
	}
	~Delaunay();

	void init();
	void initBucket();

	void reBucketPoint(size_t vid, size_t fid);
	// re-bucket when flipping edge
	void reBucketFlip(size_t fid1, size_t fid2, size_t vid1, size_t vid2);

	// Returns heId if on edge
	size_t onEdge(size_t ptId, size_t fid);
	bool toLeft(size_t oriVid, size_t targVid, size_t curVid);
	bool inCircle(size_t triP0, size_t triP1, size_t triP2, size_t targ);

	void flipEdge(HDS_HalfEdge& he, HDS_HalfEdge& hef);

	void insertIntoFace(size_t ptId, size_t fid);
	void insertAtEdge(size_t ptId, size_t edgeId);
	void swapTest(std::vector<HDS_HalfEdge*> dirtyEdges);

	void traversalPts();

	static void triangulate(std::vector<Vector2f> &inPoints);

private:
	unordered_map<size_t, std::vector<size_t>> mBucket;// Face to Pt
	std::vector<size_t> mPtToBucket;// Pt to Face

	std::vector<Vector2f>& mPoints;
	HDS_Mesh mMesh;
};

