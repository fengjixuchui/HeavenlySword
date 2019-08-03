//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/physicsmaterial.h
//!	
//!	Physics materals.
//! 
//!
//!	Author: Peter Feher
//! Created: 2006.07.07
//!
//---------------------------------------------------------------------------------------------------------

#include "Physics/config.h"
#include "physicsmaterial.h"

namespace Physics
{
	START_STD_INTERFACE	(psPhysicsMaterial)
		IFLOAT			(psPhysicsMaterial, Friction)
		IFLOAT			(psPhysicsMaterial, Restitution)
		IFLOAT			(psPhysicsMaterial, Penetrability)
		IINT			(psPhysicsMaterial, EffectMaterialBit)
		DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
	END_STD_INTERFACE

	//-----------------------------------------
	// table of generic physics materials

	// Ragdolls
	psPhysicsMaterial obPelvisPhysicsMaterial(CHashedString(HASH_STRING_PELVIS_MATERIAL), -1, -1, 1, 57); 
	psPhysicsMaterial obSpinePhysicsMaterial(CHashedString(HASH_STRING_SPINE_MATERIAL), -1, -1, 1, 58);
	psPhysicsMaterial obHeadPhysicsMaterial(CHashedString(HASH_STRING_HEAD_MATERIAL), -1, -1, 1, 59);
	psPhysicsMaterial obLLegPhysicsMaterial(CHashedString(HASH_STRING_L_LEG_MATERIAL), -1, -1, 1, 60);
	psPhysicsMaterial obRLegPhysicsMaterial(CHashedString(HASH_STRING_R_LEG_MATERIAL), -1, -1, 1, 61);
	psPhysicsMaterial obLArmPhysicsMaterial(CHashedString(HASH_STRING_L_ARM_MATERIAL), -1, -1, 1, 62);
	psPhysicsMaterial obRArmPhysicsMaterial(CHashedString(HASH_STRING_R_ARM_MATERIAL), -1, -1, 1, 63);
	//-----------------------------------------



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	PhysicsMaterialTable::PhysicsMaterialTable(const char * xmlFile)
	{
	    // add generic materials to table
		m_table.push_back(&obPelvisPhysicsMaterial);
		m_table.push_back(&obSpinePhysicsMaterial);
		m_table.push_back(&obHeadPhysicsMaterial);
		m_table.push_back(&obLLegPhysicsMaterial);
		m_table.push_back(&obRLegPhysicsMaterial);
		m_table.push_back(&obLArmPhysicsMaterial);
		m_table.push_back(&obRArmPhysicsMaterial);
          
		// load materials from xml
		static char acFileName[MAX_PATH];
		Util::GetFiosFilePath( xmlFile, acFileName );

		if ( File::Exists( acFileName ) )
		{
			// Tell people what we're up to
			ntPrintf("XML loading \'%s\'\n", xmlFile);

			// Open the XML file in memory
			FileBuffer obFile( acFileName, true );

			if ( !ObjectDatabase::Get().LoadDataObject( &obFile, xmlFile ) )
			{
				ntError_p( false, ( "Failed to parse XML file '%s'", xmlFile ) );
				return;
			}

			CHashedString hashedFileName(xmlFile);
			DataObject* obj = ObjectDatabase::Get().GetDataObjectFromName( hashedFileName );			
			ntAssert(obj != NULL);

			ObjectContainer* current = (ObjectContainer*) obj->GetBasePtr();

			for (	ObjectContainer::ObjectList::iterator obIt = current->m_ContainedObjects.begin(); 
				obIt != current->m_ContainedObjects.end(); 
				obIt++ )
			{
				if ( 0 == strcmp( (*obIt)->GetClassName(), "psPhysicsMaterial") )
				{
					psPhysicsMaterial* pobMat = (psPhysicsMaterial*)((*obIt)->GetBasePtr());
					pobMat->SetMaterialName((*obIt)->GetName());  // set name of the material. Its hash will be used like an id. 
					m_table.push_back(pobMat);

					
				}
			}

			return;
		}

		ntPrintf("WARNING! Could not find XML in '%s'...\n", xmlFile );
	}

	uint32_t PhysicsMaterialTable::GetIndexFromId(uint32_t id) const
	{
		int i = 1; 
		for(TableIterator it = m_table.begin(); it != m_table.end(); it++)
		{
			if ((*it)->GetMaterialId() == id)
			{
				return i; 
			}
			i++;
		}

		return 0; 
	}

	// Find material with given id. 
	psPhysicsMaterial* PhysicsMaterialTable::GetMaterialFromId(uint32_t id) const
	{
		for(TableIterator it = m_table.begin(); it != m_table.end(); it++)
		{
			if ((*it)->GetMaterialId() == id)
			{
				return *it; 
			}
		}
		return NULL;
	}



	uint64_t PhysicsMaterialTable::GetEffectMaterialBit (const CHashedString& obMaterialName)
	{
		for(TableIterator it = m_table.begin(); it != m_table.end(); it++)
		{
			if (obMaterialName.GetHash() == (*it)->GetMaterialId())
			{
				return (*it)->GetEffectMaterialBit(); 
			}
		}

		return 0;
	}

	uint64_t PhysicsMaterialTable::GetEffectMaterialBitfield (const char* pcString)
	{
		ntPrintf("PhysicsMaterialTable::GetEffectMaterialBitfield -> %s\n",pcString);

		char acTemp [64];

		uint64_t returnvalue=0;

		int i=0;
		int j=0;

		while(1)
		{
			if (pcString[i]==',' || pcString[i]==' ' || pcString[i]=='\0')
			{
				acTemp[j]='\0';

				CHashedString obMaterial(acTemp);

				uint64_t colEffectMat=GetEffectMaterialBit(obMaterial);

				if (colEffectMat)
				{
					returnvalue|=colEffectMat;
					ntPrintf("    %s = %d\n",acTemp,colEffectMat);
				}

				if (pcString[i]=='\0') // We are finished
					break;

				j=0;
				++i;
			}
			else
			{
				acTemp[j++]=pcString[i++];
			}
		}

		ntPrintf("    Result = %d\n",returnvalue);

		return returnvalue;
	}

	void PhysicsMaterialTable::Debug_GetMaterialsFromBitfield (uint64_t uiEffectMaterialBitfield,char* pcOutput)
	{
		#ifndef _RELEASE

		int i=0;

		for(TableIterator it = m_table.begin(); it != m_table.end(); it++)
		{
			if (uiEffectMaterialBitfield & (*it)->GetEffectMaterialBit())
			{
				const char* pcMaterialName=(*it)->GetMaterialName();

				if (pcMaterialName)
				{
					int j=0;

					while(pcMaterialName[j]!='\0')
					{
						pcOutput[i++]=pcMaterialName[j++];
					}

					pcOutput[i++]=',';
				}
			}
		}

		pcOutput[i-1]='\0';

		#else

		UNUSED(uiEffectMaterialBitfield);
		UNUSED(pcOutput);

		#endif
	}
}
