#include "hair/effectchain.h"
#include "hair/chaincore.h"

#include <hkbase\config\hkConfigVersion.h>
#include "tbd/xmlfileinterface.h"
#include "anim/hierarchy.h"
#include "input/inputhardware.h"
#include "core/timer.h"
#include "core/OSDDisplay.h"
#include "objectdatabase/dataobject.h"
#include "tbd/franktmp.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/luaattrtable.h"
#include "game/luaexptypes.h"
#include "game/entitybindings.h"
#include "game/shellconfig.h"
#include "forcefielditem.h"
#include "core/spericalcoordinate.h"

#ifdef PLATFORM_PS3
	#include "exec/ppu/ElfManager.h"
#endif // PLATFORM_PS3


#include "game/keybinder.h"






//--------------------------------------------------
//!
//!	Short Class Description.
//!	Long Class Description
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------

class UpdateEvent : public TimeSequence
{
public:
	//! pointer to
	ChainRessource* m_pChainRessource;
	
	//! constructor
	UpdateEvent(ChainRessource* pChainRessource):
		TimeSequence(5.0f,TimeSequence::F_PLAYWHENCREATED | TimeSequence::F_REPEAT),
		m_pChainRessource(pChainRessource)
	{
		// nothing
	}
	
	virtual void OneMore()
	{
		m_pChainRessource->LowFreqUpdate();
	}
};

















//! constructor
ChainRessource::ChainRessource():
	m_pGlobalWelderDef(0),
	m_debugMode(7,0),
	m_hairMaterial("hair"),
	m_iNbDetection(0)
{
	// load interface from welder
	LoadDefFile();
	
	// add sequence
	m_sequence.AddNewSequence(NT_NEW UpdateEvent(this));
	
	// set init me flag
	m_gameMask.Set(F_INITME);

	// set debug fats
	if(g_ShellOptions->m_bHairOnSPU)
	{
		m_debugMask.Set(F_USESPU);
		ntPrintf("FRANKHAIR -> SPU\n");
#ifdef PLATFORM_PS3
		ElfManager::Get().Load("hair_spu_ps3.mod");
#endif // PLATFORM_PS3
	}
	else
	{
		ntPrintf("FRANKHAIR -> NO SPU\n");
	}

	RegisterKey();
}

ChainRessource::~ChainRessource()
{
	// nothing
}





// register key
void ChainRessource::RegisterKey()
{
	// Register some commands
	CommandBaseInput<const int&>* pToggle = CommandManager::Get().CreateCommand("ChainRessource", this, &ChainRessource::CommandToggleFlags, "ChainRessource Toggle flags");
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Draw collision sphere", int(F_DRAWCOLSPHERE), KEYS_PRESSED, KEYC_F1, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "Draw anti-collision sphere", int(F_DRAWANTICOLSPHERE), KEYS_PRESSED, KEYC_F1, KEYM_ALT | KEYM_CTRL);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "change debug mode", int(F_SWAPDEBUGMODE), KEYS_PRESSED, KEYC_F3, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "restart simulation", int(F_RESTARTSIM), KEYS_PRESSED, KEYC_F4, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "add a wind gust", int(F_ADDWIND), KEYS_PRESSED, KEYC_F5, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "draw wind", int(F_DRAWWIND), KEYS_PRESSED, KEYC_F7, KEYM_ALT);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "toggle spu mode", int(F_USESPU), KEYS_PRESSED, KEYC_F10, KEYM_ALT | KEYM_CTRL);

	// spu debug
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "toggle hair spu performances report", int(F_SPU_DEBUG_PERFORMANCE), KEYS_PRESSED, KEYC_F1, KEYM_SHIFT | KEYM_CTRL);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "toggle hair spu breakpoint", int(F_SPU_DEBUG_BREAKPOINT), KEYS_PRESSED, KEYC_F2, KEYM_SHIFT | KEYM_CTRL);
	KeyBindManager::Get().RegisterKey("dynamics", pToggle, "toggle hair spu in/out notifyer", int(F_SPU_DEBUG_BEGINEND), KEYS_PRESSED, KEYC_F3, KEYM_SHIFT | KEYM_CTRL);
}



