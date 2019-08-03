//--------------------------------------------------
//!
//!	\file functioncurve.cpp
//!	XML object that represents an editable 1Dimensional
//! n-segment curve, with debug display 
//!
//--------------------------------------------------

#include "functioncurve.h"
#include "functiongraph.h"

#include "input/mouse.h"
#include "effect_error.h"

#include "core/listtools.h"
#include "game/luaglobal.h"

// this is just here for autonaming
#include "effect_util.h"
#include "objectdatabase/neteditinterface.h"
#include "objectdatabase/dataobject.h"

#include "core/fileattribute.h"

START_STD_INTERFACE( FCurveNode_Hermite )
	IFLOAT(	FCurveNode_Hermite,	Start )
	IFLOAT(	FCurveNode_Hermite,	Out )
	IFLOAT(	FCurveNode_Hermite,	In )
	IFLOAT(	FCurveNode_Hermite,	Time )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_STD_INTERFACE( FunctionCurve_User )
	IINT(		FunctionCurve_User,	NumSegments )
	IFLOAT(		FunctionCurve_User,	Tolerance )
	IFLOAT(		FunctionCurve_User,	Scale )
	IFLOAT(		FunctionCurve_User,	Offset )
	PUBLISH_PTR_CONTAINER_AS(		m_obCurveNodes,	CurveNodes )
	ISTRING(	FunctionCurve_User, FunctionScript )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
	DECLARE_DEBUGRENDER_FROMNET_CALLBACK( DebugRender )
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	function curve node ctor
//!
//--------------------------------------------------
FCurveNode_Hermite::FCurveNode_Hermite() :
	m_fStart( 0.0f ),
	m_fOut( 0.0f ),
	m_fIn( 0.0f ),
	m_fTime( 0.0f )
{
}

//--------------------------------------------------
//!
//!	record highest unique name counter
//!
//--------------------------------------------------
void FCurveNode_Hermite::PostConstruct()
{
}

//--------------------------------------------------
//!
//!	function curve ctor
//!
//--------------------------------------------------
FunctionCurve_User::FunctionCurve_User() :
	m_iNumSegments( 1 ),
	m_fTolerance(0.01f),
	m_fScale(1.0f),
	m_fOffset(0.0f),
	m_pFittedCurve(0),
	m_pBufferCurve(0),
	m_bScriptChecked(true),
	m_bFirstTime(true),
	m_bCurveChanged(false),
	m_scriptModDate(-1)
{
}

//--------------------------------------------------
//!
//!	cleanup our curve
//!
//--------------------------------------------------
FunctionCurve_User::~FunctionCurve_User()
{
	if( m_pFittedCurve )
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pFittedCurve );
	}

	if( m_pBufferCurve )
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pBufferCurve );
	}
}

//--------------------------------------------------
//!
//!	initialise internal curve objects from nodes.
//!
//--------------------------------------------------
void FunctionCurve_User::PostConstruct()
{
	RebuildCurve();
}

