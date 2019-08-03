//------------------------------------------------------------------------------------------
//!	@file musiccontrol.h
//!	@author Chip Bell (SCEE)
//!	@date 09.10.06
//!
//!	@brief Declaration of the MusicControlManager and related classes.
//------------------------------------------------------------------------------------------


#ifndef _MUSICCONTROLMANAGER_H_
#define _MUSICCONTROLMANAGER_H_


#if defined(_DEBUG) || defined(_DEVELOPMENT)
#define _MUSIC_CONTROL_DEBUG
#endif


#include "core/keystring.h"
#include "core/nt_std.h"
#include "core/singleton.h"
#include "editable/enumlist.h"


#ifdef _MUSIC_CONTROL_DEBUG
#include "gfx/graphing.h"
#endif // _MUSIC_CONTROL_DEBUG


#define SET_FLAG(store, flag)		\
	store |= ((unsigned int)flag)
#define CLEAR_FLAG(store, flag)		\
	store &= (~((unsigned int)flag))
#define TOGGLE_FLAG(store, flag)	\
	store ^= ((unsigned int)flag)
#define CHECK_FLAG(store, flag)		\
	((((unsigned int)store)&((unsigned int)flag)) == ((unsigned int)flag))


//------------------------------------------------------------------------------------------
//!	@class MusicControlManagerSettings
//!	@brief Data binding for music controller manager setup.
//------------------------------------------------------------------------------------------
class MusicControlManagerSettings
{
public:
	MusicControlManagerSettings(void);
	~MusicControlManagerSettings(void);

	void PostConstruct(void);
	bool EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue);
	void EditorSelect(bool bSelect);
	void EditorSelectParent(bool bSelect);


	// Serialised data
	float m_fGlobalActivationLapse;	//!< After any tracking element activates, music control manager will suspend for this period

protected:
	void Synchronise(void);
	void Populate(void);
};


//------------------------------------------------------------------------------------------
//!	@class MusicControlTrackingElement
//!	@brief Data binding for music controllers. Encompasses an input, rule and action.
//------------------------------------------------------------------------------------------
class MusicControlTrackingElement
{
public:
	MusicControlTrackingElement(void);
	~MusicControlTrackingElement(void);

	void PostConstruct(void);
	bool EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue);
	void EditorSelect(bool bSelect);
	void EditorSelectParent(bool bSelect);


	void SetName(const CHashedString& obName);

	//!	Retrieves tracking element name.
	//!	@return Name used for tracking element rules and actions.
	CHashedString GetName(void) const
	{
		return m_obName;
	}


	// Serialised data
	CHashedString m_obName;						//!< Name of tracking element (used for rule and action)
	CHashedString m_obInputName;				//!< Name of associated input
	CHashedString m_obGroupLabel;				//!< Tracking element group identifier
	TRACKING_ELEMENT_RULE_TYPE m_eRuleType;		//!< Type of associated rule, indicates purpose of rule data values
	float m_fRuleData1;							//!< Rule data element 1, indicates threshold, range min. or delta
	float m_fRuleData2;							//!< Rule data element 2, indicates range max. or delta window
	TRACKING_ELEMENT_ACTION_TYPE m_eActionType;	//!< Type of associated action, indicates purpose of action data values
	float m_fActionData1;						//!< Action data element 1, indicates intensity min.
	float m_fActionData2;						//!< Action data element 1, indicates intensity max.
	float m_fInfluenceMin;						//!< Action influence range min. extent
	float m_fInfluenceMax;						//!< Action influence range max. extent
	float m_fActivationLapse;					//!< Delay after de-activation before re-activation can occur
	bool m_bEnable;								//!< Tracking element (i.e. rule) enabled/disabled
	bool m_bExemptFromSuspend;					//!< Tracking element always updated

protected:
	bool Rename(const CHashedString& obNewName);
	void Synchronise(void);
	void Populate(void);

private:
};


