//--------------------------------------------------
//!
//!	\file listspace_ps3.h
//!	Declares factory methods for creation/destruction of ps3 implementation of a shadowfrustum clipper
//!
//--------------------------------------------------

#ifndef LISTSPACE_PS3_H
#define LISTSPACE_PS3_H

class ListSpace;

struct IFrustumClipper
{
    virtual void SetVisibleFrustum( const float* pShadowPercents = 0 ) = 0;
    virtual ~IFrustumClipper() {}
};

IFrustumClipper*    CreateClipper(ListSpace* listSpace);
void                DestroyClipper(IFrustumClipper* clipper);

#endif
