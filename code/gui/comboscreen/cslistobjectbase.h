/***************************************************************************************************
*
*	DESCRIPTION		The provides an interface for objects that are to appear in the list
*
*	NOTES			Actually holds 
*
***************************************************************************************************/

#ifndef CSLISTOBJECTBASE_H
#define CSLISTOBJECTBASE_H

#include "anim/transform.h"

class CSListObjectBase
{
public:
	// Construction Destruction
	CSListObjectBase( void );
	virtual ~CSListObjectBase( void );

	//passthroughs to all objects
	virtual void Render( void );
	virtual void Update( void );
	virtual void HasMoved( void );

	//global info we need to know about the objects in order to position and move them
	virtual void SetPosition( const float &fX, const float &fY );
	virtual void GetPosition( float &fX, float &fY ) const;

	virtual void SetSize( const float &fWidth, const float &fHeight );
	virtual void GetSize( float &fWidth, float &fHeight ) const;

	virtual void SetTransform( Transform* pTransform );
	virtual Transform* GetTransform( void ) const;

	virtual void SetSelectableOffset( const float &fOffset );  //curently not used
	virtual float GetSelectableOffset( void ) const;

protected:
	Transform*	m_pOffsetTransform;		///<this is the transform to use to offset all the objects
	float		m_fX;					///<position of the left side of the control in pixels   
	float		m_fY;					///<position of the center of the control in pixels		 
	float		m_fWidth;				///<NOT USED
	float		m_fHeight;				///<HEIGHT OF OBJECT IN SCROLL LENGTH, 1.0 = full height, 2.0 = twice height, 0.5 = half height etc
	float		m_fSlecetableOffset;	///<This is the pixel offset from the top to locate the select icon
	bool		m_bPosSizeUpdated;		///<this is used to tell update that the position or size of the control has changed.

}; //end CSListObjectBase

#endif //CSLISTOBJECTBASE_H
