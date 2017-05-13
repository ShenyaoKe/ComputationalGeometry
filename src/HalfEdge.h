#pragma once
#include "common.h"

static const size_t cInvalidIndex = static_cast<size_t>(-1);

/*
* Vertex
*/
class HDS_Vertex
{
public:
	HDS_Vertex() : index(uid++), pid(cInvalidIndex), heid(cInvalidIndex) {}
	~HDS_Vertex() {}

	static void resetIndex() { uid = 0; }

	size_t pid;
	size_t index;
	size_t heid;
private:
	static size_t uid;
};


/*
* Half-Edge
*/
class HDS_HalfEdge
{
public:
	static void resetIndex() { uid = 0; }
	static void matchIndexToSize(size_t	size) { uid = size; }

	HDS_HalfEdge() : index(uid++), fid(cInvalidIndex), vid(cInvalidIndex)
		, prev_offset(0), next_offset(0), flip_offset(0) {}
	~HDS_HalfEdge() {}

	// Get the explicit pointer to corresponding edges
	HDS_HalfEdge* prev() { return this + prev_offset; }
	HDS_HalfEdge* next() { return this + next_offset; }
	HDS_HalfEdge* flip() { return this + flip_offset; }
	HDS_HalfEdge* rotCW() { return flip()->next(); }
	HDS_HalfEdge* rotCCW() { return prev()->flip(); }
	const HDS_HalfEdge* prev() const { return this + prev_offset; }
	const HDS_HalfEdge* next() const { return this + next_offset; }
	const HDS_HalfEdge* flip() const { return this + flip_offset; }
	const HDS_HalfEdge* rotCW() const { return flip()->next(); }
	const HDS_HalfEdge* rotCCW() const { return prev()->flip(); }

	bool isBoundary() const { return flip_offset == 0; }

	// No self-loop edge
	void setToInvalid() { prev_offset = next_offset = 0; fid = cInvalidIndex; }
	bool isInvalid() const { return prev_offset == 0 || next_offset == 0 || fid == cInvalidIndex; }

	void setFlip(HDS_HalfEdge* f_e)
	{
		flip_offset = f_e - this;
		f_e->flip_offset = -flip_offset;
	}
	void breakFlip()
	{
		flip_offset = flip()->flip_offset = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t index;
	size_t fid;
	size_t vid;
	// Offset to index of previous/next/flip edge
	// previous/next/flip edge doesn't exist
	// when (previous/nex/flip == 0)
	ptrdiff_t prev_offset, next_offset, flip_offset;


private:

	static size_t uid;
};

/*
* Face
*/
class HDS_Face
{
public:
	static void resetIndex() { uid = 0; }

	HDS_Face() : index(uid++), heid(cInvalidIndex) {}
	~HDS_Face() {}

	// Get the connected half-edge id
	// Explicit pointer access is handled by HDS_Mesh
	size_t heID() const { return heid; }


	void setToInvalid() { heid = cInvalidIndex; }
	bool isInvalid() const { return heid == cInvalidIndex; }

	// Member data
	size_t index;
	size_t heid;

private:
	static size_t uid;
};

/*
* Mesh
*/
class HDS_Mesh
{
public:
	HDS_Mesh() {}
	/*HDS_Mesh(vector<vert_t> &vs, vector<he_t> &hes, vector<face_t> &fs)
		: verts(std::move(vs))
		, halfedges(std::move(hes))
		, faces(std::move(fs)) {}*/
	HDS_Mesh(const HDS_Mesh &other)
		: /*verts(other.verts),*/ halfedges(other.halfedges), faces(other.faces) {}
	~HDS_Mesh() {}

	// Reset UID in each component
	// Mask(Bitwise Operation): face|edge|vertex
	//    e.g. All(111==3), Vertex Only(001==1), Vertex+Edge(011==3)
	static void resetIndex(uint8_t reset_mask = 7)
	{
		//if (reset_mask & 1) HDS_Vertex::resetIndex();
		if (reset_mask & 2) HDS_HalfEdge::resetIndex();
		if (reset_mask & 4) HDS_Face::resetIndex();
	}

	void printInfo(const std::string &msg = "")
	{
		if (!msg.empty()) std::cout << msg << std::endl;
		//std::cout << "#vertices = " << verts.size() << std::endl;
		std::cout << "#faces = " << faces.size() << std::endl;
		std::cout << "#half edges = " << halfedges.size() << std::endl;
	}

	HDS_HalfEdge* heFromFace(size_t fid) { return &halfedges[faces[fid].heid]; }
	//he_t* heFromVert(size_t vid) { return &halfedges[verts[vid].heid]; }
	//vert_t* vertFromHe(size_t heid) { return &verts[halfedges[heid].vid]; }
	HDS_Face* faceFromHe(size_t heid) { return &faces[halfedges[heid].fid]; }
	const HDS_HalfEdge* heFromFace(size_t fid) const { return &halfedges[faces[fid].heid]; }
	//const he_t* heFromVert(size_t vid) const { return &halfedges[verts[vid].heid]; }
	//const vert_t* vertFromHe(size_t heid) const { return &verts[halfedges[heid].vid]; }
	const HDS_Face* faceFromHe(size_t heid) const { return &faces[halfedges[heid].fid]; }

	// Returns pointer to new Half-Edges
	void insertNewVertexOnEdge(HDS_HalfEdge*& outNewHE, HDS_Face*& outNewFace, size_t vId, size_t heId);

	void insertNewVertexInFace(HDS_HalfEdge*& outNewHE, HDS_Face*& outNewFace, size_t vId, size_t fId);

	//vector<vert_t> verts;
	vector<HDS_HalfEdge> halfedges;
	vector<HDS_Face>     faces;
};

inline void linkHalfEdge(HDS_HalfEdge& prev, HDS_HalfEdge& next)
{
	prev.next_offset = next.index - prev.index;
	next.prev_offset = -prev.next_offset;
}

inline void linkEdgeLoop(HDS_HalfEdge& he1, HDS_HalfEdge& he2, HDS_HalfEdge& he3)
{
	linkHalfEdge(he1, he2);
	linkHalfEdge(he2, he3);
	linkHalfEdge(he3, he1);
}

inline void linkVertex(HDS_HalfEdge& he1, size_t ptId)
{
	he1.vid = ptId;
}

inline void linkFace(HDS_HalfEdge& he1, HDS_HalfEdge& he2, HDS_HalfEdge& he3, HDS_Face& face)
{
	// Link HE-F
	he1.fid = he2.fid = he3.fid = face.index;
	face.heid = he1.index;
}

// Given an original edge, two new HE
inline void constructFace(
	HDS_HalfEdge& newHE0, HDS_HalfEdge& newHE1, HDS_HalfEdge& newHE2, HDS_Face& face)
{
	// Link HE-HE
	linkEdgeLoop(newHE0, newHE1, newHE2);

	// Link HE-F
	linkFace(newHE0, newHE1, newHE2, face);
}