COMMAND_RESULT ChainRessource::CommandToggleFlags(const int& kind)
{
	switch(kind)
	{
	case F_USESPU:
		{
			m_debugMask.Toggle(F_USESPU);
			break;
		}
	case F_DRAWCOLSPHERE:
		{
			m_debugMask.Toggle(F_DRAWCOLSPHERE);
			m_pGlobalWelderDef->m_bDrawCollisionSphere = m_debugMask[F_DRAWCOLSPHERE];
			break;
		}
	case F_DRAWANTICOLSPHERE:
		{
			m_debugMask.Toggle(F_DRAWANTICOLSPHERE);
			m_pGlobalWelderDef->m_bDrawAntiCollisionSphere = m_debugMask[F_DRAWANTICOLSPHERE];
			break;
		}
	case F_ADDWIND:
		{
			GustOfWind* pt = NT_NEW_CHUNK ( Mem::MC_PROCEDURAL ) GustOfWind(
				TimeSequence(2.0f,TimeSequence::F_PLAYWHENCREATED),
				CPoint(-10.0f,2.0f,0.0f),
				CPoint(10.0f,2.0f,0.0f),
				20.0f);
			ForceFieldManager::Get().AddNewSequence(pt);
			break;
		}
	case F_SWAPDEBUGMODE:
		{
			m_debugMode.m_value++;
			m_debugMode.m_value%=m_debugMode.m_max;
			break;
		}
	case F_RESTARTSIM:
		{
			for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
				it != m_chainRegisters.end();
				it++)
			{
				OneChain& elem = *(*it);
				bool bIsActive = elem.Toggle();
				if(bIsActive)
				{
					elem.GetDataFromHierarchy();
				}
			}
			break;
		}
	case F_DRAWWIND:
		{
			m_debugMask.Toggle(F_DRAWWIND);
			break;
		}
	case F_SPU_DEBUG_PERFORMANCE:
		{
			m_debugMask.Toggle(F_SPU_DEBUG_PERFORMANCE);
			break;
		}
	case F_SPU_DEBUG_BREAKPOINT:
		{
			m_debugMask.Set(F_SPU_DEBUG_BREAKPOINT_AUX,true);
			break;
		}
	case F_SPU_DEBUG_BEGINEND:
		{
			m_debugMask.Toggle(F_SPU_DEBUG_BEGINEND);
			break;
		}
	default:
		{
			ntAssert_p(false, ("beurk !"));
			break;
		}
	}

	return CR_SUCCESS;
}


void ChainRessource::Init()
{
	//LoadDefFile();

	if(false)
	{
		for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
			it != m_chainRegisters.end();
			it++)
		{
			OneChain& elem = *(*it);
			elem.GetDataFromHierarchy();
			elem.TurnOn();
		}
	}
	
	// create breeze
	//CVector windForce = GetGlobalDef().m_obWindForce;
	//Vec3 spherical = SphericalCoordinate::CartesianToSpherical(CDirection(windForce));
	//m_wind.m_pMainBreeze = NT_NEW WindBreeze(spherical[2], spherical[0]);
	//m_wind.AddNewSequence(m_wind.m_pMainBreeze);
	
	

	//PlayAll(false);
}

void ChainRessource::LoadMayaHairCut()
{
	//// load everything on the fly
	//if(true)
	//{
	//	return;
	//}
	//// load all the hair style found in welder
	//else
	//{
	//	for(ntstd::List<HairStyleFromWelder*>::const_iterator it = m_pGlobalWelderDef->m_obHairstyle.begin();
	//		it != m_pGlobalWelderDef->m_obHairstyle.end();
	//		it++)
	//	{
	//		HairStyleFromWelder* pElem = *it;
	//		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pElem );
	//		ntstd::string name = pDO->GetName();
	//		
	//		XmlFileInterface* ptFile = XmlFileInterface::Create(name.c_str(),HairPath().c_str());
	//		ntAssert_p(ptFile, ("Cannot load def file"));
	//		m_onthefly.insert(ptFile);
	//	}
	//}
}

