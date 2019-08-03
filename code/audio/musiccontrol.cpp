//--------------------------------------------------------------------------------------
//!	@file musiccontrol.cpp
//!	@author Chip Bell (SCEE)
//!	@date 10.10.06
//!
//!	@brief Implementation of the MusicControlManager and related classes.
//--------------------------------------------------------------------------------------


#include "audio/musiccontrol.h"

#include "audio/imusic.h"
#include "core/timer.h"
#include "objectdatabase/dataobject.h"
#include "game/shellconfig.h"

#ifdef _MUSIC_CONTROL_DEBUG
#include "audio/audiosystem.h"
#include "core/visualdebugger.h"
#include "input/inputhardware.h"
#endif // _MUSIC_CONTROL_DEBUG


#define FLOAT_COMPARE(first, second, tolerance)	\
	(((first) > ((second) - (tolerance))) && ((first) < ((second) + (tolerance))))


START_STD_INTERFACE(MusicControlManagerSettings)
	PUBLISH_VAR_AS(m_fGlobalActivationLapse,	GlobalActivationLapse)

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditorChangeValue)
	DECLARE_EDITORSELECT_CALLBACK(EditorSelect)
	DECLARE_EDITORSELECTPARENT_CALLBACK(EditorSelectParent)
END_STD_INTERFACE


//--------------------------------------------------------------------------------------
//	MusicControlManagerSettings
//!	Default ctor.
//--------------------------------------------------------------------------------------
MusicControlManagerSettings::MusicControlManagerSettings(void)
:
	// Intialise to defaults
	m_fGlobalActivationLapse(0.0f)
{}


//--------------------------------------------------------------------------------------
//	~MusicControlManagerSettings
//!	Default dtor.
//--------------------------------------------------------------------------------------
MusicControlManagerSettings::~MusicControlManagerSettings(void)
{}


//--------------------------------------------------------------------------------------
//	PostContruct
//!	Finalises initialisation, applies music control manager settings.
//--------------------------------------------------------------------------------------
void MusicControlManagerSettings::PostConstruct(void)
{
	Synchronise();
}


//------------------------------------------------------------------------------------------
//	EditorChangeValue
//!	Debug editor callback.
//!	@param obItem	Name of item which has been changed.
//!	@param obValue	New value.
//!	@return True if notification processed successfully, false otherwise (indicates default
//!	change behavior should not proceed).
//------------------------------------------------------------------------------------------
bool MusicControlManagerSettings::EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue)
{
	UNUSED(obItem);
	UNUSED(obValue);

	// Just sync variable update
	Synchronise();
	return true;
}


//------------------------------------------------------------------------------------------
//	EditorSelect
//!	Indicates item has been selected/deselected in Welder.
//!	@param bSelect	Selected (true) or deselected (false).
//------------------------------------------------------------------------------------------
void MusicControlManagerSettings::EditorSelect(bool bSelect)
{
	UNUSED(bSelect);
	/*
	if (bSelect)
	{
		Populate();
	}
	*/
}


//------------------------------------------------------------------------------------------
//	EditorSelectParent
//!	Indicates item parent has been selected/deselected in Welder.
//!	@param bSelect	Selected (true) or deselected (false).
//------------------------------------------------------------------------------------------
void MusicControlManagerSettings::EditorSelectParent(bool bSelect)
{
	if (bSelect)
	{
		Populate();
	}
}


//------------------------------------------------------------------------------------------
//	Synchronise
//!	Synchronises settings data with music control manager.
//------------------------------------------------------------------------------------------
void MusicControlManagerSettings::Synchronise(void)
{
	// Verify music control manager exists
	if (!MusicControlManager::Get().Exists())
	{
#ifdef _MUSIC_CONTROL_DEBUG
		user_warn_msg(("Attempting to synchronise music control manager settings when manager does not exist!"));
#endif // MUSIC_CONTROL_DEBUG
		return;
	}

	MusicControlManager::Get().SetGlobalActivationLapse(m_fGlobalActivationLapse);
}


//------------------------------------------------------------------------------------------
//	Populate
//!	Populates settings members with information from music control manager.
//------------------------------------------------------------------------------------------
void MusicControlManagerSettings::Populate(void)
{
	// Verify music control manager exists
	if (!MusicControlManager::Get().Exists())
		return;

	m_fGlobalActivationLapse = MusicControlManager::Get().GetGlobalActivationLapse();
}




START_STD_INTERFACE(MusicControlTrackingElement)
	PUBLISH_ACCESSOR(CHashedString,			TrackingElementName,	GetName, SetName)
	PUBLISH_VAR_AS(m_obGroupLabel,			GroupLabel)
	PUBLISH_VAR_AS(m_obInputName,			InputName)
	PUBLISH_GLOBAL_ENUM_AS(m_eRuleType,		RuleType,				TRACKING_ELEMENT_RULE_TYPE)
	PUBLISH_VAR_AS(m_fRuleData1,			RuleData1)
	PUBLISH_VAR_AS(m_fRuleData2,			RuleData2)
	PUBLISH_GLOBAL_ENUM_AS(m_eActionType,	ActionType,				TRACKING_ELEMENT_ACTION_TYPE)
	PUBLISH_VAR_AS(m_fActionData1,			ActionData1)
	PUBLISH_VAR_AS(m_fActionData2,			ActionData2)
	PUBLISH_VAR_AS(m_fInfluenceMin,			InfluenceMin)
	PUBLISH_VAR_AS(m_fInfluenceMax,			InfluenceMax)
	PUBLISH_VAR_AS(m_fActivationLapse,		ActivationLapseTime)
	PUBLISH_VAR_AS(m_bEnable,				Enable)
	PUBLISH_VAR_AS(m_bExemptFromSuspend,	ExemptFromGlobalLapse)

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditorChangeValue)
	DECLARE_EDITORSELECT_CALLBACK(EditorSelect)
	DECLARE_EDITORSELECTPARENT_CALLBACK(EditorSelectParent)
END_STD_INTERFACE


//--------------------------------------------------------------------------------------
//	MusicControlTrackingElement
//!	Default ctor.
//--------------------------------------------------------------------------------------
MusicControlTrackingElement::MusicControlTrackingElement(void)
:
	// Intialise to defaults
	m_eRuleType(THRESHOLD),
	m_fRuleData1(0.0f),
	m_fRuleData2(0.0f),
	m_eActionType(FIXED_INTENSITY),
	m_fActionData1(0.0f),
	m_fActionData2(0.0f),
	m_fInfluenceMin(0.0f),
	m_fInfluenceMax(1.0f), // Default action influence is the full range
	m_fActivationLapse(0.0f),
	m_bEnable(false),
	m_bExemptFromSuspend(false)
{}


//--------------------------------------------------------------------------------------
//	~MusicControlTrackingElement
//!	Default dtor.
//--------------------------------------------------------------------------------------
MusicControlTrackingElement::~MusicControlTrackingElement(void)
{
	if (MusicControlManager::Exists())
	{
		MusicControlManager::Get().DeregisterRule(m_obName, true);
		MusicControlManager::Get().DeregisterAction(m_obName, true);
	}
}


//--------------------------------------------------------------------------------------
//	PostContruct
//!	Finalises initialisation, registering tracking element with music control manager as
//!	appropriate.
//--------------------------------------------------------------------------------------
void MusicControlTrackingElement::PostConstruct(void)
{
	Synchronise();
}


//------------------------------------------------------------------------------------------
//	EditorChangeValue
//!	Debug editor callback.
//!	@param obItem	Name of item which has been changed.
//!	@param obValue	New value.
//!	@return True if notification processed successfully, false otherwise (indicates default
//!	change behavior should not proceed).
//------------------------------------------------------------------------------------------
bool MusicControlTrackingElement::EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue)
{
	UNUSED(obItem);
	UNUSED(obValue);

	// Just sync variable update
	Synchronise();
	return true;
}


//------------------------------------------------------------------------------------------
//	EditorSelect
//!	Indicates item has been selected/deselected in Welder.
//!	@param bSelect	Selected (true) or deselected (false).
//------------------------------------------------------------------------------------------
void MusicControlTrackingElement::EditorSelect(bool bSelect)
{
	UNUSED(bSelect);
	/*
	if (bSelect)
	{
		Populate();
	}
	*/
}


//------------------------------------------------------------------------------------------
//	EditorSelectParent
//!	Indicates item parent has been selected/deselected in Welder.
//!	@param bSelect	Selected (true) or deselected (false).
//------------------------------------------------------------------------------------------
void MusicControlTrackingElement::EditorSelectParent(bool bSelect)
{
	if (bSelect)
	{
		Populate();
	}
}


//------------------------------------------------------------------------------------------
//	SetName
//!	Sets tracking element name (for Welder integration). Wrapper for internal rename processing.
//!	@note New name may not be accepted (name clash with existing tracking element). Newly
//!	set names can be verified only by checking the result of subsequent get name calls. If new
//!	name is unacceptable, name will be nullified!
//!	@sa MusicControlTrackingElement::GetName(void) const
//!	@sa MusicControlTrackingElement::Rename(CHashedString obNewName)
//------------------------------------------------------------------------------------------
void MusicControlTrackingElement::SetName(const CHashedString& obName)
{
	if (!Rename(obName))
		Rename(CHashedString::nullString);
}


