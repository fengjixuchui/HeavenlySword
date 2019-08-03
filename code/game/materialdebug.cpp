//--------------------------------------------------
//!
//!	\file materialdebug.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "materialdebug.h"

//--------------------------------------------------
//!
//!	Short Function description.
//!
//--------------------------------------------------
void	MaterialDebug::ShortenName( const char* inStr, char* pOutStr )
{
	ntAssert( inStr );
	ntAssert( pOutStr );

	pOutStr[0] = NULL;

	char pInStr[2048];
    strcpy(pInStr, inStr );

    char* pTempStr = strtok( pInStr, "_" );
    bool prefixUnderscore = false;

    while( pTempStr != 0 )
    {
Reparse: // evil I know but I'm in league with the devil so I don't care
        if( strcmp( pTempStr, "base" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "b_");
        }
        else if( strcmp( pTempStr, "lambert" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "l_");
        }
        else if( strcmp( pTempStr, "phong" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "p_");
		}
		else if( strcmp( pTempStr, "phong1" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "p1_");
		}
		else if( strcmp( pTempStr, "phong1n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "p1n_");
		}
		else if( strcmp( pTempStr, "phong2" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "p2_");
		}
		else if( strcmp( pTempStr, "phong2n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "p2n_");
		}
		else if( strcmp( pTempStr, "phong3" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "p3_");
		}
		else if( strcmp( pTempStr, "phong3n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "p3n_");
		}
        else if( strcmp( pTempStr, "metallic" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "m_");
        }
		else if( strcmp( pTempStr, "metallic1" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "m1_");
        }
		else if( strcmp( pTempStr, "metallic1n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "m1n_");
        }
		else if( strcmp( pTempStr, "metallic2" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "m2_");
        }
		else if( strcmp( pTempStr, "metallic2n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "m2n_");
        }
		else if( strcmp( pTempStr, "metallic3" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "m3_");
        }
		else if( strcmp( pTempStr, "metallic3n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "m3n_");
        }
        else if( strcmp( pTempStr, "true" ) == 0 )
        {
			strcat( pOutStr, "1");
            prefixUnderscore = true;
        }
        else if( strcmp( pTempStr, "false" ) == 0 )
        {
			strcat( pOutStr, "0");
            prefixUnderscore = true;
        }
        else if( strcmp( pTempStr, "jambert1" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "j1_");
        }
		else if( strcmp( pTempStr, "jambert1n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "j1n_");
        }
        else if( strcmp( pTempStr, "jambert2" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "j2_");
		}
	    else if( strcmp( pTempStr, "jambert2n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "j2n_");
		}
        else if( strcmp( pTempStr, "jambert3" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "j3_");
        }
        else if( strcmp( pTempStr, "jambert3n" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "j3n_");
        }
        else if( strcmp( pTempStr, "parametric" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "para_");
        }
        else if( strcmp( pTempStr, "alphablend" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "ab_");
        }
        else if( strcmp( pTempStr, "subtract" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "sub_");
        }
        else if( strcmp( pTempStr, "modulate" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "mod_");
        }
        else if( strcmp( pTempStr, "overwrite" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "ow_");
        }
        else if( strcmp( pTempStr, "alphatest" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "at_");
        }
        else if( strcmp( pTempStr, "layered" ) == 0 )
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, "lay_");
        }
        else if( strcmp( pTempStr, "depth" ) == 0 )
        {
            // special case depth_haze depth can be followed by hazed
            pTempStr = strtok( 0, "_" );
            if( strcmp( pTempStr, "hazed" ) == 0 )
            {
                if( prefixUnderscore == true )
                {
					strcat( pOutStr, "_");
                    prefixUnderscore = false;
                }
				strcat( pOutStr, "dh_");
            } else
            {
                if( prefixUnderscore == true )
                {
					strcat( pOutStr, "_");
                    prefixUnderscore = false;
                }
				strcat( pOutStr, "d_");
                goto Reparse;
            }
        }
        else
        {
            if( prefixUnderscore == true )
            {
				strcat( pOutStr, "_");
                prefixUnderscore = false;
            }
			strcat( pOutStr, pTempStr );
			strcat( pOutStr, "_" );
        }

        pTempStr = strtok( 0, "_" );
    }
    // remove trailing _
	int iSize = strlen( pOutStr );
    if( pOutStr[iSize-1] == '_' )
		pOutStr[iSize-1] = NULL;
}

//--------------------------------------------------
//!
//!	Insert a name into the list
//!
//--------------------------------------------------
void ExclusiveNameList::InsertIntoList( const char* pName )
{
	if (!m_pNameList)
		m_pNameList = NEW CList<NameCounter*>;

	NameCounter	test(pName);

	for (	CList<NameCounter*>::iterator it = m_pNameList->begin();
			it != m_pNameList->end(); ++it )
	{
		int iResult = test.Compare(**it);

		if (iResult == 0)
		{
			(*it)->Increment();
			return;
		}
		
		if (iResult < 0)
		{
			NameCounter* pRecord = NEW NameCounter( pName );
			m_pNameList->insert( it, pRecord );
			return;
		}
	}

	// must be at the end of the list
	NameCounter* pRecord = NEW NameCounter( pName );
	m_pNameList->push_back( pRecord );
}

//--------------------------------------------------
//!
//! dump the current list
//!
//--------------------------------------------------
void ExclusiveNameList::PrintList()
{
	if (m_pNameList)
	{
		int iCount = 0;
		for (	CList<NameCounter*>::iterator it = m_pNameList->begin();
				it != m_pNameList->end(); ++it, iCount++ )
		{
			ntPrintf( "%d:\t%s(%d)\n", iCount, (*it)->GetName(), (*it)->GetCount() );
		}
	}
}

//--------------------------------------------------
//!
//! clear the current list
//!
//--------------------------------------------------
void ExclusiveNameList::ClearList()
{
	if (m_pNameList)
	{
		while (!m_pNameList->empty())
		{
			delete m_pNameList->back();
			m_pNameList->pop_back();
		}

		delete m_pNameList;
		m_pNameList = NULL;
	}
}