void ChainRessource::LoadCollisionSpheres()
{
	//// list
	//m_sphereRessource["collision_hero"] = 0;
	//m_sphereRessource["collision_stuff"] = 0;
	//
	//// load
	//for(SphereRessource::iterator it = m_sphereRessource.begin();
	//	it != m_sphereRessource.end();
	//	it++)
	//{
	//	it->second = 
	//}
}

// ALEXEY_TODO : baaad change the name to be HashedString
const HairStyleFromWelder* ChainRessource::GetWelderHairStyle(const ntstd::String& pcNameAux)
{
	HairStyleFromWelder* pt = ObjectDatabase::Get().GetPointerFromName<HairStyleFromWelder*>(CHashedString(pcNameAux));
	if(!pt)
	{
		CHashedString pcName(HASH_STRING_HAIR_DEFAULT_GLOBAL);
		pt = ObjectDatabase::Get().GetPointerFromName<HairStyleFromWelder*>(pcName);
		ntError_p(pt!=0, ("CreateComponent_Hair: Cannot fin default hair style."));
		OSD::Add( OSD::HAIR, 0xffffffff, "Using default welder hair style for: %s.", ntStr::GetString(pcNameAux) );
	}
	return pt;
}

const HairStyleFromMaya* ChainRessource::GetMayaHairStyle(const ntstd::String& pcName)
{
	HairStyleFromMaya* pt = ObjectDatabase::Get().GetPointerFromName<HairStyleFromMaya*>(CHashedString(pcName));
	if(!pt)
	{
		XmlFileInterface* ptFile = XmlFileInterface::Create(pcName.c_str(),HairPath().c_str());
		m_onthefly.insert(ptFile);
		pt = static_cast<HairStyleFromMaya*>(ptFile);
	}
	return pt;
}

const HairSphereSetDef* ChainRessource::GetCollisionSet(const ntstd::String& pcName)
{
	HairSphereSetDef* pt = ObjectDatabase::Get().GetPointerFromName<HairSphereSetDef*>(CHashedString(pcName));
	if(!pt)
	{
		XmlFileInterface* ptFile = XmlFileInterface::Create(pcName.c_str(),HairPath().c_str());
		m_onthefly.insert(ptFile);
		pt = static_cast<HairSphereSetDef*>(ptFile);
	}
	return pt;
}

ntstd::String ChainRessource::HairPath()
{
	return "entities\\characters\\extra\\hair\\";
}

bool ChainRessource::Update()
{
	// MONSTERS\Frank 22/01/2006 17:40:08
	// big hack to handle jumping code
	static int iReset = 0;
	if(m_gameMask[F_RESET])
	{
		iReset=3;
		m_gameMask.Unset(F_RESET);
	}
	if(iReset>0)
	{
		for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
			it != m_chainRegisters.end();
			it++)
		{
			(*it)->GetDataFromPose();
		}
		--iReset;
	}


	if(m_gameMask[F_INITME])
	{
		Init();
		m_gameMask.Unset(F_INITME);
	}

	m_debugMask.Set(F_SPU_DEBUG_BREAKPOINT,false);
	if(m_debugMask[F_SPU_DEBUG_BREAKPOINT_AUX])
	{
		m_debugMask.Set(F_SPU_DEBUG_BREAKPOINT,true);
		m_debugMask.Set(F_SPU_DEBUG_BREAKPOINT_AUX,false);
	}

	// unset debug mode
	//m_debugMask.Unset(ChainRessource::F_DEBUG);

	//KeyEvent();
		
	m_sequence.Update(CTimer::Get().GetGameTimeInfo());
	//m_wind.Update(CTimer::Get().GetGameTimeInfo());
	
	if(ChainRessource::Get().GetGlobalDef().m_bDrawInfo)
	{
		OSD::Add( OSD::HAIR, 0xffffffff, "m_iNbDetection = %d", m_iNbDetection);
	}

	m_iNbDetection=0;
	
	if(m_chainRegisters.size()==0)
	{
		return false;
	}

	DrawDebug();
	
	return true;
}