//------------------------------------------------------------------------------------------
//	Rename
//!	Renames tracker element.
//!	@param obNewName	Desired new name.
//!	@return True if rename successful, false otherwise. If rename fails, no change is made.
//!	@note Tracking element can be renamed to null (it will, however, be deleted).
//!	@note Will be resynchronised if rename successful.
//------------------------------------------------------------------------------------------
bool MusicControlTrackingElement::Rename(const CHashedString& obNewName)
{
	// Check if new name matches current
	if (m_obName == obNewName)
	{
		// Should simply sync
		Synchronise();
		return true;
	}

	// User is changing the tracking element name, see if new name already exists
	if (!obNewName.IsNull() && MusicControlManager::Get().GetRule(obNewName))
	{
		// Name is not unique, and it really should be
		user_warn_msg(("Tracking element name %s already exists!", obNewName.GetDebugString()));
		return false;
	}

	// Deregister current tracking element components as req'd
	if (!m_obName.IsNull())
	{
		MusicControlManager::Get().DeregisterAction(m_obName, true);
		MusicControlManager::Get().DeregisterRule(m_obName, true);
	}

	m_obName = obNewName;
	Synchronise();
	return true;
}


//------------------------------------------------------------------------------------------
//	Synchronise
//!	Synchronises tracker element data with manager inputs/action/rule registrations.
//------------------------------------------------------------------------------------------
void MusicControlTrackingElement::Synchronise(void)
{
	// Obtain tracking element name (identifies associated rule and action)
	/*
	DataObject* pobThis = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	ntAssert_p(pobThis, ("Sorry, tracking elements should only be created using the data object system."));
	CHashedString obName = CHashedString(pobThis->GetName());
	*/
	CHashedString obName = m_obName;
	if (obName.IsNull())
	{
		// Nothing to sync
		return;
	}

	// Lookup/create rule
	MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(obName);
	if (!pobRule)
	{
		// Create and register rule
		pobRule = NT_NEW MusicControlManager::MusicControlRule();
		if (!MusicControlManager::Get().RegisterRule(pobRule, obName))
		{
			// Won't happen :)
			ntAssert_p(0, ("Error registering rule for tracking element %s!", obName.GetDebugString()));
			delete pobRule;
			return;
		}
	}

	// Lookup/create action
	MusicControlManager::MusicControlAction* pobAction = MusicControlManager::Get().GetAction(obName);
	if (!pobAction)
	{
		// Create and register action
		pobAction = NT_NEW MusicControlManager::MusicControlAction();
		if (!MusicControlManager::Get().RegisterAction(pobAction, obName))
		{
			// Won't happen :)
			ntAssert_p(0, ("Error registering action for tracking element %s!", obName.GetDebugString()));
			delete pobAction; // Leave rule registered... might as well
			return;
		}
	}

	// Lookup and set input on rule
	MusicControlManager::MusicControlInput* pobInput = MusicControlManager::Get().GetInput(m_obInputName);
	pobRule->SetInput(pobInput);
	// user_warn_p(pobInput, ("No input %s for tracking element %s!", m_obInputName.GetDebugString(), obName.GetDebugString()));

	// Set action on rule
	pobRule->SetAction(pobAction);

	// Setup rule
	pobRule->SetGroupLabel(m_obGroupLabel);
	switch (m_eRuleType)
	{
	case THRESHOLD:
		pobRule->SetThreshold(m_fRuleData1, false);
		break;
	case THRESHOLD_INVERTED:
		pobRule->SetThreshold(m_fRuleData1, true);
		break;
	case RANGE:
		pobRule->SetRange(m_fRuleData1, m_fRuleData2, false);
		break;
	case RANGE_INVERTED:
		pobRule->SetRange(m_fRuleData1, m_fRuleData2, true);
		break;
	case DELTA:
		pobRule->SetDelta(m_fRuleData1, m_fRuleData2);
		break;
	default:
		return;
	}
	pobRule->SetActivationLapse(m_fActivationLapse);
	pobRule->Enable(m_bEnable);
	pobRule->ExemptFromSuspend(m_bExemptFromSuspend);

	// Setup action
	switch(m_eActionType)
	{
	case FIXED_INTENSITY:
		pobAction->SetFixedIntensity(m_fActionData1, false);
		break;
	case FIXED_INTENSITY_SUSTAINED:
		pobAction->SetFixedIntensity(m_fActionData1, true);
		break;
	case SCALED_INTENSITY:
		pobAction->SetIntensityRange(m_fActionData1, m_fActionData2);
		break;
	case RELATIVE_INTENSITY:
		pobAction->SetRelativeIntensity(m_fActionData1);
		break;
	}
	pobAction->SetInfluenceRange(m_fInfluenceMin, m_fInfluenceMax);
}


//------------------------------------------------------------------------------------------
//	Populate
//!	Populates members of this tracking element with information from associated rule and action.
//------------------------------------------------------------------------------------------
void MusicControlTrackingElement::Populate(void)
{
	// Obtain tracking element name (identifies associated rule and action)
	/*
	DataObject* pobThis = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	ntAssert_p(pobThis, ("Sorry, tracking elements should only be created using the data object system."));
	CHashedString obName = CHashedString(pobThis->GetName());
	*/
	CHashedString obName = m_obName;
	if (obName.IsNull())
	{
		// Nothing to populate
		return;
	}

	// Lookup rule and action
	MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(obName);
	MusicControlManager::MusicControlAction* pobAction = MusicControlManager::Get().GetAction(obName);
	if (!pobRule || !pobAction)
	{
		// Nothing to populate
		return;
	}

	// Lookup input name
	m_obInputName = MusicControlManager::Get().GetInputHash(pobRule->GetInput());
	// user_warn_p(!m_obInputName.IsNull(), ("No input for tracking element %s!", m_obInputName.GetDebugString(), obName.GetDebugString()));

	// Setup rule data
	m_obGroupLabel = pobRule->GetGroupLabel();
	m_eRuleType = pobRule->GetType();
	switch (m_eRuleType)
	{
	case THRESHOLD:
	case THRESHOLD_INVERTED:
		pobRule->GetThresholdValue(m_fRuleData1);
		m_fRuleData2 = 0.0f;
		break;
	case RANGE:
	case RANGE_INVERTED:
		pobRule->GetRangeValues(m_fRuleData1, m_fRuleData2);
		break;
	case DELTA:
		pobRule->GetDeltaInfo(m_fRuleData1, m_fRuleData2);
		break;
	}
	m_fActivationLapse = pobRule->GetActivationLapse();
	m_bEnable = pobRule->IsEnabled();
	m_bExemptFromSuspend = pobRule->IsExemptFromSuspend();

	// Setup action data
	m_eActionType = pobAction->GetType();
	switch (m_eActionType)
	{
	case FIXED_INTENSITY:
	case FIXED_INTENSITY_SUSTAINED:
	case RELATIVE_INTENSITY:
		m_fActionData1 = pobAction->GetIntensityMin();
		m_fActionData2 = 0.0f;
		break;
	case SCALED_INTENSITY:
		m_fActionData1 = pobAction->GetIntensityMin();
		m_fActionData2 = pobAction->GetIntensityMax();
		break;
	}
	m_fInfluenceMin = pobAction->GetInfluenceMin();
	m_fInfluenceMax = pobAction->GetInfluenceMax();
}




//------------------------------------------------------------------------------------------
//	MusicControlRule (ctor)
//!	Initialises a rule.
//!	@note By default rules are disabled.
//------------------------------------------------------------------------------------------
MusicControlManager::MusicControlAction::MusicControlAction(void)
:
	// Be slightly careful initialising the action to a valid state
	// (note type and intensity req's should match, and the influenced range)
	m_eActionType(FIXED_INTENSITY),
	m_fIntensityReqMin(0.0f),
	m_fIntensityReqMax(0.0f),
	m_fInfluenceRangeMin(0.0f),
	m_fInfluenceRangeMax(1.0f),
	m_bActive(false)
#ifdef _MUSIC_CONTROL_DEBUG
	,
	m_dLastActivationTime(-1.0)
#endif // _MUSIC_CONTROL_DEBUG
{}


//------------------------------------------------------------------------------------------
//	~MusicControlAction (dtor)
//!	Deinitialises action and cleans up as req'd.
//------------------------------------------------------------------------------------------
MusicControlManager::MusicControlAction::~MusicControlAction(void)
{}


//------------------------------------------------------------------------------------------
//!	Indicates whether or not current music intensity is within the influenced range.
//!	@return Whether or not action influences current music intensity.
//------------------------------------------------------------------------------------------
bool MusicControlManager::MusicControlAction::IsInfluence(void)
{
	return InteractiveMusicManager::Get().GetIntensity() >= m_fInfluenceRangeMin
		&& InteractiveMusicManager::Get().GetIntensity() <= m_fInfluenceRangeMax;
}


//------------------------------------------------------------------------------------------
//	Activate
//!	Activates action. Following activation, behaviour is determined by the action.
//!	@return True if action will remaain activated and requires updates, false otherwise.
//!	@sa MusicControlManager::MusicControlAction::Deactivate(void)
//!	@sa MusicControlManager::MusicControlAction::Update(float fValue)
//------------------------------------------------------------------------------------------
bool MusicControlManager::MusicControlAction::Activate(void)
{
	// Verify
	if (m_bActive)
		return m_bActive;

	switch (m_eActionType)
	{
	case FIXED_INTENSITY_SUSTAINED:
	case SCALED_INTENSITY:
		// Variable/sustained intensity always continuously updated (variable requires actual reading from rule)
		m_bActive = true;
		break;
	case FIXED_INTENSITY:
	case RELATIVE_INTENSITY:
		// If intensity setting is fixed or relative, simply apply it here
		RequestIntensity(1.0f);
		m_bActive = false;
		break;
	default:
		break;
	}

	return m_bActive;
}


