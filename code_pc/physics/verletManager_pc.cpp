#include "verletManager_pc.h"

#include "game/keybinder.h"
#include "gfx/shader.h"
#include "gfx/sector.h"
#include "anim/transform.h"

#include "physics/verlet.h"
#include "physics/verletdef.h"


namespace Physics
{

bool VerletManager::m_gVerletEnable = false;


void VerletManager::RendererUpdate()
{
	//for(VerletList::iterator it = m_verletList.begin();
	//	it != m_verletList.end();
	//	++it)
	//{
	//	(*it)->GetRenderable()->RendererUpdate();
	//}
}



//! constructor
VerletManager::VerletManager()
{
	RegisterKey();
	m_flags.Set(RENDERING);
	//m_pGameLink = NT_NEW VerletGameLink();
}



//! constructor
VerletManager::~VerletManager()
{
	//NT_DELETE(m_pGameLink);
}


// toggle flags
COMMAND_RESULT VerletManager::CommandToggleFlags(const int& kind)
{
	switch(kind)
	{
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
void VerletManager::Update( float )
{
	// nothing
}

void VerletManager::RegisterKey()
{
	// Register some commands
	CommandBaseInput<const int&>* pToggle = CommandManager::Get().CreateCommand("VerletManagerFlags", this, &VerletManager::CommandToggleFlags, "VerletManager Toggle flags");
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Verlet rendering", int(RENDERING), KEYS_PRESSED, KEYC_V, KEYM_ALT | KEYM_CTRL);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Verlet Pause", int(IS_PAUSED), KEYS_PRESSED, KEYC_P, KEYM_ALT);
}


VerletInstance* VerletManager::CreateASimpleGenericVerletSystem( Transform* pParentTransform, Template_Flag* pobTemplate )
{
	UNUSED(pParentTransform);
	UNUSED(pobTemplate);
	return 0;
};

VerletInstance* VerletManager::CreateASimpleGenericVerletSystemWithOffset(Transform* pParentTransform, const CMatrix& offset)
{
	UNUSED(pParentTransform);
	UNUSED(offset);
	return 0;
};


} // end of namespace Physics



