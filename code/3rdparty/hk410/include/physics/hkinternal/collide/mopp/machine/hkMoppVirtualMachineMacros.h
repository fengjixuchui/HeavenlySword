/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



//
//	Macros for virtual machines:
//  Assumptions:<br>
//  The machine defines the following inline function<br>
//	   - void hkMoppXXXXXXXXxxVirtualMachine::addHit(unsigned int key, const unsigned int properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES])
//
//	The following variables are used:<br>
//		- PC:		the current program counter
//		- offsetl   an integer variable
//		- offseth   an integer variable
//		- query     the const query input holding an integer m_primitiveOffset and m_properties[1] member
//      - scaledQuery a temporary copy of query to hold a modified version of the query



#define	HK_MOPP_JUMP_MACRO								\
		case HK_MOPP_JUMP8:								\
			{											\
				offsetl = PC[1];		\
				PC += 2;								\
				PC += offsetl;							\
				continue;								\
			}											\
		case HK_MOPP_JUMP16:								\
			{											\
				offseth = PC[1];		\
				offsetl = PC[2];		\
				PC += 3;								\
				PC += (offseth << 8) + offsetl;			\
				continue;								\
			}											\
		case HK_MOPP_JUMP24:								\
			{											\
				offseth = PC[1];		\
				const unsigned int offsetm = PC[2];		\
				offsetl = PC[3];		\
				PC += 4;								\
				PC += (offseth << 16) + (offsetm << 8) + offsetl;	\
				continue;								\
			}



#define HK_MOPP_REOFFSET_MACRO									\
		case HK_MOPP_TERM_REOFFSET8:								\
			{													\
				const unsigned int offset = PC[1];				\
				if ( query != &scaledQuery)						\
				{												\
					scaledQuery = *query;						\
					query = &scaledQuery;						\
				}												\
				scaledQuery.m_primitiveOffset += offset;		\
				PC+=2;											\
				continue;										\
			}													\
		case HK_MOPP_TERM_REOFFSET16:							\
			{													\
				const unsigned int offset = (PC[1]<<8) + PC[2];	\
				if ( query != &scaledQuery)						\
				{												\
					scaledQuery = *query;						\
					query = &scaledQuery;						\
				}												\
				scaledQuery.m_primitiveOffset += offset;		\
				PC+=3;											\
				continue;										\
			}													\
		case HK_MOPP_TERM_REOFFSET32:							\
			{													\
				const unsigned int offset = (PC[1]<<24) + (PC[2]<<16) + (PC[3]<<8) + PC[4];						\
				if ( query != &scaledQuery)						\
				{												\
					scaledQuery = *query;						\
					query = &scaledQuery;						\
				}												\
				scaledQuery.m_primitiveOffset = offset;			\
				PC+=5;											\
				continue;										\
			}


