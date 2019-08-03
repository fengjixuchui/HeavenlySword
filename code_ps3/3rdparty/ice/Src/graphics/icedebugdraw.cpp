/*
* Copyright (c) 2005 Naughty Dog, Inc.
* A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
* Use and distribution without consent strictly prohibited
*/

#include "icedebugdraw.h"
#include "icegraphics.h"
#include "icefx.h"
#include "iceeffects.h"
#include "icetextures.h"

namespace Ice
{
	namespace DebugDraw3d
	{
		using namespace Render;
		using namespace Graphics;
		using namespace Bucketer;

		float const kPi = 3.1415927f;
		// Number of segments per 180 degrees / kPi radians (must be >= 2)
		int const kSphereTesselation = 6;
		int const kConeTesselation = 6;
		float const kSphereTesselationInv = 1.0f/kSphereTesselation;
		float const kConeTesselationInv = 1.0f/kConeTesselation;

		enum ObjectType
		{
			kObjectBox = 0,
			kObjectSphere,
			kObjectCone,
			kObjectTexturedBox,
			kObjectTexturedSphere,
			kObjectTexturedCone,

			kNumObjectTypes,
			kObjectTextured = kObjectTexturedBox,
		};

		struct ObjectDef
		{
			void *pVB_XYZ;
			void *pVB_ST;
			void *pIB;
			U32 numVertices;
			U32 numIndices;
		};

		// debug draw 3d helper functions
		static void CreateBoxObjectDef(ObjectDef &obj);
		static void CreateSphereObjectDef(ObjectDef &obj);
		static void CreateConeObjectDef(ObjectDef &obj);
		static void CreateTexturedBoxObjectDef(ObjectDef &obj);
		static void CreateTexturedSphereObjectDef(ObjectDef &obj);
		static void CreateTexturedConeObjectDef(ObjectDef &obj);

		static void SetWorldTransform(Transform const &transform, Point_arg origin, Vector scale);
		static void DrawObject( ObjectType eType, Vector color, Texture *pTexture );

		// debug draw 3d resources
		static VertexProgram *s_pTintedVS = NULL;
		static FragmentProgram *s_pTintedPS = NULL;
		static VertexProgram *s_pTintedTextureVS = NULL;
		static FragmentProgram *s_pTintedTexturePS = NULL;
		static ObjectDef s_aObjectDef[kNumObjectTypes] = { { NULL, NULL, NULL, 0, 0, }, };

		// current debug draw 3d state
		static Texture *s_pTexture = NULL;
		static VertexProgram *s_pVS = NULL;
		static FragmentProgram *s_pPS = NULL;

		//----------------------------------------------------------------------------
		// 3D object rendering functions

		// Create the base procedural objects and shaders used by the debug draw system
		void Initialize()
		{
			CreateBoxObjectDef(s_aObjectDef[kObjectBox]);
			CreateSphereObjectDef(s_aObjectDef[kObjectSphere]);
			CreateConeObjectDef(s_aObjectDef[kObjectCone]);
			CreateTexturedBoxObjectDef(s_aObjectDef[kObjectTexturedBox]);
			CreateTexturedSphereObjectDef(s_aObjectDef[kObjectTexturedSphere]);
			CreateTexturedConeObjectDef(s_aObjectDef[kObjectTexturedCone]);

			s_pTintedTextureVS = LoadVertexProgram("shaders/tintedTexture.vbin");
			s_pTintedTexturePS = LoadFragmentProgram("shaders/tintedTexture.fbin");
			s_pTintedVS = LoadVertexProgram("shaders/positionOnly.vbin");
			s_pTintedPS = LoadFragmentProgram("shaders/colorOnly.fbin");
		}

		// Deallocate the buffers and shaders used by the debug draw system
		void Shutdown()
		{
			for (int i = 0; i < kNumObjectTypes; ++i) {
				if (s_aObjectDef[i].pVB_XYZ) {
					ReleaseLinearVideoMemory(s_aObjectDef[i].pVB_XYZ), s_aObjectDef[i].pVB_XYZ = NULL;
				}
				if (s_aObjectDef[i].pVB_ST) {
					ReleaseLinearVideoMemory(s_aObjectDef[i].pVB_ST), s_aObjectDef[i].pVB_ST = NULL;
				}
				if (s_aObjectDef[i].pIB) {
					ReleaseLinearVideoMemory(s_aObjectDef[i].pIB), s_aObjectDef[i].pIB = NULL;
				}
			}
			delete s_pTintedVS, s_pTintedVS = NULL;
			delete s_pTintedPS, s_pTintedPS = NULL;
			delete s_pTintedTextureVS, s_pTintedTextureVS = NULL;
			delete s_pTintedTexturePS, s_pTintedTexturePS = NULL;
		}

		// Set the debug draw texture.
		// If pTexture is NULL, use untextured rendering.
		void SetTexture( Texture *pTexture )
		{
			s_pTexture = pTexture;
		}

		// Draw a world aligned box
		void DrawBox( const Point_arg position_center, Vector size, Vector color )
		{
			SetWorldTransform(kIdentity, position_center, size);
			DrawObject(kObjectBox, color, s_pTexture);
		}

		// Draw a box at the origin in the local space defined by transform
		void DrawBox( Transform const &transform, Vector size, Vector color )
		{
			Point origin = Point(0.0f, 0.0f, 0.0f);
			SetWorldTransform(transform, origin, size);
			DrawObject(kObjectBox, color, s_pTexture);
		}

		// Draw a box in the local space defined by transform
		void DrawBox( Transform const &transform, const Point_arg position_center, Vector size, Vector color )
		{
			SetWorldTransform(transform, position_center, size);
			DrawObject(kObjectBox, color, s_pTexture);
		}

		// Draw a world aligned sphere
		void DrawSphere( const Point_arg position_center, float radius, Vector color )
		{
			Vector scale = Vector(radius, radius, radius);
			SetWorldTransform(kIdentity, position_center, scale);
			DrawObject(kObjectSphere, color, s_pTexture);
		}

		// Draw a sphere at the origin in the local space defined by transform
		void DrawSphere( Transform const &transform, float radius, Vector color )
		{
			Vector scale = Vector(radius, radius, radius);
			Point origin = Point(0.0f, 0.0f, 0.0f);
			SetWorldTransform(transform, origin, scale);
			DrawObject(kObjectSphere, color, s_pTexture);
		}

		// Draw a box in the local space defined by transform
		void DrawSphere( Transform const &transform, const Point_arg position_center, float radius, Vector color )
		{
			Vector scale = Vector(radius, radius, radius);
			SetWorldTransform(transform, position_center, scale);
			DrawObject(kObjectSphere, color, s_pTexture);
		}

		// Draw a cone with the apex at position_apex and with the axis aligned point "up" with world y
		void DrawCone( const Point_arg position_apex, float radius_base, float height, Vector color )
		{
			Vector scale = Vector(radius_base, height, radius_base);
			SetWorldTransform(kIdentity, position_apex, scale);
			DrawObject(kObjectCone, color, s_pTexture);
		}

		// Draw a cone with the apex at the origin in the local space defined by transform
		void DrawCone( Transform const &transform, float radius_base, float height, Vector color )
		{
			Vector scale = Vector(radius_base, height, radius_base);
			Point origin = Point(0.0f, 0.0f, 0.0f);
			SetWorldTransform(transform, origin, scale);
			DrawObject(kObjectCone, color, s_pTexture);
		}