//------------------------------------------------------------------------------------------
//	Deactivate
//!	Deactivates action. Indicates updates have stopped.
//!	@sa MusicControlManager::MusicControlAction::Activate(void)
//!	@sa MusicControlManager::MusicControlAction::Update(float fValue)
//------------------------------------------------------------------------------------------
void MusicControlManager::MusicControlAction::Deactivate(void)
{
	// Verify
	if (!m_bActive)
		return;
	m_bActive = false;
}


//------------------------------------------------------------------------------------------
//	Update
//!	Updates action. Actions are activated before updating commences and deactivated when
//!	updating ceases.
//!	@sa MusicControlManager::MusicControlAction::Activate(void)
//!	@sa MusicControlManager::MusicControlAction::Deactivate(void)
//------------------------------------------------------------------------------------------
void MusicControlManager::MusicControlAction::Update(float fValue)
{
	// Only accept updates when active
	if (!m_bActive)
		return;

	// Continuous update as requested (e.g. to sustain a fixed intensity) or assumed for variable intensity
	if (FIXED_INTENSITY_SUSTAINED == m_eActionType || SCALED_INTENSITY == m_eActionType)
		RequestIntensity(fValue);
}


//------------------------------------------------------------------------------------------
//	RequestIntensity
//!	Verifies current music intensity in allowable range, then conforms a requested intensity
//!	value and sets it on the music manager.
//!	@param fRawValue	Raw value from rule (applied as a scalar to the intensity range set
//!						on this action.
//------------------------------------------------------------------------------------------
void MusicControlManager::MusicControlAction::RequestIntensity(float fRawValue)
{
	// Check if in active range
	if (!IsInfluence())
	{
		// Out of range
		return;
	}

	// Conform intensity
	float fIntensity = m_fIntensityReqMin + fRawValue*(m_fIntensityReqMax - m_fIntensityReqMin);
	if (RELATIVE_INTENSITY == m_eActionType)
		fIntensity += InteractiveMusicManager::Get().GetIntensity();

	// Cap
	if (fIntensity > 1.0f)
		fIntensity = 1.0f;
	else if (fIntensity < 0.0f)
		fIntensity = 0.0f;

	// Before requesting new intensity, store current for debug display
#ifdef _MUSIC_CONTROL_DEBUG
	float fCurrentIntensity = InteractiveMusicManager::Get().GetIntensity();
#endif // _MUSIC_CONTROL_DEBUG

	// Request
	InteractiveMusicManager::Get().SetIntensity(fIntensity);

#ifdef _MUSIC_CONTROL_DEBUG
	ntPrintf("Music control intensity req: %0.2f\n", fIntensity);

	m_dLastActivationTime = MusicControlManager::Get().GetMusicControlTime();

	MusicControlDebugDisplay::ActivationHistoryEntry stEntry;
	stEntry.m_obActionName = MusicControlManager::Get().GetActionHash(this);
	stEntry.m_dTime = MusicControlManager::Get().GetMusicControlTime();
	stEntry.m_fCurrentIntensity = fCurrentIntensity;
	stEntry.m_fRequestedIntensity = fIntensity;
	stEntry.m_fResultantIntensity = InteractiveMusicManager::Get().GetIntensity();

	MusicControlDebugDisplay::Get().AddActivationHistoryEntry(stEntry);
#endif // _MUSIC_CONTROL_DEBUG
}




//------------------------------------------------------------------------------------------
//	MusicControlRule (ctor)
//!	Initialises a rule.
//!	@note By default rules are disabled and cannot be enabled until a valid input and action
//!	have been set.
//!	@sa MusicControlManager::MusicControlRule::SetInput(MusicControlInput* pobInput)
//!	@sa MusicControlManager::MusicControlRule::SetAction(MusicControlAction* pobAction)
//!	@sa MusicControlManager::MusicControlRule::bool Enable(bool bEnable)
//------------------------------------------------------------------------------------------
MusicControlManager::MusicControlRule::MusicControlRule(void)
:
	m_eRuleType(THRESHOLD),
	m_pobInput(0),
	m_pobAction(0),
	m_fActivationLapse(0.0f),
	m_dActivationLapseComplete(0.0f),
	m_bActive(false),
	m_bEnable(false),
	m_bExemptFromSuspend(false),
	m_bLapsePending(false)
{}


//------------------------------------------------------------------------------------------
//	~MusicControlRule (dtor)
//!	Deinitialises rule and cleans up as req'd.
//------------------------------------------------------------------------------------------
MusicControlManager::MusicControlRule::~MusicControlRule(void)
{}


//------------------------------------------------------------------------------------------
//	Update
//!	Updates rule, activating/deactivating and updating associated action as req'd.
//------------------------------------------------------------------------------------------
void MusicControlManager::MusicControlRule::Update(void)
{
	if (!IsEnabled())
		return;

	// Update lapse
	if (IsLapsing())
	{
		if (m_pobAction)
		{
			if (!m_pobAction->IsInfluence())
			{
				// Cancel current lapse to resume when influence returns
				m_dActivationLapseComplete = 0.0;
				m_bLapsePending = true;
			}
		}

		return;
	}
	else if (m_bLapsePending)
	{
		if (m_pobAction)
		{
			if (m_pobAction->IsInfluence())
			{
				m_dActivationLapseComplete = MusicControlManager::Get().GetMusicControlTime() + (double)m_fActivationLapse;
				m_bLapsePending = false;
				return;
			}
		}
	}

	// Activation checks
	float fResult;
	switch (m_eRuleType)
	{
	case THRESHOLD:
	case THRESHOLD_INVERTED:
	case RANGE:
	case RANGE_INVERTED:
		fResult = ActivationCheckRange();
		break;
	case DELTA:
		fResult = ActivationCheckDelta();
		break;
	default:
		return;
	}

	if (fResult < 0.0f)
	{
		if (m_bActive)
			Activate(false);
		return;
	}

	if (!m_bActive)
		Activate(true);
	m_pobAction->Update(fResult);
}


//------------------------------------------------------------------------------------------
//	ActivationCheckRnage
//!	Handles range based activation.
//!	@return Negative if rule is inactive, otherwise normalised rule value.
//------------------------------------------------------------------------------------------
float MusicControlManager::MusicControlRule::ActivationCheckRange(void)
{
	if (!m_pobInput)
		return -1.0f;

	float fInput = m_pobInput->GetValue();
	float fValue;

	// Threshold or range
	if (m_ActivationData.RangedActivation.m_fActivationMin == m_ActivationData.RangedActivation.m_fActivationMax)
	{
		// Rule provides a boolean value
		if (THRESHOLD_INVERTED == m_eRuleType)
			fValue = (fInput <= m_ActivationData.RangedActivation.m_fActivationMin) ? 1.0f:0.0f;
		else
			fValue = (fInput >= m_ActivationData.RangedActivation.m_fActivationMin) ? 1.0f:0.0f;

		// Threshold based activation
		if (fValue <= 0.0f)
		{
			// Done (inactive)
			return -1.0f;
		}
	}
	else if (fInput >= m_ActivationData.RangedActivation.m_fActivationMin && fInput <= m_ActivationData.RangedActivation.m_fActivationMax)
	{
		// Rule provides a normalised value
		fValue = (fInput - m_ActivationData.RangedActivation.m_fActivationMin)/(m_ActivationData.RangedActivation.m_fActivationMax - m_ActivationData.RangedActivation.m_fActivationMin);

		// Invert as req'd
		if (RANGE_INVERTED == m_eRuleType)
			fValue = 1.0f - fValue;
	}
	else
	{
		// Done (inactive)
		return -1.0f;
	}

	// Done
	return fValue;
}


