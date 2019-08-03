#include "audio/audioconsole.h"
#include "audio/audiosystem.h"
#include "audio/audiomixer.h"
#include "audio/ambientsound.h"
#include "audio/audioreverb.h"
#include "audio/musiccontrol.h"
#include "game/chatterboxman.h"

#include "core/visualdebugger.h"
#include "input/inputhardware.h"

#include "fmod_event.h"
#include "fmod_errors.h"

#include "core/gatso.h"




#ifndef _GOLD_MASTER

#ifdef _AUDIO_SYSTEM_ENABLE
#define _ENABLE_AUDIO_CONSOLE
#endif // _AUDIO_SYSTEM_ENABLE

#endif // _GOLD_MASTER









AudioConsole::AudioConsole () :
	m_bEnabled(false),
#ifdef PLATFORM_PS3
	m_fX(600.0f),
	m_fY(10.0f),
#else
	m_fX(10.0f),
	m_fY(10.0f),
#endif
	m_uiDebugFlags(0),
	m_iMenu(0),
	m_iMenuItem(0),
	m_iMenuItemViewable(0),
	m_fEventSystemTime(0.0f),
	m_fEventStartTime(0.0f),
	m_fEventSystemTimeHigh(0.0f),
	m_fEventStartTimeHigh(0.0f)

{
}

void AudioConsole::Update ()
{
#ifdef _ENABLE_AUDIO_CONSOLE

	CInputKeyboard* pobKeyboard=CInputHardware::Get().GetKeyboardP();

	if (!pobKeyboard) // Make sure the keyboard is available
		return;

	if (pobKeyboard->IsKeyPressed(KEYC_A,KEYM_CTRL | KEYM_ALT))
	{
		m_bEnabled=!m_bEnabled;
		//ntPrintf("AudioConsole: Enabled=%d\n",m_bEnabled);
	}

	// Gatso timings
	m_fEventSystemTime=CGatso::Retrieve("FMOD:EventSystem:Update");
	m_fEventStartTime=CGatso::Retrieve("FMOD:Event:start");

	if (m_fEventSystemTime>m_fEventSystemTimeHigh)
		m_fEventSystemTimeHigh=m_fEventSystemTime;

	if (m_fEventStartTime>m_fEventStartTimeHigh)
		m_fEventStartTimeHigh=m_fEventStartTime;

	if (!m_bEnabled)
		return;

	// Menu navigation
	if (pobKeyboard->IsKeyPressed(KEYC_LEFT_ARROW))
		PreviousMenu();
	if (pobKeyboard->IsKeyPressed(KEYC_RIGHT_ARROW))
		NextMenu();
	if (pobKeyboard->IsKeyPressed(KEYC_UP_ARROW))
		PreviousMenuItem();
	if (pobKeyboard->IsKeyPressed(KEYC_DOWN_ARROW))
		NextMenuItem();
	if (pobKeyboard->IsKeyHeld(KEYC_PAGE_UP))
		PreviousMenuItem();
	if (pobKeyboard->IsKeyHeld(KEYC_PAGE_DOWN))
		NextMenuItem();

	// Custom menu controls
	switch(m_iMenu)
	{
		case AUDIO_MENU_EVENTS:
		{
			break;
		}

		case AUDIO_MENU_ASSETS:
		{
			if (pobKeyboard->IsKeyPressed(KEYC_SPACE))
			{
				m_fEventSystemTimeHigh=0.0f;
				m_fEventStartTimeHigh=0.0f;
			}

			break;
		}

		case AUDIO_MENU_CATEGORIES:
		{
			break;
		}

		case AUDIO_MENU_MIXERPROFILES:
		{
			if (pobKeyboard->IsKeyPressed(KEYC_SPACE))
			{
				int i=0;

				for(ntstd::List<AudioMixerProfile*>::iterator obIt=AudioMixer::Get().m_obProfileList.begin(); obIt!=AudioMixer::Get().m_obProfileList.end(); ++obIt)
				{
					if (i==m_iMenuItem)
					{
						AudioMixer::Get().SetProfile((*obIt)->GetID(),1.0f);
						break;
					}

					++i;
				}
			}

			break;
		}

		case AUDIO_MENU_REVERBZONES:
		{
			if (pobKeyboard->IsKeyPressed(KEYC_SPACE))
			{
				int i=0;

				for(ntstd::List<AudioReverbZone*>::iterator obIt=AudioReverbManager::Get().m_obReverbZoneList.begin(); obIt!=AudioReverbManager::Get().m_obReverbZoneList.end(); ++obIt)
				{
					if (i==m_iMenuItem)
					{
						AudioReverbZone* pobReverbZone=*obIt;

						int iBitFlag=(pobReverbZone->m_bEnabled ? 1 : 0) + (pobReverbZone->IsDebugEnabled() ? 2 : 0) + 1;

						if (iBitFlag>3)
							iBitFlag=0;

						pobReverbZone->SetEnabled(iBitFlag & 1);
						pobReverbZone->SetDebug(iBitFlag & 2);

						break;
					}

					++i;
				}

			}

			break;
		}

		case AUDIO_MENU_AMBIENCE:
		{
			if (pobKeyboard->IsKeyPressed(KEYC_SPACE))
			{
				int i=0;

				for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obIt=AmbientSoundManager::Get().m_obAmbientSoundDefList.begin(); obIt!=AmbientSoundManager::Get().m_obAmbientSoundDefList.end(); ++obIt)
				{
					if (i==m_iMenuItem)
					{
						BaseAmbientSoundDefinition* pobDef=*obIt;

						int iBitFlag=(pobDef->IsActive() ? 1 : 0) + (pobDef->IsDebugRenderEnabled() ? 2 : 0) + 1;

						if (iBitFlag>3)
							iBitFlag=0;

						pobDef->SetActive(iBitFlag & 1);
						pobDef->SetDebug(iBitFlag & 2);
						break;
					}

					++i;
				}
			}

			break;
		}

		case AUDIO_MENU_OPTIONS:
		{
			if (pobKeyboard->IsKeyPressed(KEYC_SPACE))
			{
				switch(m_iMenuItem)
				{
					case 0: AudioSystem::Get().ToggleDebugOption(DEBUG_RENDER_SOUND_POS); break;
					case 1: AudioSystem::Get().ToggleDebugOption(DEBUG_TTY_SYSTEM); break;
					case 2: AudioSystem::Get().ToggleDebugOption(DEBUG_TTY_RESOURCE); break;
					case 3: AudioSystem::Get().ToggleDebugOption(DEBUG_TTY_EVENT_PLAY); break;
					case 4: AudioSystem::Get().ToggleDebugOption(DEBUG_TTY_EVENT_STOP); break;
					case 5: AudioSystem::Get().ToggleDebugOption(DEBUG_TTY_EVENT_CALLBACK); break;
					case 6: AudioSystem::Get().ToggleDebugOption(DEBUG_TTY_EVENT_ERROR); break;
					case 7: AudioSystem::Get().ToggleDebugOption(DEBUG_TTY_TRIGGER_STATS); break;
					case 8: AudioSystem::Get().ToggleDebugOption(DEBUG_TTY_HIDE_PAUSED_ENT); break;
					case 9: MusicControlManager::Get().ToggleDebugging(); break;
					default: break;
				}
			}

			break;
		}

		default:
			break;
	}