		// Draw a cone in the local space defined by transform
		void DrawCone( Transform const &transform, const Point_arg position_apex, float radius_base, float height, Vector color )
		{
			Vector scale = Vector(radius_base, height, radius_base);
			SetWorldTransform(transform, position_apex, scale);
			DrawObject(kObjectCone, color, s_pTexture);
		}

		//----------------------------------------------------------------------------
		// 3D object rendering helper functions

		static void SetWorldTransform(Transform const &transform, Point_arg origin, Vector scale)
		{
			Transform worldTransform( transform );
			worldTransform.SetTranslation( origin * transform );
			worldTransform.SetXAxis( worldTransform.GetXAxis() * scale.X() );
			worldTransform.SetYAxis( worldTransform.GetYAxis() * scale.Y() );
			worldTransform.SetZAxis( worldTransform.GetZAxis() * scale.Z() );

			Mat44 worldMatrix( worldTransform.GetMat44() );
			Mat44 mvpMatrix;
			mvpMatrix = worldMatrix * g_viewProjectionMatrix;
			mvpMatrix = Transpose(mvpMatrix);
			SetVertexProgramConstants(0, 4, (float *)&mvpMatrix);
		}

		static void DrawObject( ObjectType eType, Vector color, Texture *pTexture )
		{
			// Disable all vertex attributes not used here.
			for (U32F iAttrib = 1; iAttrib < 7; iAttrib++) {
				DisableVertexAttribArray(iAttrib);
			}
			for (U32F iAttrib = 9; iAttrib < 16; iAttrib++) {
				DisableVertexAttribArray(iAttrib);
			}

			if (pTexture) {
				if (eType < kObjectTextured) {
					eType = (ObjectType)((U32)eType + kObjectTextured);
				}
				s_pVS = s_pTintedTextureVS;
				s_pPS = s_pTintedTexturePS;

				SetVertexAttribFormat(0, kAttribFloat, kAttribCount3, 12);
				SetVertexAttribFormat(8, kAttribFloat, kAttribCount2, 8);
				SetVertexAttribPointer(0, TranslateAddressToOffset(s_aObjectDef[eType].pVB_XYZ), kAttribVideoMemory);
				SetVertexAttribPointer(8, TranslateAddressToOffset(s_aObjectDef[eType].pVB_ST), kAttribVideoMemory);
				SetTexture(0, pTexture);
			} else {
				if (eType >= kObjectTextured) {
					eType = (ObjectType)((U32)eType % kObjectTextured);
				}
				s_pVS = s_pTintedVS;
				s_pPS = s_pTintedPS;

				SetVertexAttribFormat(0, kAttribFloat, kAttribCount3, 12);
				DisableVertexAttribArray(8);
				SetVertexAttribPointer(0, TranslateAddressToOffset(s_aObjectDef[eType].pVB_XYZ), kAttribVideoMemory);
			}

			SetVertexProgram(s_pVS);
			SetFragmentProgram(s_pPS);

			DisableRenderState(kRenderBlend);
			DisableRenderState(kRenderCullFace);

			SetVertexProgramConstant( 4, color.X(), color.Y(), color.Z(), 1.0f );
			DrawElements( kDrawTriangles, 0, s_aObjectDef[eType].numIndices, kIndex16, TranslateAddressToOffset(s_aObjectDef[eType].pIB), kIndexVideoMemory);
		}

		//----------------------------------------------------------------------------
		// 3D object def creation functions

		static inline void AddVertex(F32* &pDst, F32 x, F32 y, F32 z)
		{
			*pDst++ = x;
			*pDst++ = y;
			*pDst++ = z;
		}

		static inline void AddTexCoord(F32* &pDst, F32 s, F32 t)
		{
			*pDst++ = s;
			*pDst++ = t;
		}

		static void CreateBoxObjectDef(ObjectDef &obj)
		{
			obj.numVertices = 8;
			obj.numIndices = (6 * 2) * 3;
			obj.pVB_XYZ = AllocateLinearVideoMemory( obj.numVertices * 3 * sizeof(F32) );
			obj.pVB_ST = NULL;
			obj.pIB = AllocateLinearVideoMemory( obj.numIndices * sizeof(U16) );

			F32 *pDst_XYZ = (F32*)obj.pVB_XYZ;
			U16 *pDst = (U16*)obj.pIB;

			const F32 sizeX = 0.5f, sizeY = 0.5f, sizeZ = 0.5f;

			AddVertex(pDst_XYZ, 1.0f * sizeX, 1.0f * sizeY, 1.0 * sizeZ);
			AddVertex(pDst_XYZ,-1.0f * sizeX, 1.0f * sizeY, 1.0 * sizeZ);
			AddVertex(pDst_XYZ, 1.0f * sizeX, 1.0f * sizeY,-1.0 * sizeZ);
			AddVertex(pDst_XYZ,-1.0f * sizeX, 1.0f * sizeY,-1.0 * sizeZ);
			AddVertex(pDst_XYZ, 1.0f * sizeX,-1.0f * sizeY, 1.0 * sizeZ);
			AddVertex(pDst_XYZ,-1.0f * sizeX,-1.0f * sizeY, 1.0 * sizeZ);
			AddVertex(pDst_XYZ, 1.0f * sizeX,-1.0f * sizeY,-1.0 * sizeZ);
			AddVertex(pDst_XYZ,-1.0f * sizeX,-1.0f * sizeY,-1.0 * sizeZ);

			// Back
			*pDst++ = 0;	*pDst++ = 4;	*pDst++ = 5;
			*pDst++ = 0;	*pDst++ = 5;	*pDst++ = 1;
			// Left
			*pDst++ = 1;	*pDst++ = 3;	*pDst++ = 5;
			*pDst++ = 3;	*pDst++ = 7;	*pDst++ = 5;
			// Front
			*pDst++ = 2;	*pDst++ = 6;	*pDst++ = 3;
			*pDst++ = 3;	*pDst++ = 6;	*pDst++ = 7;
			// Right
			*pDst++ = 0;	*pDst++ = 4;	*pDst++ = 2;
			*pDst++ = 2;	*pDst++ = 4;	*pDst++ = 6;
			// Top
			*pDst++ = 0;	*pDst++ = 2;	*pDst++ = 1;
			*pDst++ = 1;	*pDst++ = 2;	*pDst++ = 3;
			// Bottom
			*pDst++ = 4;	*pDst++ = 5;	*pDst++ = 6;
			*pDst++ = 5;	*pDst++ = 7;	*pDst++ = 6;
		}

