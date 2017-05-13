#pragma once
#include "common.h"
#include "Vector2.h"
#include "HalfEdge.h"


class Delaunay
{
public:
	Delaunay(std::vector<Vector2f>& inPoints);
	~Delaunay();

	void extractTriangleIndices(std::vector<uint32_t>& outIndices) const;

private:
	void initFirstTriangle();
	void initBucket();

	void reBucketPoint(size_t vid, size_t fid);
	// re-bucket when flipping edge
	void reBucketFlip(const HDS_HalfEdge& he/*, const HDS_HalfEdge& hef*/);
	// re-bucket when split by new edge
	// TODO: Rmove this use above
	void reBucketStarSplit(/*const size_t oriFid,*/ const HDS_HalfEdge& he0, const HDS_HalfEdge& he1, const HDS_HalfEdge& he2);
	//void reBucketSplit(const size_t fid, const HDS_HalfEdge& splitHE);

	// Returns heId if on edge
	size_t onEdge(size_t ptId, size_t fid);

	// Origin Point v0 must NOT be infinite points
	// P is to the left of vector v0-v1
	//             * P
	//     v0 *--------->* v1
	bool toLeft(size_t vId0, size_t vId1, size_t pId);
	bool toRight(size_t oriVid, size_t targVid, size_t curVid);

	bool inCircle(size_t triP0, size_t triP1, size_t triP2, size_t targ);

	void flipEdge(HDS_HalfEdge& he, HDS_HalfEdge& hef);

	void insertIntoFace(size_t ptId, size_t fid);
	void insertAtEdge(size_t ptId, size_t edgeId);

	void legalizeEdge(std::vector<HDS_HalfEdge*>& frontierEdges, size_t insertedPtId);
	//void legalizeEdge(HDS_HalfEdge& dirtyEdge, size_t insertedPtId);

	void traversalPts();
	//bool keepInsertion();

	void finalizeHDS();


private:
	// Structure to store vertex position
	// aka the triangle face id where the vertex belongs
	unordered_map<size_t, std::vector<size_t>> mBucket;// Face to Pt
	std::vector<size_t> mVertexToBucketMap;// Pt to Face

	// Vertex positions
	std::vector<Vector2f> mPoints;

	// Triangle mesh represented by HalfEdge Data Structure to maintain mesh traversal
	HDS_Mesh mMesh;

	//std::stack<size_t> mUnusedPtIds;

	//static size_t sCurPtId;
	const static size_t cVertexIdNegOne = static_cast<size_t>(-1);
	const static size_t cVertexIdNegTwo = static_cast<size_t>(-2);
};

