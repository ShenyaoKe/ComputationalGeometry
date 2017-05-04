#pragma once
#include "common.h"
//#include "Geometry/TriangleMesh.h"

static const size_t cInvalidHDS = (size_t)-1;

/*
* Vertex
*/
/*
class HDS_Vertex
{
public:
	HDS_Vertex() : index(uid++), pid(cInvalidHDS), heid(cInvalidHDS) {}
	~HDS_Vertex() {}

	static void resetIndex() { uid = 0; }

	size_t pid;
	size_t index;
	size_t heid;
private:
	static size_t uid;
};
*/

/*
* Half-Edge
*/
class HDS_HalfEdge
{
public:
	static void resetIndex() { uid = 0; }
	static void matchIndexToSize(size_t	size) { uid = size; }

	HDS_HalfEdge() : index(uid++), fid(cInvalidHDS), vid(cInvalidHDS)
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

	void setFlip(HDS_HalfEdge* f_e)
	{
		flip_offset = f_e - this;
		f_e->flip_offset = -flip_offset;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t index;
	size_t fid;
	size_t vid;
	// Offset to index of previous/nex/flip edge
	// previous/nex/flip edge doesn't exist
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

	HDS_Face() : index(uid++), heid(cInvalidHDS) {}
	~HDS_Face() {}

	// Get the connected half-edge id
	// Explicit pointer access is handled by HDS_Mesh
	size_t heID() const { return heid; }

	// Member data
	size_t index;
	size_t heid;

private:
	static size_t uid;
};

//using vert_t = HDS_Vertex;
using he_t = HDS_HalfEdge;
using face_t = HDS_Face;
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

	he_t* heFromFace(size_t fid) { return &halfedges[faces[fid].heid]; }
	//he_t* heFromVert(size_t vid) { return &halfedges[verts[vid].heid]; }
	//vert_t* vertFromHe(size_t heid) { return &verts[halfedges[heid].vid]; }
	face_t* faceFromHe(size_t heid) { return &faces[halfedges[heid].fid]; }
	const he_t* heFromFace(size_t fid) const { return &halfedges[faces[fid].heid]; }
	//const he_t* heFromVert(size_t vid) const { return &halfedges[verts[vid].heid]; }
	//const vert_t* vertFromHe(size_t heid) const { return &verts[halfedges[heid].vid]; }
	const face_t* faceFromHe(size_t heid) const { return &faces[halfedges[heid].fid]; }



	//vector<vert_t> verts;
	vector<he_t>   halfedges;
	vector<face_t> faces;
};