		static void CreateSphereObjectDef(ObjectDef &obj)
		{
			obj.numVertices = (kSphereTesselation*2)*(kSphereTesselation - 1) + 2;
			obj.numIndices = (kSphereTesselation*(kSphereTesselation-1)*4) * 3;
			obj.pVB_XYZ = AllocateLinearVideoMemory( obj.numVertices * 3 * sizeof(F32) );
			obj.pVB_ST = NULL;
			obj.pIB = AllocateLinearVideoMemory( obj.numIndices * sizeof(U16) );

			F32 *pDst_XYZ = (F32*)obj.pVB_XYZ;
			U16 *pDst = (U16*)obj.pIB;

			static const F32 radius = 1.0f;

			U16 iLat, iLong;
			// Add bottom vertex
			AddVertex(pDst_XYZ, 0.0f,-1.0, 0.0f);
			// Add center latitude rings
			for (iLat = 1; iLat < kSphereTesselation; ++iLat) {
				F32 fLat = (iLat*kSphereTesselationInv - 0.5f) * kPi;
				F32 fRho = cosf(fLat) * radius;
				F32 fY = sinf(fLat) * radius;
				for (iLong = 0; iLong < kSphereTesselation*2; ++iLong) {
					F32 fLong = iLong*kSphereTesselationInv * kPi;
					F32 fX = cosf(fLong) * fRho;
					F32 fZ = sinf(fLong) * fRho;

					AddVertex(pDst_XYZ, fX, fY, fZ);
				}
			}
			// Add top vertex
			AddVertex(pDst_XYZ, 0.0f, 1.0, 0.0f);

			// Add bottom disc
			static const U16 BOTTOM_VERTEX = 0;
			static const U16 FIRST_RING = 1;
			for (iLong = 0; iLong < kSphereTesselation*2-1; ++iLong) {
				*pDst++ = BOTTOM_VERTEX;			*pDst++ = FIRST_RING + iLong;		*pDst++ = FIRST_RING + iLong + 1;
			}
			*pDst++ = BOTTOM_VERTEX;			*pDst++ = FIRST_RING + iLong;		*pDst++ = FIRST_RING + 0;
			// Add center latitude rings
			for (iLat = 1; iLat < kSphereTesselation-1; ++iLat) {
				const U16 THIS_RING = FIRST_RING + (iLat-1) * (kSphereTesselation*2);
				const U16 NEXT_RING = THIS_RING + (kSphereTesselation*2);
				for (iLong = 0; iLong < kSphereTesselation*2-1; ++iLong) {
					*pDst++ = THIS_RING + iLong;		*pDst++ = NEXT_RING + iLong;		*pDst++ = NEXT_RING + iLong + 1;
					*pDst++ = THIS_RING + iLong + 1;	*pDst++ = THIS_RING + iLong;		*pDst++ = NEXT_RING + iLong + 1;
				}
				*pDst++ = THIS_RING + iLong;		*pDst++ = NEXT_RING + iLong;		*pDst++ = NEXT_RING + 0;
				*pDst++ = THIS_RING + 0;			*pDst++ = THIS_RING + iLong;		*pDst++ = NEXT_RING + 0;
			}
			// Add top disc
			static const U16 LAST_RING = FIRST_RING + (kSphereTesselation-2) * (kSphereTesselation*2);
			static const U16 TOP_VERTEX = LAST_RING + (kSphereTesselation*2);
			for (iLong = 0; iLong < kSphereTesselation*2-1; ++iLong) {
				*pDst++ = TOP_VERTEX;				*pDst++ = LAST_RING + iLong + 1;	*pDst++ = LAST_RING + iLong;
			}
			*pDst++ = TOP_VERTEX;				*pDst++ = LAST_RING + 0;			*pDst++ = LAST_RING + iLong;
		}

		static void CreateConeObjectDef(ObjectDef &obj)
		{
			obj.numVertices = kConeTesselation*2 + 1;
			obj.numIndices = (kConeTesselation*4 - 2) * 3;
			obj.pVB_XYZ = AllocateLinearVideoMemory( obj.numVertices * 3 * sizeof(F32) );
			obj.pVB_ST = NULL;
			obj.pIB = AllocateLinearVideoMemory( obj.numIndices * sizeof(U16) );

			F32 *pDst_XYZ = (F32*)obj.pVB_XYZ;
			U16 *pDst = (U16*)obj.pIB;

			static const F32 radius_base = 1.0f;
			static const F32 height = 1.0f;

			U16 iLong;
			// edge of base vertices
			for (iLong = 0; iLong < kConeTesselation*2; ++iLong) {
				F32 fLong = (iLong * kConeTesselationInv) * kPi;
				F32 fX = radius_base * cosf(fLong);
				F32 fZ = radius_base * sinf(fLong);
				AddVertex(pDst_XYZ, fX, -height, fZ);
			}
			// apex vertex
			AddVertex(pDst_XYZ, 0.0f, 0.0f, 0.0f);

			// base
			static const U16 BASE_EDGE = 0;
			for (iLong = 0; iLong < kConeTesselation*2 - 2; ++iLong) {
				*pDst++ = BASE_EDGE;		*pDst++ = BASE_EDGE + iLong + 1;	*pDst++ = BASE_EDGE + iLong + 2;
			}
			// cone
			static const U16 APEX_VERTEX = BASE_EDGE + (kConeTesselation*2);
			for (iLong = 0; iLong < kConeTesselation*2-1; ++iLong) {
				*pDst++ = APEX_VERTEX;		*pDst++ = BASE_EDGE + iLong + 1;	*pDst++ = BASE_EDGE + iLong;
			}
			*pDst++ = APEX_VERTEX;		*pDst++ = BASE_EDGE + 0;			*pDst++ = BASE_EDGE + iLong;
		}

