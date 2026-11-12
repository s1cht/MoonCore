#pragma once

// Constants
#define PI (3.14159265358979323846)

// Math functions

// Linear interpolation
#define LERP(a, b, t) ((a) + ((b) - (a)) * (t))

// To radians
#define RAD(x) ((x) * (PI / 180.0f))

// Sum of 2 squares
#define SQUARE_SUM_2(a, b)\
					(a*a + b*b)

// Sum of 3 squares
#define SQUARE_SUM_3(a, b, c)\
					(a*a + b*b + c*c)

// Sum of 4 squares
#define SQUARE_SUM_4(a, b, c, d)\
					(a*a + b*b + c*c + d*d)

// Sum of 5 squares
#define SQUARE_SUM_5(a, b, c, d, e)	\
					(a*a + b*b + c*c + d*d + e*e)

// Determinant of the 2x2 matrix
#define MAT2x2_DET(a, b, c, d) ((a) * (d) - (b) * (c))

// Determinant of the 3x3 matrix
#define MAT3x3_DET(a11, a12, a13, a21, a22, a23, a31, a32, a33) \
    (a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 - a13 * a22 * a31 - a12 * a21 * a33 - a11 * a23 * a32)