void ChainRessource::DrawDebug()
{
	if(m_pGlobalWelderDef->m_bDrawChain)
	{
		DrawChainDebug();
	}
	if(m_pGlobalWelderDef->m_bDrawDefaultPosition)
	{
		DrawDefaultPosition();
	}
	if(m_pGlobalWelderDef->m_bDrawCollisionSphere)
	{
		DrawCollisionSpheres();
	}
	if(m_pGlobalWelderDef->m_bDrawAntiCollisionSphere)
	{
		DrawAntiCollisionSpheres();
	}
	if(m_pGlobalWelderDef->m_bDrawWind)
	{
		ForceFieldManager::Get().DrawDebug();
	}
	if(m_pGlobalWelderDef->m_bDrawCollisionDiff)
	{
		DrawDebugCollision();
	}
}

void ChainRessource::DrawDebugCollision()
{
	for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
		it != m_chainRegisters.end();
		it++)
	{
		(*it)->DrawDebugCollision();
	}
}

void ChainRessource::DrawMainFrame()
{
	for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
		it != m_chainRegisters.end();
		it++)
	{
		(*it)->DrawMainFrame();
	}
}

void ChainRessource::DrawCollisionSpheres()
{
	for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
		it != m_chainRegisters.end();
		it++)
	{
		(*it)->DrawCollisionSpheres();
	}
}

void ChainRessource::DrawAntiCollisionSpheres()
{
	for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
		it != m_chainRegisters.end();
		it++)
	{
		(*it)->DrawAntiCollisionSpheres();
	}

}


void ChainRessource::DrawChainDebug()
{
	for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
		it != m_chainRegisters.end();
		it++)
	{
		switch(m_debugMode.m_value)
		{
			case 0:
			{
				// nothing
				break;
			}
			case 1:
			{
				(*it)->DrawDebug(GetGlobalDef().m_bDrawAxis);
				break;
			}
			case 2:
			{
				(*it)->DrawDebugHierarchy();
				break;
			}
			case 3:
			{
				(*it)->DrawDebugRawHierarchy();
				break;
			}
			case 4:
			{
				(*it)->DrawDefaultPosition();
				break;
			}
			case 5:
			{
				(*it)->DrawSpringDebug();
				break;
			}
			case 6:
			{
				(*it)->DrawDummy();
				break;
			}
			default:
			{
				break;
			}
		}
	}		
}

void ChainRessource::DrawDefaultPosition()
{
	for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
		it != m_chainRegisters.end();
		it++)
	{
		(*it)->DrawDefaultPosition();
	}
}
void ChainRessource::Reset()
{
	m_gameMask.Set(F_RESET);
}
void ChainRessource::LevelReset()
{
	// clear registered resources that only have level scope lifetime
	m_onthefly.clear();
}