		static void CreateTexturedBoxObjectDef(ObjectDef &obj)
		{
			obj.numVertices = 18;
			obj.numIndices = 16 * 3;
			obj.pVB_XYZ = AllocateLinearVideoMemory( obj.numVertices * 3 * sizeof(F32) );
			obj.pVB_ST = AllocateLinearVideoMemory( obj.numVertices * 2 * sizeof(F32) );
			obj.pIB = AllocateLinearVideoMemory( obj.numIndices * sizeof(U16) );

			F32 *pDst_XYZ = (F32*)obj.pVB_XYZ;
			F32 *pDst_ST = (F32*)obj.pVB_ST;
			U16 *pDst = (U16*)obj.pIB;

			const F32 sizeX = 0.5f, sizeY = 0.5f, sizeZ = 0.5f;
			const F32 T0 = 0.00f;	//center of top
			const F32 T1 = 0.25f;	//top corners - or 0.6959f to map by latitude at corner rather than edge centers
			const F32 T2 = 0.75f;	//bottom corners - or 0.3041f to map by latitude at corner rather than edge centers
			const F32 T3 = 1.00f;	//center of bottom

			obj.numVertices = 18;
			// normal cube vertices
			AddVertex(pDst_XYZ, 1.0f * sizeX, 1.0f * sizeY, 1.0 * sizeZ);	AddTexCoord(pDst_ST, 1.0f/8.0f, T1);
			AddVertex(pDst_XYZ,-1.0f * sizeX, 1.0f * sizeY, 1.0 * sizeZ);	AddTexCoord(pDst_ST, 3.0f/8.0f, T1);
			AddVertex(pDst_XYZ, 1.0f * sizeX, 1.0f * sizeY,-1.0 * sizeZ);	AddTexCoord(pDst_ST, 7.0f/8.0f, T1);
			AddVertex(pDst_XYZ,-1.0f * sizeX, 1.0f * sizeY,-1.0 * sizeZ);	AddTexCoord(pDst_ST, 5.0f/8.0f, T1);
			AddVertex(pDst_XYZ, 1.0f * sizeX,-1.0f * sizeY, 1.0 * sizeZ);	AddTexCoord(pDst_ST, 1.0f/8.0f, T2);
			AddVertex(pDst_XYZ,-1.0f * sizeX,-1.0f * sizeY, 1.0 * sizeZ);	AddTexCoord(pDst_ST, 3.0f/8.0f, T2);
			AddVertex(pDst_XYZ, 1.0f * sizeX,-1.0f * sizeY,-1.0 * sizeZ);	AddTexCoord(pDst_ST, 7.0f/8.0f, T2);
			AddVertex(pDst_XYZ,-1.0f * sizeX,-1.0f * sizeY,-1.0 * sizeZ);	AddTexCoord(pDst_ST, 5.0f/8.0f, T2);
			// center of top and bottom face
			AddVertex(pDst_XYZ, 0.0f * sizeX, 1.0f * sizeY, 0.0 * sizeZ);	AddTexCoord(pDst_ST, 2.0f/8.0f, T0);
			AddVertex(pDst_XYZ, 0.0f * sizeX, 1.0f * sizeY, 0.0 * sizeZ);	AddTexCoord(pDst_ST, 4.0f/8.0f, T0);
			AddVertex(pDst_XYZ, 0.0f * sizeX, 1.0f * sizeY, 0.0 * sizeZ);	AddTexCoord(pDst_ST, 8.0f/8.0f, T0);
			AddVertex(pDst_XYZ, 0.0f * sizeX, 1.0f * sizeY, 0.0 * sizeZ);	AddTexCoord(pDst_ST, 6.0f/8.0f, T0);
			AddVertex(pDst_XYZ, 0.0f * sizeX,-1.0f * sizeY, 0.0 * sizeZ);	AddTexCoord(pDst_ST, 2.0f/8.0f, T3);
			AddVertex(pDst_XYZ, 0.0f * sizeX,-1.0f * sizeY, 0.0 * sizeZ);	AddTexCoord(pDst_ST, 4.0f/8.0f, T3);
			AddVertex(pDst_XYZ, 0.0f * sizeX,-1.0f * sizeY, 0.0 * sizeZ);	AddTexCoord(pDst_ST, 8.0f/8.0f, T3);
			AddVertex(pDst_XYZ, 0.0f * sizeX,-1.0f * sizeY, 0.0 * sizeZ);	AddTexCoord(pDst_ST, 6.0f/8.0f, T3);
			// extra copy of vertices 0, 4 with wrapped S texture coordinate
			AddVertex(pDst_XYZ, 1.0f * sizeX, 1.0f * sizeY, 1.0 * sizeZ);	AddTexCoord(pDst_ST, 9.0f/8.0f, T1);
			AddVertex(pDst_XYZ, 1.0f * sizeX,-1.0f * sizeY, 1.0 * sizeZ);	AddTexCoord(pDst_ST, 9.0f/8.0f, T2);

			obj.numIndices = 16 * 3;
			// Back
			*pDst++ = 0;	*pDst++ = 4;	*pDst++ = 5;
			*pDst++ = 0;	*pDst++ = 5;	*pDst++ = 1;
			// Left
			*pDst++ = 1;	*pDst++ = 3;	*pDst++ = 5;
			*pDst++ = 3;	*pDst++ = 7;	*pDst++ = 5;
			// Front
			*pDst++ = 2;	*pDst++ = 6;	*pDst++ = 3;
			*pDst++ = 3;	*pDst++ = 6;	*pDst++ = 7;
			// Right
			*pDst++ = 16;	*pDst++ = 17;	*pDst++ = 2;
			*pDst++ = 2;	*pDst++ = 17;	*pDst++ = 6;
			// Top
			*pDst++ = 0;	*pDst++ = 8;	*pDst++ = 1;
			*pDst++ = 1;	*pDst++ = 9;	*pDst++ = 3;
			*pDst++ = 3;	*pDst++ = 11;	*pDst++ = 2;
			*pDst++ = 2;	*pDst++ = 10;	*pDst++ = 16;
			// Bottom
			*pDst++ = 4;	*pDst++ = 5;	*pDst++ = 12;
			*pDst++ = 5;	*pDst++ = 7;	*pDst++ = 13;
			*pDst++ = 7;	*pDst++ = 6;	*pDst++ = 15;
			*pDst++ = 6;	*pDst++ = 17;	*pDst++ = 14;
		}

		static void CreateTexturedSphereObjectDef(ObjectDef &obj)
		{
			obj.numVertices= (kSphereTesselation*2 + 1)*(kSphereTesselation - 1) + kSphereTesselation*2*2;
			obj.numIndices = (kSphereTesselation*(kSphereTesselation-1)*4) * 3;
			obj.pVB_XYZ = AllocateLinearVideoMemory( obj.numVertices * 3 * sizeof(F32) );
			obj.pVB_ST = AllocateLinearVideoMemory( obj.numVertices * 2 * sizeof(F32) );
			obj.pIB = AllocateLinearVideoMemory( obj.numIndices * sizeof(U16) );

			F32 *pDst_XYZ = (F32*)obj.pVB_XYZ;
			F32 *pDst_ST = (F32*)obj.pVB_ST;
			U16 *pDst = (U16*)obj.pIB;

			static const F32 radius = 1.0f;

			U16 iLat, iLong;
			// Add bottom vertex, multiple copies to converge tex coords
			for (iLong = 0; iLong < kSphereTesselation*2; ++iLong) {
				F32 fS = (iLong + 0.5f) * 0.5f * kSphereTesselationInv;
				AddVertex(pDst_XYZ, 0.0f,-1.0, 0.0f);	AddTexCoord(pDst_ST, fS, 1.0f);
			}
			// Add center latitude rings
			for (iLat = 1; iLat < kSphereTesselation; ++iLat) {
				F32 fLat = (iLat*kSphereTesselationInv - 0.5f) * kPi;
				F32 fRho = cosf(fLat) * radius;
				F32 fY = sinf(fLat) * radius;
				F32 fT = 1.0f - (iLat * kSphereTesselationInv);
				for (iLong = 0; iLong <= kSphereTesselation*2; ++iLong) {
					F32 fLong = iLong*kSphereTesselationInv * kPi;
					F32 fX = cosf(fLong) * fRho;
					F32 fZ = sinf(fLong) * fRho;
					F32 fS = iLong * 0.5f * kSphereTesselationInv;

					AddVertex(pDst_XYZ, fX, fY, fZ);	AddTexCoord(pDst_ST, fS, fT);
				}
			}
			// Add top vertex, multiple copies to converge tex coords
			for (iLong = 0; iLong < kSphereTesselation*2; ++iLong) {
				F32 fS = (iLong + 0.5f) * 0.5f * kSphereTesselationInv;
				AddVertex(pDst_XYZ, 0.0f, 1.0, 0.0f);	AddTexCoord(pDst_ST, fS, 0.0f);
			}

			// Add bottom disc
			static const U16 BOTTOM_VERTEX = 0;
			static const U16 FIRST_RING = kSphereTesselation*2;
			for (iLong = 0; iLong < kSphereTesselation*2; ++iLong) {
				*pDst++ = BOTTOM_VERTEX + iLong;	*pDst++ = FIRST_RING + iLong;		*pDst++ = FIRST_RING + iLong + 1;
			}
			// Add center latitude rings
			for (iLat = 1; iLat < kSphereTesselation-1; ++iLat) {
				const U16 THIS_RING = FIRST_RING + (iLat-1) * (kSphereTesselation*2 + 1);
				const U16 NEXT_RING = THIS_RING + (kSphereTesselation*2 + 1);
				for (iLong = 0; iLong < kSphereTesselation*2; ++iLong) {
					*pDst++ = THIS_RING + iLong;		*pDst++ = NEXT_RING + iLong;		*pDst++ = NEXT_RING + iLong + 1;
					*pDst++ = THIS_RING + iLong + 1;	*pDst++ = THIS_RING + iLong;		*pDst++ = NEXT_RING + iLong + 1;
				}
			}
			// Add top disc
			static const U16 LAST_RING = FIRST_RING + (kSphereTesselation-2) * (kSphereTesselation*2 + 1);
			static const U16 TOP_VERTEX = LAST_RING + (kSphereTesselation*2 + 1);
			for (iLong = 0; iLong < kSphereTesselation*2; ++iLong) {
				*pDst++ = TOP_VERTEX + iLong;		*pDst++ = LAST_RING + iLong + 1;	*pDst++ = LAST_RING + iLong;
			}
		}