#define		HK_MOPP_TERMINAL_MACRO			\
		case HK_MOPP_RETURN:				\
			{								\
				goto end_of_function;		\
			}								\
		case HK_MOPP_TERM8:					\
			{								\
				offsetl = PC[1];			\
				goto add_Terminal;			\
			}								\
		case HK_MOPP_TERM16:				\
			{								\
				offseth = PC[1];			\
				offseth <<=8;				\
				offsetl = PC[2];			\
				offsetl += offseth;			\
				goto add_Terminal;			\
			}								\
		case HK_MOPP_TERM24:				\
			{								\
				offsetl = PC[1];			\
				offsetl <<=16;				\
				offseth = PC[2];			\
				offseth <<=8;				\
				offseth += PC[3];			\
				offsetl += offseth;			\
				goto add_Terminal;			\
			}								\
		case HK_MOPP_TERM32:				\
			{								\
				offsetl = PC[1];			\
				offsetl <<=24;				\
				offseth = PC[2];			\
				offseth <<=16;				\
				offsetl += offseth;			\
				offseth = PC[3];			\
				offsetl += PC[4];			\
				offseth <<=8;				\
				offsetl += offseth;			\
				goto add_Terminal;			\
			}								\
		case HK_MOPP_TERM4_0:				\
		case HK_MOPP_TERM4_1:				\
		case HK_MOPP_TERM4_2:				\
		case HK_MOPP_TERM4_3:				\
		case HK_MOPP_TERM4_4:				\
		case HK_MOPP_TERM4_5:				\
		case HK_MOPP_TERM4_6:				\
		case HK_MOPP_TERM4_7:				\
		case HK_MOPP_TERM4_8:				\
		case HK_MOPP_TERM4_9:				\
		case HK_MOPP_TERM4_A:				\
		case HK_MOPP_TERM4_B:				\
		case HK_MOPP_TERM4_C:				\
		case HK_MOPP_TERM4_D:				\
		case HK_MOPP_TERM4_E:				\
		case HK_MOPP_TERM4_F:				\
		case HK_MOPP_TERM4_10:				\
		case HK_MOPP_TERM4_11:				\
		case HK_MOPP_TERM4_12:				\
		case HK_MOPP_TERM4_13:				\
		case HK_MOPP_TERM4_14:				\
		case HK_MOPP_TERM4_15:				\
		case HK_MOPP_TERM4_16:				\
		case HK_MOPP_TERM4_17:				\
		case HK_MOPP_TERM4_18:				\
		case HK_MOPP_TERM4_19:				\
		case HK_MOPP_TERM4_1A:				\
		case HK_MOPP_TERM4_1B:				\
		case HK_MOPP_TERM4_1C:				\
		case HK_MOPP_TERM4_1D:				\
		case HK_MOPP_TERM4_1E:				\
		case HK_MOPP_TERM4_1F:				\
			offsetl = command - HK_MOPP_TERM4_0;				\
			{								\
add_Terminal:								\
				offsetl += query->m_primitiveOffset;		\
				addHit(offsetl,query->m_properties);		\
				goto end_of_function;		\
			}								\



#define		HK_MOPP_PROPERTY_MACRO				\
		case HK_MOPP_PROPERTY8_0:				\
		case HK_MOPP_PROPERTY8_1:				\
		case HK_MOPP_PROPERTY8_2:				\
		case HK_MOPP_PROPERTY8_3:				\
			{									\
				unsigned int property; property = PC[0] - HK_MOPP_PROPERTY8_0;	\
				unsigned int value; value = PC[1];						\
				PC += 2;												\
				scaledQuery.m_properties[property] = value;				\
propertyCopy:															\
				const unsigned int v = scaledQuery.m_properties[0];					\
				if ( query != &scaledQuery)								\
				{														\
					scaledQuery = *query;								\
					query = &scaledQuery;								\
				}														\
				scaledQuery.m_properties[0] = v;						\
				continue;												\
			}															\
		case HK_MOPP_PROPERTY16_0:				\
		case HK_MOPP_PROPERTY16_1:				\
		case HK_MOPP_PROPERTY16_2:				\
		case HK_MOPP_PROPERTY16_3:				\
			{									\
				const unsigned int property = PC[0] - HK_MOPP_PROPERTY16_0;	\
				const unsigned int value = (PC[1]<<8) + PC[2];			\
				scaledQuery.m_properties[property] = value;				\
				PC += 3;												\
				goto propertyCopy;										\
			}															\
		case HK_MOPP_PROPERTY32_0:				\
		case HK_MOPP_PROPERTY32_1:				\
		case HK_MOPP_PROPERTY32_2:				\
		case HK_MOPP_PROPERTY32_3:				\
			{									\
				const unsigned int property = PC[0] - HK_MOPP_PROPERTY32_0;	\
				const unsigned int value = (PC[1]<<24) + (PC[2]<<16) + (PC[3]<<8) + PC[4];						\
				scaledQuery.m_properties[property] = value;				\
				PC += 5;												\
				goto propertyCopy;										\
			}														
							

#define	HK_MOPP_DEFAULT_MACRO								\
		default:											\
			HK_ERROR(0x1298fedd, "Unknown command - This mopp data has been corrupted (check for memory trashing), or an hkMoppBvTreeShape has been pointed at invalid mopp data.\n");

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
