#pragma once
#include "Vector2.h"

namespace Utils
{

inline bool inCircle(const Vector2f& Va, const Vector2f& Vb, const Vector2f& Vc, const Vector2f& P)
{
	// Determinant representation of circle(A, B, C)
	//     | Ax    Ay    Ax^2+Ay^2    1 |
	//     | Bx    By    Bx^2+By^2    1 |
	//     | Cx    Cy    Cx^2+Cy^2    1 |
	//     | Px    Py    Px^2+Py^2    1 |
	// By reduction of the order of a determinant
	//     | Ax-Px    Ay-Py    A^2-P^2 |
	//     | Bx-Px    By-Py    B^2-P^2 |
	//     | Cx-Px    Cy-Py    C^2-P^2 |
	float pSq = P.lengthSquared();
	float Det[3][3]{
		Va.x - P.x, Va.y - P.y, Va.lengthSquared() - pSq,
		Vb.x - P.x, Vb.y - P.y, Vb.lengthSquared() - pSq,
		Vc.x - P.x, Vc.y - P.y, Vc.lengthSquared() - pSq,
	};

	float detResult =
		Det[0][0] * Det[1][1] * Det[2][2] +
		Det[0][1] * Det[1][2] * Det[2][0] +
		Det[0][2] * Det[1][0] * Det[2][1] -
		Det[0][2] * Det[1][1] * Det[2][0] -
		Det[0][1] * Det[1][0] * Det[2][2] -
		Det[0][0] * Det[1][2] * Det[2][1];

	return detResult > 0;
}

}