//------------------------------------------------------------------------------------------
//	ActivationCheckDelta
//!	Handles delta based activation.
//!	@return Negative if rule is inactive, otherwise normalised rule value.
//------------------------------------------------------------------------------------------
float MusicControlManager::MusicControlRule::ActivationCheckDelta(void)
{
	if (!m_pobInput)
		return -1.0f;

	// Calc rate of change
	double dCurTime = MusicControlManager::Get().GetMusicControlTime();
	float fCurInterval = (float)(dCurTime - m_stDeltaMemory.m_dTime);
	float fCurValue = m_pobInput->GetValue();
	float fCurDelta = fCurValue - m_stDeltaMemory.m_fValue;
	float fDerivative = fCurDelta/fCurInterval;
	float fTargetDerivative = m_ActivationData.DeltaActivation.m_fActivationTimeout > 0.0f 
		? m_ActivationData.DeltaActivation.m_fActivationDelta/m_ActivationData.DeltaActivation.m_fActivationTimeout
		: 0.0f;

	// Update delta memory
	m_stDeltaMemory.m_dTime = dCurTime;
	m_stDeltaMemory.m_fValue = fCurValue;

	// Toggle monitor as req'd
	if (m_stDeltaMonitor.m_dTime < 0.0f)
	{
		// Check for minimum rate of change
		if (fDerivative >= fTargetDerivative)
		{
			// Start delta monitor
			m_stDeltaMonitor.m_dTime = dCurTime;
			m_stDeltaMonitor.m_fValue = fCurValue;
			m_ActivationData.DeltaActivation.m_bDeltaTargetMet = true;
		}
		else
		{
			// Done (inactive)
			return -1.0f;
		}
	}
	else
	{
		// Currently tracking
		if (fDerivative >= fTargetDerivative)
		{
			// Delta recovery tracks derivative at last update
			if (!m_ActivationData.DeltaActivation.m_bDeltaTargetMet)
			{
				// Save window recover point
				m_stDeltaRecover.m_dTime = dCurTime;
				m_stDeltaRecover.m_fValue = fCurValue;
				m_ActivationData.DeltaActivation.m_bDeltaTargetMet = true;
			}
		}
		else if (m_ActivationData.DeltaActivation.m_bDeltaTargetMet)
		{
			// Continue tracking, but sub-target
			m_ActivationData.DeltaActivation.m_bDeltaTargetMet = false;
		}
	}

	// Handle timeout
	float fTotalInterval;
	while ((fTotalInterval = (float)(dCurTime - m_stDeltaMonitor.m_dTime)) > m_ActivationData.DeltaActivation.m_fActivationTimeout)
	{
		// Shift window if possible
		if (m_stDeltaRecover.m_dTime >= 0.0)
		{
			// Recovery position stored
			m_stDeltaMonitor.m_dTime = m_stDeltaRecover.m_dTime;
			m_stDeltaMonitor.m_fValue = m_stDeltaRecover.m_fValue;

			// Clear recovery data
			m_stDeltaRecover.m_dTime = -1.0;
			m_stDeltaRecover.m_fValue = 0.0f;
		}
		else
		{
			// No recovery point and timeout has expired, cleanup and bail
			m_stDeltaMonitor.m_dTime = -1.0;
			m_stDeltaMonitor.m_fValue = 0.0f;
			m_ActivationData.DeltaActivation.m_bDeltaTargetMet = false;

			// Done
			return -1.0f;
		}
	}

	// Calc total delta
	float fTotalDelta = fCurValue - m_stDeltaMonitor.m_fValue;
	if ((m_ActivationData.DeltaActivation.m_fActivationDelta >= 0.0f && fTotalDelta >= m_ActivationData.DeltaActivation.m_fActivationDelta)
		|| (m_ActivationData.DeltaActivation.m_fActivationDelta <= 0.0f && fTotalDelta <= m_ActivationData.DeltaActivation.m_fActivationDelta))
	{
		//Active!
		m_stDeltaMonitor.m_dTime = -1.0;
		m_stDeltaMonitor.m_fValue = 0.0f;
		m_ActivationData.DeltaActivation.m_bDeltaTargetMet = false;

		return 1.0f;
	}

	// Inactive
	return -1.0f;
}


//------------------------------------------------------------------------------------------
//!	Toggles associated action activation.
//!	@param bActive	Indicates whether or not rule is active.
//------------------------------------------------------------------------------------------
void MusicControlManager::MusicControlRule::Activate(bool bActive)
{
	// Desired activation state already set
	if (m_bActive == bActive)
		return;

	// Notify associated action if possible
	if (m_pobAction)
	{
		// Cannot activate without an associated action
		if (bActive)
		{
			if (!m_bActive)
				m_bActive = m_pobAction->Activate();
		}
		else if (m_bActive)
		{
			m_pobAction->Deactivate();
			m_bActive = false;
		}

		// Lapse if action is not (or no longer) updating
		if (!m_bActive && m_fActivationLapse > 0.0f)
		{
			if (m_pobAction->IsInfluence())
			{
				m_dActivationLapseComplete = MusicControlManager::Get().GetMusicControlTime() + (double)m_fActivationLapse;
			}
			else
			{
				m_dActivationLapseComplete = 0.0;
				m_bLapsePending = true;
			}
		}
	}
	else if (!bActive)
		m_bActive = false;
}





//------------------------------------------------------------------------------------------
//	MusicControlManager (ctor)
//!	Initialises music control manager.
//!	@note Creates and registers built-in inputs.
//------------------------------------------------------------------------------------------
MusicControlManager::MusicControlManager(void)
:
	m_dResumeTime(0.0),
	m_fGlobalActivationLapse(0.0f)
{
#ifdef _MUSIC_CONTROL_DEBUG
	NT_NEW MusicControlDebugDisplay;
#endif // _MUSIC_CONTROL_DEBUG

	RegisterBuiltinInputs();
}


//------------------------------------------------------------------------------------------
//	MusicControlManager (dtor)
//!	Deinitialises music control manager and cleans up as req'd.
//------------------------------------------------------------------------------------------
MusicControlManager::~MusicControlManager(void)
{
	DeregisterEverything(true);

#ifdef _MUSIC_CONTROL_DEBUG
	MusicControlDebugDisplay::Kill();
#endif // _MUSIC_CONTROL_DEBUG
}


//------------------------------------------------------------------------------------------
//!	Retrieves music control system time. This may pause with game pause, etc.
//!	@return A suitable value for music control system time.
//------------------------------------------------------------------------------------------
double MusicControlManager::GetMusicControlTime(void) const
{
	// double dTime = CTimer::Get().GetGameTime();
	double dTime = CTimer::Get().GetSystemTime();
	return dTime;
}


//------------------------------------------------------------------------------------------
//	Update
//!	Updates music control manager (all inputs and rules updated, rules in turn update actions).
//------------------------------------------------------------------------------------------
void MusicControlManager::Update(void)
{
	// Always update inputs
	for (ntstd::Map<CHashedString, MusicControlInput*>::iterator obItr = m_obInputsList.begin(); obItr != m_obInputsList.end(); ++obItr)
	{
		obItr->second->Update();
	}

	// Update rules (will update actions accordingly)
	float fCurrentIntensity = InteractiveMusicManager::Get().GetIntensity();
	bool bIntensityChanged = false;
	if (IsSuspended())
	{
		for (ntstd::Map<CHashedString, MusicControlRule*>::iterator obItr = m_obRulesList.begin(); !bIntensityChanged && obItr != m_obRulesList.end(); ++obItr)
		{
			if (obItr->second->IsExemptFromSuspend())
				obItr->second->Update();

			bIntensityChanged = (fCurrentIntensity != InteractiveMusicManager::Get().GetIntensity());
		}
	}
	else
	{
		for (ntstd::Map<CHashedString, MusicControlRule*>::iterator obItr = m_obRulesList.begin(); !bIntensityChanged && obItr != m_obRulesList.end(); ++obItr)
		{
			obItr->second->Update();

			bIntensityChanged = (fCurrentIntensity != InteractiveMusicManager::Get().GetIntensity());
		}
	}

	// Handle intensity change resulting in a global suspend
	if (bIntensityChanged && m_fGlobalActivationLapse != 0.0f)
	{
		Suspend(m_fGlobalActivationLapse);
	}

#ifdef _MUSIC_CONTROL_DEBUG
	MusicControlDebugDisplay::Get().Update();
#endif // _MUSIC_CONTROL_DEBUG
}


//------------------------------------------------------------------------------------------
//	Suspend
//!	Ceases rule (and consequently action) update for a given period.
//!	@param fDuration	Suspend duration. Use negative (-1.0f is the default) for infinite suspend.
//!	@note If music control manager is suspended indefinitely, it must be resumed manually.
//!	@sa MusicControlManager::Resume(void)
//------------------------------------------------------------------------------------------
void MusicControlManager::Suspend(float fDuration)
{
	if (fDuration < 0.0f)
		m_dResumeTime = -1.0;
	else
		m_dResumeTime = GetMusicControlTime() + (double)fDuration;
}


//------------------------------------------------------------------------------------------
//	Resume
//!	Un-suspends music control manager (irrespective of whether or not manager is indefinitely
//!	suspended or the suspend time has yet to expire).
//------------------------------------------------------------------------------------------
void MusicControlManager::Resume(void)
{
	m_dResumeTime = 0.0f;
}


//------------------------------------------------------------------------------------------
//	IsSuspended
//!	Indicates whether or not music control manager is suspended.
//!	@return True if music control manager is suspended, false otherwise.
//------------------------------------------------------------------------------------------
bool MusicControlManager::IsSuspended(void) const
{
	return (m_dResumeTime < 0.0f || GetMusicControlTime() < m_dResumeTime);
}


//------------------------------------------------------------------------------------------
//	EnableRule
//!	Enables/disables a rule.
//!	@param obName	Name of rule.
//!	@param bEnable	Indicates whether rule should be enabled (true) or disabled (false).
//!	@return True on success, false otherwise.
//------------------------------------------------------------------------------------------
bool MusicControlManager::EnableRule(const CHashedString& obName, bool bEnable)
{
	MusicControlRule* pobRule = GetRule(obName);
	return pobRule && pobRule->Enable(bEnable);
}

//------------------------------------------------------------------------------------------
//	EnableRuleGroup
//!	Enables/disables rules with a given group label.
//!	@param obLabel	Desired group label (can be null string).
//!	@param bEnable	Indicates whether rules should be enabled (true) or disabled (false).
//!	@return True on success, false otherwise.
//------------------------------------------------------------------------------------------
bool MusicControlManager::EnableRuleGroup(const CHashedString& obLabel, bool bEnable)
{
	// Toggle rules with matching label
	bool bResult = false;
	for (ntstd::Map<CHashedString, MusicControlRule*>::iterator obItr = m_obRulesList.begin(); obItr != m_obRulesList.end(); ++obItr)
	{
		if (obItr->second->GetGroupLabel() == obLabel)
		{
			obItr->second->Enable(bEnable);
			bResult = true;
		}
	}

	return bResult;
}


