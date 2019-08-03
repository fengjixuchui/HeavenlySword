#ifndef _LUAEXPTYPES_H
#define _LUAEXPTYPES_H

#include "lua/ninjalua.h"

// Forward declare types
class CEntity;
class CAttackComponent;
class CAIComponent;
class SceneElementComponent;
class BasicCameraTemplate;
class CamView;
class CMessageHandler;
class CAnimator;
class CMovement;
class CRenderableComponent;
class AIFormationManager;
class AIFormation;
class AIFormationAttack;
class AIBehaviourPool;
class CAIStateMachine;
class CEntityManager;
class CEntityInfo;
class CInteractionComponent;
class CInputComponent;
class Army;
class ArmyUnit;
class AICombatComponent;
class ChainmanChains;
class HitCounter;
class CEntityAudioChannel;
class FormationComponent;
class AIFormationComponent;
class LuaAttributeTable;
namespace Physics {class System;}
class CChatterBoxMan;

#if !defined(PLATFORM_PS3)
	class CAINavigationSystemMan; // !!! -  New Navigation System ( under test ) (Dario)
#endif

class Message;
class Player;

// declare the types
/*LV_DECLARE_USERDATA(CEntity);
LV_DECLARE_USERDATA(CAttackComponent);
LV_DECLARE_USERDATA(CAIComponent);
LV_DECLARE_USERDATA(CEntityInfo);
LV_DECLARE_USERDATA(SceneElementComponent);
LV_DECLARE_USERDATA(BasicCameraTemplate);
LV_DECLARE_USERDATA(CamView);
LV_DECLARE_USERDATA(CMessageHandler);
LV_DECLARE_USERDATA(AIFormationManager);
LV_DECLARE_USERDATA(AIFormation);
LV_DECLARE_USERDATA(AIFormationAttack);
LV_DECLARE_USERDATA(CAIStateMachine);
LV_DECLARE_USERDATA(AIBehaviourPool);
LV_DECLARE_USERDATA(CEntityManager);
LV_DECLARE_USERDATA(CAnimator);
LV_DECLARE_USERDATA(CMovement);
LV_DECLARE_USERDATA(CRenderableComponent);
LV_DECLARE_USERDATA(CInputComponent);
LV_DECLARE_USERDATA(CInteractionComponent);
LV_DECLARE_USERDATA(Army);
LV_DECLARE_USERDATA(ArmyUnit);
LV_DECLARE_USERDATA(AICombatComponent);
LV_DECLARE_USERDATA(ChainmanChains);
LV_DECLARE_USERDATA(HitCounter);
LV_DECLARE_USERDATA(CEntityAudioChannel);
LV_DECLARE_USERDATA(FormationComponent);
LV_DECLARE_USERDATA(AIFormationComponent);
LV_DECLARE_USERDATA(LuaAttributeTable);
LV_DECLARE_USERDATA2(Physics::System, System);
LV_DECLARE_USERDATA(CChatterBoxMan);
#if !defined(PLATFORM_PS3)
	LV_DECLARE_USERDATA(CAINavigationSystemMan); // New Navigation System Manager (Dario)
#endif
LV_DECLARE_USERDATA(Message);
LV_DECLARE_USERDATA(Player);*/


#endif // _LUAEXPTYPES_H