void FunctionCurve_User::RebuildCurve()
{
	ntError( m_iNumSegments >= 1 );
	
	ntstd::List< FCurveNode_Hermite* >	m_TempList;

	float fMaxTime = 0.0f;

	// dump all the current nodes into a temp list and empty the member list.

	while ( !m_obCurveNodes.empty() )
	{
		FCurveNode_Hermite* pCurr = m_obCurveNodes.front();

		if (pCurr->m_fTime > fMaxTime)
			fMaxTime = pCurr->m_fTime;

		m_TempList.push_back( pCurr );
		m_obCurveNodes.pop_front();
	}

	// sort the nodes based on time
	
	bubble_sort( m_TempList.begin(), m_TempList.end(), CComparator_FCurveNode_LT() );

	// create a new list based on our required number of segments and copy their values
	
	int iReqNodes = m_iNumSegments + 1;

	for ( int i = 0; i < iReqNodes; i++ )
	{
		if ( !m_TempList.empty() )
		{
			m_obCurveNodes.push_back( m_TempList.front() );
			m_TempList.pop_front();			
		}
		else
		{
			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
			ObjectContainer* pParentContainer = pDO->GetParent();
			FCurveNode_Hermite* pCurr = ObjectDatabase::Get().Construct<FCurveNode_Hermite>( "FCurveNode_Hermite", DataObject::GetUniqueName(), pParentContainer );
			pCurr->m_fTime = fMaxTime;
			m_obCurveNodes.push_back( pCurr );
		}
	}

	// clean up dead nodes if we just got smaller.

	while ( !m_TempList.empty() )
	{
		ObjectDatabase::Get().Destroy( m_TempList.back() );
		m_TempList.pop_back();
	}

	// now finally create our real internal list.

	if (m_pFittedCurve)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pFittedCurve );
	}

	// [scee_st] @todo can't chunk easily unless I template the CCubicTimeHermite class (or add a param to it)
	// or ignore the delete mismatch
	float*		pTimes = NT_NEW float[ m_iNumSegments + 1 ];
	CVector*	pVerts = NT_NEW CVector[ m_iNumSegments + 1 ];

	CVector*	pStarts = NT_NEW CVector[ m_iNumSegments ];
	CVector*	pEnds = NT_NEW CVector[ m_iNumSegments ];

	float fOneThird = 1.0f / 3.0f;

	ntstd::List< FCurveNode_Hermite* >::iterator curr = m_obCurveNodes.begin();

	for (int i = 0; i <= m_iNumSegments; i++, ++curr )
	{
		pTimes[i]		= (*curr)->m_fTime;
		pVerts[i]		= CVector( (*curr)->m_fStart, 0.0f, 0.0f, 0.0f );

		if (i < m_iNumSegments)
		{
			pStarts[i] = CVector( (*curr)->m_fOut * fOneThird, 0.0f, 0.0f, 0.0f );
			pEnds[i] = CVector( (*curr)->m_fIn * fOneThird, 0.0f, 0.0f, 0.0f );
		}		
	}

	CCubicTimeHermite* pCurve = NT_NEW CCubicTimeHermite( pTimes, true, true,  m_iNumSegments + 1, pVerts, pStarts, pEnds, true, true );

	m_pFittedCurve = NT_NEW FunctionCurve_Fitted( m_fTolerance );
	m_pFittedCurve->Finalise( pCurve );
}

//--------------------------------------------------
//!
//!	refresh our internal state after editing
//!
//--------------------------------------------------
bool FunctionCurve_User::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	CHashedString pcMember(param);

	if ( pcMember == CHashedString(HASH_STRING_CURVE_NODES) )
	{
		// special case, do not do anything in this case, rely on external tool
		return false;
	}

	// correct invalid segment number
	if (m_iNumSegments < 1)
		m_iNumSegments = 1;

	if (m_fTolerance < EPSILON)
		m_fTolerance = EPSILON; 

	if	( pcMember == CHashedString(HASH_STRING_TOLERANCE) || pcMember == CHashedString(HASH_STRING_FUNCTIONSCRIPT)	)
	{
		m_bScriptChecked = false;
		m_scriptModDate = -1;
	}

	if	( pcMember == CHashedString(HASH_STRING_SCALE) || pcMember == CHashedString(HASH_STRING_OFFSET) )
	{
		m_bCurveChanged = true;
	}

	PostConstruct();
	return true;
}

//--------------------------------------------------
//!
//!	refresh our internal state after sub object editing
//!
//--------------------------------------------------
void FunctionCurve_User::EditorChangeParent()
{
	PostConstruct();
}

