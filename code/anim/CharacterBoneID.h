/***************************************************************************************************
*
*	Defines the enum for character bone indices.
*
***************************************************************************************************/

#ifndef CHARACTERBONEID_H_
#define CHARACTERBONEID_H_

/***************************************************************************************************
*
*	ENUMERATION		Character Bones
*
*	DESCRIPTION		This defines the list of bones that are shared between all character models. 
*					The enumerated values are used to directly index into lookup tables to allow
*					full separation of transform names and their associated hierarchy array indices.	
*
***************************************************************************************************/
enum	CHARACTER_BONE_ID
{
	CHARACTER_BONE_INVALID = -1,									//	- Invalid - 

	// ----- High Detail -----

	CHARACTER_BONE_HIGH_DETAIL_START,

	CHARACTER_BONE_ROOT = CHARACTER_BONE_HIGH_DETAIL_START,			//0	'root' 
	CHARACTER_BONE_PELVIS,											//1	'pelvis'
	CHARACTER_BONE_SPINE_00,										//2	'spine_00'
	CHARACTER_BONE_SPINE_01,										//3	'spine_01'
	CHARACTER_BONE_SPINE_02,										//4	'spine_02'
	CHARACTER_BONE_NECK,											//5	'neck'
	CHARACTER_BONE_HEAD,											//6	'head'
	CHARACTER_BONE_HIPS,											//7	'hips'

	CHARACTER_BONE_L_SHOULDER,										//8	'l_shoulder'
	CHARACTER_BONE_L_ARM,											//9	'l_arm'
	CHARACTER_BONE_L_ELBOW,											//10	'l_elbow'
	CHARACTER_BONE_L_WRIST,											//11	'l_wrist'
	CHARACTER_BONE_L_WEAPON,										//12	'l_weapon'

	CHARACTER_BONE_L_LEG,											//13	'l_leg'
	CHARACTER_BONE_L_KNEE,											//14	'l_knee'

	CHARACTER_BONE_R_SHOULDER,										//15	'r_shoulder'
	CHARACTER_BONE_R_ARM,											//16	'r_arm'
	CHARACTER_BONE_R_ELBOW,											//17	'r_elbow'
	CHARACTER_BONE_R_WRIST,											//18	'r_wrist'
	CHARACTER_BONE_R_WEAPON,										//19	'r_weapon'

	CHARACTER_BONE_R_LEG,											//20	'r_leg'
	CHARACTER_BONE_R_KNEE,											//21	'r_knee'

	// ----- Medium Detail -----

	CHARACTER_BONE_MEDIUM_DETAIL_START,

	CHARACTER_BONE_L_ANKLE = CHARACTER_BONE_MEDIUM_DETAIL_START,	//	'l_ankle'
	CHARACTER_BONE_L_BALL,											//	'l_ball'

	CHARACTER_BONE_L_THUMB_00,										//	'l_thumb_00'
	CHARACTER_BONE_L_THUMB_01,										//	'l_thumb_01'
	CHARACTER_BONE_L_THUMB_02,										//	'l_thumb_02'
	CHARACTER_BONE_L_MIDDLE_00,										//	'l_middle_00'
	CHARACTER_BONE_L_MIDDLE_01,										//	'l_middle_01'
	CHARACTER_BONE_L_MIDDLE_02,										//	'l_middle_02'

	CHARACTER_BONE_R_ANKLE,											//	'r_ankle'
	CHARACTER_BONE_R_BALL,											//	'r_ball'

	CHARACTER_BONE_R_THUMB_00,										//	'r_thumb_00'
	CHARACTER_BONE_R_THUMB_01,										//	'r_thumb_01'
	CHARACTER_BONE_R_THUMB_02,										//	'r_thumb_02'

	CHARACTER_BONE_R_MIDDLE_00,										//	'r_middle_00'
	CHARACTER_BONE_R_MIDDLE_01,										//	'r_middle_01'
	CHARACTER_BONE_R_MIDDLE_02,										//	'r_middle_02'

	// ----- Low Detail -----

	CHARACTER_BONE_LOW_DETAIL_START,

	CHARACTER_BONE_L_INDEX_00 = CHARACTER_BONE_LOW_DETAIL_START,	//	'l_index_00'
	CHARACTER_BONE_L_INDEX_01,										//	'l_index_01'
	CHARACTER_BONE_L_INDEX_02,										//	'l_index_02'
	CHARACTER_BONE_L_RING_00,										//	'l_ring_00'
	CHARACTER_BONE_L_RING_01,										//	'l_ring_01'
	CHARACTER_BONE_L_RING_02,										//	'l_ring_02'
	CHARACTER_BONE_L_PINKY_00,										//	'l_pinky_00'
	CHARACTER_BONE_L_PINKY_01,										//	'l_pinky_01'
	CHARACTER_BONE_L_PINKY_02,										//	'l_pinky_02'

	CHARACTER_BONE_R_INDEX_00,										//	'r_index_00'
	CHARACTER_BONE_R_INDEX_01,										//	'r_index_01'
	CHARACTER_BONE_R_INDEX_02,										//	'r_index_02'
	CHARACTER_BONE_R_RING_00,										//	'r_ring_00'
	CHARACTER_BONE_R_RING_01,										//	'r_ring_01'
	CHARACTER_BONE_R_RING_02,										//	'r_ring_02'
	CHARACTER_BONE_R_PINKY_00,										//	'r_pinky_00'
	CHARACTER_BONE_R_PINKY_01,										//	'r_pinky_01'
	CHARACTER_BONE_R_PINKY_02,										//	'r_pinky_02'

	// ----- Character Specific -----

	CHARACTER_BONE_USER_START,

	NUM_CHARACTER_BONES = 64,
};

#endif // !CHARACTERBONEID_H_