//------------------------------------------------------------------------------------------
//!	@class MusicControlManager
//!	@brief Maintains music controllers.
//------------------------------------------------------------------------------------------
class MusicControlManager : public Singleton<MusicControlManager>
{
public:
	//--------------------------------------------------------------------------------------
	//!	@class MusicControlInput
	//!	@brief Base class for implementing controller rule access to game variables/states.
	//--------------------------------------------------------------------------------------
	class MusicControlInput
	{
	public:
		//!	Default ctor
		//!	@note Inputs enabled by default.
		MusicControlInput(void)
		:
			m_iDeps(0)
		{}

		//!	Default dtor.
		virtual ~MusicControlInput(void)
		{}

		//!	Updates input. If input is enabled, internal update method will be called.
		void Update(void)
		{
			if (m_iDeps > 0)
				UpdateInternal();
		}

		//!	Adds a dependent rule. Rules should add themselves as dependents before requesting input value.
		//!	@note If this is the first dependent, input will be updated (as internal update will have been skipped).
		void IncrementDependents(void)
		{
			m_iDeps += 1;
			if (m_iDeps <= 1)
				Update();
		}

		//!	Removes a dependent rule.
		void DecrementDependents(void)
		{
			m_iDeps -= 1;
		}

		//!	Retrieves number of dependents.
		//!	@return Number of rules requiring this input.
		int GetDependents(void) const
		{
			return m_iDeps;
		}

		//!	Retrieves input value.
		//!	@sa MusicControlInput::UpdateInternal(void)
		virtual float GetValue(void) = 0;

		//!	Sets an input parameter.
		//!	@param obArgument	Parameter identifier.
		//!	@param fValue		Parameter value.
		//!	@return True on success, false otherwise.
		virtual bool SetParameter(const CHashedString& obParameter, float fValue)
		{
			UNUSED(obParameter);
			UNUSED(fValue);
			return false;
		}

		//!	Gets an input parameter.
		//!	@param obArgument	Parameter identifier.
		//!	@param rfValue		Reference which receives parameter value on success. Only modified on success.
		//!	@return True on success, false otherwise.
		virtual bool GetParameter(const CHashedString& obParameter, float& rfValue)
		{
			UNUSED(obParameter);
			UNUSED(rfValue);
			return false;
		}

	protected:
		//!	Updates input. Input implementations should take advantage of this method to
		//!	reduce the amount of computation per frame should multiple rules be requesting
		//!	the associated value.
		//!	@sa MusicControlInput::GetValue(void)
		virtual void UpdateInternal(void)
		{}

	private:
		int m_iDeps;	//!< Number of dependent rules.
	};


	//--------------------------------------------------------------------------------------
	//!	@class MusicControlAction
	//!	@brief Actions ultimately adjust the music intensity.
	//--------------------------------------------------------------------------------------
	class MusicControlAction
	{
	public:
		MusicControlAction(void);
		~MusicControlAction(void);

		//!	Set intensities associated with action. Changes to scaled intensity type.
		//!	@param fMin	Minimum intensity action will set during activation.
		//!	@param fMax	Maximum intensity action will set during activation.
		//!	@note When an intensity range is set on an action, the current music intensity will
		//!	be modified per update (similar to sustained).
		void SetIntensityRange(float fMin, float fMax)
		{
			m_fIntensityReqMin = fMin;
			m_fIntensityReqMax = fMax;
			m_eActionType = SCALED_INTENSITY;
		}

		//!	Set single fixed intensity associated with action. Changes to fixed intensity or fixed intensity sustained type.
		//!	@param fValue	Intensity action will set upon activation.
		//!	@param bSustain	Indicates intensity should be sustained during activation (i.e. set per update).
		void SetFixedIntensity(float fValue, bool bSustain = false)
		{
			SetIntensityRange(fValue, fValue);
			m_eActionType = bSustain ? FIXED_INTENSITY_SUSTAINED:FIXED_INTENSITY;
		}

		//!	Set single relative intensity associated with action. Changes to relative intensity type.
		//!	@param fValue	Intensity delta action will set upon activation (+ve or -ve).
		void SetRelativeIntensity(float fValue)
		{
			SetIntensityRange(fValue, fValue);
			m_eActionType = RELATIVE_INTENSITY;
		}

