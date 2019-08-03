//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Boss.cpp
//!
//------------------------------------------------------------------------------------------

#include "coolcam_boss.h"
#include "camera/sceneelementcomponent.h"
#include "camera/camutils.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "core/visualdebugger.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "objectdatabase/dataobject.h"
#include "physics/world.h"
#include "game/entityboss.h"

//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Boss::CoolCam_Boss
//!	Construction
//!
//------------------------------------------------------------------------------------------
CoolCam_Boss::CoolCam_Boss(const CamView& view, const CEntity* pobPlayer, const CEntity* pobBoss)
: CoolCamera(view),	
  m_pobPlayer(pobPlayer),
  m_pobBoss(pobBoss)
{
}

//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Boss::~CoolCam_Boss
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CoolCam_Boss::~CoolCam_Boss()
{
}

//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Boss::Update
//!	Update the aiming camera.
//!
//------------------------------------------------------------------------------------------
void CoolCam_Boss::Update(float fTimeDelta)
{
	CPoint obPlayerPosition(m_pobPlayer->GetPosition());
	CPoint obBossPosition(m_pobBoss->GetPosition());

	CDirection obPlayerToBoss( obBossPosition - obPlayerPosition );
	obPlayerToBoss.Normalise();
	obPlayerToBoss *= 4.5f;

	CPoint obCameraPosition = obPlayerPosition - obPlayerToBoss;
	obCameraPosition.Y() += 2.0f;
	if (obCameraPosition.Y() < m_pobPlayer->GetPosition().Y())
		obCameraPosition.Y() = m_pobPlayer->GetPosition().Y() + 0.05f;
	m_obLookAt = obBossPosition;
	m_obLookAt.Y() += 0.6f;

	CCamUtil::CreateFromPoints(m_obTransform, obCameraPosition, m_obLookAt);
}
