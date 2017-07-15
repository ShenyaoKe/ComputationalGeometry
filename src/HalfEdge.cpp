#include "HalfEdge.h"

namespace HDS
{

size_t Vertex::uid = 0;
size_t HalfEdge::uid = 0;
size_t Face::uid = 0;

void Mesh::insertNewVertexOnEdge(HalfEdge*& outNewHE,
								 Face*& outNewFace,
								 size_t vId,
								 size_t heId)
{
	size_t newHeId = halfedges.size();
	size_t newFaceId = faces.size();

	halfedges.resize(newHeId + 6);
	faces.resize(newFaceId + 2);

	outNewHE = &halfedges[newHeId];
	outNewFace = &faces[newFaceId];

	// Assume mesh is triangulated
	HalfEdge& he = halfedges[heId];
	HalfEdge& hef = *he.flip();

	HalfEdge& oriHe0 = *he.next();
	HalfEdge& oriHe1 = *oriHe0.next();
	HalfEdge& oriHe2 = *hef.next();
	HalfEdge& oriHe3 = *oriHe2.next();

	outNewHE[0].vid = oriHe1.vid;
	outNewHE[2].vid = oriHe2.vid;
	outNewHE[4].vid = oriHe3.vid;
	// update he
	he.vid = outNewHE[1].vid = outNewHE[3].vid = outNewHE[5].vid = vId;

	outNewHE[0].setFlip(outNewHE + 1);
	outNewHE[2].setFlip(outNewHE + 3);
	outNewHE[4].setFlip(outNewHE + 5);

	Face& face0 = faces[he.fid];
	Face& face1 = faces[newFaceId];
	Face& face2 = faces[newFaceId + 1];
	Face& face3 = faces[hef.fid];

	constructFace(oriHe0, outNewHE[0], he, face0);
	constructFace(oriHe1, outNewHE[2], outNewHE[1], face1);
	constructFace(oriHe2, outNewHE[4], outNewHE[3], face2);
	constructFace(oriHe3, hef, outNewHE[5], face3);
}

void Mesh::insertNewVertexInFace(HalfEdge*& outNewHE,
								 Face*& outNewFace,
								 size_t vId,
								 size_t fId)
{
	size_t newEdgeId = halfedges.size();
	size_t newFaceId = faces.size();

	halfedges.resize(newEdgeId + 6);
	faces.resize(newFaceId + 2);

	outNewHE = &halfedges[newEdgeId];
	outNewFace = &faces[newFaceId];

	HalfEdge& oriHe0 = *heFromFace(fId);
	HalfEdge& oriHe1 = *oriHe0.next();
	HalfEdge& oriHe2 = *oriHe1.next();

	// Link face loop
	outNewHE[1].vid = oriHe1.vid;
	outNewHE[3].vid = oriHe2.vid;
	outNewHE[5].vid = oriHe0.vid;
	outNewHE[0].vid = outNewHE[2].vid = outNewHE[4].vid = vId;

	constructFace(oriHe0, outNewHE[1], outNewHE[0], faces[fId]);
	constructFace(oriHe1, outNewHE[3], outNewHE[2], faces[newFaceId]);
	constructFace(oriHe2, outNewHE[5], outNewHE[4], faces[newFaceId + 1]);

	outNewHE[1].setFlip(outNewHE + 2);
	outNewHE[3].setFlip(outNewHE + 4);
	outNewHE[5].setFlip(outNewHE);
}

}
