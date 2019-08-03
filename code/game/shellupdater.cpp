//------------------------------------------------------------------------------------------
//!
//!	\file shellupdater.cpp
//! main update and render loops for the game
//!
//------------------------------------------------------------------------------------------

#include "game/shellmain.h"
#include "game/shellupdater.h"
#include "game/shellconfig.h"
#include "game/shelldebug.h"
#include "game/shellglobal.h"
#include "game/shellgame.h"
#include "game/shelllevel.h"
#include "game/shellrenderables.h"

#include "effect/effect_manager.h"

#include "gfx/renderer.h"
#include "gfx/pictureinpicture.h"

#include "core/timer.h"
#include "core/gatso.h"
#include "core/visualdebugger.h"
#include "input/inputhardware.h"

//------------------------------------------------------------------------------------------
//!
//!	SimpleShellUpdater::ctor
//!
//------------------------------------------------------------------------------------------
SimpleShellUpdater::SimpleShellUpdater( const char* pBackground ) :
	m_Lifetime(0.0f)
{
	// our background image
	ShellImageDef def;
	def.m_pTexName = pBackground;

	m_pBackground = NT_NEW ShellImage(def);
	m_pBackground->FadeUp();

	// our animated icon
	ShellIconDef iconDef;
	m_pAnimatedIcon = NT_NEW ShellAnimIcon(iconDef);

	char pTextureName[MAX_PATH];
	if ( g_ShellOptions->m_bUseCounter )
	{
		for (int i = 0; i < 10; i++)
		{
			sprintf( pTextureName, "hud/0%d_colour_alpha_nomip.dds", i );
			m_pAnimatedIcon->AddImage( pTextureName );
		}
	}
	else
	{
		for (int i = 0; i < 9; i++)
		{
			sprintf( pTextureName, "hud/savingicon/savingicon0%d_colour_alpha_nomip.dds", i );
			m_pAnimatedIcon->AddImage( pTextureName );
		}
	}

	// switch to vsync mode only
	Renderer::Get().RequestPresentMode( Renderer::PM_VBLANK );
}

//------------------------------------------------------------------------------------------
//!
//!	SimpleShellUpdater::dtor
//!
//------------------------------------------------------------------------------------------
SimpleShellUpdater::~SimpleShellUpdater()
{
	// finish fade up
	while ( m_pBackground->GetStatus() != ShellImage::VISIBLE )
	{
		Update();
		Render();
	}

	// now fade down
	m_pBackground->FadeDown();
	while ( m_pBackground->GetStatus() != ShellImage::INVISIBLE )
	{
		Update();
		Render();
	}
	
	// set back to regular sync mode
	Renderer::Get().RequestPresentMode( Renderer::PM_AUTO );

#ifdef PLATFORM_PS3
	// sync here so rendering is finished before our sprite memory goes out of context
	GcKernel::SyncPreviousFrame();
#endif

	// release our resources
	NT_DELETE( m_pBackground );
	NT_DELETE( m_pAnimatedIcon );
}

//------------------------------------------------------------------------------------------
//!
//!	SimpleShellUpdater::Update
//!
//------------------------------------------------------------------------------------------
void SimpleShellUpdater::Update()
{
	// primitive update, we just tick the timer class here, and remember to check for exit
	// requests from the OSD. (TBD)

	CTimer::Get().Update();
	CInputHardware::Get().Update( _R(CTimer::Get().GetSystemTimeChange()) );
}

//------------------------------------------------------------------------------------------
//!
//!	SimpleShellUpdater::Render
//!
//------------------------------------------------------------------------------------------
void SimpleShellUpdater::Render()
{
	Renderer::Get().m_pPIPManager->RenderBasic();
	
	// do our fade / icon animation
	float fTimeChange = _R(CTimer::Get().GetSystemTimeChange());
	m_pBackground->Update( fTimeChange );
	m_Lifetime += fTimeChange;

	// display backdrop
	//-------------------------------------------------------

	// render background image
	m_pBackground->Render();

	// display simple timer in place of loading icon / bar
	//-------------------------------------------------------
	if ( g_ShellOptions->m_bUseCounter )
	{
		float fTime = m_Lifetime;
		float fSize = 0.0625f;

		float fX = 1.0f - fSize;
		float fY = fSize;

		CVector col = CVector(1.0f,0.0f,0.0f,1.0f);
		NumberAt( fTime - floor(fTime), fX, fY, fSize, col );

		// second counter
		if (fTime >= 1.0f)
		{
			uint32_t numChars = (uint32_t)(log10f( fTime )) + 1;
			for (uint32_t i = 0; i < numChars; i++)
			{
				fX -= fSize * 0.6f;
				fTime *= 0.1f;

				col = CVector(1.0f,1.0f,1.0f,1.0f);
				NumberAt( fTime - floor(fTime), fX, fY, fSize, col );
			}
		}

		Renderer::Get().SetBlendMode( GFX_BLENDMODE_OVERWRITE );
	}
	// display simple loading icon
	//-------------------------------------------------------
	else
	{
		m_pAnimatedIcon->SetAlpha( m_pBackground->GetAlpha() );

		m_pAnimatedIcon->GetDefinition().m_x = 0.12f;
		m_pAnimatedIcon->GetDefinition().m_y = 0.850f;
		m_pAnimatedIcon->GetDefinition().m_width = 0.05f;
		m_pAnimatedIcon->GetDefinition().m_height = 0.088888f;
		m_pAnimatedIcon->GetDefinition().m_loopTime = 0.5f;

		m_pAnimatedIcon->RenderTime(m_Lifetime);
	}

	Renderer::Get().m_pPIPManager->PresentBasic();
}

