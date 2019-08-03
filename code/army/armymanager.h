//------------------------------------------------------
//
// The Army manager, where the armies are at
//------------------------------------------------------

#ifndef ARMYMANAGER_H_
#define ARMYMANAGER_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "game/commandresult.h"
#include "gfx/material.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************
struct General;
struct Battlefield;
struct Battalion;
struct Grunt;
struct Unit;
class ArmySection;
class ArmyBattlefield;
class ArmyAudioDefinition;

//**************************************************************************************
//	Typedefs.
//**************************************************************************************
#define ARMY_SPU_ELF			"army_spu_ps3.mod"
#define ARMY2_SPU_ELF			"army2_spu_ps3.mod"
#define BATCH_RENDER_SPU_ELF	"bren_spu_ps3.mod"

//------------------------------------------------------
//!
//! ArmyManager, singleton that gets ticked and manages
//! armies... Say what it does on the tin.
//!
//------------------------------------------------------
class ArmyManager : public Singleton< ArmyManager >
{
public:
	// Construction / Destruction
	// --------------------------
	ArmyManager();
	~ArmyManager();

	// Update
	void ForceMaterials( CMaterial const* pobMaterial );
	void Update( float fTimeStep );

	// do not use yet..
	void UpdateKickSPUs( float fTimeStep );
	void Render();

	void Activate( const ArmyBattlefield* pBattlefield );
	void Deactivate(); 

	void GlobalEventMessage( const CHashedString obGlobalEvent );

	void RegisterAudioDef (ArmyAudioDefinition* pAudioDef);

	// every frame update its position
	void SetBazookaShotPosition( const CPoint& pos, CHashedString obRocketName );
	// explode a bit
	void ExplodeBazookaShot( const CPoint& pos );

	// Wake attack/strike volume stuff
	void SyncAttackStrikeVolumeAt( const CPoint& pos, float fRadius );
	void SpeedAttackStrikeVolumeAt( const CPoint& pos, float fRadius );
	void PowerAttackStrikeVolumeAt( const CPoint& pos, float fRadius );
	void RangeAttackStrikeVolumeAt( const CPoint& pos, float fRadius );

	// every frame update the main camera position
	void SetCamera( const CPoint& pos, const CDirection& dir );

	unsigned int GetBodyCount();

	COMMAND_RESULT ToggleDebugMode();
private:
	ArmySection* m_pSection;

	ArmyAudioDefinition* m_pAudioDef;

	void RegisterSection( ArmySection* pSection );
	void DeleteCurrentSection();
};


//------------------------------------------------------
//!
//! ArmyAudioDefinition
//! 
//!
//------------------------------------------------------
class ArmyAudioDefinition
{
public:

	ArmyAudioDefinition ();
	~ArmyAudioDefinition ();

	void PostConstruct ();
	bool EditorChangeValue(CallBackParameter /*obItem*/,CallBackParameter /*obValue*/);

	void Update (ArmySection* pArmySection);
	void DebugRender ();

	void ExplosionEvent (ArmySection* pArmySection,const CPoint& obPosition);

	// ----- Serialised members -----

	bool m_bDebugRender;

	float m_fMaxBattalionRange; // The range at which the sound will actually kick in

	float m_fBlownUpTestRadius; // The radius at which we check how many units are still alive
	float m_fBlownUpSoundRadius; // The radius at which we will hear the blownup sound

	//float m_fSoundFadeTime;
	
	CKeyString m_obSoundForwardMarch;
	CKeyString m_obSoundHoldPosition;
	CKeyString m_obSoundRetreat;
	CKeyString m_obSoundFlee;
	CKeyString m_obSoundAttack;
	CKeyString m_obSoundCharge;
	CKeyString m_obSoundTaunt;
	CKeyString m_obSoundBlownUp;

protected:

	const char* GetSound (int iOrders);
	void UpdateInfo (ArmySection* pArmySection,int iOrders);

	static const int iTOTAL_GROUPS = 10;

	struct BATTALION_GROUP_INFO
	{
		int				m_iTotalUnits;
		CPoint			m_obMinExtents;
		CPoint			m_obMaxExtents;
		CDirection		m_obHalfExtents;
		CPoint			m_obPosition;
		unsigned int	m_uiSoundID;
		float			m_fVolume;
		bool			m_bInvalidSound;
	};

	BATTALION_GROUP_INFO m_BattalionInfo [iTOTAL_GROUPS]; // Each battalion group corresponds to an order type
};




#endif	// !UNIT_H_