		//!	Retrieves current action type.
		//!	@return Internal action type identifier.
		//!	@note The action type is not set directly, it is modified internally as appropriate when
		//!	different set methods are used.
		TRACKING_ELEMENT_ACTION_TYPE GetType(void) const
		{
			return m_eActionType;
		}

		//!	Retrieves minimum extent of the range of intensities this action may set.
		//!	@return Min. action intensity request.
		float GetIntensityMin(void) const
		{
			return m_fIntensityReqMin;
		}

		//!	Retrieves Maximum extent of the range of intensities this action may set.
		//!	@return Max. action intensity request.
		float GetIntensityMax(void) const
		{
			return m_fIntensityReqMax;
		}

		//!	Actions only modify music intensity if it currently lies within the influence range.
		//!	@param fMin	Minimum extent of influenced range (normalised).
		//!	@param fMax	Maximum extent of influenced range (normalised).
		void SetInfluenceRange(float fMin, float fMax)
		{
			m_fInfluenceRangeMin = (fMin < fMax) ? fMin:fMax;
			if (m_fInfluenceRangeMin < 0.0f)
				m_fInfluenceRangeMin = 0.0f;
			else if (m_fInfluenceRangeMin > 1.0f)
				m_fInfluenceRangeMin = 1.0f;
			m_fInfluenceRangeMax = (fMax > fMin) ? fMax:fMin;
			if (m_fInfluenceRangeMax < 0.0f)
				m_fInfluenceRangeMax = 0.0f;
			else if (m_fInfluenceRangeMax > 1.0f)
				m_fInfluenceRangeMax = 1.0f;
		}

		//!	Retrieves lowest influenced music intensity.
		//!	@return Minimum extent of influenced range (normalised).
		float GetInfluenceMin(void) const
		{
			return m_fInfluenceRangeMin;
		}

		//!	Retrieves highest influenced music intensity.
		//!	@return Maximum extent of influenced range (normalised).
		float GetInfluenceMax(void) const
		{
			return m_fInfluenceRangeMax;
		}

		bool IsInfluence(void);

		bool Activate(void);
		void Deactivate(void);

		//!	Indicates whether or not action is currently being updated.
		//!	@return True if action active, false otherwise.
		bool IsActive(void) const
		{
			return m_bActive;
		}

		void Update(float fValue);

	protected:
		void RequestIntensity(float fRawValue);

	private:
		TRACKING_ELEMENT_ACTION_TYPE m_eActionType;	//!< Action type determines update and activation/deactivation behaviour
		float m_fIntensityReqMin;					//!< Action minimum intensity request
		float m_fIntensityReqMax;					//!< Action maximum intensity request
		float m_fInfluenceRangeMin;					//!< Minimum extent of intensity range action influence (action has no effect if current music intensity below this threshold)
		float m_fInfluenceRangeMax;					//!< Maximum extent of intensity range action influence (action has no effect if current music intensity above this threshold)
		bool m_bActive;								//!< Indicates action is currently active and requires updates

#ifdef _MUSIC_CONTROL_DEBUG
	public:
		//!	Retrieves time when action last activated.
		//!	@return System time at last activation request.
		double GetLastActivationTime(void) const
		{
			return m_dLastActivationTime;
		}

	private:
		double m_dLastActivationTime;				//!< System time at last activation
#endif // _MUSIC_CONTROL_DEBUG
	};

	
	//--------------------------------------------------------------------------------------
	//!	@class MusicControlRule
	//!	@brief Music control rules do all the work.
	//--------------------------------------------------------------------------------------
	class MusicControlRule
	{
	public:
		//!	Data store for controlling delta based activation.
		struct InputDeltaMonitor
		{
			//!	Intialises input delta monitor data.
			InputDeltaMonitor(void) : m_dTime(0.0), m_fValue(0.0f) {}

			double m_dTime;	//!< Time at which input value occurred
			float m_fValue;	//!< Input value for given time
		};


		MusicControlRule(void);
		~MusicControlRule(void);

		void Update(void);

		//!	Sets group label.
		//!	@param obLabel	Label to set.
		//!	@note A group label can be applied to help organise and manage tracking elements.
		void SetGroupLabel(const CHashedString& obLabel)
		{
			m_obGroupLabel = obLabel;
		}

