/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

// icedebugdraw.h

#ifndef ICEDEBUGDRAW_H
#define ICEDEBUGDRAW_H

#include "shared/math/point.h"
#include "shared/math/vector.h"
#include "shared/math/transform.h"

#include "icerender.h"

#define MAKE_RGBA_COLOR(r, g, b, a) ((a)|((r)<<24)|((g)<<16)|((b)<<8))

namespace Ice
{
	namespace DebugDraw3d
	{
		using namespace SMath;

		//----------------------------------------------------------------------------------------
		// 3D object rendering functions

		/// Create the base procedural objects and shaders used by the debug draw system
		void Initialize();
		/// Deallocate the base procedural objects and shaders used by the debug draw system
		void Shutdown();

		/// Set the debug draw object rendering texture.
		/// If pTexture is NULL, use untextured rendering.
		void SetTexture( Render::Texture *pTexture );

		/// Draw a world aligned box 
		void DrawBox( const Point_arg position_center, const Vector_arg size = Vector(20.0f, 20.0f, 20.0f), const Vector_arg color = Vector(1.0f, 1.0f, 1.0f) );

		/// Draw a box at the origin in the local space defined by transform
		void DrawBox( Transform const &transform, Vector size = Vector(20.0f, 20.0f, 20.0f), Vector color = Vector(1.0f, 1.0f, 1.0f) );
		/// Draw a box in the local space defined by transform
		void DrawBox( Transform const &transform, const Point_arg position_center, Vector size = Vector(1.0f, 1.0f, 1.0f), Vector color = Vector(1.0f, 1.0f, 1.0f) );

		/// Draw a world aligned sphere
		void DrawSphere( const Point_arg position_center, float radius = 10.0f, Vector color = Vector(1.0f, 1.0f, 1.0f) );
		/// Draw a sphere at the origin in the local space defined by transform
		void DrawSphere( Transform const &transform, float radius = 10.0f, Vector color = Vector(1.0f, 1.0f, 1.0f) );
		/// Draw a box in the local space defined by transform
		void DrawSphere( Transform const &transform, const Point_arg position_center, float radius = 10.0f, Vector color = Vector(1.0f, 1.0f, 1.0f) );

		/// Draw a cone with the apex at position_apex and with the axis aligned point "up" with world y
		void DrawCone( const Point_arg position_apex, float radius_base = 10.0f, float height = 10.0f, Vector color = Vector(1.0f, 1.0f, 1.0f) );
		/// Draw a cone with the apex at the origin in the local space defined by transform
		void DrawCone( Transform const &transform, float radius_base = 10.0f, float height = 10.0f, Vector color = Vector(1.0f, 1.0f, 1.0f) );
		/// Draw a cone in the local space defined by transform
		void DrawCone( Transform const &transform, const Point_arg position_apex, float radius_base = 10.0f, float height = 10.0f, Vector color = Vector(1.0f, 1.0f, 1.0f) );
	} // namespace DebugDraw3d

	namespace DebugDraw2d
	{
		// Actual pixel size of font
		int const kFontWidthPixels = 9;
		int const kFontHeightPixels = 16;
		// Constants designed for 1280x720 resolution with a safe region border of about 7.5% on all sides
		int const kNumTextCols = 120;
		int const kNumTextRows =  40;
		float const kSafeScreenWidth = (float)kFontWidthPixels * kNumTextCols / 1280.0f;	// actually 0.84375
		float const kSafeScreenHeight = (float)kFontHeightPixels * kNumTextRows / 720.0f;	// actually 0.88888

		// Default buffer sizes
		int const kDefaultMaxDebugDrawChars = kNumTextCols*kNumTextRows + 200;
		int const kDefaultMaxDebugDrawLines = 256;
		int const kDefaultMaxDebugDrawQuads = 256;
		int const kDefaultBufferDepth = 4;	// currently need to quadruple buffer for omega to work

		typedef U32 RgbaColor;
		RgbaColor const kColorWhite = MAKE_RGBA_COLOR(255, 255, 255, 255);

		struct Point2d {
			float x, y;
		};

		//----------------------------------------------------------------------------------------
		// 2D object rendering functions
		//  All coordinates are in screen space normalized safe coordinates with a range from -1.0 to 1.0 at the safe boundaries of
		//  the screen, with (0.0f, 0.0f) at the center of the screen, and (-1.0f, -1.0f) at the bottom left corner.
		//  Unlike 3d objects, 2d elements are buffered for speed, and must be bracketed by a call to BeginFrame()
		//	and a call to Flush(), which flushes everything that has been drawn so far.
		//  Filled objects are rendered first, followed by line drawing objects, and then text.
		//  Otherwise, order of draw calls is preserved.
		//	If more specific ordering is desired, Flush() may be called multiple times to order groups of 2d elements

