#include "tbd/franktmp.h"

#include "core/visualdebugger.h"
//#include "tbd/filedate.h"
#include "anim/transform.h"
#include "core/boostarray.inl"








void FrankMisc::DrawGrid(Pixel3 size, const CPoint& origin, const CDirection& realSize, uint32_t color)
{
#ifndef _GOLD_MASTER
	CDirection invSize(1.0f/size[0],1.0f/size[1],1.0f/size[2]);
	invSize *= realSize;
	
	for(int iMod = 0 ; iMod < 3 ; iMod++ )
	{
		Pixel3 layer((iMod)%3,(iMod+1)%3,(iMod+2)%3);
		Pixel3 coord(0,0,0);
		for(coord[layer[0]] = 0 ; coord[layer[0]] <= size[layer[0]] ; coord[layer[0]]++ )
		{
			for(coord[layer[1]] = 0 ; coord[layer[1]] <= size[layer[1]] ; coord[layer[1]]++ )
			{
				CDirection p = Conversion::Pixel2CDirection(coord);
				p *= invSize;
				p[layer[2]] = 0;
				CPoint p1 = p + origin;
				CPoint p2 = p1;
				p2[layer[2]] += realSize[layer[2]];
				
				g_VisualDebug->RenderLine(p1,p2,color);
					//CPoint(p1[0],p1[1],p1[2]),
					//CPoint(p2[0],p2[1],p2[2]),
					//color);
			}
		}
	}
#endif
}




void FrankMisc::Split(const ntstd::String& in, ntstd::Vector<ntstd::String>& out, const ntstd::String& sep)
{
	out.clear();
	
	u_int iBegin = 0;
	while(iBegin!=in.size())
	{
		u_int iEnd = in.find(sep);
		out.push_back(in.substr(iBegin,iEnd));
		iBegin = iEnd;
	}
}






///////////////////////////////////////////////////
// Group FrankMisc

CMatrix FrankMisc::CreateScaleMatrix(float fSx, float fSy, float fSz)
{
	return CMatrix(
		fSx,  0.0f, 0.0f, 0.0f,
		0.0f, fSy,  0.0f, 0.0f,
		0.0f, 0.0f, fSz,  0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}


CMatrix FrankMisc::GetRelativeMatrix(const Transform* pLocal, const Transform* pRelative)
{
	ntAssert(pLocal && pRelative);
	
	CMatrix res = pLocal->GetLocalMatrix();
	const Transform* pWalk = pLocal->GetParent();
	while(pWalk!=pRelative)
	{
		ntAssert(pWalk);
		res *= pWalk->GetLocalMatrix();
		pWalk = pWalk->GetParent();
	}
	
	return res;
}


// End of Group FrankMisc
///////////////////////////////////////////////////












