/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES
*
***************************************************************************************************/

#ifndef	_GUIACTION_H
#define	_GUIACTION_H

#include "lua/ninjalua.h"

class CGuiScreen;

class CGuiAction
{
public:
	// Actions Types
	enum ACTION_TYPES
	{
		NO_ACTION	= ( 1 << 0 ),
		MOVE_ON		= ( 1 << 1 ),
		MOVE_BACK	= ( 1 << 2 ),
		EXIT		= ( 1 << 3 ),
		RUN_SCRIPT	= ( 1 << 4 )
	};

	enum EXECUTE_FROM
	{
		FROM_INLINE	= (1<<0),
		FROM_FILE	= (1<<1)
	};

	CGuiAction(ACTION_TYPES eAction = NO_ACTION);
	~CGuiAction();

	bool ProcessAttribute( const char* pcTitle, const char* pcValue );

	bool TriggerAction(CGuiScreen* pobContext);

	ACTION_TYPES GetAction() const { return m_eAction; }
	const char* GetParam() const { return m_pcParam; }

private:
	ACTION_TYPES m_eAction;
	const char* m_pcParam;
	EXECUTE_FROM m_eExecFrom;
	const char* m_pcScript;
};

#endif // _GUIACTION_H