		/// Create the buffers and shaders used by the debug draw 2d system
		void Initialize(int maxDebugDrawChars = kDefaultMaxDebugDrawChars, int maxDebugDrawLines = kDefaultMaxDebugDrawLines, int maxDebugDrawQuads = kDefaultMaxDebugDrawQuads, int bufferDepth = kDefaultBufferDepth);
		/// Deallocate the buffers and shaders used by the debug draw 2d system
		void Shutdown();

		/// Draw a screenspace line
		void DrawLine( float x0, float y0, float x1, float y1, RgbaColor color );
		/// Draw a screenspace outline rectangle
		void DrawRect( float x0, float y0, float x1, float y1, RgbaColor color );
		/// Draw a screenspace outline triangle
		void DrawTriangle( float x0, float y0, float x1, float y1, float x2, float y2, RgbaColor color );
		/// Draw a screenspace outline quad.  Vertices should be ordered around the outside, not crossing the middle.
		void DrawQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, RgbaColor color );
		/// Draw a screenspace outline polygon.
		void DrawPoly( U32 numVertices, Point2d const *pVertices, RgbaColor color );

		/// Draw a screenspace filled rectangle
		void DrawFilledRect( float x0, float y0, float x1, float y1, RgbaColor color );
		/// Draw a screenspace filled triangle
		void DrawFilledTriangle( float x0, float y0, float x1, float y1, float x2, float y2, RgbaColor color );
		/// Draw a screenspace filled convex quad.  Vertices should be ordered around the outside, not crossing the middle.
		/// Specific triangulation may result in incorrect results when passed concave vertices.
		void DrawFilledConvexQuad( float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, RgbaColor color );
		/// Draw a screenspace filled convex polygon.
		/// Specific triangulation may result in incorrect results when passed concave vertices.
		void DrawFilledConvexPoly( U32 numVertices, Point2d const *pVertices, RgbaColor color );

		/// Draw a screenspace filled multicolored (per vertex) convex polygon.
		/// Specific triangulation may result in incorrect results when passed concave vertices, and unexpected color gradient behavior.
		void DrawFilledConvexPoly( U32 numVertices, Point2d const *pVertices, RgbaColor const *pColors );

		/// Draw colored scaled screenspace text at an arbitrary position
		void DrawText( float x0, float y0, float scaleX, float scaleY, RgbaColor color, char const *text);
		/// Draw colored screenspace text at an arbitrary position in the default size (the same size as DrawTextAtPos)
		inline void DrawText( float x0, float y0, RgbaColor color, char const *text)
		{
			DrawText(x0, y0, 1.0f, 1.0f, color, text);
		}
		/// Draw white screenspace text at an arbitrary position in the default size (the same size as DrawTextAtPos)
		inline void DrawText( float x0, float y0, char const *text)
		{
			DrawText(x0, y0, 1.0f, 1.0f, kColorWhite, text);
		}
		/// Draw colored screenspace text at a given character position, where the screen holds 80x30 characters.
		void DrawTextAtPos( int col, int row, RgbaColor color, char const *text);
		/// Draw white screenspace text at a given character position, where the screen holds 80x30 characters.
		void DrawTextAtPos( int col, int row, char const *text);
		/// Returns the height and width of the given string IN CHARACTERS (i.e. columns and rows) if printed.
		void GetTextWidthHeight(char const* pText, int& widthInCols, int& heightInRows);

		/// Draw colored scaled screenspace text (printf style) at an arbitrary position
		void Printf( float x0, float y0, float scaleX, float scaleY, RgbaColor color, char const *fmt, ...);
		/// Draw colored screenspace text (printf style) at an arbitrary position in the default size (the same size as DebugDraw2dTextAtPos)
		void Printf( float x0, float y0, RgbaColor color, char const *fmt, ...);
		/// Draw white screenspace text (printf style) at an arbitrary position in the default size (the same size as DebugDraw2dTextAtPos)
		void Printf( float x0, float y0, char const *fmt, ...);
		/// Draw colored screenspace text (printf style) at a given character position, where the screen holds 80x30 characters.
		void PrintfAtPos( int col, int row, RgbaColor color, char const *fmt, ...);
		/// Draw white screenspace text (printf style) at a given character position, where the screen holds 80x30 characters.
		void PrintfAtPos( int col, int row, char const *fmt, ...);

		/// Begin a new frame of DebugDraw2d commands
		void BeginFrame();
		/// Flush all DebugDraw2d commands to the screen
		void Flush();
	}
}

#endif//ICEDEBUGDRAW_H