//------------------------------------------------------------------------------------------
//	EnableAllRules
//!	Enables/disables all rule.
//!	@param bEnable	Indicates whether rules should be enabled (true) or disabled (false).
//------------------------------------------------------------------------------------------
void MusicControlManager::EnableAllRules(bool bEnable)
{
	// Toggle rules
	for (ntstd::Map<CHashedString, MusicControlRule*>::iterator obItr = m_obRulesList.begin(); obItr != m_obRulesList.end(); ++obItr)
	{
		obItr->second->Enable(bEnable);
	}
}


//------------------------------------------------------------------------------------------
//	RegisterInput
//!	Registers an input.
//!	@param pobCandidate	Input to register.
//!	@param obName		Name of input (hashed).
//!	@return True on success, false otherwise.
//!	@note Will fail (warning user) if an input with the same name is already registered.
//------------------------------------------------------------------------------------------
bool MusicControlManager::RegisterInput(MusicControlInput* pobCandidate, const CHashedString& obName)
{
	if (!pobCandidate || obName.IsNull())
		return false;

	if (GetInput(obName))
	{
		user_warn_msg(("Music control input \"%s\" already registered!", obName.GetDebugString()));
		return false;
	}

	m_obInputsList.insert(ntstd::Map<CHashedString, MusicControlInput*>::value_type(obName, pobCandidate));

#ifdef _MUSIC_CONTROL_DEBUG
	MusicControlDebugDisplay::Get().Refresh();
#endif // _MUSIC_CONTROL_DEBUG

	// Success
	return true;
}


//------------------------------------------------------------------------------------------
//	RegisterAction
//!	Registers an action.
//!	@param pobCandidate	Action to register.
//!	@param obName		Name of action (hashed).
//!	@return True on success, false otherwise.
//!	@note Will fail (warning user) if an action with the same name is already registered.
//------------------------------------------------------------------------------------------
bool MusicControlManager::RegisterAction(MusicControlAction* pobCandidate, const CHashedString& obName)
{
	if (!pobCandidate || obName.IsNull())
		return false;

	if (GetAction(obName))
	{
		user_warn_msg(("Music control action \"%s\" already registered!", obName.GetDebugString()));
		return false;
	}

	m_obActionsList.insert(ntstd::Map<CHashedString, MusicControlAction*>::value_type(obName, pobCandidate));

#ifdef _MUSIC_CONTROL_DEBUG
	MusicControlDebugDisplay::Get().Refresh();
#endif // _MUSIC_CONTROL_DEBUG

	// Success
	return true;
}


//------------------------------------------------------------------------------------------
//	RegisterRule
//!	Registers a rule.
//!	@param pobCandidate	Rule to register.
//!	@param obName		Name of rule (hashed).
//!	@return True on success, false otherwise.
//!	@note Will fail (warning user) if a rule with the same name is already registered.
//------------------------------------------------------------------------------------------
bool MusicControlManager::RegisterRule(MusicControlRule* pobCandidate, const CHashedString& obName)
{
	if (!pobCandidate || obName.IsNull())
		return false;

	if (GetRule(obName))
	{
		user_warn_msg(("Music control rule \"%s\" already registered!", obName.GetDebugString()));
		return false;
	}

	m_obRulesList.insert(ntstd::Map<CHashedString, MusicControlRule*>::value_type(obName, pobCandidate));

#ifdef _MUSIC_CONTROL_DEBUG
	MusicControlDebugDisplay::Get().Refresh();
#endif // _MUSIC_CONTROL_DEBUG

	// Success
	return true;
}


//------------------------------------------------------------------------------------------
//	DeregisterInput
//!	Deregisters and optionally deletes a previously registered input.
//!	@param obName	Name of input (hashed).
//!	@param bDestroy	Indicates whether or not rule object should be deleted.
//!	@return True if input was found and deregistered, false otherwise.
//------------------------------------------------------------------------------------------
bool MusicControlManager::DeregisterInput(const CHashedString& obName, bool bDestroy)
{
	ntstd::Map<CHashedString, MusicControlInput*>::iterator obItr = m_obInputsList.find(obName);
	if (obItr == m_obInputsList.end())
		return false;

	if (bDestroy)
	{
		delete obItr->second;
		obItr->second = 0;
	}

	m_obInputsList.erase(obItr);

	return true;
}


//------------------------------------------------------------------------------------------
//	DeregisterAction
//!	Deregisters and optionally deletes a previously registered action.
//!	@param obName	Name of action (hashed).
//!	@param bDestroy	Indicates whether or not action object should be deleted.
//!	@return True if action was found and deregistered, false otherwise.
//------------------------------------------------------------------------------------------
bool MusicControlManager::DeregisterAction(const CHashedString& obName, bool bDestroy)
{
	ntstd::Map<CHashedString, MusicControlAction*>::iterator obItr = m_obActionsList.find(obName);
	if (obItr == m_obActionsList.end())
		return false;

	if (bDestroy)
	{
		delete obItr->second;
		obItr->second = 0;
	}

	m_obActionsList.erase(obItr);

	return true;
}


//------------------------------------------------------------------------------------------
//	DeregisterRule
//!	Deregisters and optionally deletes a previously registered rule.
//!	@param obName	Name of rule (hashed).
//!	@param bDestroy	Indicates whether or not rule object should be deleted.
//!	@return True if rule was found and deregistered, false otherwise.
//------------------------------------------------------------------------------------------
bool MusicControlManager::DeregisterRule(const CHashedString& obName, bool bDestroy)
{
	ntstd::Map<CHashedString, MusicControlRule*>::iterator obItr = m_obRulesList.find(obName);
	if (obItr == m_obRulesList.end())
		return false;

	if (bDestroy)
	{
		delete obItr->second;
		obItr->second = 0;
	}

	m_obRulesList.erase(obItr);

	return true;
}


//------------------------------------------------------------------------------------------
//	DeregisterAllInputs
//!	Deregisters and optionally destroys all registered inputs.
//------------------------------------------------------------------------------------------
void MusicControlManager::DeregisterAllInputs(bool bDestroy)
{
	if (bDestroy)
	{
		for (ntstd::Map<CHashedString, MusicControlInput*>::iterator obItr = m_obInputsList.begin(); obItr != m_obInputsList.end(); ++obItr)
		{
			delete obItr->second;
			obItr->second = 0;
		}
	}

	m_obInputsList.clear();
}


//------------------------------------------------------------------------------------------
//	DeregisterAllActions
//!	Deregisters and optionally destroys all registered actions.
//------------------------------------------------------------------------------------------
void MusicControlManager::DeregisterAllActions(bool bDestroy)
{
	if (bDestroy)
	{
		for (ntstd::Map<CHashedString, MusicControlAction*>::iterator obItr = m_obActionsList.begin(); obItr != m_obActionsList.end(); ++obItr)
		{
			delete obItr->second;
			obItr->second = 0;
		}
	}

	m_obActionsList.clear();
}


//------------------------------------------------------------------------------------------
//	DeregisterAllRules
//!	Deregisters and optionally destroys all registered rules.
//------------------------------------------------------------------------------------------
void MusicControlManager::DeregisterAllRules(bool bDestroy)
{
	if (bDestroy)
	{
		for (ntstd::Map<CHashedString, MusicControlRule*>::iterator obItr = m_obRulesList.begin(); obItr != m_obRulesList.end(); ++obItr)
		{
			delete obItr->second;
			obItr->second = 0;
		}
	}

	m_obRulesList.clear();
}


//------------------------------------------------------------------------------------------
//	GetInput
//!	Retrieves a previously registered input.
//!	@param obName	Name of input (hashed).
//!	@return Pointer to requested input if it is found, 0 otherwise.
//------------------------------------------------------------------------------------------
MusicControlManager::MusicControlInput* MusicControlManager::GetInput(const CHashedString& obName)
{
	if (obName.IsNull())
		return 0;

	ntstd::Map<CHashedString, MusicControlInput*>::iterator obItr = m_obInputsList.find(obName);
	if (obItr == m_obInputsList.end())
		return 0;

	return obItr->second;
}


//------------------------------------------------------------------------------------------
//	GetAction
//!	Retrieves a previously registered action.
//!	@param obName	Name of action (hashed).
//!	@return Pointer to requested action if it is found, 0 otherwise.
//------------------------------------------------------------------------------------------
MusicControlManager::MusicControlAction* MusicControlManager::GetAction(const CHashedString& obName)
{
	if (obName.IsNull())
		return 0;

	ntstd::Map<CHashedString, MusicControlAction*>::iterator obItr = m_obActionsList.find(obName);
	if (obItr == m_obActionsList.end())
		return 0;

	return obItr->second;
}


//------------------------------------------------------------------------------------------
//	GetRule
//!	Retrieves a previously registered rule.
//!	@param obName	Name of rule (hashed).
//!	@return Pointer to requested rule if it is found, 0 otherwise.
//------------------------------------------------------------------------------------------
MusicControlManager::MusicControlRule* MusicControlManager::GetRule(const CHashedString& obName)
{
	if (obName.IsNull())
		return 0;

	ntstd::Map<CHashedString, MusicControlRule*>::iterator obItr = m_obRulesList.find(obName);
	if (obItr == m_obRulesList.end())
		return 0;

	return obItr->second;
}


