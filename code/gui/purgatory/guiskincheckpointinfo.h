/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINCHECKPOINTINFO_H
#define _GUISKINCHECKPOINTINFO_H

// Includes
#include "gui/guiunit.h"
#include "core/nt_std.h"
#include "gui/guitext.h"

// Forward Declarations
class MessageDataManager;

/***************************************************************************************************
*
*	CLASS			CGuiSkinCheckpointInfo
*
*	DESCRIPTION		Represents the checkpoint info component.
*
***************************************************************************************************/

class CGuiSkinCheckpointInfo : public CGuiUnit
{
	typedef CGuiUnit super;
public:
	
	// Construction Destruction
	CGuiSkinCheckpointInfo( void );
	virtual ~CGuiSkinCheckpointInfo( void );

	void ClearInfo();
	void AddInfo(const char* pcName, const char* pcValue);

	//used to swap info when switching chapters (so that fades trigger...)
	void			OnChapterSwitchBegin();
	void			OnChapterSwitchEnd();

	//used when only switching checkpoints
	void			UpdateCheckpointInfo();

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessChild( CXMLElement* pobChild );
	virtual bool	ProcessEnd( void );

	// main
	virtual bool	Update( void );
	virtual bool	Render( void );
	virtual void	UpdateFadeIn();
	virtual void	UpdateFadeOut();

private:

	void UnpackLifeclock(double dLifeclock, int& iHours, int& iMinutes, int& iSeconds);

	void SetImageTemplate(const char* pcImageTemplate);

	const char* m_pcImageTemplate;

	struct InfoItem
	{
		CString* m_pobTitle;
		CString* m_pobValue;
	};

	typedef ntstd::List< InfoItem* > InfoItemList;
	InfoItemList m_obInfoItemList;

	CStringDefinition m_obTitleStrDef;
	CStringDefinition m_obValueStrDef;

	float m_fYCurrentOffset;

	const char* m_pcLifeClockTitleId;
	const char* m_pcLifeClockValueTitleId;
	const char* m_pcLifeClockBestTitleId;
	const char* m_pcLifeClockBestValueTitleId;

	MessageDataManager* m_pobMessageDataManager;

	bool m_bCheckpointSwitchFadeRunning;
	int m_iCheckpointSwitchFadeDir;
	float m_fCheckpointSwitchFade;

	//cache GuiUnit pointers to image and title
	CGuiUnit* m_pobTitleUnit;
	CGuiUnit* m_pobImageUnit;
};

#endif // _GUISKINCHECKPOINTINFO_H