//void ChainRessource::KeyEvent()
//{
//	// F1 -> show sphere
//	if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F1, KEYM_ALT ))
//	{
//		m_pGlobalWelderDef->m_bDrawCollisionSphere = !m_pGlobalWelderDef->m_bDrawCollisionSphere;
//	}
//
//	// F1 -> show sphere
//	if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F1, KEYM_ALT ))
//	{
//		m_pGlobalWelderDef->m_bDrawAntiCollisionSphere = !m_pGlobalWelderDef->m_bDrawAntiCollisionSphere;
//	}
//	
//	//// F2 -> add wind
//	//if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F2 ))
//	//{
//	//	GustOfWind* pt = NT_NEW GustOfWind(
//	//		TimeSequence(2.0f,TimeSequence::F_PLAYWHENCREATED),
//	//		CPoint(-10.0f,2.0f,0.0f),
//	//		CPoint(10.0f,2.0f,0.0f),
//	//		20.0f);
//	//	ForceFieldManager::Get().AddNewSequence(pt);
//	//		
//	//	//m_wind.AddNewSequence(NT_NEW WindGust(CVector(CONSTRUCT_CLEAR),0.0f,5.0f,10.0f,0.5f,10.0f));
//	//}
//	
//	// F3 -> swap debug rendering mode
//	if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F3, KEYM_ALT ))
//	{
//		m_debugMode.m_value++;
//		m_debugMode.m_value%=m_debugMode.m_max;
//	}
//	
//	// f4 -> restart simulation
//	if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F6, KEYM_ALT ))
//	{
//		for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
//			it != m_chainRegisters.end();
//			it++)
//		{
//			OneChain& elem = *(*it);
//			bool bIsActive = elem.Toggle();
//			if(bIsActive)
//			{
//				elem.GetDataFromHierarchy();
//			}
//		}
//	}	
//	
//	// f5 -> restart simulation
//	if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F2, KEYM_ALT ))
//	{
//		PlayAll(true);
//	}
//	
//	// F1 -> show sphere
//	if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F7, KEYM_ALT ))
//	{
//		m_mask.Toggle(F_DRAWWIND);
//	}
//	
//	//// f5 -> draw simulation frame
//	//if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F8, KEYM_ALT ))
//	//{
//	//	m_mask.Toggle(F_DRAWMAINFRAME);
//	//}
//
//	//// f5 -> draw simulation frame
//	//m_mask.Unset(ChainRessource::F_DEBUG);
//	//if(CInputHardware::Get().GetKeyboardP()->IsKeyPressed( KEYC_F9, KEYM_ALT ))
//	//{
//	//	m_mask.Toggle(F_DEBUG);
//	//}
//}

void ChainRessource::PlayAll(bool bToggle)
{
	for(RegisterContainer<OneChain>::iterator it = m_chainRegisters.begin();
		it != m_chainRegisters.end();
		it++)
	{
		OneChain& elem = *(*it);
		bool bIsActive = true;
		if(bToggle)
		{
			bIsActive = elem.Toggle();
		}
		else
		{
			elem.TurnOn();
		}
		
		if(bIsActive)
		{
			elem.GetDataFromPose();
		}
	}
}


void ChainRessource::LowFreqUpdate()
{
#if !defined (_RELEASE) && !defined(_MASTER)
	for(ntstd::Set<XmlFileInterface*>::iterator it = m_onthefly.begin();
		it != m_onthefly.end();
		it++)
	{
		XmlFileInterface* pInterface = *it;
		if(XmlFileInterface::Reload(pInterface))
		{
			OSD::Add( OSD::HAIR, 0xffffffff, "%s reloaded", ntStr::GetString(pInterface->m_pMainObject->GetName()));
		}
	}
#endif
}




namespace 
{
	
	DataObject* LoadDataObjectWithinFile(const ntstd::String& fileName, const ntstd::String& objectName = ntstd::String())
	{
		if(!ObjectDatabase::Get().GetDataObjectFromName( CHashedString(fileName)))
		{
			static char acFileName[MAX_PATH];
			Util::GetFiosFilePath( fileName.c_str(), acFileName );
			FileBuffer file(acFileName);
			ObjectDatabase::Get().LoadDataObject( &file, fileName.c_str() );
		}
		
		if(!objectName.empty())
		{
			DataObject* pRes = ObjectDatabase::Get().GetDataObjectFromName(CHashedString(objectName));
			ntAssert(pRes); 
			return pRes;
		}
		else
		{
			return 0;
		}
	}
	
	
	
} // end of namespace 


