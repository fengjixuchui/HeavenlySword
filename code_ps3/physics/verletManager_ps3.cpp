#include "verletManager_ps3.h"

#include "game/keybinder.h"
#include "exec/ppu/ElfManager.h"
#include "gfx/shader.h"
#include "gfx/sector.h"

#include "verletinstance_ps3.h"
#include "verletrenderable_ps3.h"
#include "physics/verlet.h"
#include "physics/verletdef.h"
#include "physics/verletdef_ps3.h"
#include "physics/globalwind.h"
#include "physics/softflag.h"


namespace Physics
{

bool VerletManager::m_gVerletEnable = false;


void VerletManager::RendererUpdate()
{
	for(VerletList::iterator it = m_verletList.begin();	it != m_verletList.end(); ++it)
	{
		(*it)->GetRenderable()->RendererUpdate();
	}
}



//! constructor
VerletManager::VerletManager() :
	m_obGravity(0.0f, -0.5f, 0.0f, 0.0f)
{
	ElfManager::Get().Load("flags_spu_ps3.mod");
	RegisterKey();
	m_flags.Set(RENDERING);
	m_pGameLink = NT_NEW VerletGameLink();
	m_pobDefaultGlobalWind = NT_NEW GlobalWind();
}

//! destructor
VerletManager::~VerletManager()
{
	Reset();
	NT_DELETE(m_pobDefaultGlobalWind);
	NT_DELETE(m_pGameLink);
}

void VerletManager::Reset()
{
	VerletList::iterator it = m_verletList.begin();
	while (!m_verletList.empty())
	{
		Physics::VerletInstance* pLitter = *it;
		NT_DELETE( pLitter );
		it = m_verletList.erase( it );
	}
}

// toggle flags
COMMAND_RESULT VerletManager::CommandToggleFlags(const int& kind)
{
	switch(kind)
	{
	case RESET_SIMULATION:
		{
			ResetPosition();
			return CR_SUCCESS;
		}
	case SPU_BREAKPOINT:
		{
			m_flags.Set(SPU_BREAKPOINT);
			return CR_SUCCESS;
		}
	case DEBUG_RENDERING:
		{
			m_flags.Toggle(DEBUG_RENDERING);
			return CR_SUCCESS;
		}
	case RENDERING:
		{
			m_flags.Toggle(RENDERING);
			return CR_SUCCESS;
		}
	case IS_PAUSED:
		{
			m_flags.Toggle(IS_PAUSED);
			return CR_SUCCESS;
		}
	default:
		{
			ntPrintf("VerletManager::CommandToggleFlags: unknown flag: %i", kind);
			return CR_FAILED;
		}
	}
}
// update (doesn't so much for now)
void VerletManager::Update(float fTimeDelta)
{
	if( m_flags[SPU_BREAKPOINT_AUX] )
	{
		m_flags.Unset(SPU_BREAKPOINT_AUX);
	}
	if( m_flags[SPU_BREAKPOINT] )
	{
		m_flags.Set(SPU_BREAKPOINT_AUX);
		m_flags.Unset(SPU_BREAKPOINT);
	}
	if(m_flags[DEBUG_RENDERING])
	{
		DebugRender();
	}

	for(VerletList::iterator it = m_verletList.begin(); it != m_verletList.end(); ++it)
	{
		(*it)->Update(fTimeDelta);
	}

	for(WindList::iterator it = m_obWindList.begin(); it != m_obWindList.end(); ++it)
	{
		(*it)->Update(fTimeDelta);
	}

}

void VerletManager::RegisterKey()
{
	// Register some commands
	CommandBaseInput<const int&>* pToggle = CommandManager::Get().CreateCommand("VerletManagerFlags", this, &VerletManager::CommandToggleFlags, "VerletManager Toggle flags");
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Toggle breakpoint", int(SPU_BREAKPOINT), KEYS_PRESSED, KEYC_B, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Reset verlet position", int(RESET_SIMULATION), KEYS_PRESSED, KEYC_R, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Verlet debug rendering", int(DEBUG_RENDERING), KEYS_PRESSED, KEYC_D, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Verlet rendering", int(RENDERING), KEYS_PRESSED, KEYC_V, KEYM_ALT | KEYM_CTRL);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Verlet Pause", int(IS_PAUSED), KEYS_PRESSED, KEYC_P, KEYM_ALT);
}

//register a verlet
void VerletManager::Register(VerletInstance* pVerletInstance)
{
	//CSector::Get().GetRenderables().AddRenderable(pVerletInstance->m_pRenderable);
	m_verletList.push_back(pVerletInstance);
}
//register a verlet
void VerletManager::Unregister(VerletInstance* pVerletInstance)
{
	m_verletList.remove(pVerletInstance);
	//CSector::Get().GetRenderables().RemoveRenderable(pVerletInstance->m_pRenderable);
}

// register key
void VerletManager::ResetPosition()
{
	for(VerletList::iterator it = m_verletList.begin(); it != m_verletList.end(); ++it)
	{
		(*it)->ResetToDefaultPosition();
	}
}

// register key
void VerletManager::DebugRender()
{
	for(VerletList::iterator it = m_verletList.begin();	it != m_verletList.end(); ++it)
	{
		(*it)->DebugRender();
	}

	for(WindList::iterator it = m_obWindList.begin();	it != m_obWindList.end(); ++it)
	{
		(*it)->DebugRender();
	}
}


VerletInstance* VerletManager::CreateRectangularFlag(Transform* pParentTransform, const VerletInstanceDef& def, const VerletMaterialInstanceDef& matDef)
{
	// owned by calling function:
	VerletInstance* pInstance = NT_NEW VerletInstance(pParentTransform, def);
	// owned by renderer
	VerletRenderableInstance* pRenderable = NT_NEW VerletRenderableInstance(pInstance,def, matDef,GetGameLink());
	pInstance->SetRenderable(pRenderable);
	return pInstance;
}

VerletInstance* VerletManager::CreateASimpleGenericVerletSystem(Transform* pParentTransform, Template_Flag* pobTemplate)
{
	// Bail if not needed
	if(!Physics::VerletManager::Get().IsEnabled()) return 0;

	user_error_p(pobTemplate, ("%s(%d): Null Template_Flag passed to Verlet Creation.", __FILE__, __LINE__, ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(pobTemplate)))); 
	user_error_p(pobTemplate->GetSoftMaterial(), ("%s(%d): Template_Flag (%s) missing soft material.", __FILE__, __LINE__, ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(pobTemplate)))); 
	
	// could get rid if that eventually, it's here for historical  reason only
	VerletInstanceDef defAux(Vec2(pobTemplate->GetSizeX(), pobTemplate->GetSizeY()), Pixel2(pobTemplate->GetResolutionX(), pobTemplate->GetResolutionY()), pobTemplate->GetSoftMaterial()->GetParticleMass(), CMatrix(CONSTRUCT_IDENTITY));

	// create new verlet
	VerletInstance* pVerletInstance = VerletManager::Get().CreateRectangularFlag(pParentTransform, defAux, VerletMaterialInstanceDef(pobTemplate->GetTexture(), pobTemplate->GetNormalMap()));
	pVerletInstance->SetTemplate(pobTemplate);
	Register(pVerletInstance);
	return pVerletInstance;
}

VerletInstance* VerletManager::CreateASimpleGenericVerletSystemWithOffset(Transform* pParentTransform, const CMatrix& offset)
{
	ntAssert_p(0, ("Not implemented"));
	return 0;
}

} // end of namespace Physics



