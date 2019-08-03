#ifndef _CHAIN2RENDER_H_
#define _CHAIN2RENDER_H_

class ChainElem;

//--------------------------------------------------
//!
//!	This structure is basically a jump from the chain/cloth/hair animation
//! system to the chain rendering system
//!
//--------------------------------------------------

class ChainAnimation2Render: CNonCopyable
{
public:
	// external curve, used to create the structure
	typedef ntstd::List<ChainElem*, Mem::MC_GFX> ExternalCurve;
	
	// one "point"
	class OnePoint
	{
	protected:
		ChainElem* m_pElem;
	public:
		// just an indirection, the point is already computed at this point
		CPoint GetWorldPosition() const;
		OnePoint():m_pElem(0){}
		OnePoint(ChainElem* pElem):m_pElem(pElem){}
	}; // end of class OnePoint

	// internal curve base class
	typedef ntstd::Vector<OnePoint, Mem::MC_GFX> InternalCurveBase;
	
	// one curve
	class OneCurve: public InternalCurveBase
	{
	public:
		void DebugDraw();
		OneCurve(const ExternalCurve& curve);
	}; // end of class OneCurve
	
	// many curves
	typedef ntstd::List<OneCurve*, Mem::MC_GFX> Container;
	Container m_container;
public:
	// ctor
	ChainAnimation2Render();
	void Add(const ExternalCurve& curve);
	
	// dtor
	~ChainAnimation2Render();
	
	// draw some white line just to be sure everything is here
	void DebugDraw();
}; // end of class ChainAnimation2Render


#endif // end of _CHAIN2RENDER_H_