		static void CreateTexturedConeObjectDef(ObjectDef &obj)
		{
			obj.numVertices = kConeTesselation*2*2 + (kConeTesselation*2+1);
			obj.numIndices = (kConeTesselation*2*2) * 3;
			obj.pVB_XYZ = AllocateLinearVideoMemory( obj.numVertices * 3 * sizeof(F32) );
			obj.pVB_ST = AllocateLinearVideoMemory( obj.numVertices * 2 * sizeof(F32) );
			obj.pIB = AllocateLinearVideoMemory( obj.numIndices * sizeof(U16) );

			F32 *pDst_XYZ = (F32*)obj.pVB_XYZ;
			F32 *pDst_ST = (F32*)obj.pVB_ST;
			U16 *pDst = (U16*)obj.pIB;

			static const F32 radius_base = 1.0f;
			static const F32 height = 1.0f;
			const F32 T_EDGE = 0.66666667f;	// t coord at edge of base

			U16 iLong;
			// center of base vertices
			for (iLong = 0; iLong < kConeTesselation*2; ++iLong) {
				F32 fS = (iLong + 0.5f) * 0.5f * kConeTesselationInv;
				AddVertex(pDst_XYZ, 0.0f, -height, 0.0f);	AddTexCoord(pDst_ST, fS, 1.0f);
			}
			// edge of base vertices
			for (iLong = 0; iLong <= kConeTesselation*2; ++iLong) {
				F32 fLong = (iLong * kConeTesselationInv) * kPi;
				F32 fX = radius_base * cosf(fLong);
				F32 fZ = radius_base * sinf(fLong);
				F32 fS = iLong * 0.5f * kConeTesselationInv;
				AddVertex(pDst_XYZ, fX, -height, fZ);		AddTexCoord(pDst_ST, fS, T_EDGE);
			}
			// apex vertices
			for (iLong = 0; iLong < kConeTesselation*2; ++iLong) {
				F32 fS = (iLong + 0.5f) * 0.5f * kConeTesselationInv;
				AddVertex(pDst_XYZ, 0.0f, 0.0f, 0.0f);		AddTexCoord(pDst_ST, fS, 0.0f);
			}

			// base
			static const U16 BASE_VERTEX = 0;
			static const U16 BASE_EDGE = kConeTesselation*2;
			for (iLong = 0; iLong < kConeTesselation*2; ++iLong) {
				*pDst++ = BASE_VERTEX + iLong;		*pDst++ = BASE_EDGE + iLong;		*pDst++ = BASE_EDGE + iLong + 1;
			}
			// cone
			static const U16 APEX_VERTEX = BASE_EDGE + (kConeTesselation*2+1);
			for (iLong = 0; iLong < kConeTesselation*2; ++iLong) {
				*pDst++ = APEX_VERTEX + iLong;		*pDst++ = BASE_EDGE + iLong + 1;	*pDst++ = BASE_EDGE + iLong;
			}
		}
	} // namespace DebugDraw3d

	namespace DebugDraw2d
	{
		using namespace Render;
		using namespace Graphics;

		unsigned const kCharsPerRow = 14;	// fit 14 characters in one row of the texture
		float const kCharSizeU =  kFontWidthPixels/128.0f;		// kFontWidthPixels pixels per character in a 128x128 texture
		float const kCharSizeV =-kFontHeightPixels/128.0f;		// top of the texture is at 1.0, bottom is at 0.0, each char is kFontHeightPixels pixels high.

		float const kColSize = kSafeScreenWidth * 2 / (float)kNumTextCols;
		float const kRowSize = kSafeScreenHeight * 2 / (float)kNumTextRows;

		struct TexturedVertex2d
		{
			float x, y, z, u, v;
			RgbaColor color;
		};
		struct Vertex2d
		{
			float x, y, z;
			RgbaColor color;
		};

		// debug draw 2d helper functions
		static void SetVertex(Vertex2d &vertex, float x, float y, RgbaColor color);
		static void SetVertex(TexturedVertex2d &vertex, float x, float y, float u, float v, RgbaColor color);
		// debug draw 2d resources
		static Texture *s_pFontTexture = NULL;
		static VertexProgram *s_pColor2dVS = NULL;
		static FragmentProgram *s_pColor2dPS = NULL;
		static VertexProgram *s_pColorTexture2dVS = NULL;
		static FragmentProgram *s_pColorTexture2dPS = NULL;
		static TexturedVertex2d **s_pTextVertices = NULL;
		static Vertex2d **s_pLineVertices = NULL;
		static Vertex2d **s_pQuadVertices = NULL;
		static U32 s_maxChars = 0;
		static U32 s_maxLines = 0;
		static U32 s_maxQuads = 0;
		static U32 s_bufferDepth = 0;

		// current debug draw 2d state
		static U32 s_numChars = 0;
		static U32 s_numLines = 0;
		static U32 s_numQuads = 0;
		static U32 s_firstChar = 0;
		static U32 s_firstLine = 0;
		static U32 s_firstQuad = 0;
		static U32 s_buffer = 0;

		//----------------------------------------------------------------------------------------
		// 2D object rendering functions
		//  All coordinates are in screen space normalized safe coordinates with a range from -1.0 to 1.0 at the safe boundaries of
		//  the screen, with (0.0f, 0.0f) at the center of the screen, and (-1.0f, -1.0f) at the bottom left corner.
		//  Unlike 3d objects, 2d elements are buffered for speed, and do not render until a call to Flush().
		//  Filled objects are rendered first, followed by line drawing objects, and then text.
		//  Otherwise, order of draw calls is preserved.

		// Create the 2d buffers and shaders used by the debug draw 2d system
		void Initialize(int maxChars, int maxLines, int maxQuads, int bufferDepth)
		{
			s_maxChars = maxChars;
			s_maxLines = maxLines;
			s_maxQuads = maxQuads;
			s_bufferDepth = bufferDepth;

			s_pTextVertices = new TexturedVertex2d*[ s_bufferDepth ];
			s_pLineVertices = new Vertex2d*[ s_bufferDepth ];
			s_pQuadVertices = new Vertex2d*[ s_bufferDepth ];

			s_pTextVertices[0] = (TexturedVertex2d*)AllocateLinearVideoMemory(s_bufferDepth * sizeof(TexturedVertex2d)*s_maxChars*4);
			s_pLineVertices[0] = (Vertex2d*)AllocateLinearVideoMemory(s_bufferDepth * sizeof(Vertex2d)*s_maxLines*2);
			s_pQuadVertices[0] = (Vertex2d*)AllocateLinearVideoMemory(s_bufferDepth * sizeof(Vertex2d)*s_maxQuads*4);
			for (U32 i = 1; i < s_bufferDepth; ++i) {
				s_pTextVertices[i] = s_pTextVertices[0] + s_maxChars*4*i;
				s_pLineVertices[i] = s_pLineVertices[0] + s_maxLines*2*i;
				s_pQuadVertices[i] = s_pQuadVertices[0] + s_maxQuads*4*i;
			}

			s_pFontTexture = LoadTextureFromFile("../data/textures/debugfont2.tga");
			s_pColorTexture2dVS = LoadVertexProgram("shaders/colorTexture2d.vbin");
			s_pColorTexture2dPS = LoadFragmentProgram("shaders/colorTexture2d.fbin");
			s_pColor2dVS = LoadVertexProgram("shaders/color2d.vbin");
			s_pColor2dPS = LoadFragmentProgram("shaders/color2d.fbin");

			s_numChars = s_numLines = s_numQuads = 0;
			s_firstChar = s_firstLine = s_firstQuad = 0;
			s_buffer = 0;
		}

