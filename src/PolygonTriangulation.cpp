#include "PolygonTriangulation.h"

PolygonTriangulation::PolygonTriangulation(std::vector<Vector2f> &points)
	: mPoints(std::move(points))
{
}

PolygonTriangulation::~PolygonTriangulation()
{
}
void PolygonTriangulation::getLineIndexBuffer(std::vector<uint32_t> &indices) const
{

}