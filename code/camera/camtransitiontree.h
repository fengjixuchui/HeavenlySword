//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtransitiontree.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef CAMTRANSITIONTREE_INC
#define CAMTRANSITIONTREE_INC

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camerainterface.h"


//------------------------------------------------------------------------------------------
// External Declarations
//------------------------------------------------------------------------------------------
class CamTransitionDef;
class CamTransition;

//------------------------------------------------------------------------------------------
//!
//!	CamTransitionTree                                                                                 
//!	Controls transitions between cameras.
//!
//------------------------------------------------------------------------------------------
class CamTransitionTree
{
public:
	CamTransitionTree()	{m_pTreeTop = 0; Reset();}
	~CamTransitionTree()	{Reset();}

	bool Update(float fTimeDelta);
	void Reset();

	void ActivateTransition(const CameraInterface* pOldCam, const CameraInterface* pNewCam, const CamTransitionDef* pDef);
	bool IsTransitionActive() const                    {return (m_pTreeTop != 0);}
	bool IsUsing(CameraInterface* pCam) const;

	const CameraInterface* GetDestination() const; 
	const CameraInterface* GetCameraInterface() const;
	      CameraInterface* GetCameraInterface();
	const CamTransition*   GetActiveTransition() const {return m_pTreeTop;}

private:
	CamTransition* m_pTreeTop;
};

#endif // CAMTRANSITIONTREE_INC