//--------------------------------------------------
//!
//!	function curve debug render
//!
//--------------------------------------------------
void FunctionCurve_User::CheckForScriptFunction()
{
//#ifdef PLATFORM_PC // FIXME_WIL
#if 0 // ALEXEY_TODO : now I fucked it on pc too, yay!

	bool bFileExists = false;
	char aScriptName[512] = "INVALID";

	if (m_obFunctionScript.IsNull())
	{
		// our keystring is NULL, skip script install
		m_bScriptChecked = true;
	}
	else
	{
		// get the script filename and check its existance
		Util::GetFiosFilePath( *m_obFunctionScript, aScriptName );
		bFileExists = File::Exists( aScriptName );

		if (bFileExists)
		{
			// get its last modified date, flag wether we should check the file again...
			// but only if this is not the first time we're here

			CFileAttribute oTmpStat(aScriptName);

			if( oTmpStat.GetModifyTime() > m_scriptModDate )
			{
				m_scriptModDate = oTmpStat.GetModifyTime();

				if(m_bFirstTime)
					m_bFirstTime = false;
				else
					m_bScriptChecked = false;
			}
		}
	}

	if (!m_bScriptChecked)
	{
		if (bFileExists)
		{
			// script exists, load into lua via pcalls, find a function called
			// 'function_curve' and use this to init the graph
			//----------------------------------------------------------------

			lua_State* L = &(*CLuaGlobal::Get().State());
			
			int stackTop = lua_gettop(L);					// store stack top for restore
			int status = luaL_loadfile(L, aScriptName);		// load and install our script file

			if (status == 0)
				status = lua_pcall(L, 0, LUA_MULTRET, 0);	// read was okay, call main

			if (status == 0) // if install was okay, call 'function_curve'
			{
				lua_getglobal(L, "function_curve");
				if (lua_isfunction(L, -1))
				{
					m_pBufferCurve = m_pFittedCurve->Reset( true ); // clear our curve object
					ntError_p( m_pBufferCurve, ("We must have a curve at this point") );

					bool bSuccess = true;

					#ifdef _DEBUG
						int iNumSamples = 500;	// this is soooo sloooow
					#else
						int iNumSamples = 2000;
					#endif

					for ( int i = 0; i <= iNumSamples; i++ )
					{
						float fU = _R(i) / (iNumSamples+1);

						lua_getglobal(L, "function_curve");
						lua_pushnumber(L, fU);

						if (lua_pcall(L, 1, 1, 0) == 0)
						{
							lua_Number result = lua_tonumber(L, -1);
							m_pFittedCurve->AddSampleToFit( _R(result), fU );
						}
						else // failed in call, bail out of curve building
						{
							char ntError[512];
							sprintf( ntError, "ERROR calling 'function_curve' %s:\n", lua_tostring(L, -1) );
							EffectErrorMSG::AddDebugError( ntError );
	
							bSuccess = false;
							break;
						}
					}

					if (bSuccess)
					{
						FinaliseCurve();
					}
					else
					{
						m_pFittedCurve->Finalise( m_pBufferCurve );
						m_pBufferCurve = 0;
					}
				}
				else
				{
					char ntError[512];
					sprintf( ntError, "ERROR: Cannot find lua function 'function_curve'\n" );
					EffectErrorMSG::AddDebugError( ntError );
				}
			}
			else
			{
				char ntError[512];
				sprintf( ntError, "ERROR PARSING FILE %s: %s\n", aScriptName, lua_tostring(L, -1) );
				EffectErrorMSG::AddDebugError( ntError );
			}

			// restore stack
			lua_settop(L, stackTop);
		}
		else
		{
			char ntError[512];
			sprintf( ntError, "ERROR CANNOT FIND FILE %s\n", aScriptName );
			EffectErrorMSG::AddDebugError( ntError );
		}
	}

#endif
	m_bScriptChecked = true;
}