#endif // _ENABLE_AUDIO_CONSOLE
}

void AudioConsole::RenderUpdate ()
{
#ifdef _ENABLE_AUDIO_CONSOLE

	if (!m_bEnabled || !AudioSystem::Get().m_bInitialised)
		return;

	const float fLINE_SPACING = 12.0f;
	const int iMAX_VIEWABLE_ITEMS = 32;

	float fX=m_fX;
	float fY=m_fY;

	int iMemAlloc,iMaxAlloc,iTotalAlloc;

	AudioSystem::Get().GetMemoryUsage(iMemAlloc,iMaxAlloc,iTotalAlloc);

	g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "_____________________________Audio Console________________________________\0"); fY+=16.0f;
	g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "FMOD Ex:v%x  EventSystem:v%x  Speakers:%d  Channels:%d/%d",
		AudioSystem::Get().GetFMODVersion(),
		AudioSystem::Get().GetFMODEventVersion(),
		AudioSystem::Get().GetSpeakerCount(),
		AudioSystem::Get().GetPlayingChannels(),
		AudioSystem::Get().GetMaxChannels());
	fY+=fLINE_SPACING;

	g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "Mem:%dkb  High:%dkb  Limit:%dkb\0",iMemAlloc/1024,iMaxAlloc/1024,iTotalAlloc/1024); fY+=fLINE_SPACING;

	switch(m_iMenu)
	{
		case AUDIO_MENU_EVENTS:
		{
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "<Events> Assets  Categories  Mixer  Reverb  Ambience  Options  ChatterBox\0"); fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________\0"); fY+=16.0f;

			int iCount=0;
	
			const int iMAX_DISPLAYABLE_EVENTS=32;

			for(int i=0; i<MAX_EVENT_INSTANCES; ++i)
			{
				if (!AudioSystem::Get().m_obInstanceList[i].IsFree())
				{
					++iCount;

					if (iCount<iMAX_DISPLAYABLE_EVENTS)
					{
						char acTemp [128];
						sprintf(acTemp,"%s:%s",
							AudioSystem::Get().m_obInstanceList[i].GetEventGroupName(),
							AudioSystem::Get().m_obInstanceList[i].GetEventName());

						g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "%0.2d ID:%.5d  %-40s  V:%.2f  P:%+.2f  %c%c%c%c%c  %s\0",
							iCount,
							AudioSystem::Get().m_obInstanceList[i].GetID(),
							acTemp,
							AudioSystem::Get().m_obInstanceList[i].GetVolume(),
							AudioSystem::Get().m_obInstanceList[i].GetPitch(),
							((AudioSystem::Get().m_obInstanceList[i].GetState() & EVENT_STATE_READY) ? 'R' : '-'),
							((AudioSystem::Get().m_obInstanceList[i].GetState() & EVENT_STATE_LOADING) ? 'L' : '-'),
							((AudioSystem::Get().m_obInstanceList[i].GetState() & EVENT_STATE_ERROR) ? 'E' : '-'),
							((AudioSystem::Get().m_obInstanceList[i].GetState() & EVENT_STATE_PLAYING) ? 'P' : '-'),
							((AudioSystem::Get().m_obInstanceList[i].GetState() & EVENT_STATE_CHANNELSACTIVE) ? 'C' : '-'),
							(AudioSystem::Get().m_obInstanceList[i].IsPaused() ? "[P]" : "")
							);

						fY+=fLINE_SPACING;
					}
				}
			}

			if (iCount>=iMAX_DISPLAYABLE_EVENTS)
			{
				g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "more...%d instances active\0",iCount);
			}

			break;
		}

		case AUDIO_MENU_ASSETS:
		{
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, " Events <Assets> Categories  Mixer  Reverb  Ambience  Options  ChatterBox \0"); fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________\0"); fY+=16.0f;
 
			FMOD::System* pobSystem=0;
			FMOD::EVENT_SYSTEMINFO stInfo;

			int iNumProjects=0;
			int iNumStreams=0;

			AudioSystem::Get().m_pobEventSystem->getSystemObject(&pobSystem); // Get FMOD system object
			FMOD_RESULT eResult=AudioSystem::Get().m_pobEventSystem->getInfo(&stInfo); // Get eventsystem information

			if (m_iMenuItem<0)
				m_iMenuItem=0;

			if (eResult==FMOD_OK)
			{
				// Count the number of streams in use
				for(int i=0; i<stInfo.numwavebanks; ++i)
				{
					iNumStreams+=stInfo.wavebankinfo[i].streamsinuse;
				}
			}
			else // Somethings gone wrong
			{
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "<EventSystem::getInfo failed - %s>",FMOD_ErrorString(eResult));
				fY+=fLINE_SPACING;
			}

			// Count the number of projects we have
			for(int i=0; i<MAX_FMOD_PROJECTS; ++i)
			{
				if (AudioSystem::Get().m_obProject[i].m_pobEventProject)
					++iNumProjects;
			}

			
			// Display CPU info
			if (pobSystem)
			{
				float fCPU_Dsp,fCPU_Stream,fCPU_Update,fCPU_Total;
				if (pobSystem->getCPUUsage(&fCPU_Dsp,&fCPU_Stream,&fCPU_Update,&fCPU_Total)==FMOD_OK)
				{
					g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "CPU: Dsp=%.3f%%  Stream=%.3f%%  Update=%.3f%%  Total=%.3f%%\n",
						fCPU_Dsp,fCPU_Stream,fCPU_Update,fCPU_Total);
					fY+=fLINE_SPACING;
				}
			}

			// Display gatso timings
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "GATSO: EventSys=%.3f%% <%.3f%%>  EventStart=%.3f%% <%.3f%%>",
				m_fEventSystemTime,m_fEventSystemTimeHigh,m_fEventStartTime,m_fEventStartTimeHigh);
			fY+=fLINE_SPACING;

			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "GATSO: Mixer=%.3f%%  Reverb=%.3f%%  AmbSnd=%.3f%%",
				CGatso::Retrieve("AudioMixer:DoWork"),CGatso::Retrieve("AudioReverbManager::DoWork"),CGatso::Retrieve("AmbientSoundManager:Update"));
			fY+=fLINE_SPACING;


			// Display memory and event information
			if (eResult==FMOD_OK)
			{
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "MEMORY: Events=%dkb  Instances=%dkb  DSP=%dkb",
					stInfo.eventmemory/1024,stInfo.instancememory/1024,stInfo.dspmemory/1024);

				fY+=fLINE_SPACING;

				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "INFO: Streams=%d  Events=%d  Instances=%d  Projects=%d  Wavebanks=%d",
					iNumStreams,stInfo.numevents,stInfo.numinstances,iNumProjects,stInfo.numwavebanks);
			}

			fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________");
			fY+=4.0f;

			// Display list of loaded projects

			const int iMAX_VIEWABLE_PROJECTS = 10;

			if (iNumProjects==0)
			{
				fY+=fLINE_SPACING;
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "<No projects loaded>");
			}
			else
			{
				if (iNumProjects>iMAX_VIEWABLE_PROJECTS)
				{
					for(int i=0; i<iMAX_VIEWABLE_PROJECTS; ++i)
					{
						int iIndex=(m_iMenuItem + i) % iNumProjects;

						int iProjectIndex=0;

						for(int j=0; j<MAX_FMOD_PROJECTS; ++j)
						{
							if (AudioSystem::Get().m_obProject[iProjectIndex].m_pobEventProject)
							{
								if (iIndex==iProjectIndex)
								{
									fY+=fLINE_SPACING;
									g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "%.2d %-44s  Groups:%-3d  Events:%-3d",
										iProjectIndex+1,AudioSystem::Get().m_obProject[j].GetName(),AudioSystem::Get().m_obProject[j].GetNumGroups(),AudioSystem::Get().m_obProject[j].GetNumEvents());
									break;
								}

								iProjectIndex++;
							}
						}
					}
				}
				else
				{
					for(int i=0; i<iNumProjects; ++i)
					{
						fY+=fLINE_SPACING;
						g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "%.2d %-44s  Groups:%-3d  Events:%-3d",
							i+1,AudioSystem::Get().m_obProject[i].GetName(),AudioSystem::Get().m_obProject[i].GetNumGroups(),AudioSystem::Get().m_obProject[i].GetNumEvents());
					}
				}
			}


			// Display information on wavebanks
			if (eResult==FMOD_OK)
			{
				fY+=4.0f; g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________"); fY+=fLINE_SPACING + 4.0f;	

				const int iMAX_VIEWABLE_WAVEBANKS = 10;

				if (stInfo.numwavebanks==0)
				{
					g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "<No wavebanks loaded>");
					fY+=fLINE_SPACING;
				}
				else
				{
					if (stInfo.numwavebanks>iMAX_VIEWABLE_WAVEBANKS)
					{
						for(int i=0; i<iMAX_VIEWABLE_WAVEBANKS; ++i)
						{
							int index=(m_iMenuItem + i) % stInfo.numwavebanks;

							g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "%.2d %-44s  Streams:%2d/%-2d  Mem:%dkb",
								index+1,
								(stInfo.wavebankinfo[index].name ? stInfo.wavebankinfo[index].name : "Unknown"),
								stInfo.wavebankinfo[index].streamsinuse,
								stInfo.wavebankinfo[index].maxstreams,
								(stInfo.wavebankinfo[index].streammemory + stInfo.wavebankinfo[index].samplememory) / 1024);

							fY+=fLINE_SPACING;
						}
					}
					else
					{
						for(int i=0; i<stInfo.numwavebanks; ++i)
						{
							g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "%.2d %-44s  Streams:%2d/%-2d  Mem:%dkb",
								i+1,
								(stInfo.wavebankinfo[i].name ? stInfo.wavebankinfo[i].name : "Unknown"),
								stInfo.wavebankinfo[i].streamsinuse,
								stInfo.wavebankinfo[i].maxstreams,
								(stInfo.wavebankinfo[i].streammemory + stInfo.wavebankinfo[i].samplememory) / 1024);

							fY+=fLINE_SPACING;
						}
					}
				}
			}

			break;
		}

		case AUDIO_MENU_CATEGORIES:
		{
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, " Events  Assets <Categories> Mixer  Reverb  Ambience  Options  ChatterBox \0"); fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________\0"); fY+=16.0f;

			if (AudioMixer::Get().m_obCategoryList.size()==0)
			{
				g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "<No categories found>\0");
			}
			else
			{
				g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "Name                                  Volume  Pitch  Mute  Pause\0"); fY+=fLINE_SPACING;

				const int iTotalCategories=(int)AudioMixer::Get().m_obCategoryList.size();

				if (m_iMenuItem<0)
					m_iMenuItem=0;
					//m_iMenuItem=iTotalCategories - iMAX_VIEWABLE_ITEMS;


				if (m_iMenuItem>(iTotalCategories - iMAX_VIEWABLE_ITEMS))
					m_iMenuItem=iTotalCategories - iMAX_VIEWABLE_ITEMS;
					//m_iMenuItem=0;

				int iIndex=0;
				int iDisplayed=0;

				for(ntstd::List<AudioCategory*>::iterator obIt=AudioMixer::Get().m_obCategoryList.begin(); obIt!=AudioMixer::Get().m_obCategoryList.end(); ++obIt)
				{
					if (iIndex>=m_iMenuItem && iDisplayed<iMAX_VIEWABLE_ITEMS)
					{
						iDisplayed++;
							
						char acString [32];

						int i=0;

						while(i<(*obIt)->GetDepth())
						{	
							acString[i]='-';
							++i;
						}

						acString[i]='\0';

						strcat(acString,(*obIt)->GetName());

						g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "%-36s  %.2f    %+.2f   %c     %c\0",
							acString,(*obIt)->GetVolume(),(*obIt)->GetPitch(),((*obIt)->IsMuted() ? '*' : ' '),((*obIt)->IsPaused() ? '*' : ' '));
						
						fY+=fLINE_SPACING;
					}

					iIndex++;
				}

				if (iTotalCategories > iMAX_VIEWABLE_ITEMS)
				{
					g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "Press UP/DOWN ARROW to scroll\0");
					fY+=fLINE_SPACING;
				}
			}

			break;
		}

		case AUDIO_MENU_MIXERPROFILES:
		{
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, " Events  Assets  Categories <Mixer> Reverb  Ambience  Options  ChatterBox\0"); fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________\0"); fY+=16.0f;

			const int iMAX_VIEWABLE_MIXERPROFILES = 20;
			const int iTotalProfiles=AudioMixer::Get().m_obProfileList.size();

			if (iTotalProfiles==0)
			{
				g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "<No mixer profiles loaded>\0");
			}
			else
			{
				g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "[Active Profile: %s  Total Profiles: %d]",AudioMixer::Get().GetActiveProfile().GetDebugString(),iTotalProfiles);

				fY+=fLINE_SPACING;

				/*
				for(ntstd::List<AudioMixerProfile*>::iterator obIt=AudioMixer::Get().m_obProfileList.begin(); obIt!=AudioMixer::Get().m_obProfileList.end(); ++obIt)
				{
					g_VisualDebug->Printf2D(fX,fY,0xffffffff, DTF_ALIGN_LEFT, "%s\0",(*obIt)->GetName()); fY+=fLINE_SPACING;
				}
				*/

				if (m_iMenuItem<0)
					m_iMenuItem=iTotalProfiles - 1;

				if (m_iMenuItem>=iTotalProfiles)
					m_iMenuItem=0;

				if (m_iMenuItem<m_iMenuItemViewable)
				{
					m_iMenuItemViewable=m_iMenuItem;
				}
				else if (m_iMenuItem>=m_iMenuItemViewable+iMAX_VIEWABLE_MIXERPROFILES)
				{
					m_iMenuItemViewable=m_iMenuItem-iMAX_VIEWABLE_MIXERPROFILES+1;
				}

				int iIndex=0;

				for(ntstd::List<AudioMixerProfile*>::iterator obIt=AudioMixer::Get().m_obProfileList.begin(); obIt!=AudioMixer::Get().m_obProfileList.end(); ++obIt)
				{
					++iIndex;

					if (iIndex>=m_iMenuItemViewable+1 && iIndex<m_iMenuItemViewable+iMAX_VIEWABLE_MIXERPROFILES+1)
					{

						g_VisualDebug->Printf2D(fX,fY,0xffffffff,DTF_ALIGN_LEFT,"%c %0.2d %s %s",
							(iIndex==(m_iMenuItem+1) ? '>' : ' '),
							iIndex,
							(*obIt)->GetName(),
							(AudioMixer::Get().GetActiveProfile()==(*obIt)->GetName() ? "[ACTIVE]" : "") );

						fY+=fLINE_SPACING;
					}
				}
			}

			break;
		}

		case AUDIO_MENU_REVERBZONES:
		{
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, " Events  Assets  Categories  Mixer <Reverb> Ambience  Options  ChatterBox \0"); fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________\0"); fY+=16.0f;

			const int iMAX_VIEWABLE_REVERB = 20;
			const int iTotalZones=AudioReverbManager::Get().m_obReverbZoneList.size();

			if (!AudioReverbManager::Get().IsEnabled())
			{
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "<Reverb is disabled>\0"); fY+=fLINE_SPACING;
			}

			if (iTotalZones==0)
			{
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "<No reverb zones loaded>\0"); fY+=fLINE_SPACING;
			}
			else
			{
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "[Total reverb zones: %d]", iTotalZones); fY+=fLINE_SPACING;

				if (m_iMenuItem<0)
					m_iMenuItem=iTotalZones - 1;

				if (m_iMenuItem>=iTotalZones)
					m_iMenuItem=0;

				if (m_iMenuItem<m_iMenuItemViewable)
				{
					m_iMenuItemViewable=m_iMenuItem;
				}
				else if (m_iMenuItem>=m_iMenuItemViewable+iMAX_VIEWABLE_REVERB)
				{
					m_iMenuItemViewable=m_iMenuItem-iMAX_VIEWABLE_REVERB+1;
				}

				int iIndex=0;

				for(ntstd::List<AudioReverbZone*>::iterator obIt=AudioReverbManager::Get().m_obReverbZoneList.begin(); obIt!=AudioReverbManager::Get().m_obReverbZoneList.end(); ++obIt)
				{
					++iIndex;

					if (iIndex>=m_iMenuItemViewable+1 && iIndex<m_iMenuItemViewable+iMAX_VIEWABLE_REVERB+1)
					{

						g_VisualDebug->Printf2D(fX,fY,0xffffffff,DTF_ALIGN_LEFT,"%c %0.2d %c %c %s (Def:%s Pri:%d) %s",
							(iIndex==(m_iMenuItem+1) ? '>' : ' '),
							iIndex,
							((*obIt)->m_bEnabled ? 'A' : '-'),
							((*obIt)->IsDebugEnabled() ? 'D' : '-'),
							ntStr::GetString((*obIt)->GetName()),
							(*obIt)->GetReverbDef(),
							(*obIt)->m_iPriority,
							((*obIt)->IsActive() ? "[IN USE]" : ""));

						fY+=fLINE_SPACING;
					}
				}
			}
			
			break;
		}

		case AUDIO_MENU_AMBIENCE:
		{
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, " Events  Assets  Categories  Mixer  Reverb <Ambience> Options  ChatterBox \0"); fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________\0"); fY+=16.0f;

			if (!AmbientSoundManager::Get().IsEnabled())
			{
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "<AmbientSoundManager is disabled>"); fY+=fLINE_SPACING;
			}

			const int iMAX_VIEWABLE_AMBIENCE = 20;
			const int iTotalAmbientSounds=AmbientSoundManager::Get().m_obAmbientSoundDefList.size();

			if (iTotalAmbientSounds==0)
			{
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "<No ambience loaded>"); fY+=fLINE_SPACING;
			}
			else
			{
				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "[Total ambience definitions: %d]", iTotalAmbientSounds); fY+=fLINE_SPACING;

				if (m_iMenuItem<0)
					m_iMenuItem=iTotalAmbientSounds - 1;

				if (m_iMenuItem>=iTotalAmbientSounds)
					m_iMenuItem=0;

				if (m_iMenuItem<m_iMenuItemViewable)
				{
					m_iMenuItemViewable=m_iMenuItem;
				}
				else if (m_iMenuItem>=m_iMenuItemViewable+iMAX_VIEWABLE_AMBIENCE)
				{
					m_iMenuItemViewable=m_iMenuItem-iMAX_VIEWABLE_AMBIENCE+1;
				}

				int iIndex=0;

				for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obIt=AmbientSoundManager::Get().m_obAmbientSoundDefList.begin(); obIt!=AmbientSoundManager::Get().m_obAmbientSoundDefList.end(); ++obIt)
				{
					++iIndex;

					if (iIndex>=m_iMenuItemViewable+1 && iIndex<m_iMenuItemViewable+iMAX_VIEWABLE_AMBIENCE+1)
					{
						char acType [64];

						//switch((*obIt)->m_eType)
						switch((*obIt)->GetType())
						{
							default:
							case HRTF_LOOPING:			strcpy(acType,"Panned Loop"); break;
							case HRTF_INTERMITTENT:		strcpy(acType,"Panned Interm"); break;
							case LINEAR_LOOPING:		strcpy(acType,"Linear Loop"); break;
							case LINEAR_INTERMITTENT:	strcpy(acType,"Linear Interm"); break;
							case GLOBAL_LOOPING:		strcpy(acType,"Global Loop"); break;
							case GLOBAL_INTERMITTENT:	strcpy(acType,"Global Interm"); break;
						}

						g_VisualDebug->Printf2D(fX,fY,0xffffffff,DTF_ALIGN_LEFT,"%c %0.2d %c %c %-44s  %s",
							(iIndex==(m_iMenuItem+1) ? '>' : ' '),
							iIndex,
							((*obIt)->IsActive() ? 'A' : '-'),
							((*obIt)->IsDebugRenderEnabled() ? 'D' : '-'),
							ntStr::GetString((*obIt)->GetName()),
							acType);

						fY+=fLINE_SPACING;
					}

					/*
					if (iIndex>=iMAX_VIEWABLE_AMBIENCE)
					{
						g_VisualDebug->Printf2D(fX,fY,0xffffffff,DTF_ALIGN_LEFT,"...");

						fY+=fLINE_SPACING;

						break;
					}
					*/
				}
			}

			break;
		}

		case AUDIO_MENU_OPTIONS:
		{
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, " Events  Assets  Categories  Mixer  Reverb  Ambience <Options> ChatterBox\0"); fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________\0"); fY+=16.0f;
			
			if (m_iMenuItem<0)
				m_iMenuItem=9;

			if (m_iMenuItem>9)
				m_iMenuItem=0;

			char acText [64];

			sprintf(acText,"%c Render sound positions : %c",(m_iMenuItem==0 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_RENDER_SOUND_POS) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			sprintf(acText,"%c TTY System             : %c",(m_iMenuItem==1 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_SYSTEM) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			sprintf(acText,"%c TTY Resources          : %c",(m_iMenuItem==2 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_RESOURCE) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			sprintf(acText,"%c TTY Event play         : %c",(m_iMenuItem==3 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_PLAY) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			sprintf(acText,"%c TTY Event stop         : %c",(m_iMenuItem==4 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_STOP) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			sprintf(acText,"%c TTY Event callback     : %c",(m_iMenuItem==5 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_CALLBACK) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			sprintf(acText,"%c TTY Event error        : %c",(m_iMenuItem==6 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			sprintf(acText,"%c TTY Trigger Statistics : %c",(m_iMenuItem==7 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_TRIGGER_STATS) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			sprintf(acText,"%c TTY Hide Paused Ents.  : %c",(m_iMenuItem==8 ? '>' : ' '),(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_HIDE_PAUSED_ENT) ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;
			
			sprintf(acText,"%c Interactive Music Debug: %c",(m_iMenuItem==9 ? '>' : ' '),(MusicControlManager::Get().IsDebuggingEnabled() ? 'Y' : 'N'));
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, acText); fY+=fLINE_SPACING;

			break;
		}

		case AUDIO_MENU_CHATTERBOX:
		{
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, " Events  Assets  Categories  Mixer  Reverb  Ambience  Options <ChatterBox>\0"); fY+=4.0f;
			g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "__________________________________________________________________________\0"); fY+=16.0f;
			
			if (CChatterBoxMan::Exists() && CEntityManager::Exists())
			{
				if (m_iMenuItem<0)
					m_iMenuItem=7;

				if (m_iMenuItem>7)
					m_iMenuItem=0;

				#define GET_Y_N(x) ( x ? "Y" : "N" )

				unsigned int uiNumStdCB = CChatterBoxMan::Get().GetNumberOfStandardChatterBoxes();
				unsigned int uiNumGenCB = CChatterBoxMan::Get().GetNumberOfGenericChatterBoxes();
				CChatterBox* pActiveCB	= CChatterBoxMan::Get().GetActiveChatterBox();

				g_VisualDebug->Printf2D(fX,fY, 0xffffffff, DTF_ALIGN_LEFT, "Loaded ChatterBoxes : %d of which { Standard : %d -  Generic : %d }\n",uiNumStdCB+uiNumGenCB,uiNumStdCB,uiNumGenCB); fY+=fLINE_SPACING;
				
				// Last Playing ChatterBox
				CChatterBox* pLPCB = CChatterBoxMan::Get().GetLastPlayingChatterBox();
				if (pLPCB)
				{
					g_VisualDebug->Printf2D(fX,fY, DC_YELLOW, DTF_ALIGN_LEFT, "________Last Playing ChatterBox________"); fY+=fLINE_SPACING;
					g_VisualDebug->Printf2D(fX,fY,DC_WHITE,0,"Name: %s - Chatting[%s] - Scheduled Items [%s]", 
													pLPCB->GetName(),
													GET_Y_N(pLPCB->IsChatting()),
													GET_Y_N(pLPCB->HasScheduledChatItems())
											);
					fY+=fLINE_SPACING;
					fY+=fLINE_SPACING;
				}

				// Active ChatterBox
				g_VisualDebug->Printf2D(fX,fY, DC_YELLOW, DTF_ALIGN_LEFT, "Active ChatterBox : [%s]",pActiveCB ? pActiveCB->GetName() : "No Active ChatterBox!"); fY+=fLINE_SPACING;
				if (!pActiveCB)
					break;
				
				const CHashedString pobSubChatterBoxName = pActiveCB->GetNameSubChatterBox();
				bool bHasSubChatterBox = !pobSubChatterBoxName.IsNull();
				
				g_VisualDebug->Printf2D(fX + 32,fY, 0xffffffff, DTF_ALIGN_LEFT, ". Chatting                    : [%s]", GET_Y_N(pActiveCB->IsChatting())); fY+=fLINE_SPACING;
				g_VisualDebug->Printf2D(fX + 32,fY, 0xffffffff, DTF_ALIGN_LEFT, ". Scheduled Items             : [%s]", GET_Y_N(pActiveCB->HasScheduledChatItems())); fY+=fLINE_SPACING;
				g_VisualDebug->Printf2D(fX + 32,fY, 0xffffffff, DTF_ALIGN_LEFT, ". Playing Chat AnimEventSound : [%s]", GET_Y_N(pActiveCB->IsPlayingAnimEventSound())); fY+=fLINE_SPACING;
				g_VisualDebug->Printf2D(fX + 32,fY, 0xffffffff, DTF_ALIGN_LEFT, ". Generic ChatterBoxes        : [%d]", pActiveCB->GetNumberOfGenericChatterBoxes()); fY+=fLINE_SPACING;
				g_VisualDebug->Printf2D(fX + 32,fY, 0xffffffff, DTF_ALIGN_LEFT, ". SubChatterBox               : [%s]", bHasSubChatterBox ? ntStr::GetString(pobSubChatterBoxName) : "-None-"); fY+=fLINE_SPACING;			

				fY+=fLINE_SPACING;

				// Statistics
				if (AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_TRIGGER_STATS))
				{
					const CBStatsList* pStatList = pActiveCB->GetStatisticsList_const();
					if (pStatList)
					{
						g_VisualDebug->Printf2D(fX + 32,fY, DC_YELLOW, DTF_ALIGN_LEFT, "_____________Active Statistics_____________"); fY+=fLINE_SPACING;
						ntstd::List<SChatterBoxStatisticPair*>::const_iterator obItStat		= pStatList->begin();
						ntstd::List<SChatterBoxStatisticPair*>::const_iterator obItStatEnd	= pStatList->end();
						for ( ; obItStat != obItStatEnd; ++obItStat )
						{
							SChatterBoxStatisticPair* pStat = *obItStat;
							g_VisualDebug->Printf2D(fX + 32,fY, 0xffffffff, DTF_ALIGN_LEFT,"[%d] - %s",pStat->uiCount, ntStr::GetString(pStat->hsTriggerEvent)); fY+=fLINE_SPACING;
						}
					}				
				}

				// List of Participants
				g_VisualDebug->Printf2D(fX + 32,fY, DC_YELLOW, DTF_ALIGN_LEFT, "___________List of Participants___________"); fY+=fLINE_SPACING;
				const ntstd::List<CEntity*>* plistRemainingParticipantsList = pActiveCB->GetParticipantsList();
				if (plistRemainingParticipantsList->empty()) 
				{
					g_VisualDebug->Printf2D(fX + 32,fY, 0xffffffff, DTF_ALIGN_LEFT, "No participants added or left!"); fY+=fLINE_SPACING;
				}
				else
				{
					unsigned int uiIdx = 0;
					ntstd::List<CEntity*>::const_iterator obIt		= plistRemainingParticipantsList->begin();
					ntstd::List<CEntity*>::const_iterator obItEnd	= plistRemainingParticipantsList->end();
					for ( ; obIt != obItEnd; ++obIt )
					{
						CEntity* pEntAI = (*obIt);
						bool bChattingAI = ( (pActiveCB->IsChatting() && pActiveCB->GetLastChatter() == pEntAI)); // || (m_LastPlayingChatterBox && m_LastPlayingChatterBox->IsChatting() && m_LastPlayingChatterBox->GetLastChatter() == (*obIt)));

						if (pEntAI)
						{
							if (!(AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_HIDE_PAUSED_ENT)) || !pEntAI->ToCharacter()->IsPaused())
							{
								bool bInCSStandard = CChatterBoxMan::Get().DebugIsEntityInCSStandadrd(pEntAI);
								g_VisualDebug->Printf2D(fX + 32,fY, 0xffffffff, DTF_ALIGN_LEFT, "%d: %s - G(%d)%s%s%s%s%s",	
										++uiIdx,
										ntStr::GetString(pEntAI->GetName()), 
										pEntAI->ToAI()->GetChatGroupID(),
										bInCSStandard ? "" : " RECOVERING ",
										pEntAI->ToCharacter()->IsPaused() ? " PAUSED " : "",
										pEntAI->ToCharacter()->IsDead() ? " DEAD " : "",
										bChattingAI ? " CHATTING " : "",
										pActiveCB->HasChatterBoxChecksDisabled(pEntAI) ? " NO CHECKS" : ""
										);
								if (bChattingAI)
									g_VisualDebug->Printf3D(pEntAI->GetPosition()+CPoint(0,1.9f,0),DC_YELLOW,0,"CHATTING");
								fY+=fLINE_SPACING;
							}
						}
					}
				}
			}

			break;
		}
		
		default:
			break;
	}

	// Render 3d positions of sounds (if option is enabled)
	if (AudioSystem::Get().IsDebugOptionEnabled(DEBUG_RENDER_SOUND_POS))
	{
		for(int i=0; i<MAX_EVENT_INSTANCES; ++i)
		{
			AudioSystem::Get().m_obInstanceList[i].DebugRender();
		}
	}

	// Do debug rendering for ambient sounds
	for(ntstd::List<BaseAmbientSoundDefinition*>::iterator obIt=AmbientSoundManager::Get().m_obAmbientSoundDefList.begin(); obIt!=AmbientSoundManager::Get().m_obAmbientSoundDefList.end(); ++obIt)
	{
		(*obIt)->DebugRender();
	}

	// Do debug rendering for reverb zones
	for(ntstd::List<AudioReverbZone*>::iterator obIt=AudioReverbManager::Get().m_obReverbZoneList.begin(); obIt!=AudioReverbManager::Get().m_obReverbZoneList.end(); ++obIt)
	{
		(*obIt)->DebugRender();
	}

#endif // _ENABLE_AUDIO_CONSOLE
}


void AudioConsole::PreviousMenu ()
{
	m_iMenuItem=0;

	--m_iMenu;
	
	if (m_iMenu<0)
		m_iMenu=AUDIO_MENU_TOTAL - 1;
}

void AudioConsole::NextMenu ()
{
	m_iMenuItem=0;

	++m_iMenu;

	if (m_iMenu>=AUDIO_MENU_TOTAL)
		m_iMenu=0;
}

void AudioConsole::NextMenuItem ()
{
	++m_iMenuItem;
}

void AudioConsole::PreviousMenuItem ()
{
	--m_iMenuItem;
}