//------------------------------------------------------------------------------------------
//	GetInputHash
//!	Retrieves a registered input's hash.
//!	@param pobInput	Pointer to (previously registered) input.
//!	@return Requested hash, or null on error.
//------------------------------------------------------------------------------------------
CHashedString MusicControlManager::GetInputHash(MusicControlInput* pobInput)
{
	if (!pobInput)
		return CHashedString::nullString;

	for (ntstd::Map<CHashedString, MusicControlInput*>::iterator obItr = m_obInputsList.begin(); obItr != m_obInputsList.end(); ++obItr)
	{
		if (obItr->second == pobInput)
			return obItr->first;
	}

	return CHashedString::nullString;
}


//------------------------------------------------------------------------------------------
//	GetActionHash
//!	Retrieves a registered action's hash.
//!	@param pobAction	Pointer to (previously registered) action.
//!	@return Requested hash, or null on error.
//------------------------------------------------------------------------------------------
CHashedString MusicControlManager::GetActionHash(MusicControlAction* pobAction)
{
	if (!pobAction)
		return CHashedString::nullString;

	for (ntstd::Map<CHashedString, MusicControlAction*>::iterator obItr = m_obActionsList.begin(); obItr != m_obActionsList.end(); ++obItr)
	{
		if (obItr->second == pobAction)
			return obItr->first;
	}

	return CHashedString::nullString;
}


//------------------------------------------------------------------------------------------
//	GetRuleHash
//!	Retrieves a registered rule's hash.
//!	@param pobRule	Pointer to (previously registered) rule.
//!	@return Requested hash, or null on error.
//------------------------------------------------------------------------------------------
CHashedString MusicControlManager::GetRuleHash(MusicControlRule* pobRule)
{
	if (!pobRule)
		return CHashedString::nullString;

	for (ntstd::Map<CHashedString, MusicControlRule*>::iterator obItr = m_obRulesList.begin(); obItr != m_obRulesList.end(); ++obItr)
	{
		if (obItr->second == pobRule)
			return obItr->first;
	}

	return CHashedString::nullString;
}


//------------------------------------------------------------------------------------------
//	ToggleDebugging
//!	Enables/disables music control debug display.
//!	@sa MusicControlManager::IsDebuggingEnabled(void)
//------------------------------------------------------------------------------------------
void MusicControlManager::ToggleDebugging(void)
{
#ifdef _MUSIC_CONTROL_DEBUG
	g_ShellOptions->m_bMusicDebug = !g_ShellOptions->m_bMusicDebug;
#endif // _MUSIC_CONTROL_DEBUG
}


//------------------------------------------------------------------------------------------
//	IsDebuggingEnabled
//!	@return Whether or not music control debug display is enabled.
//!	@sa MusicControlManager::ToggleDebugging(void)
//------------------------------------------------------------------------------------------
bool MusicControlManager::IsDebuggingEnabled(void)
{
#ifdef _MUSIC_CONTROL_DEBUG
	return g_ShellOptions->m_bMusicDebug;
#else
	return false;
#endif // _MUSIC_CONTROL_DEBUG
}


#ifdef _MUSIC_CONTROL_DEBUG


#define fLINE_PAD			16.0f
#define fGRID_LINE_SPACING	1.0f

const int MusicControlDebugDisplay::s_iSampleCount = 500;


//------------------------------------------------------------------------------------------
//	MusicControlDebugDisplay
//------------------------------------------------------------------------------------------
MusicControlDebugDisplay::MusicControlDebugDisplay(void)
:
	m_eFocus(CURRENT_INTENSITY),
	m_uiVisible(CURRENT_INTENSITY|ACTIVATION_HISTORY),
	m_obInputGraph(GRAPH_TYPE_ROLLING),
	m_pobInputSampleSet(0),
	m_iNextHistoryEntryIdx(0)
{
	ResetGraphs();
}


//------------------------------------------------------------------------------------------
//	AddActivationHistoryEntry
//!	Notifies debug display of a music control action activation.
//!	@param stActHistEntry	Music intensity adjustment details.
//------------------------------------------------------------------------------------------
void MusicControlDebugDisplay::AddActivationHistoryEntry(const ActivationHistoryEntry& stActHistEntry)
{
	m_astActivationHistory[m_iNextHistoryEntryIdx].Set(stActHistEntry);

	if (++m_iNextHistoryEntryIdx >= s_iActivationHistoryEntries)
		m_iNextHistoryEntryIdx = 0;
}


//------------------------------------------------------------------------------------------
//	Update
//!	Updates debug display etc.
//------------------------------------------------------------------------------------------
void MusicControlDebugDisplay::Update(void)
{
	CInputKeyboard* pobKeyboard = CInputHardware::Get().GetKeyboardP();
	if (!pobKeyboard)
		return;

	if (pobKeyboard->IsKeyPressed(KEYC_E, KEYM_CTRL|KEYM_ALT))
		MusicControlManager::Get().ToggleDebugging();

	if (!MusicControlManager::Get().IsDebuggingEnabled())
	{
		return;
	}

	// Ensure current tracking element valid
	SetCurTrackingElementName(m_obCurTrackingElementName);

	// Internal update
	UpdateFocus();
	UpdateDisplay();
}


//------------------------------------------------------------------------------------------
//	Refresh
//------------------------------------------------------------------------------------------
void MusicControlDebugDisplay::Refresh(void)
{
	// Ensure current tracking element valid
	SetCurTrackingElementName(m_obCurTrackingElementName);
	ResetGraphs();
}