//--------------------------------------------------
//!
//!	FunctionCurve_User::InitialiseToArrayValues
//!
//--------------------------------------------------
void FunctionCurve_User::InitialiseToArrayValues( int iNumEntries, const float* pfValues, const float* pfTimes )
{
	ntError( iNumEntries > 1 );

	m_pBufferCurve = m_pFittedCurve->Reset(true);
	ntError_p( m_pBufferCurve, ("We must have a curve at this point") );

	for ( int i = 0; i < iNumEntries; i++ )
		m_pFittedCurve->AddSampleToFit( pfValues[i], pfTimes[i] );

	FinaliseCurve();
}

//--------------------------------------------------
//!
//!	function curve debug render
//!
//--------------------------------------------------

static const float fMouseXMin = 0.1f;
static const float fMouseXRange = 0.8f;

static const float fMouseYMin = 0.1f;
static const float fMouseYRange = 0.8f;

void FunctionCurve_User::DebugRender()
{
#ifndef _GOLD_MASTER
	CheckForScriptFunction();

	if ( m_pFittedCurve->IsValid() )
	{
		FunctionGraph graph( 0.3f, 0.7f, 0.3f, 0.7f );

		// find bounds of graph for debug drawing
		float fGraphMin = ntstd::Min( (m_pFittedCurve->GetFunctionMin()*m_fScale)+m_fOffset, 0.0f );
		float fGraphMax = ntstd::Max( (m_pFittedCurve->GetFunctionMax()*m_fScale)+m_fOffset, 0.0f );

		fGraphMax = (fGraphMax - fGraphMin) > 1.0f ? fGraphMax : fGraphMin + 1.0f;

		float fGraphRange = fGraphMax - fGraphMin;

		DebugRenderCurve( graph, fGraphMin, fGraphRange );
		DebugRenderGradients( graph, fGraphMin, fGraphRange );

		// check for graph editing input
		if	(
			( MouseInput::Exists() ) &&
			( CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_Q, KEYM_CTRL ) )
			)
		{
			// backup the old one incase curve generation failed
			m_pBufferCurve = m_pFittedCurve->Reset( true );
			ntError_p( m_pBufferCurve, ("We must have a curve at this point") );
			
			m_fLastTime = -1.0f;
		}
	}
	else if ( MouseInput::Exists() )
	{
		m_pFittedCurve->DebugRenderSamples(	fMouseXMin, fMouseXMin+fMouseXRange,
											fMouseYMin, fMouseYMin+fMouseYRange );

		float fX, fY;
		ConvertMouseToInputSpace( fX, fY );

		if	(
			( MouseInput::Get().GetButtonState(MOUSE_LEFT).GetHeld() ) &&
			( fX > m_fLastTime )
			)
		{
			m_fLastTime = fX;
			m_pFittedCurve->AddSampleToFit( fY, m_fLastTime );
		}

		if	(
			( MouseInput::Get().GetButtonState(MOUSE_LEFT).GetReleased() ) ||
			( !CInputHardware::Get().GetKeyboard().IsKeyHeld( KEYC_Q, KEYM_CTRL ) )
			)
		{
			FinaliseCurve();
		}
	}
#endif
}