		//!	Retrieves group label.
		//!	@return Group label identifier.
		CHashedString GetGroupLabel(void) const
		{
			return m_obGroupLabel;
		}

		//!	Sets assocated input.
		//!	@param pobInput	Pointer to input, can be null.
		//!	@note If specified input is null, rule will be disabled.
		void SetInput(MusicControlInput* pobInput)
		{
			if (m_pobInput)
				m_pobInput->DecrementDependents();

			m_pobInput = pobInput;
			if (!m_pobInput)
				Enable(false);
			else
				m_pobInput->IncrementDependents();
		}

		//!	Retrieves associated input.
		//!	@return Rule input, or null if not set.
		MusicControlInput* GetInput(void)
		{
			return m_pobInput;
		}

		//!	Sets assocated action.
		//!	@param pobAction	Pointer to action, can be null.
		//!	@note Rule will be disabled, so any current action will be deactivated.
		void SetAction(MusicControlAction* pobAction)
		{
			Enable(false);
			m_pobAction = pobAction;
		}

		//!	Retrieves associated action.
		//!	@return Rule action, or null if not set.
		MusicControlAction* GetAction(void)
		{
			return m_pobAction;
		}

		//!	Enables/disables rule.
		//!	@param bEnable	Indicates rule should be enabled (true) or disabled (false).
		//!	@return True on success, false otherwise.
		//!	@note Associated action will be deactivated if rule is disabled.
		//!	@note Rules cannot be enabled until a valid input and action have been set.
		//!	@sa MusicControlManager::MusicControlRule::SetInput(MusicControlInput* pobInput)
		//!	@sa MusicControlManager::MusicControlRule::SetAction(MusicControlAction* pobAction)
		bool Enable(bool bEnable)
		{
			if (bEnable)
			{
				if (!m_pobInput || !m_pobAction)
					return false;
			}
			else
				Activate(false);

			m_bEnable = bEnable;

			if (DELTA == m_eRuleType)
				ResetDeltaInfo();

			m_dActivationLapseComplete = 0.0;
			m_bLapsePending = false;

			return true;
		}

		//!	Indicates whether or not rule is disabled.
		//!	@return True if rule enabled, false otherwise.
		bool IsEnabled(void) const
		{
			return m_bEnable;
		}

		//!	Marks rule as exempt from global suspend.
		//!	@param	bExempt	True if rule should be updated even when music control manager suspended, false otherwise.
		void ExemptFromSuspend(bool bExempt)
		{
			m_bExemptFromSuspend = bExempt;
		}

		//!	Indicates rule should be updated even if music control manager is suspended.
		//!	@return True if rule should be always updated, false otherwise.
		bool IsExemptFromSuspend(void) const
		{
			return m_bExemptFromSuspend;
		}

		//!	Indicates rule is a normalised value, scaled over a given range.
		//!	@param fMin		Minimum extent of range.
		//!	@param fMax		Maximum extent of range.
		//!	@param bInvert	Indicates rule value should be inverted.
		//!	@note Range based rules activate when the input is in the supplied range.
		//!	@note Range based rules normalise their value against the supplied range,
		//!	from 0 to 1 for min to max respectively, or 1 to 0 if the rule is inverted.
		//!	@note New range will not take effect until next update.
		void SetRange(float fMin, float fMax, bool bInvert)
		{
			m_ActivationData.RangedActivation.m_fActivationMin = (fMin < fMax) ? fMin:fMax;
			m_ActivationData.RangedActivation.m_fActivationMax = (fMax > fMin) ? fMax:fMin;

			m_eRuleType = bInvert ? RANGE_INVERTED:RANGE;
		}

		//!	Indicates rule is boolean, activating at a given threshold.
		//!	@param fThreshold	Rule activation threshold.
		//!	@param bInvert		Indicates rule value should be inverted.
		//!	@note Threshold based rules activate at or above the threshold value. Inverted
		//!	threshold rules activate at or below the threshold value.
		//!	@note Threshold activation is simply a range of zero (i.e. equal extents).
		void SetThreshold(float fThreshold, bool bInvert)
		{
			SetRange(fThreshold, fThreshold, bInvert);
			m_eRuleType = bInvert ? THRESHOLD_INVERTED:THRESHOLD;
		}

