#pragma once

#include "Vector2.h"


class PolygonTriangulation
{
public:
	struct Edge
	{
		ptrdiff_t prev_offset, next_offset;
		size_t pid;
		size_t id;
	};
	struct MonotonePolygon
	{
		const Edge* mEdge = nullptr;
	};

	PolygonTriangulation(std::vector<Vector2f> &points);
	~PolygonTriangulation();

	void getLineIndexBuffer(std::vector<uint32_t> &indices) const;
	const Vector2f* vertexData() const { return mPoints.data(); }

private:
	std::vector<Vector2f> mPoints;
	std::vector<Edge> mEdges;
	std::vector<MonotonePolygon> mMonotones;
};