//--------------------------------------------------
//!
//!	function curve debug render
//!
//--------------------------------------------------
void FunctionCurve_User::DebugRenderCurve( FunctionGraph& graph, float fMin, float fRange )
{
#ifndef _GOLD_MASTER
	// draw graph bounds
	graph.DrawBounds( NTCOLOUR_ARGB(0x80, 0xff, 0, 0) );

	// draw our scaled and offset origin
	float fOrigin = (0.0f-fMin)/fRange;
	graph.DrawLine( CPoint( 0.0f, fOrigin, 0.0f ), CPoint( 1.0f, fOrigin, 0.0f ), NTCOLOUR_ARGB(0xff, 0xff, 0, 0)  );

#ifdef _DEBUG
	static int iNumSamples = 200;
#else
	static int iNumSamples = 1000;
#endif
	
	// draw function
	for (int i = 0; i < iNumSamples; i++)
	{
		float fU = _R(i)	/ (iNumSamples+1);
		float fV = _R(i+1)	/ (iNumSamples+1);

		CPoint curr( fU, (EvaluateScaledAndOffset(fU) - fMin) / fRange, 0.0f );
		CPoint next( fV, (EvaluateScaledAndOffset(fV) - fMin) / fRange, 0.0f );

		graph.DrawLine( curr, next, NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0xff) );
	}

	// draw CV's and times
	char aLine[128];

	float fStart = GetCurveInterface()->GetTimeModule().GetStartTime();
	float fDuration = GetCurveInterface()->GetTimeModule().GetRange();

	for (u_int i = 1; i < GetCurveInterface()->GetNumSpan(); i++ )
	{
		float fTimeActual = GetCurveInterface()->GetTimeModule().GetTime(i);
		float fU = (fTimeActual - fStart) / fDuration;
		float fValue = EvaluateScaledAndOffset(fU);

		CPoint pos( fU, (fValue - fMin) / fRange, 0.0f );

		// draw time value at this point on the axis
		sprintf( aLine, "%.2f,%.2f", fTimeActual, fValue );
		graph.DrawText( aLine, pos, NTCOLOUR_ARGB(0x80, 0xff, 0xff, 0xff), FunctionGraph::BELOW );

		// draw a segment delimiter
		graph.DrawCross( pos, NTCOLOUR_ARGB(0x80, 0xff, 0xff, 0), 5.0f );
	}

	// draw value scale
	sprintf( aLine, "%.2f", GetCurveInterface()->GetTimeModule().GetStartTime() );
	graph.DrawText( aLine, CPoint( 0.0f, 0.0f, 0.0f ), NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0xff), FunctionGraph::BELOW );

	sprintf( aLine, "%.2f", GetCurveInterface()->GetTimeModule().GetEndTime() );
	graph.DrawText( aLine, CPoint( 1.0f, 0.0f, 0.0f ), NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0xff), FunctionGraph::BELOW );

	sprintf( aLine, "MIN: %.2f", fMin );
	graph.DrawText( aLine, CPoint( 0.0f, 0.0f, 0.0f ), NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0xff), FunctionGraph::LEFT );

	sprintf( aLine, "MAX: %.2f", fMin + fRange );
	graph.DrawText( aLine, CPoint( 0.0f, 1.0f, 0.0f ), NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0xff), FunctionGraph::LEFT );
#endif
}

//--------------------------------------------------
//!
//! gradient debug render
//!
//--------------------------------------------------
void FunctionCurve_User::DebugRenderGradients( FunctionGraph& graph, float fMin, float fRange )
{
#ifndef _GOLD_MASTER
	// draw in our graph gradients if we're currently selected
	DataObject* pDO = CNetEditInterface::Get().GetSelected();
	FCurveNode_Hermite* pEdited = 0;
	if( pDO )
		pEdited = (FCurveNode_Hermite*) pDO->GetBasePtr();
	
	ntstd::List< FCurveNode_Hermite* >::iterator it = m_obCurveNodes.begin();

	for ( int i = 0; i < m_iNumSegments; i++, ++it )
	{
		if (pEdited == (*it))
		{
			float fStartTime = GetCurveInterface()->GetTimeModule().GetStartTime();
			float fDuration = GetCurveInterface()->GetTimeModule().GetRange();
			
			{
				float fOutVal = ((*it)->m_fOut * m_fScale) / fRange;
				float fU = ((*it)->m_fTime - fStartTime) / fDuration;
				float fValue = EvaluateScaledAndOffset(fU);

				CPoint bot( fU, (fValue - fMin) / fRange, 0.0f );
				CPoint top( bot.X(),  bot.Y() + fOutVal, 0.0f );

				graph.DrawLine( bot, top, NTCOLOUR_ARGB(0xff, 0, 0xff, 0) );
			}

			{
				float fInVal = ((*it)->m_fIn * m_fScale) / fRange;
				++it;
				float fU = ((*it)->m_fTime - fStartTime) / fDuration;
				float fValue = EvaluateScaledAndOffset(fU);

				CPoint bot( fU, (fValue - fMin) / fRange, 0.0f );
				CPoint top( bot.X(),  bot.Y() + fInVal, 0.0f );

				graph.DrawLine( bot, top, NTCOLOUR_ARGB(0xff, 0, 0, 0xff) );
			}

			break;
		}
	}
#endif
}

