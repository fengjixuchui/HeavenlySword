/***************************************************************************************************
*
*   $Header:: /game/VolumeTypes.h  $
*
*	Header for types of collision volume
*
*	NOTE - this file is shared between the main game and the exporter,
*	so lets be careful out there :-)
*
*	CREATED
*
*	05.12.2002	John	Created
*
***************************************************************************************************/

#ifndef _VOLUMETYPES_H
#define _VOLUMETYPES_H

enum COLLISION_VOLUME_TYPE
{
	CV_TYPE_OBB,
	CV_TYPE_SPHERE,
	CV_TYPE_TRIANGLE,
	CV_TYPE_TRIANGLEMESH,
	CV_TYPE_CAPSULE,
};

#endif	//_VOLUMETYPES_H