		//!	Indicates rule is delta based, that is, if the input value changes by a certain amount
		//!	within a specified time, then the rule activates (boolean activation).
		//!	@param	fDelta	Required change (positive or negative).
		//!	@param	fTime	Time frame.
		void SetDelta(float fDelta, float fTime)
		{
			m_ActivationData.DeltaActivation.m_fActivationDelta = fDelta;
			m_ActivationData.DeltaActivation.m_fActivationTimeout = (fTime >= 0.0f) ? fTime:0.0f;

			m_eRuleType = DELTA;

			// Prep for delta based update
			ResetDeltaInfo();
		}

		//!	Sets activation lapse.
		//!	@param fTime	Delay after de-activation before re-activation can occur (in seconds).
		void SetActivationLapse(float fTime)
		{
			m_fActivationLapse = fTime < 0.0f ? 0.0f:fTime;
		}

		//! Retrieves activation lapse.
		//!	@return Delay after de-activation before re-activation can occur (in seconds).
		float GetActivationLapse(void) const
		{
			return m_fActivationLapse;
		}

		//!	Indicates rule is current subject to an activation lapse.
		//!	@return True if rule is currently lapsing, false otherwise.
		bool IsLapsing(void) const
		{
			return m_dActivationLapseComplete > MusicControlManager::Get().GetMusicControlTime();
		}

		//!	Retrieves time at which any current or last activation lapse expires.
		//!	@return System time when last activation lapse expired, or zero if rule has never lapsed.
		double GetActivationLapseCompleteTime(void) const
		{
			return m_dActivationLapseComplete;
		}

		//!	Retrieves rule type.
		//!	@return Rule type.
		TRACKING_ELEMENT_RULE_TYPE GetType(void) const
		{
			return m_eRuleType;
		}

		//!	Obtains rule activation threshold value.
		//!	@param rfThreshold	Reference which receives threshold value.
		//!	@return True on success, false otherwise (will fail if rule is not a threshold type).
		//!	@note Supplied reference only modified on successful return.
		bool GetThresholdValue(float& rfThreshold) const
		{
			if (m_eRuleType == THRESHOLD || m_eRuleType == THRESHOLD_INVERTED)
			{
				rfThreshold = m_ActivationData.RangedActivation.m_fActivationMin;
				return true;
			}

			return false;
		}

		//!	Obtains rule activation range.
		//!	@param rfMin	Reference which receives range minimum extent.
		//!	@param rfMax	Reference which receives range maximum extent.
		//!	@return True on success, false otherwise (will fail if rule is not a range type).
		//!	@note Supplied references only modified on successful return.
		bool GetRangeValues(float& rfMin, float& rfMax) const
		{
			if (m_eRuleType == RANGE || m_eRuleType == RANGE_INVERTED)
			{
				rfMin = m_ActivationData.RangedActivation.m_fActivationMin;
				rfMax = m_ActivationData.RangedActivation.m_fActivationMax;
				return true;
			}

			return false;
		}

		//!	Obtains rule activation delta and timeout values.
		//!	@param rfDelta	Reference which receives delta value.
		//!	@param rfTime	Reference which receives timeout value.
		//!	@return True on success, false otherwise (will fail if rule is not a delta type).
		//!	@note Supplied references only modified on successful return.
		bool GetDeltaInfo(float& rfDelta, float& rfTime) const
		{
			if (m_eRuleType == DELTA)
			{
				rfDelta = m_ActivationData.DeltaActivation.m_fActivationDelta;
				rfTime = m_ActivationData.DeltaActivation.m_fActivationTimeout;
				return true;
			}

			return false;
		}

