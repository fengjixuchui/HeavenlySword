#ifndef _AINAVPREPROCESS_H
#define _AINAVPREPROCESS_H


class AINavPreprocess
{
public:
	static int TriangulatePolygon(int, int *, double (*)[2], int (*)[3], bool& );
	static int IsPointInsidePolygon(double *);
	static int GetNumOutputTris( int numPoints, int numHoles );
};

#endif // _AINAVPREPROCESS_H
