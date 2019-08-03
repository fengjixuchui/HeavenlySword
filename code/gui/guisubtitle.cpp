/**
	@file subtitles.cpp

	@author campf

	Subtitle display system
*/

#include <gui/guisubtitle.h>
#include <gui/guitext.h>
#include <anim/hierarchy.h>
#include <gfx/display.h>

/**
	Internal function for CSubtitle.

	@param strSubtitleString	Pointer to large string to take for from.
	@param i					Reference to character index.
	@param strBlock				Current subtitle block.
	@param bPrependSpace		Whether to put a space on the front of the word.
	
	@return Whether this block should end.
*/
static bool AddWord(const WCHAR_T *strSubtitleString, int &i, CSubtitleBlock &strBlock, bool bPrependSpace)
{
	WCHAR_T strWord[256];
	int iCount=0;
	bool bMore = true;
    
	WCHAR_T strDelay[16];
	int iNumLen = 0;
	while (strSubtitleString[i] != (WCHAR_T)' ')
	{
		if (!strSubtitleString[i])
		{
			bMore = false;
			break;
		}

		// check for delay code
		if (strSubtitleString[i] == (WCHAR_T)'[')
		{
			bool bAbort = false;
			int iAbortRewind = i+1;

			// read delay timing
			while (strSubtitleString[++i] != (WCHAR_T)']')
			{
				// sanity check for a number
				if ( !(strSubtitleString[i] >=(WCHAR_T)'0' && strSubtitleString[i] <= (WCHAR_T)'9') )
				{
					// argh! we gotta quit this plan
					bAbort = true;
					break;
				}
				else
				{
					ntAssert_p(iNumLen < 16, ("CSubtitle: too many characters in delay code!"));
					strDelay[iNumLen++] = strSubtitleString[i];
				}
			}
			
			// Are we still ok to go?
			if (!bAbort)
			{
				strDelay[iNumLen] = 0;
				bMore = false;

				// we must break here to allow to string to be delayed at this point
				break;
			}
			else
			{
				// get most of the text, missing the leading [ so we don't pick it up again
				i = iAbortRewind;

				// add the missing [ to the start
				strWord[iCount++] = '[';

				// reset this to 0 so we dont pick up a number
				iNumLen = 0;
			}
		}
		
		// Fill in word
		ntAssert_p(iCount < 256, ("CSubtitle: too many characters in word!"));
		strWord[iCount++] = strSubtitleString[i];

		i++;
	}

	// Don't prepend a space if the word was only a delay tag
	if (iCount == 0 && iNumLen > 0)
		bPrependSpace = false;

	// Don't prepend a space if the block hasn't started yet
	if (bPrependSpace && strBlock.m_strBlock->length() > 0)
	{
		*strBlock.m_strBlock += L" ";
	}

	// Null terminate word
	strWord[iCount] = 0;
	i++;

	// check for delay codes
	if (iNumLen > 0)
	{
		// here we should have the number of seconds to delay as a string
		
		// convert to ascii first
		char strCharDelay[16];
		for (int j=0; strDelay[j]; j++) strCharDelay[j] = (char)strDelay[j];
		strCharDelay[iNumLen] = 0;

		// insert delay into block
		strBlock.m_fDelay = (float)atof(strCharDelay);
	}

	// Increase subtitle block by one word
	*strBlock.m_strBlock += strWord;
	
	return bMore;
}

/**
	Make a CString for the current subtitle block.

	@param strString Subttile block text.
*/
void CSubtitle::MakeBlockString(const WCHAR_T *strString)
{
	m_pobCurrentBlockString = CStringManager::Get().MakeString(strString, m_obDefinition, m_pobTransform, CGuiUnit::RENDER_SCREENSPACE);
}

