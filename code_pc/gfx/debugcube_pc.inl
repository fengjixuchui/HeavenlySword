// a cube mesh
static const float g_afCubePositions[] = 
{
	-1.0f, -1.0f, -1.0f, 0.0f, 
	 1.0f, -1.0f, -1.0f, 0.0f, 
	-1.0f,  1.0f, -1.0f, 0.0f, 
	 1.0f,  1.0f, -1.0f, 0.0f, 
	-1.0f, -1.0f,  1.0f, 0.0f, 
	 1.0f, -1.0f,  1.0f, 0.0f, 
	-1.0f,  1.0f,  1.0f, 0.0f, 
	 1.0f,  1.0f,  1.0f, 0.0f
};
static const u_short g_ausCubeIndices[] = 
{
	0, 1, 3,  0, 3, 2, 
	1, 5, 7,  1, 7, 3, 
	5, 4, 6,  5, 6, 7, 
	4, 0, 2,  4, 2, 6, 
	2, 3, 7,  2, 7, 6, 
	4, 5, 1,  4, 1, 0
};