//------------------------------------------------------------------------------------------
//	UpdateFocus
//!	Handles keyboard input.
//------------------------------------------------------------------------------------------
void MusicControlDebugDisplay::UpdateFocus(void)
{
	// Get kb
	CInputKeyboard* pobKeyboard = CInputHardware::Get().GetKeyboardP();
	if (!pobKeyboard)
		return;

	// Get key
	KEY_CODE eKey;
	if (pobKeyboard->IsKeyPressed(KEYC_UP_ARROW))
		eKey = KEYC_UP_ARROW;
	else if (pobKeyboard->IsKeyPressed(KEYC_DOWN_ARROW))
		eKey = KEYC_DOWN_ARROW;
	else if (pobKeyboard->IsKeyPressed(KEYC_LEFT_ARROW))
		eKey = KEYC_LEFT_ARROW;
	else if (pobKeyboard->IsKeyPressed(KEYC_RIGHT_ARROW))
		eKey = KEYC_RIGHT_ARROW;
	else
		return;

	// Process key
	switch (m_eFocus)
	{
	case CURRENT_INTENSITY:
		switch (eKey)
		{
		case KEYC_UP_ARROW:
			if (CHECK_FLAG(m_uiVisible, CURRENT_INTENSITY))
			{
				SET_FLAG(m_uiVisible, ACTIVATION_HISTORY);
			}
			else
			{
				SET_FLAG(m_uiVisible, CURRENT_INTENSITY);
			}
			break;
		case KEYC_DOWN_ARROW:
			if (CHECK_FLAG(m_uiVisible, ACTIVATION_HISTORY))
			{
				CLEAR_FLAG(m_uiVisible, ACTIVATION_HISTORY);
			}
			else
			{
				CLEAR_FLAG(m_uiVisible, CURRENT_INTENSITY);
			}
			break;
		case KEYC_LEFT_ARROW:
			SET_FLAG(m_uiVisible, ELEMENT_LIST);
			CLEAR_FLAG(m_uiVisible, CURRENT_INTENSITY);
			m_eFocus = ELEMENT_LIST;
			break;
		case KEYC_RIGHT_ARROW:
			SET_FLAG(m_uiVisible, CURRENT_INTENSITY);
			SET_FLAG(m_uiVisible, ELEMENT_LIST);
			m_eFocus = ELEMENT_LIST;
			break;
		default:
			break;
		}
		break;
	case ACTIVATION_HISTORY:
		// Can't focus activation history
		break;
	case ELEMENT_LIST:
		switch (eKey)
		{
		case KEYC_UP_ARROW:
			SetCurTrackingElementName(GetPrevTrackingElementName());
			break;
		case KEYC_DOWN_ARROW:
			SetCurTrackingElementName(GetNextTrackingElementName());
			break;
		case KEYC_LEFT_ARROW:
			SET_FLAG(m_uiVisible, CURRENT_INTENSITY);
			CLEAR_FLAG(m_uiVisible, ELEMENT_LIST);
			m_eFocus = CURRENT_INTENSITY;
			break;
		case KEYC_RIGHT_ARROW:
			if (MusicControlManager::Get().GetRuleCount() > 0)
			{
				SET_FLAG(m_uiVisible, ELEMENT_DETAIL);
				CLEAR_FLAG(m_uiVisible, ELEMENT_LIST);
				m_eFocus = ELEMENT_DETAIL;
			}
			break;
		default:
			break;
		}
		break;
	case ELEMENT_DETAIL:
		switch (eKey)
		{
		case KEYC_UP_ARROW:
			SetCurTrackingElementName(GetPrevTrackingElementName());
			break;
		case KEYC_DOWN_ARROW:
			SetCurTrackingElementName(GetNextTrackingElementName());
			break;
		case KEYC_LEFT_ARROW:
			SET_FLAG(m_uiVisible, ELEMENT_LIST);
			CLEAR_FLAG(m_uiVisible, ELEMENT_DETAIL);
			m_eFocus = ELEMENT_LIST;
			break;
		case KEYC_RIGHT_ARROW:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}


//------------------------------------------------------------------------------------------
//	UpdateDisplay
//!	Renders debug display.
//------------------------------------------------------------------------------------------
void MusicControlDebugDisplay::UpdateDisplay(void)
{
	if (CHECK_FLAG(m_uiVisible, CURRENT_INTENSITY))
	{
		if (!MusicControlManager::Get().IsSuspended())
		{
			g_VisualDebug->Printf2D(
				fLINE_PAD,
				g_VisualDebug->GetDebugDisplayHeight() - fLINE_PAD,
				DC_RED,
				DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
				"%c Current Music Intensity: %0.2f",
				(m_eFocus == CURRENT_INTENSITY) ? '>':' ',
				InteractiveMusicManager::Get().GetIntensity());
		}
		else
		{
			double dResume = MusicControlManager::Get().GetResumeTime();
			if (dResume > MusicControlManager::Get().GetMusicControlTime())
			{
 				dResume -= MusicControlManager::Get().GetMusicControlTime();

				g_VisualDebug->Printf2D(
					fLINE_PAD,
					g_VisualDebug->GetDebugDisplayHeight() - fLINE_PAD,
					DC_PURPLE,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"%c Current Music Intensity: %0.2f (suspend %0.2f)",
					(m_eFocus == CURRENT_INTENSITY) ? '>':' ',
					InteractiveMusicManager::Get().GetIntensity(),
					dResume);
			}
			else
			{
				g_VisualDebug->Printf2D(
				fLINE_PAD,
				g_VisualDebug->GetDebugDisplayHeight() - fLINE_PAD,
				DC_PURPLE,
				DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
				"%c Current Music Intensity: %0.2f (suspended)",
				(m_eFocus == CURRENT_INTENSITY) ? '>':' ',
				InteractiveMusicManager::Get().GetIntensity());
			}
		}

		if (CHECK_FLAG(m_uiVisible, ACTIVATION_HISTORY))
		{
			float fY = g_VisualDebug->GetDebugDisplayHeight() - fLINE_PAD;

			float fMin = 0.0f;
			float fSec = modf((float)(MusicControlManager::Get().GetMusicControlTime()/60.0), &fMin)*60.0f;

			g_VisualDebug->Printf2D(
				fLINE_PAD,
				fY -= fLINE_PAD,
				DC_GREY,
				DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
				"%dm%ds",
				(int)fMin,
				(int)fSec);

			int iUseIdx = (m_iNextHistoryEntryIdx > 0) ? (m_iNextHistoryEntryIdx - 1):(s_iActivationHistoryEntries - 1);
			int iCount = 0;
			while (iCount++ < s_iActivationHistoryEntries)
			{
				if (!m_astActivationHistory[iUseIdx].m_obActionName.IsNull())
				{
					fSec = modf((float)(m_astActivationHistory[iUseIdx].m_dTime/60.0), &fMin)*60.0f;

					if (FLOAT_COMPARE(m_astActivationHistory[iUseIdx].m_fRequestedIntensity, m_astActivationHistory[iUseIdx].m_fResultantIntensity, 0.0005f))
					{
						g_VisualDebug->Printf2D(
							fLINE_PAD,
							fY -= fLINE_PAD,
							DC_GREEN,
							DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
							"%dm%ds %s set %0.2f (from %0.2f)",
							(int)fMin,
							(int)fSec,
							m_astActivationHistory[iUseIdx].m_obActionName.GetDebugString(),
							m_astActivationHistory[iUseIdx].m_fResultantIntensity,
							m_astActivationHistory[iUseIdx].m_fCurrentIntensity);
					}
					else
					{
						g_VisualDebug->Printf2D(
							fLINE_PAD,
							fY -= fLINE_PAD,
							DC_WHITE,
							DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
							"%dm%ds %s req %0.2f (from %0.2f), got %0.2f",
							(int)fMin,
							(int)fSec,
							m_astActivationHistory[iUseIdx].m_obActionName.GetDebugString(),
							m_astActivationHistory[iUseIdx].m_fRequestedIntensity,
							m_astActivationHistory[iUseIdx].m_fResultantIntensity,
							m_astActivationHistory[iUseIdx].m_fCurrentIntensity);
					}
				}

				iUseIdx = (iUseIdx > 0) ? (iUseIdx - 1):(s_iActivationHistoryEntries - 1);
			}

			g_VisualDebug->Printf2D(
				fLINE_PAD,
				fY -= fLINE_PAD,
				DC_WHITE,
				DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
				"ACTIVATION HISTORY");
		}
	}

	if (CHECK_FLAG(m_uiVisible, ELEMENT_LIST))
	{
		float fX = (m_uiVisible&CURRENT_INTENSITY) ? g_VisualDebug->GetDebugDisplayWidth()/2.0f:fLINE_PAD;
		float fY = g_VisualDebug->GetDebugDisplayHeight() - fLINE_PAD;

		if (MusicControlManager::Get().GetRuleCount() > 0)
		{
			for (ntstd::Map<CHashedString, MusicControlManager::MusicControlRule*>::reverse_iterator obItr = MusicControlManager::Get().GetRulesList()->rbegin(); obItr != MusicControlManager::Get().GetRulesList()->rend(); ++obItr)
			{
				MusicControlManager::MusicControlAction* pobAction = obItr->second->GetAction();

				g_VisualDebug->Printf2D(
					fX,
					fY,
					obItr->second->IsEnabled() ? ((pobAction && pobAction->IsInfluence()) ? DC_WHITE:DC_RED):DC_GREY,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"%c %s",
					(m_obCurTrackingElementName == obItr->first) ? '>':' ',
					obItr->first.GetDebugString());

				fY -= fLINE_PAD;
			}
		}
		else
		{
			g_VisualDebug->Printf2D(
				fX,
				fY,
				DC_GREY,
				DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
				"< None");

			fY -= fLINE_PAD;
		}

		g_VisualDebug->Printf2D(
			fX,
			fY,
			DC_WHITE,
			DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
			"TRACKING ELEMENTS");
	}
	else if (CHECK_FLAG(m_uiVisible, ELEMENT_DETAIL) && MusicControlManager::Get().GetRuleCount() > 0)
	{
		MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(m_obCurTrackingElementName);
		MusicControlManager::MusicControlAction* pobAction = pobRule->GetAction();

		float fX = (m_uiVisible&CURRENT_INTENSITY) ? g_VisualDebug->GetDebugDisplayWidth()/2.0f:fLINE_PAD;
		float fY = g_VisualDebug->GetDebugDisplayHeight()/2.0f + fLINE_PAD;

		g_VisualDebug->Printf2D(
				fX,
				fY,
				DC_WHITE,
				DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
				"TRACKING ELEMENT DETAIL");

		if (pobRule && pobAction)
		{
			if (pobAction->GetLastActivationTime() + 3.0 >= MusicControlManager::Get().GetMusicControlTime())
			{
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					DC_YELLOW,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Name: %s *",
					m_obCurTrackingElementName.GetDebugString());
			}
			else if (pobRule->IsLapsing())
			{
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					DC_PURPLE,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Name: %s (lapse %0.2fs)",
					m_obCurTrackingElementName.GetDebugString(),
					pobRule->GetActivationLapseCompleteTime() - MusicControlManager::Get().GetMusicControlTime());
			}
			else
			{
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					pobRule->IsEnabled() ? (pobAction->IsInfluence() ? DC_WHITE:DC_RED):DC_GREY,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Name: %s",
					m_obCurTrackingElementName.GetDebugString());
			}

			switch (pobRule->GetType())
			{
			case THRESHOLD:
				{
					float fVal = 0.0f;
					pobRule->GetThresholdValue(fVal);
					g_VisualDebug->Printf2D(
						fX,
						fY += fLINE_PAD,
						DC_WHITE,
						DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
						"Type: THRESHOLD at %0.2f or above",
						fVal);
				}
				break;
			case THRESHOLD_INVERTED:
				{
					float fVal = 0.0f;
					pobRule->GetThresholdValue(fVal);
					g_VisualDebug->Printf2D(
						fX,
						fY += fLINE_PAD,
						DC_WHITE,
						DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
						"Type: THRESHOLD_INVERTED at %0.2f or below",
						fVal);
				}
				break;
			case RANGE:
				{
					float fMin = -1.0f;
					float fMax = -1.0f;
					pobRule->GetRangeValues(fMin, fMax);
					g_VisualDebug->Printf2D(
						fX,
						fY += fLINE_PAD,
						DC_WHITE,
						DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
						"Type: RANGE from %0.2f to %0.2f",
						fMin,
						fMax);
				}
				break;
			case RANGE_INVERTED:
				{
					float fMin = -1.0f;
					float fMax = -1.0f;
					pobRule->GetRangeValues(fMin, fMax);
					g_VisualDebug->Printf2D(
						fX,
						fY += fLINE_PAD,
						DC_WHITE,
						DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
						"Type: RANGE_INVERTED from %0.2f to %0.2f",
						fMin,
						fMax);
				}
				break;
			case DELTA:
				{
					float fVal = -1.0f;
					float fTime = -1.0f;
					pobRule->GetDeltaInfo(fVal, fTime);
					g_VisualDebug->Printf2D(
						fX,
						fY += fLINE_PAD,
						DC_WHITE,
						DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
						"Type: DELTA of %0.2f over %0.2fs",
						fVal,
						fTime);
				}
				break;
			default:
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					DC_WHITE,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Type: UNKNOWN");
				break;
			}

			switch (pobAction->GetType())
			{
			case FIXED_INTENSITY:
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					DC_WHITE,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Action: FIXED_INTENSITY of %0.2f",
					pobAction->GetIntensityMin());
				break;
			case FIXED_INTENSITY_SUSTAINED:
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					DC_WHITE,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Action: FIXED_INTENSITY_SUSTAINED of %0.2f",
					pobAction->GetIntensityMin());
				break;
			case SCALED_INTENSITY:
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					DC_WHITE,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Action: SCALED_INTENSITY from %0.2f to %0.2f",
					pobAction->GetIntensityMin(),
					pobAction->GetIntensityMax());
				break;
			case RELATIVE_INTENSITY:
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					DC_WHITE,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Action: RELATIVE_INTENSITY of %0.2f",
					pobAction->GetIntensityMin());
				break;
			default:
				g_VisualDebug->Printf2D(
					fX,
					fY += fLINE_PAD,
					DC_WHITE,
					DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
					"Action: UNKNOWN");
				break;
			}
		}
		else
		{
			g_VisualDebug->Printf2D(
				fX,
				fY += fLINE_PAD,
				DC_WHITE,
				DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
				"Nothing to display.");
		}

		MusicControlManager::MusicControlInput* pobInput = pobRule->GetInput();
		float fInput = pobInput ? pobInput->GetValue():-1.0f;
		g_VisualDebug->Printf2D(
			fX,
			fY += fLINE_PAD,
			DC_WHITE,
			DTF_ALIGN_LEFT|DTF_ALIGN_BOTTOM,
			"Input: %s, value %0.2f",
			pobInput ? MusicControlManager::Get().GetInputHash(pobInput).GetDebugString():"NOT FOUND",
			fInput);

		// Update and render graph
		UpdateGraphs(pobRule);
		g_VisualDebug->RenderGraph(
			&m_obInputGraph,
			CPoint(fX, fY += fLINE_PAD, 0.0f),
			CPoint(fX + g_VisualDebug->GetDebugDisplayWidth()*0.25f, g_VisualDebug->GetDebugDisplayHeight() - fLINE_PAD, 0.0f),
			0);
	}
}