/**
	Construct a subtitle from a LAMS string id and a font name.

	@param strID		String ID.
	@param strFontName	Name of font to use.
*/
CSubtitle::CSubtitle(	const char *strID, const char *strFontName ) :
						m_uiCurrentlyRenderingBlock(0),
						m_fTime(0),
						m_pobCurrentBlockString(NULL),
						m_bPause(false),
						m_bStopped(false)
{
	m_obDefinition.m_pobFont = CFontManager::Get().GetFont(strFontName);
	m_obDefinition.m_fBoundsWidth = DisplayManager::Get().GetInternalWidth()*0.8f;
	m_obDefinition.m_fBoundsHeight = 50;
	m_obDefinition.m_fXOffset = DisplayManager::Get().GetInternalWidth()*0.5f;
	m_obDefinition.m_fYOffset = DisplayManager::Get().GetInternalHeight() - m_obDefinition.m_fBoundsHeight;
	m_obDefinition.m_bDynamicFormat = true;

	m_pobTransform = NT_NEW Transform();

	CMatrix obBaseMatrix( CONSTRUCT_IDENTITY );
	m_pobTransform->SetLocalMatrix( obBaseMatrix );
	CHierarchy::GetWorld()->GetRootTransform()->AddChild( m_pobTransform );

    // Get the string from the LAMS database
	const WCHAR_T *strSubtitle = CStringManager::Get().GetResourceString(strID);

	// Make substrings
	ProcessString(strSubtitle);

	// Make the first CString
	MakeBlockString( m_strBlockList[0]->m_strBlock->data() );
}

/**
	Clean up.
*/
CSubtitle::~CSubtitle()
{
	// Delete all the substrings
	for (	std::vector<CSubtitleBlock *>::iterator obBlock = m_strBlockList.begin(); 
			obBlock != m_strBlockList.end();
			obBlock++ )
	{
		delete *obBlock;
	}

	if (m_pobCurrentBlockString)
	{
		CStringManager::Get().DestroyString(m_pobCurrentBlockString);
		m_pobCurrentBlockString = NULL;
	}

	if (m_pobTransform)
	{
		m_pobTransform->RemoveFromParent();
		NT_DELETE(m_pobTransform);
	}
}

/**
	Process an enormous string into an array of smaller substrings suitable for displaying as subtitles.

	@param strSubtitleString Large string to process.
*/
void CSubtitle::ProcessString(const WCHAR_T *strSubtitleString)
{
	// Go through this huge string and break it up into lots of smaller chunks

	const int iLen = wcslen(strSubtitleString);

	int i=0;
	while (i < iLen)
	{
		CSubtitleBlock *strBlock = NT_NEW CSubtitleBlock;
		bool bPrependSpace = false;
	
		// While there are more words, make a block which is of the required length.
		while ( AddWord(strSubtitleString, i, *strBlock, bPrependSpace) )
		{
			if (strBlock->m_strBlock->length() > NUM_ALLOWED_CHARACTERS)
			{
				break;
			}
			bPrependSpace = true;
		}

		m_strBlockList.push_back(strBlock);
	}
}

/**
	Advance time for this subtitle.

	@param fDt Number of seconds in timestep.
*/
void CSubtitle::Update(const float fDt)
{
	float fMetric;

	// If this current subtitle has a manual delay, override the automatic guess.
	if (m_strBlockList[m_uiCurrentlyRenderingBlock]->m_fDelay > 0.0f)
		fMetric = m_strBlockList[m_uiCurrentlyRenderingBlock]->m_fDelay;
	else
		fMetric = m_strBlockList[m_uiCurrentlyRenderingBlock]->m_strBlock->length() * 0.2f;

	// Is it time to move to next subtitle block?
	if (m_fTime > fMetric)
	{
		// Yes, so do it
		m_uiCurrentlyRenderingBlock++;

		// Are we done?
		if (m_uiCurrentlyRenderingBlock >= m_strBlockList.size())
		{
			// Yes, stop
			Stop(true);
		}
		else
		{
			// No, lets carry on
			m_fTime = 0;

			// Make the string to be rendered
			CStringManager::Get().DestroyString(m_pobCurrentBlockString);
			m_pobCurrentBlockString = NULL;
			MakeBlockString( m_strBlockList[m_uiCurrentlyRenderingBlock]->m_strBlock->data() );
		}
	}

	if (!m_bPause) m_fTime += fDt;
}

/**
	Render this subtitle.
*/
void CSubtitle::Render(void)
{
	m_pobCurrentBlockString->Render();
}

/**
	Play the given LAMS string ID as a subtitled stream of text.
*/
void CSubtitleMan::Play(const char *strID)
{
	if (m_bEnable)
	{
		// Stop any existing subtitle
		Stop();

		if ( CStringManager::Get().GetLevelLanguage() )
		{
			m_pobSubtitle = NT_NEW CSubtitle(strID, m_strFont);
		}
		else
		{
			ntPrintf("CSubtitleMan::Play(%s): no level language loaded, no subtitles will happen!\n", strID);
		}
	}
}

/**
	Stop the currently playing subtitle stream.
*/
void CSubtitleMan::Stop(void)
{
	NT_DELETE(m_pobSubtitle);
	m_pobSubtitle = NULL;
}