// create the singleton
void ChainRessource::LoadDefFile()
{
	m_pGlobalWelderDef = static_cast<ChainGlobalDef*>(LoadDataObjectWithinFile(HairPath() + "chain.xml", "GlobalHairStuff")->GetBasePtr());
	LoadDataObjectWithinFile(HairPath() + "artificialwind.xml");
}

// reset dynamic
void ChainRessource::Register(OneChain* pOneChain)
{
	m_chainRegisters.insert(pOneChain);
	//pOneChain->TurnOn();
}

// create the singleton
void ChainRessource::Unregister(OneChain* pOneChain)
{
	m_chainRegisters.erase(pOneChain);
}

// auxiliary function
OneChain* ChainRessource::CreateComponent_Hair(CEntity* pobEnt, LuaAttributeTable* pAttrTable)
{
	ntError_p(ChainRessource::Exists(), ("Trying to create hair wihtout ChainRessource"));
	ntError_p(pobEnt->GetHierarchy()!=0, ("CreateComponent_Hair: Entity has no hierarchy"));
	ntError_p(pAttrTable!=0, ("No Lua Attribuate table (null pointer)"));
	//ntAssert( pAttrTable );
		
	// set parent entity
	CEntity* pCharacter = pAttrTable->GetAttribute("Parent").GetUserData<CEntity*>();
	ntError_p(pCharacter!=0, ("CreateComponent_Hair: bad parent pointer"));
	pobEnt->SetParentEntity(pCharacter);
	
	// find collision informatin
	ntstd::String collisionName("");
	if(!pAttrTable->GetAttribute("CollisionSetName").IsNil())
	{
		collisionName = pAttrTable->GetString("CollisionSetName");
	}
	
	// find hair style information (welder)
	ntstd::String pcGlobalDefName = pAttrTable->GetString("HairStyleName") + ntstd::String("_global");
	if(!pAttrTable->GetAttribute("GlobalDefName").IsNil())
	{
		pcGlobalDefName = pAttrTable->GetString("GlobalDefName");
	}
	const HairStyleFromWelder* pWelderHairStyle = ChainRessource::Get().GetWelderHairStyle(pcGlobalDefName);
	ntError_p(pWelderHairStyle!=0, ("CreateComponent_Hair: Bad Welder hair style: %s",pcGlobalDefName.c_str()));

	// find hair style information (maya)
	ntstd::String pcStyleName = pAttrTable->GetString("HairStyleName");
	const HairStyleFromMaya* pMayaHairStyle = ChainRessource::Get().GetMayaHairStyle(pcStyleName);
	ntError_p(pMayaHairStyle!=0, ("CreateComponent_Hair: bad maya hair style: %s",pcStyleName.c_str()));

	// Try to find the requested transform on the new parent heirarchy
	int iIdx = pCharacter->GetHierarchy()->GetTransformIndex( CHashedString( pMayaHairStyle->m_parentTransformName.c_str() ) );

	// Make sure that we found it
	if ( iIdx == -1 )
	{
		user_warn_p( 0, ( "Can't find transform '%s' on '%s' to reparent to.\n", pMayaHairStyle->m_parentTransformName.c_str(), pCharacter->GetName().c_str() ) );
		return 0;
	}

	Transform* pobParentTransform = pCharacter->GetHierarchy()->GetTransform( iIdx );
	Transform* pobTargTransform = pobEnt->GetHierarchy()->GetRootTransform();	

	// Set the parent pointer on the entity
	pobEnt->SetParentEntity( pCharacter );

	// Now deal explicitly with the transforms
	pobTargTransform->RemoveFromParent();
	pobParentTransform->AddChild( pobTargTransform );

	//pMayaHairStyle->AjustHierarchy(pobEnt->GetHierarchy());
	OneChain* pHair = NT_NEW OneChain(pobEnt,collisionName,pWelderHairStyle,pMayaHairStyle);
//	pobEnt->AddAnonComponent(pHair);

	pobEnt->SetOneChain(pHair);
//	pHair->Register();
	
	return pHair;
}