//------------------------------------------------------------------------------------------
//	UpdateGraphs
//------------------------------------------------------------------------------------------
void MusicControlDebugDisplay::UpdateGraphs(MusicControlManager::MusicControlRule* pobUseRule)
{
	if (!pobUseRule)
		return;

	switch (pobUseRule->GetType())
	{
	case THRESHOLD:
	case THRESHOLD_INVERTED:
	case RANGE:
	case RANGE_INVERTED:
		{
			MusicControlManager::MusicControlInput* pobInput = pobUseRule->GetInput();
			if (!pobInput)
				return;
			m_pobInputSampleSet->AddSample(pobInput->GetValue());
		}
		break;
	case DELTA:
		{
			const MusicControlManager::MusicControlRule::InputDeltaMonitor* pobMon = 0;
			const MusicControlManager::MusicControlRule::InputDeltaMonitor* pobMem = 0;
			const MusicControlManager::MusicControlRule::InputDeltaMonitor* pobRec = 0;
			if (!pobUseRule->GetDeltaMonitors(pobMon, pobMem, pobRec))
				return;
			m_pobInputSampleSet->AddSample(pobMon->m_dTime > 0.0 ? (pobMem->m_fValue - pobMon->m_fValue):0.0f);
		}
		break;
	default:
		return;
	}
}


//------------------------------------------------------------------------------------------
//	ResetGraphs
//------------------------------------------------------------------------------------------
void MusicControlDebugDisplay::ResetGraphs(void)
{
	if (m_pobInputSampleSet)
		m_pobInputSampleSet->SetUsedSamples(0);
	else
		m_pobInputSampleSet = m_obInputGraph.AddSampleSet("INPUT", s_iSampleCount, DC_GREEN);

	MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(m_obCurTrackingElementName);
	int iType = pobRule ? pobRule->GetType():-1;
	if (THRESHOLD == iType || THRESHOLD_INVERTED == iType)
	{
		float fValue = 0.0f;
		pobRule->GetThresholdValue(fValue);

		m_obInputGraph.SetXAxis(0.0f, 1.0f, fGRID_LINE_SPACING);
		if (fValue == 0.0f)
			m_obInputGraph.SetYAxis(-1.0f, 1.0f, fGRID_LINE_SPACING);
		else
			m_obInputGraph.SetYAxis(-2.0f*fValue, 2.0f*fValue, fGRID_LINE_SPACING);
		m_obInputGraph.SetLimitValue(fValue);
	}
	else if (RANGE == iType || RANGE_INVERTED == iType)
	{
		float fMin = 0.0f;
		float fMax = 1.0f;
		pobRule->GetRangeValues(fMin, fMax);

		m_obInputGraph.SetXAxis(0.0f, 1.0f, fGRID_LINE_SPACING);
		if (fMin == 0.0f && fMax == 0.0f)
			m_obInputGraph.SetYAxis(-1.0f, 1.0f, fGRID_LINE_SPACING);
		else
			m_obInputGraph.SetYAxis(min(0.0f, fMin - 1.0f), fMax + 1.0f, fGRID_LINE_SPACING);
		m_obInputGraph.SetLimitValue(fMin);
	}
	else if (DELTA == iType)
	{
		float fDelta = 0.0f;
		float fTime = 0.0f;
		pobRule->GetDeltaInfo(fDelta, fTime);

		m_obInputGraph.SetXAxis(0.0f, 1.0f, fGRID_LINE_SPACING);
		if (fDelta == 0.0f)
			m_obInputGraph.SetYAxis(-1.0f, 1.0f, fGRID_LINE_SPACING);
		else
			m_obInputGraph.SetYAxis(-2.0f*abs(fDelta), 2.0f*abs(fDelta), abs(fDelta));
		m_obInputGraph.SetLimitValue(fDelta);
	}
	else
	{
		m_obInputGraph.SetXAxis(0.0f, 1.0f, fGRID_LINE_SPACING);
		m_obInputGraph.SetYAxis(-1.0f, 2.0f, fGRID_LINE_SPACING);
		m_obInputGraph.SetLimitValue(0.0f);
	}
}


//------------------------------------------------------------------------------------------
//	SetCurTrackingElementName
//!	@param obName	Name to set.
//!	@note If requested name cannot be set, current tracking element remains unchanged, or if
//!	the current name is invalid is set to the first valid tracking element name found, or if
//!	none exist, is set to null.
//------------------------------------------------------------------------------------------
void MusicControlDebugDisplay::SetCurTrackingElementName(CHashedString obName)
{
	// Determine what should be the new name
	MusicControlManager::MusicControlRule* pobRule = MusicControlManager::Get().GetRule(obName);
	if (!pobRule)
	{
		pobRule = MusicControlManager::Get().GetRule(m_obCurTrackingElementName);
		if (!pobRule)
		{
			if (MusicControlManager::Get().GetRuleCount() > 0)
			{
				m_obCurTrackingElementName = MusicControlManager::Get().GetRulesList()->begin()->first;
				pobRule = MusicControlManager::Get().GetRulesList()->begin()->second;
			}
			else
			{
				m_obCurTrackingElementName = CHashedString::nullString;
			}
		}
		else
		{
			// No change
			return;
		}
	}
	else if (m_obCurTrackingElementName != obName)
	{
		m_obCurTrackingElementName = obName;
	}
	else
	{
		// No change
		return;
	}

	ResetGraphs();
}


//------------------------------------------------------------------------------------------
//	GetNextTrackingElementName
//!	Retrieves name of next tracking element in manager list.
//------------------------------------------------------------------------------------------
CHashedString MusicControlDebugDisplay::GetNextTrackingElementName(void)
{
	ntstd::Map<CHashedString, MusicControlManager::MusicControlRule*>::iterator obItr = MusicControlManager::Get().GetRulesList()->begin();

	if (!m_obCurTrackingElementName.IsNull())
	{
		while (obItr != MusicControlManager::Get().GetRulesList()->end())
		{
			if (obItr->first == m_obCurTrackingElementName)
				break;
			++obItr;
		}

		if (++obItr == MusicControlManager::Get().GetRulesList()->end())
			obItr = MusicControlManager::Get().GetRulesList()->begin();
	}

	return (obItr == MusicControlManager::Get().GetRulesList()->end()) ? CHashedString::nullString:obItr->first;
}


//------------------------------------------------------------------------------------------
//	GetPrevTrackingElementName
//!	Retrieves name of previous tracking element in manager list.
//------------------------------------------------------------------------------------------
CHashedString MusicControlDebugDisplay::GetPrevTrackingElementName(void)
{
	ntstd::Map<CHashedString, MusicControlManager::MusicControlRule*>::reverse_iterator obItr = MusicControlManager::Get().GetRulesList()->rbegin();

	if (!m_obCurTrackingElementName.IsNull())
	{
		while (obItr != MusicControlManager::Get().GetRulesList()->rend())
		{
			if (obItr->first == m_obCurTrackingElementName)
				break;
			++obItr;
		}

		if (++obItr == MusicControlManager::Get().GetRulesList()->rend())
			obItr = MusicControlManager::Get().GetRulesList()->rbegin();
	}

	return (obItr == MusicControlManager::Get().GetRulesList()->rend()) ? CHashedString::nullString:obItr->first;
}


#endif // _MUSIC_CONTROL_DEBUG