		//!	Obtains internal delta monitor information for delta based rules.
		//!	@param rpobDeltaMonitor	Reference to a pointer which receives address of internal delta monitor.
		//!	@param rpobDeltaMemory	Reference to a pointer which receives address of internal delta memory (stats from last frame).
		//!	@param rpobDeltaRecover	Reference to a pointer which receives address of internal delta timeout window recovery point.
		//!	@return True on success, false otherwise (will fail if rule is not a delta type).
		//!	@note Supplied references only modified on successful return.
		bool GetDeltaMonitors(const InputDeltaMonitor*& rpobDeltaMonitor, const InputDeltaMonitor*& rpobDeltaMemory, const InputDeltaMonitor*& rpobDeltaRecover)
		{
			if (m_eRuleType == DELTA)
			{
				rpobDeltaMonitor = &m_stDeltaMonitor;
				rpobDeltaMemory = &m_stDeltaMemory;
				rpobDeltaRecover = &m_stDeltaRecover;
				return true;
			}

			return false;
		}

	protected:
		float ActivationCheckRange(void);
		float ActivationCheckDelta(void);

		void Activate(bool bActive);

		//! Resets delta monitoring info.
		void ResetDeltaInfo(void)
		{
			m_ActivationData.DeltaActivation.m_bDeltaTargetMet = false;
			m_stDeltaMemory.m_dTime = MusicControlManager::Get().GetMusicControlTime();
			m_stDeltaMemory.m_fValue = m_pobInput ? m_pobInput->GetValue():0.0f;
			m_stDeltaMonitor.m_dTime = -1.0f;
			m_stDeltaMonitor.m_fValue = 0.0f;
			m_stDeltaRecover.m_dTime = -1.0;
			m_stDeltaRecover.m_fValue = 0.0f;
		}

	private:
		union ActivationData
		{
			//!	Initialises all data members.
			ActivationData(void)
			{
				RangedActivation.m_fActivationMin = 0.0f;
				RangedActivation.m_fActivationMax = 0.0f;
			}

			struct
			{
				float m_fActivationMin;			//!< Range minimum extent
				float m_fActivationMax;			//!< Range maximum extent
			} RangedActivation;					//!< Data associated with ranged rule activation
			struct
			{
				float m_fActivationDelta;		//!< Required change in monitored variable for activation
				float m_fActivationTimeout;		//!< Time window in which delta must be achieved
				bool m_bDeltaTargetMet;			//!< Used internally for delta tracking
			} DeltaActivation;					//!< Data associated with delta based rule activation
		} m_ActivationData;						//!< Holds data associated with rule activation and value calculation

		CHashedString m_obGroupLabel;			//!< Group label

		TRACKING_ELEMENT_RULE_TYPE m_eRuleType;	//!< Rule type determines update and activation behaviour

		MusicControlInput* m_pobInput;			//!< Associated input
		MusicControlAction* m_pobAction;		//!< Associated action

		float m_fActivationLapse;				//!< Delay after de-activation before re-activation can occur
		double m_dActivationLapseComplete;		//!< Time at which any current activation lapse expires
		bool m_bActive;							//!< Indicates whether or not associated action is active
		bool m_bEnable;							//!< Indicates whether or not rule is enabled
		bool m_bExemptFromSuspend;				//!< Indicates rule is exempt from music control manager suspend (is updated regardless)
		bool m_bLapsePending;					//!< Indicates rule should lapse when associated action is again influential

		InputDeltaMonitor m_stDeltaMonitor;		//!< For delta rules, stores details for start point of a tracked time window
		InputDeltaMonitor m_stDeltaMemory;		//!< For delta rules, stores details for last update
		InputDeltaMonitor m_stDeltaRecover;		//!< For delta rules, stores recovery details for a point in currently tracked time window
	};




	MusicControlManager(void);
	~MusicControlManager(void);

	double GetMusicControlTime(void) const;

	void Update(void);

	void Suspend(float fDuration = -1.0f);
	bool IsSuspended(void) const;

	void Resume(void);

	//!	Retrieves time at which music control manager will resume.
	//!	@return System time at which current suspend will end, negative if suspend is indefinite, zero if not suspended.
	double GetResumeTime(void) const
	{
		return m_dResumeTime;
	}