//------------------------------------------------------------------------------------------
//!
//!	SimpleShellUpdater::NumberAt
//!
//------------------------------------------------------------------------------------------
void SimpleShellUpdater::NumberAt( float frac, float fX, float fY, float fSize, CVector& col )
{
	m_pAnimatedIcon->SetAlpha( 1.0f );

	m_pAnimatedIcon->GetDefinition().m_x = fX;
	m_pAnimatedIcon->GetDefinition().m_y = fY;
	m_pAnimatedIcon->GetDefinition().m_width = fSize;
	m_pAnimatedIcon->GetDefinition().m_height = fSize * (16.f / 9.f);
	m_pAnimatedIcon->GetDefinition().m_rgba = col;

	m_pAnimatedIcon->RenderNormalised(frac,false);
}








//------------------------------------------------------------------------------------------
//!
//!	CreateLevelUpdater::ctor
//!
//------------------------------------------------------------------------------------------
CreateLevelUpdater::CreateLevelUpdater() :
	m_pBackup(0),
	m_loadTime(0.0f)
{
	m_pBackup = NT_NEW SimpleShellUpdater( "gui/tgs_menu/start_page_colour_npow2_nomip.dds" );
}

//------------------------------------------------------------------------------------------
//!
//!	CreateLevelUpdater::dtor
//!
//------------------------------------------------------------------------------------------
CreateLevelUpdater::~CreateLevelUpdater()
{
	NT_DELETE( m_pBackup );
}

//------------------------------------------------------------------------------------------
//!
//!	CreateLevelUpdater::Update
//!
//------------------------------------------------------------------------------------------
void CreateLevelUpdater::Update()
{
	ShellMain::Get().m_pDebugShell->UpdateGlobal();

	// we dont update exec, as that screws with our async install function
	// execution
	ShellGlobal::Update( false );

	m_loadTime += CTimer::Get().GetSystemTimeChange();

	CGatso::Start( "ShellMain::Update" );
	ShellGame::Update( false );
	CGatso::Stop( "ShellMain::Update" );
}

//------------------------------------------------------------------------------------------
//!
//!	CreateLevelUpdater::Render
//!
//------------------------------------------------------------------------------------------
#ifndef _RELEASE
extern void GetLastAccessedFile( char* pResult );
#endif

void CreateLevelUpdater::Render()
{
	// must be here so is updated post entity / hierarchy update
	CGatso::Start( "CreateLevelUpdater::EffectManager.Update()" );
	EffectManager::Get().UpdateManager();
	CGatso::Stop( "CreateLevelUpdater::EffectManager.Update()" );

	ShellMain::Get().m_pDebugShell->Render();

	CGatso::Start( "ShellMain::Render" );

#ifndef _RELEASE
	g_VisualDebug->Printf2D( 20.0f, 100.0f, 
		NTCOLOUR_ARGB(0xff,0xff,0xff,0xff), 0, "Loading Level %s for %.1f seconds.", 
		ShellMain::Get().GetCurrLoadingLevel()->GetLevelName(), m_loadTime );

	char lastFile[MAX_PATH];
	GetLastAccessedFile(lastFile);
	g_VisualDebug->Printf2D( 20.0f, 112.0f, 
		NTCOLOUR_ARGB(0xff,0xff,0xff,0xff), 0, "Last file accessed: %s.", 
		lastFile );
#endif

	m_pBackup->Render();

	CGatso::Stop( "ShellMain::Render" );
}