//--------------------------------------------------
//!
//!	convert mouse input into valid area
//!
//--------------------------------------------------
void FunctionCurve_User::ConvertMouseToInputSpace( float& fX, float& fY )
{
	fX = (MouseInput::Get().GetMousePos().X() - fMouseXMin) / fMouseXRange;
	fY = (MouseInput::Get().GetMousePos().Y() - fMouseYMin) / fMouseYRange;

	fX = ntstd::Clamp( fX, 0.0f, 1.0f );
	fY = ntstd::Clamp( fY, 0.0f, 1.0f );

	fY = 1.0f - fY; // invert Y direction
}

//--------------------------------------------------
//!
//!	finish the curve
//!
//--------------------------------------------------
void FunctionCurve_User::FinaliseCurve()
{
	ntError_p( !m_pFittedCurve->IsValid(), ("cant finalise a valid curve matey") );
	ntError_p( m_pBufferCurve, ("we should have a backup here if this fails") );

	if (!m_pFittedCurve->Finalise())
	{
		// curve generation failed, use the old one..
		m_pFittedCurve->Finalise( m_pBufferCurve );
		m_pBufferCurve = 0;
	}
	else
	{
		// curve generation successful,
		// we generate our NT_NEW FCurveNode_Hermite nodes now

		ResetXMLData();
	}

	if (m_pBufferCurve)
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pBufferCurve );
	}

	m_pBufferCurve = 0;
}

//--------------------------------------------------
//!
//!	force the curve to be something else
//!
//--------------------------------------------------
void FunctionCurve_User::ForceCurve( CCubicTimeHermite* pForcedCurve )
{
	ntError_p( pForcedCurve, ("this curve is no good") );

	if (m_pFittedCurve->IsValid())
		m_pFittedCurve->Reset();

	m_pFittedCurve->Finalise( pForcedCurve );
	ResetXMLData();

	if (m_pBufferCurve)
		NT_DELETE( m_pBufferCurve );
	m_pBufferCurve = 0;
}

//--------------------------------------------------
//!
//!	finish the curve
//!
//--------------------------------------------------
void FunctionCurve_User::ResetXMLData()
{
	while ( !m_obCurveNodes.empty() )
	{
		ObjectDatabase::Get().Destroy( m_obCurveNodes.back() );
		m_obCurveNodes.pop_back();
	}

	m_iNumSegments = m_pFittedCurve->GetCurve().GetNumSpan();

	for (int i = 0; i <= m_iNumSegments; i++)
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
		ObjectContainer* pParentContainer = pDO->GetParent();
		FCurveNode_Hermite* pNew = ObjectDatabase::Get().Construct<FCurveNode_Hermite>( "FCurveNode_Hermite", DataObject::GetUniqueName(), pParentContainer );

		pNew->m_fTime = m_pFittedCurve->GetCurve().GetTimeModule().GetTime(i);
		pNew->m_fStart = m_pFittedCurve->GetCurve().GetCV(i).X();

		if (i < m_iNumSegments)
		{
			pNew->m_fOut = m_pFittedCurve->GetCurve().GetStartTan(i).X() * 3.0f;
			pNew->m_fIn = m_pFittedCurve->GetCurve().GetEndTan(i).X() * 3.0f;
		}

		m_obCurveNodes.push_back( pNew );
	}
}