	//!	Sets a global activation lapse period (automatic suspend after any tracking element activation)
	//!	@param fTime	Lapse time. Use zero to disable, or negative for a permanent suspend after activation.
	//!	@note Does not effect any current lapse which may be in effect.
	void SetGlobalActivationLapse(float fTime)
	{
		m_fGlobalActivationLapse = fTime;
	}

	//!	Retrieves global activation lapse time
	float GetGlobalActivationLapse(void) const
	{
		return m_fGlobalActivationLapse;
	}

	void RegisterBuiltinInputs(void);

	bool RegisterInput(MusicControlInput* pobCandidate, const CHashedString& obName);
	bool RegisterAction(MusicControlAction* pobCandidate, const CHashedString& obName);
	bool RegisterRule(MusicControlRule* pobCandidate, const CHashedString& obName);

	bool DeregisterInput(const CHashedString& obName, bool bDestroy = true);
	bool DeregisterAction(const CHashedString& obName, bool bDestroy = true);
	bool DeregisterRule(const CHashedString& obName, bool bDestroy = true);

	void DeregisterAllInputs(bool bDestroy = true);
	void DeregisterAllActions(bool bDestroy = true);
	void DeregisterAllRules(bool bDestroy = true);

	//!	Deregisters all actions, inputs and rules (in that order).
	//!	@param bDestroy	Indicates deregistered actions, inputs and rules should be deleted.
	void DeregisterEverything(bool bDestroy = true)
	{
		DeregisterAllRules(bDestroy);
		DeregisterAllActions(bDestroy);
		DeregisterAllInputs(bDestroy);
	}

	//!	Retrieves list of inputs.
	//!	@return Pointer to internal inputs list.
	ntstd::Map<CHashedString, MusicControlInput*>* GetInputsList(void)
	{
		return &m_obInputsList;
	}

	//!	Retrieves list of actions.
	//!	@return Pointer to internal actions list.
	ntstd::Map<CHashedString, MusicControlAction*>* GetActionsList(void)
	{
		return &m_obActionsList;
	}

	//!	Retrieves list of rules.
	//!	@return Pointer to internal rules list.
	ntstd::Map<CHashedString, MusicControlRule*>* GetRulesList(void)
	{
		return &m_obRulesList;
	}

	MusicControlInput* GetInput(const CHashedString& obName);
	MusicControlAction* GetAction(const CHashedString& obName);
	MusicControlRule* GetRule(const CHashedString& obName);

	//!	@return Number of inputs.
	unsigned int GetInputCount(void) const
	{
		return (unsigned int)m_obInputsList.size();
	}

	//!	@return Number of actions.
	unsigned int GetActionCount(void) const
	{
		return (unsigned int)m_obActionsList.size();
	}

	//!	@return Number of rules.
	unsigned int GetRuleCount(void) const
	{
		return (unsigned int)m_obRulesList.size();
	}

	CHashedString GetInputHash(MusicControlInput* pobInput);
	CHashedString GetActionHash(MusicControlAction* pobAction);
	CHashedString GetRuleHash(MusicControlRule* pobRule);

	bool EnableRule(const CHashedString& obName, bool bEnable);
	bool EnableRuleGroup(const CHashedString& obLabel, bool bEnable);
	void EnableAllRules(bool bEnable);

	// Light wrappers
	bool RegisterInput(MusicControlInput* pobCandidate, const char* pcName) {return (pobCandidate && pcName) ? RegisterInput(pobCandidate, CHashedString(pcName)):false;}
	bool RegisterAction(MusicControlAction* pobCandidate, const char* pcName) {return (pobCandidate && pcName) ? RegisterAction(pobCandidate, CHashedString(pcName)):false;}
	bool RegisterRule(MusicControlRule* pobCandidate, const char* pcName) {return (pobCandidate && pcName) ? RegisterRule(pobCandidate, CHashedString(pcName)):false;}
	bool DeregisterInput(const char* pcName, bool bDestroy = true) {return pcName ? DeregisterInput(CHashedString(pcName), bDestroy):false;}
	bool DeregisterAction(const char* pcName, bool bDestroy = true) {return pcName ? DeregisterAction(CHashedString(pcName), bDestroy):false;}
	bool DeregisterRule(const char* pcName, bool bDestroy = true) {return pcName ? DeregisterRule(CHashedString(pcName), bDestroy):false;}
	MusicControlInput* GetInput(const char* pcName) {return pcName ? GetInput(CHashedString(pcName)):0;}
	MusicControlAction* GetAction(const char* pcName) {return pcName ? GetAction(CHashedString(pcName)):0;}
	MusicControlRule* GetRule(const char* pcName) {return pcName ? GetRule(CHashedString(pcName)):0;}
	bool EnableRule(const char* pcName, bool bEnable) {return pcName ? EnableRule(CHashedString(pcName), bEnable):false;}
	bool EnableRuleGroup(const char* pcName, bool bEnable) {return pcName ? EnableRuleGroup(CHashedString(pcName), bEnable):false;}

