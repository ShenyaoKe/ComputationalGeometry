#pragma once
#include "common.h"
#include "Vector2.h"
#include "HalfEdge.h"


class Delaunay
{
public:
	Delaunay(std::vector<Vector2f>& inPoints);
	~Delaunay();

	void init();
	void initBucket();

	void reBucketPoint(size_t vid, size_t fid);
	// re-bucket when flipping edge
	void reBucketFlip(const HDS_HalfEdge& he, const HDS_HalfEdge& hef);
	// re-bucket when split by new edge
	void reBucketSplit(const size_t fid, const HDS_HalfEdge& splitHE);

	// Returns heId if on edge
	size_t onEdge(size_t ptId, size_t fid);
	bool toLeft(size_t oriVid, size_t targVid, size_t curVid);
	bool toRight(size_t oriVid, size_t targVid, size_t curVid);

	bool inCircle(size_t triP0, size_t triP1, size_t triP2, size_t targ);

	void flipEdge(HDS_HalfEdge& he, HDS_HalfEdge& hef);

	void insertIntoFace(size_t ptId, size_t fid);
	void insertAtEdge(size_t ptId, size_t edgeId);

	void swapTest(std::vector<HDS_HalfEdge*> dirtyEdges, size_t insertedPtId);

	void traversalPts();
	bool keepInsertion();

	void extractTriangles(std::vector<uint32_t>& outIndices) const;

private:
	unordered_map<size_t, std::vector<size_t>> mBucket;// Face to Pt
	std::vector<size_t> mPtToBucket;// Pt to Face

	std::vector<Vector2f>& mPoints;
	HDS_Mesh mMesh;

	std::stack<size_t> mUnusedPtIds;

	//static size_t sCurPtId;
	const static size_t pid_neg1 = (size_t)-1;
	const static size_t pid_neg2 = (size_t)-2;
};

