#pragma once
#include "common.h"

using SizeType = size_t;
using OffsetType = ptrdiff_t;

static const SizeType cInvalidIndex = static_cast<SizeType>(-1);

namespace HDS
{

// Vertex
class Vertex
{
public:
	Vertex() : index(uid++), pid(cInvalidIndex), heid(cInvalidIndex) {}
	~Vertex() {}

	static void resetIndex() { uid = 0; }

	SizeType pid;
	SizeType index;
	SizeType heid;
private:
	static SizeType uid;
};


// Half-Edge
class HalfEdge
{
public:
	static void resetIndex() { uid = 0; }
	static void matchIndexToSize(SizeType	size) { uid = size; }

	HalfEdge() : index(uid++), fid(cInvalidIndex), vid(cInvalidIndex)
		, prev_offset(0), next_offset(0), flip_offset(0)
	{
	}
	~HalfEdge() {}

	// Get the explicit pointer to corresponding edges
	HalfEdge* prev() { return this + prev_offset; }
	HalfEdge* next() { return this + next_offset; }
	HalfEdge* flip() { return this + flip_offset; }
	HalfEdge* rotCW() { return flip()->next(); }
	HalfEdge* rotCCW() { return prev()->flip(); }
	const HalfEdge* prev() const { return this + prev_offset; }
	const HalfEdge* next() const { return this + next_offset; }
	const HalfEdge* flip() const { return this + flip_offset; }
	const HalfEdge* rotCW() const { return flip()->next(); }
	const HalfEdge* rotCCW() const { return prev()->flip(); }

	bool isBoundary() const { return flip_offset == 0; }

	// No self-loop edge
	void setToInvalid() { prev_offset = next_offset = 0; fid = cInvalidIndex; }
	bool isInvalid() const { return prev_offset == 0 || next_offset == 0 || fid == cInvalidIndex; }

	void setFlip(HalfEdge* f_e)
	{
		flip_offset = f_e - this;
		f_e->flip_offset = -flip_offset;
	}
	void breakFlip()
	{
		flip_offset = flip()->flip_offset = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	SizeType index;
	SizeType fid;
	SizeType vid;
	// Offset to index of previous/next/flip edge
	// previous/next/flip edge doesn't exist
	// when (previous/nex/flip == 0)
	OffsetType prev_offset, next_offset, flip_offset;

private:

	static SizeType uid;
};

// Face
class Face
{
public:
	static void resetIndex() { uid = 0; }

	Face() : index(uid++), heid(cInvalidIndex) {}
	~Face() {}

	// Get the connected half-edge id
	// Explicit pointer access is handled by HDS_Mesh
	SizeType heID() const { return heid; }


	void setToInvalid() { heid = cInvalidIndex; }
	bool isInvalid() const { return heid == cInvalidIndex; }

	// Member data
	SizeType index;
	SizeType heid;

private:
	static SizeType uid;
};

// Mesh
class Mesh
{
public:
	Mesh() {}
	/*HDS_Mesh(vector<Vertex> &vs, vector<HalfEdge> &hes, vector<Face> &fs)
		: verts(std::move(vs))
		, halfedges(std::move(hes))
		, faces(std::move(fs)) {}*/
	Mesh(const Mesh &other)
		: /*verts(other.verts),*/ halfedges(other.halfedges), faces(other.faces)
	{
	}
	~Mesh() {}

	// Reset UID in each component
	// Mask(Bitwise Operation): face|edge|vertex
	//    e.g. All(111==3), Vertex Only(001==1), Vertex+Edge(011==3)
	static void resetIndex(uint8_t reset_mask = 7)
	{
		//if (reset_mask & 1) HDS_Vertex::resetIndex();
		if (reset_mask & 2) HalfEdge::resetIndex();
		if (reset_mask & 4) Face::resetIndex();
	}

	void printInfo(const std::string &msg = "")
	{
		if (!msg.empty()) std::cout << msg << std::endl;
		//std::cout << "#vertices = " << verts.size() << std::endl;
		std::cout << "#faces = " << faces.size() << std::endl;
		std::cout << "#half edges = " << halfedges.size() << std::endl;
	}

	HalfEdge* heFromFace(SizeType fid) { return &halfedges[faces[fid].heid]; }
	//HalfEdge* heFromVert(SizeType vid) { return &halfedges[verts[vid].heid]; }
	//Vertex* vertFromHe(SizeType heid) { return &verts[halfedges[heid].vid]; }
	Face* faceFromHe(SizeType heid) { return &faces[halfedges[heid].fid]; }
	const HalfEdge* heFromFace(SizeType fid) const { return &halfedges[faces[fid].heid]; }
	//const HalfEdge* heFromVert(SizeType vid) const { return &halfedges[verts[vid].heid]; }
	//const Vertex* vertFromHe(SizeType heid) const { return &verts[halfedges[heid].vid]; }
	const Face* faceFromHe(SizeType heid) const { return &faces[halfedges[heid].fid]; }

	// Returns pointer to new Half-Edges
	void insertNewVertexOnEdge(HalfEdge*& outNewHE,
							   Face*& outNewFace,
							   SizeType vId,
							   SizeType heId);

	void insertNewVertexInFace(HalfEdge*& outNewHE,
							   Face*& outNewFace,
							   SizeType vId,
							   SizeType fId);

	//vector<Vertex> verts;
	vector<HalfEdge> halfedges;
	vector<Face>     faces;
};

inline void linkHalfEdge(HalfEdge& prev, HalfEdge& next)
{
	prev.next_offset = next.index - prev.index;
	next.prev_offset = -prev.next_offset;
}

inline void linkEdgeLoop(HalfEdge& he1, HalfEdge& he2, HalfEdge& he3)
{
	linkHalfEdge(he1, he2);
	linkHalfEdge(he2, he3);
	linkHalfEdge(he3, he1);
}

inline void linkVertex(HalfEdge& he1, SizeType ptId)
{
	he1.vid = ptId;
}

inline void linkFace(HalfEdge& he1, HalfEdge& he2, HalfEdge& he3, Face& face)
{
	// Link HE-F
	he1.fid = he2.fid = he3.fid = face.index;
	face.heid = he1.index;
}

// Given an original edge, two new HE
inline void constructFace(
	HalfEdge& newHE0, HalfEdge& newHE1, HalfEdge& newHE2, Face& face)
{
	// Link HE-HE
	linkEdgeLoop(newHE0, newHE1, newHE2);

	// Link HE-F
	linkFace(newHE0, newHE1, newHE2, face);
}

}