		// Deallocate the buffers and shaders used by the debug draw 2d system
		void Shutdown()
		{
			if (s_pTextVertices) {
				if (s_pTextVertices[0])
					ReleaseLinearVideoMemory(s_pTextVertices[0]);
				delete[] s_pTextVertices, s_pTextVertices = NULL;
			}
			if (s_pLineVertices) {
				if (s_pLineVertices[0])
					ReleaseLinearVideoMemory(s_pLineVertices[0]);
				delete[] s_pLineVertices, s_pLineVertices = NULL;
			}
			if (s_pQuadVertices) {
				if (s_pQuadVertices[0])
					ReleaseLinearVideoMemory(s_pQuadVertices[0]);
				delete[] s_pQuadVertices, s_pQuadVertices = NULL;
			}
			delete s_pFontTexture, s_pFontTexture = NULL;
			delete s_pColorTexture2dVS, s_pColorTexture2dVS = NULL;
			delete s_pColorTexture2dPS, s_pColorTexture2dPS = NULL;
			delete s_pColor2dVS, s_pColor2dVS = NULL;
			delete s_pColor2dPS, s_pColor2dPS = NULL;
		}

		// Draw a screenspace line
		void DrawLine( float x0, float y0, float x1, float y1, RgbaColor color )
		{
			if (s_numLines + 1 > s_maxLines) {
				ICE_ASSERT(s_numLines + 1 <= s_maxLines);
				return;
			}
			Vertex2d verts[2];
			SetVertex(verts[0], x0, y0, color);
			SetVertex(verts[1], x1, y1, color);
			memcpy(s_pLineVertices[s_buffer] + s_numLines*2, verts, 2*sizeof(Vertex2d));
			s_numLines ++;
		}

		// Draw a screenspace line-drawing rectangle
		void DrawRect( float x0, float y0, float x1, float y1, RgbaColor color )
		{
			if (s_numLines + 4 > s_maxLines) {
				ICE_ASSERT(s_numLines + 4 <= s_maxLines);
				return;
			}
			Vertex2d verts[4*2];
			SetVertex(verts[0], x0, y0, color);
			SetVertex(verts[1], x0, y1, color);
			SetVertex(verts[2], x0, y1, color);
			SetVertex(verts[3], x1, y1, color);
			SetVertex(verts[4], x1, y1, color);
			SetVertex(verts[5], x1, y0, color);
			SetVertex(verts[6], x1, y0, color);
			SetVertex(verts[7], x0, y0, color);
			memcpy(s_pLineVertices[s_buffer] + s_numLines*2, verts, 4*2*sizeof(Vertex2d));
			s_numLines += 4;
		}

		// Draw a screenspace outline triangle
		void DrawTriangle( float x0, float y0, float x1, float y1, float x2, float y2, RgbaColor color )
		{
			if (s_numLines + 3 > s_maxLines) {
				ICE_ASSERT(s_numLines + 3 <= s_maxLines);
				return;
			}
			Vertex2d verts[3*2];
			SetVertex(verts[0], x0, y0, color);
			SetVertex(verts[1], x1, y1, color);
			SetVertex(verts[2], x1, y1, color);
			SetVertex(verts[3], x2, y2, color);
			SetVertex(verts[4], x2, y2, color);
			SetVertex(verts[5], x0, y0, color);
			memcpy(s_pLineVertices[s_buffer] + s_numLines*2, verts, 3*2*sizeof(Vertex2d));
			s_numLines += 3;
		}

		// Draw a screenspace outline quad.  Vertices should be ordered around the outside, not crossing the middle.
		void DrawQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, RgbaColor color )
		{
			if (s_numLines + 4 > s_maxLines) {
				ICE_ASSERT(s_numLines + 4 <= s_maxLines);
				return;
			}
			Vertex2d verts[4*2];
			SetVertex(verts[0], x0, y0, color);
			SetVertex(verts[1], x1, y1, color);
			SetVertex(verts[2], x1, y1, color);
			SetVertex(verts[3], x2, y2, color);
			SetVertex(verts[4], x2, y2, color);
			SetVertex(verts[5], x3, y3, color);
			SetVertex(verts[6], x3, y3, color);
			SetVertex(verts[7], x0, y0, color);
			memcpy(s_pLineVertices[s_buffer] + s_numLines*2, verts, 4*2*sizeof(Vertex2d));
			s_numLines += 4;
		}

		// Draw a screenspace outline polygon.
		void DrawPoly( U32 numVertices, Point2d const *pVertices, RgbaColor color )
		{
			const int kMaxLinesPerBuffer = 32;
			int numLines = numVertices;
			if (s_numLines + numLines > s_maxLines) {
				ICE_ASSERT(s_numLines + numLines <= s_maxLines);
				return;
			}
			Vertex2d verts[kMaxLinesPerBuffer*2];

			int iLine0 = 0;
			while (numLines > 0) {
				int numLinesInBuffer = (numLines > kMaxLinesPerBuffer) ? kMaxLinesPerBuffer : numLines;
				int maxLineInBuffer = iLine0 + numLinesInBuffer;
				for (int iLine = iLine0; iLine < maxLineInBuffer; ++iLine) {
					SetVertex(verts[iLine*2], pVertices[iLine].x, pVertices[iLine].y, color);
					SetVertex(verts[iLine*2+1], pVertices[iLine+1].x, pVertices[iLine+1].y, color);
				}
				memcpy(s_pLineVertices[s_buffer] + s_numLines*2, verts, numLinesInBuffer*2*sizeof(Vertex2d));
				iLine0 += numLinesInBuffer;
				s_numLines += numLinesInBuffer;
				numLines -= numLinesInBuffer;
			}
		}

		// Draw a screenspace filled rectangle
		void DrawFilledRect( float x0, float y0, float x1, float y1, RgbaColor color )
		{
			if (s_numQuads + 1 > s_maxQuads) {
				ICE_ASSERT(s_numQuads + 1 <= s_maxQuads);
				return;
			}
			Vertex2d verts[4];
			SetVertex(verts[0], x0, y0, color);
			SetVertex(verts[1], x0, y1, color);
			SetVertex(verts[2], x1, y1, color);
			SetVertex(verts[3], x1, y0, color);
			memcpy(s_pQuadVertices[s_buffer] + s_numQuads*4, verts, 4*sizeof(Vertex2d));
			s_numQuads ++;
		}

		// Draw a screenspace filled triangle
		void DrawFilledTriangle( float x0, float y0, float x1, float y1, float x2, float y2, RgbaColor color )
		{
			if (s_numQuads + 1 > s_maxQuads) {
				ICE_ASSERT(s_numQuads + 1 <= s_maxQuads);
				return;
			}
			Vertex2d verts[4];
			SetVertex(verts[0], x0, y0, color);
			SetVertex(verts[1], x1, y1, color);
			SetVertex(verts[2], x1, y1, color);
			SetVertex(verts[3], x2, y2, color);
			memcpy(s_pQuadVertices[s_buffer] + s_numQuads*4, verts, 4*sizeof(Vertex2d));
			s_numQuads ++;
		}