	void ToggleDebugging(void);
	bool IsDebuggingEnabled(void);

private:
	ntstd::Map<CHashedString, MusicControlInput*> m_obInputsList;	//!< Rule inputs
	ntstd::Map<CHashedString, MusicControlAction*> m_obActionsList;	//!< Rule actions
	ntstd::Map<CHashedString, MusicControlRule*> m_obRulesList;		//!< List of all rules in the system
	double m_dResumeTime;											//!< Time at which music control system update will resume
	float m_fGlobalActivationLapse;									//!< Global music manager supend after any tracking element activation
};


#ifdef _MUSIC_CONTROL_DEBUG


//------------------------------------------------------------------------------------------
//!	@class MusicControlDebugDisplay
//!	@brief Renders tracking element debug information.
//------------------------------------------------------------------------------------------
class MusicControlDebugDisplay : public Singleton<MusicControlDebugDisplay>
{
public:
	struct ActivationHistoryEntry
	{
		ActivationHistoryEntry(void)
		:
			m_dTime(0.0),
			m_fCurrentIntensity(0.0f),
			m_fRequestedIntensity(0.0f),
			m_fResultantIntensity(0.0f)
		{}

		void Set(const ActivationHistoryEntry& stActHistEntry)
		{
			m_obActionName = stActHistEntry.m_obActionName;
			m_dTime = stActHistEntry.m_dTime;
			m_fCurrentIntensity = stActHistEntry.m_fCurrentIntensity;
			m_fRequestedIntensity = stActHistEntry.m_fRequestedIntensity;
			m_fResultantIntensity = stActHistEntry.m_fResultantIntensity;
		}

		CHashedString m_obActionName;
		double m_dTime;
		float m_fCurrentIntensity;
		float m_fRequestedIntensity;
		float m_fResultantIntensity;
	};

	MusicControlDebugDisplay(void);

	//!	Default dtor.
	~MusicControlDebugDisplay(void)
	{}

	void Update(void);
	void Refresh(void);

	void AddActivationHistoryEntry(const ActivationHistoryEntry& stActHistEntry);

private:
	enum DEBUG_DISPLAY_ELEMENT
	{
		CURRENT_INTENSITY	= 1,
		ACTIVATION_HISTORY	= 2,
		ELEMENT_LIST		= 4,
		ELEMENT_DETAIL		= 8,
	};

	static const int s_iSampleCount;
	static const int s_iActivationHistoryEntries = 4;

	void UpdateFocus(void);
	void UpdateDisplay(void);
	void UpdateGraphs(MusicControlManager::MusicControlRule* pobUseRule);

	void ResetGraphs(void);

	void SetCurTrackingElementName(CHashedString obName);
	CHashedString GetNextTrackingElementName(void);
	CHashedString GetPrevTrackingElementName(void);

	DEBUG_DISPLAY_ELEMENT m_eFocus;
	unsigned int m_uiVisible;
	CHashedString m_obCurTrackingElementName;
	CGraph m_obInputGraph;
	CGraphSampleSet* m_pobInputSampleSet;

	int m_iNextHistoryEntryIdx;
	ActivationHistoryEntry m_astActivationHistory[s_iActivationHistoryEntries];
};


#endif // _MUSIC_CONTROL_DEBUG


#endif // _MUSICCONTROLMANAGER_H_