		// Draw a screenspace filled convex quad.  Vertices should be ordered around the outside, not crossing the middle.
		void DrawFilledConvexQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, RgbaColor color )
		{
			if (s_numQuads + 1 > s_maxQuads) {
				ICE_ASSERT(s_numQuads + 1 <= s_maxQuads);
				return;
			}
			Vertex2d verts[4];
			SetVertex(verts[0], x0, y0, color);
			SetVertex(verts[1], x1, y1, color);
			SetVertex(verts[2], x2, y2, color);
			SetVertex(verts[3], x3, y3, color);
			memcpy(s_pQuadVertices[s_buffer] + s_numQuads*4, verts, 4*sizeof(Vertex2d));
			s_numQuads ++;
		}

		// Draw a screenspace filled convex polygon.
		void DrawFilledConvexPoly( U32 numVertices, Point2d const *pVertices, RgbaColor color )
		{
			const int kMaxQuadsPerBuffer = 16;
			int numQuads = (numVertices - 1)/2;
			if (s_numQuads + numQuads > s_maxQuads) {
				ICE_ASSERT(s_numQuads + numQuads <= s_maxQuads);
				return;
			}
			Vertex2d verts[ kMaxQuadsPerBuffer*4 ];

			int iQuad0 = 0;
			while (numQuads > 0) {
				int numQuadsInBuffer = (numQuads > kMaxQuadsPerBuffer) ? kMaxQuadsPerBuffer : numQuads;
				int maxQuadInBuffer = iQuad0 + numQuadsInBuffer;
				for (int iQuad = iQuad0; iQuad < maxQuadInBuffer; ++iQuad) {
					SetVertex(verts[iQuad*4], pVertices[iQuad].x, pVertices[iQuad].y, color);
					SetVertex(verts[iQuad*4+1], pVertices[iQuad+1].x, pVertices[iQuad+1].y, color);
					SetVertex(verts[iQuad*4+2], pVertices[numVertices-2 - iQuad].x, pVertices[numVertices-2 - iQuad].y, color);
					SetVertex(verts[iQuad*4+3], pVertices[numVertices-1 - iQuad].x, pVertices[numVertices-1 - iQuad].y, color);
				}
				memcpy(s_pQuadVertices[s_buffer] + s_numQuads*4, verts, numQuadsInBuffer*4*sizeof(Vertex2d));
				iQuad0 += numQuadsInBuffer;
				s_numQuads += numQuadsInBuffer;
				numQuads -= numQuadsInBuffer;
			}
		}

		// Draw a screenspace filled multicolored (per vertex) convex polygon.
		// Specific triangulation may result in incorrect results when passed concave vertices, and unexpected color gradient behavior.
		void DrawFilledConvexPoly( U32 numVertices, Point2d const *pVertices, RgbaColor const *pColors )
		{
			const int kMaxQuadsPerBuffer = 16;
			int numQuads = (numVertices - 1)/2;
			if (s_numQuads + numQuads > s_maxQuads) {
				ICE_ASSERT(s_numQuads + numQuads <= s_maxQuads);
				return;
			}
			Vertex2d verts[ kMaxQuadsPerBuffer*4 ];

			int iQuad0 = 0;
			while (numQuads > 0) {
				int numQuadsInBuffer = (numQuads > kMaxQuadsPerBuffer) ? kMaxQuadsPerBuffer : numQuads;
				int maxQuadInBuffer = iQuad0 + numQuadsInBuffer;
				for (int iQuad = iQuad0; iQuad < maxQuadInBuffer; ++iQuad) {
					SetVertex(verts[iQuad*4], pVertices[iQuad].x, pVertices[iQuad].y, pColors[iQuad]);
					SetVertex(verts[iQuad*4+1], pVertices[iQuad+1].x, pVertices[iQuad+1].y, pColors[iQuad+1]);
					SetVertex(verts[iQuad*4+2], pVertices[numVertices-2 - iQuad].x, pVertices[numVertices-2 - iQuad].y, pColors[numVertices-2 - iQuad]);
					SetVertex(verts[iQuad*4+3], pVertices[numVertices-1 - iQuad].x, pVertices[numVertices-1 - iQuad].y, pColors[numVertices-1 - iQuad]);
				}
				memcpy(s_pQuadVertices[s_buffer] + s_numQuads*4, verts, numQuadsInBuffer*4*sizeof(Vertex2d));
				iQuad0 += numQuadsInBuffer;
				s_numQuads += numQuadsInBuffer;
				numQuads -= numQuadsInBuffer;
			}
		}

		// Draw colored scaled screenspace text at an arbitrary position
		void DrawText( float x0, float y0, float scaleX, float scaleY, RgbaColor color, char const *text)
		{
			float const kStepX = kColSize * scaleX;
			float const kStepY =-kRowSize * scaleY;
			static const U64 kNumVertsInBuffer = 4*kNumTextCols;
			TexturedVertex2d verts[kNumVertsInBuffer];

			float x = x0, y = y0;
			U64 iVert = 0;
			for (char const *p = text; *p; ++p) {
				if (*p > ' ' && *p < 0x7F) {
					int n = (*p) - ' ';
					float u = (n % kCharsPerRow) * kCharSizeU;
					float v = 1.0f + (n / kCharsPerRow) * kCharSizeV;
					SetVertex(verts[iVert++], x,          y,          u,              v,              color);
					SetVertex(verts[iVert++], x,          y + kStepY, u,              v + kCharSizeV, color);
					SetVertex(verts[iVert++], x + kStepX, y + kStepY, u + kCharSizeU, v + kCharSizeV, color);
					SetVertex(verts[iVert++], x + kStepX, y,          u + kCharSizeU, v,              color);
					if (iVert == kNumVertsInBuffer) {
						if (s_numChars + iVert/4 > s_maxChars) {
							ICE_ASSERT(s_numChars + iVert/4 <= s_maxChars);
							if (s_numChars < s_maxChars) {
								memcpy( s_pTextVertices[s_buffer] + s_numChars*4, verts, (s_maxChars - s_numChars)*4*sizeof(TexturedVertex2d) );
								s_numChars = s_maxChars;
							}
							return;
						}
						memcpy( s_pTextVertices[s_buffer] + s_numChars*4, verts, iVert*sizeof(TexturedVertex2d) );
						s_numChars += iVert/4;
						iVert = 0;
					}
					x += kStepX;
				} else if (*p == ' ') {
					x += kStepX;
				} else if (*p == '\n') {
					x = x0;
					y += kStepY;
				}
			}
			if (iVert > 0) {
				if (s_numChars + iVert/4 > s_maxChars) {
					ICE_ASSERT(s_numChars + iVert/4 <= s_maxChars);
					if (s_numChars < s_maxChars) {
						memcpy( s_pTextVertices[s_buffer] + s_numChars*4, verts, (s_maxChars - s_numChars)*4*sizeof(TexturedVertex2d) );
						s_numChars = s_maxChars;
					}
					return;
				}
				memcpy( s_pTextVertices[s_buffer] + s_numChars*4, verts, iVert*sizeof(TexturedVertex2d) );
				s_numChars += iVert/4;
			}
		}

		// Draw colored screenspace text at a given character position, where the screen holds 80x30 characters.
		void DrawTextAtPos( int col, int row, RgbaColor color, char const *text)
		{
			float x0 = col * kColSize - kSafeScreenWidth;
			float y0 = kSafeScreenHeight - row * kRowSize;
			DrawText(x0, y0, 1.0f, 1.0f, color, text);
		}

		// Draw white screenspace text at a given character position, where the screen holds 80x30 characters.
		void DrawTextAtPos( int col, int row, char const *text)
		{
			float x0 = col * kColSize - kSafeScreenWidth;
			float y0 = kSafeScreenHeight - row * kRowSize;
			DrawText(x0, y0, 1.0f, 1.0f, kColorWhite, text);
		}

		// Returns the height and width of the given string IN CHARACTERS (i.e. columns and rows) if printed.
		void GetTextWidthHeight(char const* pText, int& widthInCols, int& heightInRows)
		{
			int linewidth = 0, maxwidth = 0;
			int numlines = 0;
			for (char const *p = pText; ; ++p) {
				if (*p >= ' ' && *p < 0x7F) {
					linewidth++;
				} else if (*p == '\n' || *p == '\0') {
					if (maxwidth < linewidth) {
						maxwidth = linewidth;
					}
					numlines++;
					if (*p == '\0') {
						break;
					}
					linewidth = 0;
				}
			}
			widthInCols = maxwidth;
			heightInRows = numlines;
		}

		// Draw colored scaled screenspace text (printf style) at an arbitrary position
		void Printf( float x0, float y0, float scaleX, float scaleY, RgbaColor color, char const *fmt, ...)
		{
			// use 2k of stack space to write this temp string out, since stack is cheap
			char tmp[2048];
			va_list vlist;
			va_start(vlist, fmt);
			vsnprintf(tmp, 2048, fmt, vlist);
			va_end(vlist);
			tmp[2047] = '\0';
			DrawText(x0, y0, scaleX, scaleY, color, tmp);
		}

		// Draw colored screenspace text (printf style) at an arbitrary position in the default size (the same size as DrawTextAtPos)
		void Printf( float x0, float y0, RgbaColor color, char const *fmt, ...)
		{
			// use 2k of stack space to write this temp string out, since stack is cheap
			char tmp[2048];
			va_list vlist;
			va_start(vlist, fmt);
			vsnprintf(tmp, 2048, fmt, vlist);
			va_end(vlist);
			tmp[2047] = '\0';
			DrawText(x0, y0, color, tmp);
		}

		// Draw white screenspace text (printf style) at an arbitrary position in the default size (the same size as DrawTextAtPos)
		void Printf( float x0, float y0, char const *fmt, ...)
		{
			// use 2k of stack space to write this temp string out, since stack is cheap
			char tmp[2048];
			va_list vlist;
			va_start(vlist, fmt);
			vsnprintf(tmp, 2048, fmt, vlist);
			va_end(vlist);
			tmp[2047] = '\0';
			DrawText(x0, y0, tmp);
		}

		// Draw colored screenspace text (printf style) at a given character position, where the screen holds 80x30 characters.
		void PrintfAtPos( int col, int row, RgbaColor color, char const *fmt, ...)
		{
			// use 2k of stack space to write this temp string out, since stack is cheap
			char tmp[2048];
			va_list vlist;
			va_start(vlist, fmt);
			vsnprintf(tmp, 2048, fmt, vlist);
			va_end(vlist);
			tmp[2047] = '\0';
			DrawTextAtPos(col, row, color, tmp);
		}

		// Draw white screenspace text (printf style) at a given character position, where the screen holds 80x30 characters.
		void PrintfAtPos( int col, int row, char const *fmt, ...)
		{
			// use 2k of stack space to write this temp string out, since stack is cheap
			char tmp[2048];
			va_list vlist;
			va_start(vlist, fmt);
			vsnprintf(tmp, 2048, fmt, vlist);
			va_end(vlist);
			tmp[2047] = '\0';
			DrawTextAtPos(col, row, tmp);
		}

		// Begin a new frame of DebugDraw2d commands
		void BeginFrame()
		{
			s_numChars = s_numLines = s_numQuads = 0;
			s_firstChar = s_firstLine = s_firstQuad = 0;
			s_buffer = (s_buffer + 1) % s_bufferDepth;
		}

		// Flush all DebugDraw2d commands to the screen
		void Flush()
		{
			DisableRenderState(kRenderDepthTest);
			DisableRenderState(kRenderCullFace);
			EnableRenderState(kRenderBlend);
			SetBlendFunc(kBlendSrcAlpha, kBlendOneMinusSrcAlpha);
			SetVertexProgram(s_pColor2dVS);
			SetFragmentProgram(s_pColor2dPS);

			// Disable all vertex attributes not used here.
			for (U32F iAttrib = 3; iAttrib < 16; iAttrib++) {
				DisableVertexAttribArray(iAttrib);
			}

			if ( s_numQuads > s_firstQuad ) {
				Vertex2d *pVertices = s_pQuadVertices[s_buffer] + s_firstQuad*4;
				SetVertexAttribFormat(0, kAttribFloat, kAttribCount3, sizeof(Vertex2d));
				DisableVertexAttribArray(1);
				SetVertexAttribFormat(2, kAttribUnsignedByteNormalized, kAttribCount4, sizeof(Vertex2d));
				SetVertexAttribPointer(0, TranslateAddressToOffset(pVertices), kAttribVideoMemory);
				SetVertexAttribPointer(2, TranslateAddressToOffset((U8*)pVertices + 12), kAttribVideoMemory);

				DrawArrays(kDrawQuads, 0, (s_numQuads - s_firstQuad)*4);
				s_firstQuad = s_numQuads;
			}
			if ( s_numLines > s_firstLine) {
				Vertex2d *pVertices = s_pLineVertices[s_buffer] + s_firstLine*2;
				SetVertexAttribFormat(0, kAttribFloat, kAttribCount3, sizeof(Vertex2d));
				DisableVertexAttribArray(1);
				SetVertexAttribFormat(2, kAttribUnsignedByteNormalized, kAttribCount4, sizeof(Vertex2d));
				SetVertexAttribPointer(0, TranslateAddressToOffset(pVertices), kAttribVideoMemory);
				SetVertexAttribPointer(2, TranslateAddressToOffset((U8*)pVertices + 12), kAttribVideoMemory);

				DrawArrays(kDrawLines, 0, (s_numLines - s_firstLine)*2);
				s_firstLine = s_numLines;
			}
			if ( s_numChars > s_firstChar) {
				TexturedVertex2d *pVertices = s_pTextVertices[s_buffer] + s_firstChar*4;
				SetVertexAttribFormat(0, kAttribFloat, kAttribCount3, sizeof(TexturedVertex2d));
				SetVertexAttribFormat(1, kAttribFloat, kAttribCount2, sizeof(TexturedVertex2d));
				SetVertexAttribFormat(2, kAttribUnsignedByteNormalized, kAttribCount4, sizeof(TexturedVertex2d));
				SetVertexAttribPointer(0, TranslateAddressToOffset(pVertices), kAttribVideoMemory);
				SetVertexAttribPointer(1, TranslateAddressToOffset((U8*)pVertices + 12), kAttribVideoMemory);
				SetVertexAttribPointer(2, TranslateAddressToOffset((U8*)pVertices + 20), kAttribVideoMemory);
				SetVertexProgram(s_pColorTexture2dVS);
				SetFragmentProgram(s_pColorTexture2dPS);
				SetTexture(0, s_pFontTexture);

				DrawArrays(kDrawQuads, 0, (s_numChars - s_firstChar)*4);
				s_firstChar = s_numChars;
			}

			EnableRenderState(kRenderDepthTest);
			EnableRenderState(kRenderCullFace);
			DisableRenderState(kRenderBlend);
		}

		//----------------------------------------------------------------------------
		// 2D rendering helper functions

		static inline void SetVertex(Vertex2d &vertex, float x, float y, RgbaColor color)
		{
			vertex.x = x;
			vertex.y = y;
			vertex.z = 0.0f;
			vertex.color = color;
		}
		static inline void SetVertex(TexturedVertex2d &vertex, float x, float y, float u, float v, RgbaColor color)
		{
			vertex.x = x;
			vertex.y = y;
			vertex.z = 0.0f;
			vertex.u = u;
			vertex.v = v;
			vertex.color = color;
		}
	} //namespace DebugDraw2d
} //namespace Ice


