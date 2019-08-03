//------------------------------------------------------
//!
//!	\file core\typeof.h
//!
//------------------------------------------------------
#if !defined(CORE_TYPEOF_H)
#define CORE_TYPEOF_H

#include "objectdatabase/gameguid.h"
#include "core/explicittemplate.h"


// Portable "typeof" operator
//
// Written by Bill Gibbons 2/18/2000
// Modified by Deano on 26/10/2004 add string typename
//
// This example uses function overloading and template specialization
// to implement a restricted form of the "typeof" operator.
//
// Each type for which "typeof" must work must be registered with
// the REGISTER_TYPEOF macro, which generates the required template
// specialization and overloaded function declaration.
//
// An ordinal 1..n is assigned to each type and used to pass type
// information by encoding the type as a number (in an array size)
// and using "sizeof" to extract the value as a constant.
//==================== The "typeof" machinery ====================
template<int N> struct typeof_class;
template<class T> struct WrapType { typedef T U; };

#define REGISTER_TYPEOF_AS(N, T, T2)												\
	template<> struct typeof_class< N >										\
	{																		\
		typedef WrapType< T >::U V;											\
		static const char* typeof_nameFUNC()								\
		{																	\
			return T2;														\
		}																	\
	};																		\
	char (*typeof_fct(const WrapType< T >::U &))[ N ];

#define typeof(x) typeof_class<sizeof(*typeof_fct(x))>::V
#define typeof_name(x) typeof_class<sizeof(*typeof_fct(x))>::typeof_nameFUNC()
#define typeof_num(x) sizeof(*typeof_fct(x))

#define typeof_num_declare(x) typeof_class<x>::V


//================

//======== annoying stuff to get Chunks and lists working ========
typedef ntstd::List<void*, Mem::MC_GFX>				TypeOfGfxVoidStarList;
typedef ntstd::List<void*, Mem::MC_MISC>			TypeOfMiscVoidStarList;
typedef ntstd::List<void*, Mem::MC_ODB>				TypeOfOdbVoidStarList;
typedef ntstd::List<void*, Mem::MC_AI>				TypeOfAiVoidStarList;
typedef ntstd::List<void*, Mem::MC_LOADER>			TypeOfLoaderVoidStarList;
typedef ntstd::List<void*, Mem::MC_ARMY>			TypeOfArmyVoidStarList;
typedef ntstd::List<void*, Mem::MC_LUA>				TypeOfLuaVoidStarList;
typedef ntstd::List<void*, Mem::MC_ENTITY>			TypeOfEntityVoidStarList;
typedef ntstd::List<void*, Mem::MC_CAMERA>			TypeOfCameraVoidStarList;
typedef ntstd::List<void*, Mem::MC_ANIMATION>		TypeOfAnimationVoidStarList;
typedef ntstd::List<void*, Mem::MC_EFFECTS>			TypeOfEffectsVoidStarList;
typedef ntstd::List<void*, Mem::MC_PROCEDURAL>		TypeOfProceduralVoidStarList;
typedef ntstd::List<void*, Mem::MC_HAVOK>			TypeOfHavokVoidStarList;
//================                                  
typedef ntstd::List<int, Mem::MC_GFX>				TypeOfGfxIntList;
typedef ntstd::List<int, Mem::MC_MISC>				TypeOfMiscIntList;
typedef ntstd::List<int, Mem::MC_ODB>				TypeOfOdbIntList;
typedef ntstd::List<int, Mem::MC_AI>				TypeOfAiIntList;
typedef ntstd::List<int, Mem::MC_LOADER>			TypeOfLoaderIntList;
typedef ntstd::List<int, Mem::MC_ARMY>				TypeOfArmyIntList;
typedef ntstd::List<int, Mem::MC_LUA>				TypeOfLuaIntList;
typedef ntstd::List<int, Mem::MC_ENTITY>			TypeOfEntityIntList;
typedef ntstd::List<int, Mem::MC_CAMERA>			TypeOfCameraIntList;
typedef ntstd::List<int, Mem::MC_ANIMATION>			TypeOfAnimationIntList;
typedef ntstd::List<int, Mem::MC_EFFECTS>			TypeOfEffectsIntList;
typedef ntstd::List<int, Mem::MC_PROCEDURAL>		TypeOfProceduralIntList;
typedef ntstd::List<int, Mem::MC_HAVOK>				TypeOfHavokIntList;
//================                                  
typedef ntstd::List<float, Mem::MC_GFX>				TypeOfGfxFloatList;
typedef ntstd::List<float, Mem::MC_MISC>			TypeOfMiscFloatList;
typedef ntstd::List<float, Mem::MC_ODB>				TypeOfOdbFloatList;
typedef ntstd::List<float, Mem::MC_AI>				TypeOfAiFloatList;
typedef ntstd::List<float, Mem::MC_LOADER>			TypeOfLoaderFloatList;
typedef ntstd::List<float, Mem::MC_ARMY>			TypeOfArmyFloatList;
typedef ntstd::List<float, Mem::MC_LUA>				TypeOfLuaFloatList;
typedef ntstd::List<float, Mem::MC_ENTITY>			TypeOfEntityFloatList;
typedef ntstd::List<float, Mem::MC_CAMERA>			TypeOfCameraFloatList;
typedef ntstd::List<float, Mem::MC_ANIMATION>		TypeOfAnimationFloatList;
typedef ntstd::List<float, Mem::MC_EFFECTS>			TypeOfEffectsFloatList;
typedef ntstd::List<float, Mem::MC_PROCEDURAL>		TypeOfProceduralFloatList;
typedef ntstd::List<float, Mem::MC_HAVOK>			TypeOfHavokFloatList;
//================                                  
typedef ntstd::List<bool, Mem::MC_GFX>				TypeOfGfxBoolList;
typedef ntstd::List<bool, Mem::MC_MISC>				TypeOfMiscBoolList;
typedef ntstd::List<bool, Mem::MC_ODB>				TypeOfOdbBoolList;
typedef ntstd::List<bool, Mem::MC_AI>				TypeOfAiBoolList;
typedef ntstd::List<bool, Mem::MC_LOADER>			TypeOfLoaderBoolList;
typedef ntstd::List<bool, Mem::MC_ARMY>				TypeOfArmyBoolList;
typedef ntstd::List<bool, Mem::MC_LUA>				TypeOfLuaBoolList;
typedef ntstd::List<bool, Mem::MC_ENTITY>			TypeOfEntityBoolList;
typedef ntstd::List<bool, Mem::MC_CAMERA>			TypeOfCameraBoolList;
typedef ntstd::List<bool, Mem::MC_ANIMATION>		TypeOfAnimationBoolList;
typedef ntstd::List<bool, Mem::MC_EFFECTS>			TypeOfEffectsBoolList;
typedef ntstd::List<bool, Mem::MC_PROCEDURAL>		TypeOfProceduralBoolList;
typedef ntstd::List<bool, Mem::MC_HAVOK>			TypeOfHavokBoolList;
//================                                  
typedef ntstd::List<GameGUID, Mem::MC_GFX>			TypeOfGfxGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_MISC>			TypeOfMiscGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_ODB>			TypeOfOdbGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_AI>			TypeOfAiGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_LOADER>		TypeOfLoaderGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_ARMY>			TypeOfArmyGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_LUA>			TypeOfLuaGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_ENTITY>		TypeOfEntityGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_CAMERA>		TypeOfCameraGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_ANIMATION>	TypeOfAnimationGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_EFFECTS>		TypeOfEffectsGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_PROCEDURAL>	TypeOfProceduralGameGUIDList;
typedef ntstd::List<GameGUID, Mem::MC_HAVOK>		TypeOfHavokGameGUIDList;
//================
typedef ntstd::List<class DataObject*, Mem::MC_GFX>			TypeOfGfxDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_MISC>		TypeOfMiscDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_ODB>			TypeOfOdbDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_AI>			TypeOfAiDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_LOADER>		TypeOfLoaderDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_ARMY>		TypeOfArmyDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_LUA>			TypeOfLuaDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_ENTITY>		TypeOfEntityDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_CAMERA>		TypeOfCameraDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_ANIMATION>	TypeOfAnimationDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_EFFECTS>		TypeOfEffectsDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_PROCEDURAL>	TypeOfProceduralDataObjectList;
typedef ntstd::List<class DataObject*, Mem::MC_HAVOK>		TypeOfHavokDataObjectList;
//================
typedef ntstd::List<CHashedString, Mem::MC_GFX>			TypeOfGfxStringList;
typedef ntstd::List<CHashedString, Mem::MC_MISC>		TypeOfMiscStringList;
typedef ntstd::List<CHashedString, Mem::MC_ODB>			TypeOfOdbStringList;
typedef ntstd::List<CHashedString, Mem::MC_AI>			TypeOfAiStringList;
typedef ntstd::List<CHashedString, Mem::MC_LOADER>		TypeOfLoaderStringList;
typedef ntstd::List<CHashedString, Mem::MC_ARMY>		TypeOfArmyStringList;
typedef ntstd::List<CHashedString, Mem::MC_LUA>			TypeOfLuaStringList;
typedef ntstd::List<CHashedString, Mem::MC_ENTITY>		TypeOfEntityStringList;
typedef ntstd::List<CHashedString, Mem::MC_CAMERA>		TypeOfCameraStringList;
typedef ntstd::List<CHashedString, Mem::MC_ANIMATION>	TypeOfAnimationStringList;
typedef ntstd::List<CHashedString, Mem::MC_EFFECTS>		TypeOfEffectsStringList;
typedef ntstd::List<CHashedString, Mem::MC_PROCEDURAL>	TypeOfProceduralStringList;
typedef ntstd::List<CHashedString, Mem::MC_HAVOK>		TypeOfHavokStringList;
//================
typedef ntstd::List<uint32_t, Mem::MC_GFX>				TypeOfGfxUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_MISC>				TypeOfMiscUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_ODB>				TypeOfOdbUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_AI>				TypeOfAiUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_LOADER>			TypeOfLoaderUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_ARMY>				TypeOfArmyUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_LUA>				TypeOfLuaUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_ENTITY>			TypeOfEntityUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_CAMERA>			TypeOfCameraUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_ANIMATION>		TypeOfAnimationUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_EFFECTS>			TypeOfEffectsUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_PROCEDURAL>		TypeOfProceduralUint32_tList;
typedef ntstd::List<uint32_t, Mem::MC_HAVOK>			TypeOfHavokUint32_tList;
//================                                  	
typedef ntstd::Vector<CVector, Mem::MC_GFX>				TypeOfGfxCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_MISC>			TypeOfMiscCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_ODB>				TypeOfOdbCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_AI>				TypeOfAiCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_LOADER>			TypeOfLoaderCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_ARMY>			TypeOfArmyCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_LUA>				TypeOfLuaCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_ENTITY>			TypeOfEntityCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_CAMERA>			TypeOfCameraCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_ANIMATION>		TypeOfAnimationCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_EFFECTS>			TypeOfEffectsCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_PROCEDURAL>		TypeOfProceduralCVectorList;
typedef ntstd::Vector<CVector, Mem::MC_HAVOK>			TypeOfHavokCVectorList;
//================                                  	
typedef ntstd::Vector<CPoint, Mem::MC_GFX>				TypeOfGfxCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_MISC>				TypeOfMiscCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_ODB>				TypeOfOdbCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_AI>				TypeOfAiCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_LOADER>			TypeOfLoaderCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_ARMY>				TypeOfArmyCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_LUA>				TypeOfLuaCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_ENTITY>			TypeOfEntityCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_CAMERA>			TypeOfCameraCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_ANIMATION>		TypeOfAnimationCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_EFFECTS>			TypeOfEffectsCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_PROCEDURAL>		TypeOfProceduralCPointList;
typedef ntstd::Vector<CPoint, Mem::MC_HAVOK>			TypeOfHavokCPointList;
//================
typedef ntstd::List<CKeyString, Mem::MC_GFX>			TypeOfGfxKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_MISC>			TypeOfMiscKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_ODB>			TypeOfOdbKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_AI>				TypeOfAiKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_LOADER>			TypeOfLoaderKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_ARMY>			TypeOfArmyKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_LUA>			TypeOfLuaKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_ENTITY>			TypeOfEntityKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_CAMERA>			TypeOfCameraKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_ANIMATION>		TypeOfAnimationKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_EFFECTS>		TypeOfEffectsKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_PROCEDURAL>		TypeOfProceduralKeyStringList;
typedef ntstd::List<CKeyString, Mem::MC_HAVOK>			TypeOfHavokKeyStringList;	

//======== Registration of types to be used with "typeof" ========

REGISTER_TYPEOF_AS( 1, char,								"char" )
REGISTER_TYPEOF_AS( 2, signed char,							"signed char" )
REGISTER_TYPEOF_AS( 3, unsigned char,						"unsigned char" )
REGISTER_TYPEOF_AS( 4, short,								"short" )
REGISTER_TYPEOF_AS( 5, unsigned short,						"unsigned short" )
REGISTER_TYPEOF_AS( 6, int,									"int" )
REGISTER_TYPEOF_AS( 7, unsigned int,						"unsigned int" )
REGISTER_TYPEOF_AS( 8, int64_t,								"int64" )
REGISTER_TYPEOF_AS( 9, uint64_t,							"uint64" )
REGISTER_TYPEOF_AS( 10, float,								"float" )
REGISTER_TYPEOF_AS( 11, double,								"double" )
REGISTER_TYPEOF_AS( 12, bool,								"bool" )
REGISTER_TYPEOF_AS( 13, ntstd::String,						"string" )
REGISTER_TYPEOF_AS( 14, GameGUID,							"GameGUID" )
REGISTER_TYPEOF_AS( 15, void*,								"void*" )
REGISTER_TYPEOF_AS( 16, CHashedString,						"hash" )
REGISTER_TYPEOF_AS( 17, CPoint,								"point" )
REGISTER_TYPEOF_AS( 18, CVector,							"fvector" )
REGISTER_TYPEOF_AS( 19, CQuat,								"quat" )
REGISTER_TYPEOF_AS( 20, CDirection,							"direction" )
REGISTER_TYPEOF_AS( 21, CKeyString,						    "KeyString" )
// inplace REGISTER_TYPEOF_AS( 22, CFlipFlop,				"flipflop" )
// inplace REGISTER_TYPEOF_AS( 23, DeepContainer,			"DeepContainer" )

// for list we have to register every combination of type and chunk and we 
// have to use a typedef cause we can't have commas in macro <sigh>
REGISTER_TYPEOF_AS( 24, TypeOfGfxVoidStarList,		"std::list<void*>" )
REGISTER_TYPEOF_AS( 25, TypeOfMiscVoidStarList,		"std::list<void*>" )
REGISTER_TYPEOF_AS( 26, TypeOfOdbVoidStarList,		"std::list<void*>" )
REGISTER_TYPEOF_AS( 27, TypeOfAiVoidStarList,		"std::list<void*>" )
REGISTER_TYPEOF_AS( 28, TypeOfLoaderVoidStarList,	"std::list<void*>" )
REGISTER_TYPEOF_AS( 29, TypeOfArmyVoidStarList,		"std::list<void*>" )
REGISTER_TYPEOF_AS( 30, TypeOfLuaVoidStarList,		"std::list<void*>" )
REGISTER_TYPEOF_AS( 31, TypeOfEntityVoidStarList,	"std::list<void*>" )
REGISTER_TYPEOF_AS( 32, TypeOfCameraVoidStarList,	"std::list<void*>" )
//================
REGISTER_TYPEOF_AS( 34, TypeOfGfxIntList,		"std::list<int>" )
REGISTER_TYPEOF_AS( 35, TypeOfMiscIntList,		"std::list<int>" )
REGISTER_TYPEOF_AS( 36, TypeOfOdbIntList,		"std::list<int>" )
REGISTER_TYPEOF_AS( 37, TypeOfAiIntList,		"std::list<int>" )
REGISTER_TYPEOF_AS( 38, TypeOfLoaderIntList,	"std::list<int>" )
REGISTER_TYPEOF_AS( 39, TypeOfArmyIntList,		"std::list<int>" )
REGISTER_TYPEOF_AS( 40, TypeOfLuaIntList,		"std::list<int>" )
REGISTER_TYPEOF_AS( 41, TypeOfEntityIntList,	"std::list<int>" )
REGISTER_TYPEOF_AS( 42, TypeOfCameraIntList,	"std::list<int>" )
//================
REGISTER_TYPEOF_AS( 44, TypeOfGfxFloatList,		"std::list<float>" )
REGISTER_TYPEOF_AS( 45, TypeOfMiscFloatList,	"std::list<float>" )
REGISTER_TYPEOF_AS( 46, TypeOfOdbFloatList,		"std::list<float>" )
REGISTER_TYPEOF_AS( 47, TypeOfAiFloatList,		"std::list<float>" )
REGISTER_TYPEOF_AS( 48, TypeOfLoaderFloatList,	"std::list<float>" )
REGISTER_TYPEOF_AS( 49, TypeOfArmyFloatList,	"std::list<float>" )
REGISTER_TYPEOF_AS( 50, TypeOfLuaFloatList,		"std::list<float>" )
REGISTER_TYPEOF_AS( 51, TypeOfEntityFloatList,	"std::list<float>" )
REGISTER_TYPEOF_AS( 52, TypeOfCameraFloatList,	"std::list<float>" )
//================
REGISTER_TYPEOF_AS( 54, TypeOfGfxBoolList,		"std::list<bool>" )
REGISTER_TYPEOF_AS( 55, TypeOfMiscBoolList,		"std::list<bool>" )
REGISTER_TYPEOF_AS( 56, TypeOfOdbBoolList,		"std::list<bool>" )
REGISTER_TYPEOF_AS( 57, TypeOfAiBoolList,		"std::list<bool>" )
REGISTER_TYPEOF_AS( 58, TypeOfLoaderBoolList,	"std::list<bool>" )
REGISTER_TYPEOF_AS( 59, TypeOfArmyBoolList,		"std::list<bool>" )
REGISTER_TYPEOF_AS( 60, TypeOfLuaBoolList,		"std::list<bool>" )
REGISTER_TYPEOF_AS( 61, TypeOfEntityBoolList,	"std::list<bool>" )
REGISTER_TYPEOF_AS( 62, TypeOfCameraBoolList,	"std::list<bool>" )
//================
REGISTER_TYPEOF_AS( 64, TypeOfGfxGameGUIDList,		"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 65, TypeOfMiscGameGUIDList,		"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 66, TypeOfOdbGameGUIDList,		"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 67, TypeOfAiGameGUIDList,		"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 68, TypeOfLoaderGameGUIDList,	"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 69, TypeOfArmyGameGUIDList,		"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 70, TypeOfLuaGameGUIDList,		"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 71, TypeOfEntityGameGUIDList,	"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 72, TypeOfCameraGameGUIDList,	"std::list<GameGUID>" )
//================
REGISTER_TYPEOF_AS( 74, TypeOfGfxDataObjectList,		"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 75, TypeOfMiscDataObjectList,		"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 76, TypeOfOdbDataObjectList,		"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 77, TypeOfAiDataObjectList,			"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 78, TypeOfLoaderDataObjectList,		"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 79, TypeOfArmyDataObjectList,		"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 80, TypeOfLuaDataObjectList,		"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 81, TypeOfEntityDataObjectList,		"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 82, TypeOfCameraDataObjectList,		"std::list< class DataObject*>" )
//================
REGISTER_TYPEOF_AS( 84, TypeOfGfxStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 85, TypeOfMiscStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 86, TypeOfOdbStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 87, TypeOfAiStringList,			"std::list<string>" )
REGISTER_TYPEOF_AS( 88, TypeOfLoaderStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 89, TypeOfArmyStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 90, TypeOfLuaStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 91, TypeOfEntityStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 92, TypeOfCameraStringList,		"std::list<string>" )
//================
REGISTER_TYPEOF_AS( 94, TypeOfGfxUint32_tList,		"std::list<uint32>" )
REGISTER_TYPEOF_AS( 95, TypeOfMiscUint32_tList,		"std::list<uint32>" )
REGISTER_TYPEOF_AS( 96, TypeOfOdbUint32_tList,		"std::list<uint32>" )
REGISTER_TYPEOF_AS( 97, TypeOfAiUint32_tList,		"std::list<uint32>" )
REGISTER_TYPEOF_AS( 98, TypeOfLoaderUint32_tList,	"std::list<uint32>" )
REGISTER_TYPEOF_AS( 99, TypeOfArmyUint32_tList,		"std::list<uint32>" )
REGISTER_TYPEOF_AS( 100, TypeOfLuaUint32_tList,		"std::list<uint32>" )
REGISTER_TYPEOF_AS( 101, TypeOfEntityUint32_tList,	"std::list<uint32>" )
REGISTER_TYPEOF_AS( 102, TypeOfCameraUint32_tList,	"std::list<uint32>" )
//================
REGISTER_TYPEOF_AS( 104, TypeOfGfxCVectorList,		"std::list<fvector>" )
REGISTER_TYPEOF_AS( 105, TypeOfMiscCVectorList,		"std::list<fvector>" )
REGISTER_TYPEOF_AS( 106, TypeOfOdbCVectorList,		"std::list<fvector>" )
REGISTER_TYPEOF_AS( 107, TypeOfAiCVectorList,		"std::list<fvector>" )
REGISTER_TYPEOF_AS( 108, TypeOfLoaderCVectorList,	"std::list<fvector>" )
REGISTER_TYPEOF_AS( 109, TypeOfArmyCVectorList,		"std::list<fvector>" )
REGISTER_TYPEOF_AS( 110, TypeOfLuaCVectorList,		"std::list<fvector>" )
REGISTER_TYPEOF_AS( 111, TypeOfEntityCVectorList,	"std::list<fvector>" )
REGISTER_TYPEOF_AS( 112, TypeOfCameraCVectorList,	"std::list<fvector>" )
//================
REGISTER_TYPEOF_AS( 114, TypeOfGfxCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 115, TypeOfMiscCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 116, TypeOfOdbCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 117, TypeOfAiCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 118, TypeOfLoaderCPointList,	"std::list<point>" )
REGISTER_TYPEOF_AS( 119, TypeOfArmyCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 120, TypeOfLuaCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 121, TypeOfEntityCPointList,	"std::list<point>" )
REGISTER_TYPEOF_AS( 122, TypeOfCameraCPointList,	"std::list<point>" )
//================
REGISTER_TYPEOF_AS( 154, TypeOfGfxKeyStringList,		"std::list<KeyString>" )
REGISTER_TYPEOF_AS( 155, TypeOfMiscKeyStringList,	"std::list<KeyString>" )
REGISTER_TYPEOF_AS( 156, TypeOfOdbKeyStringList,		"std::list<KeyString>" )
REGISTER_TYPEOF_AS( 157, TypeOfAiKeyStringList,		"std::list<KeyString>" )
REGISTER_TYPEOF_AS( 158, TypeOfLoaderKeyStringList,	"std::list<KeyString>" )
REGISTER_TYPEOF_AS( 159, TypeOfArmyKeyStringList,	"std::list<KeyString>" )
REGISTER_TYPEOF_AS( 160, TypeOfLuaKeyStringList,		"std::list<KeyString>" )
REGISTER_TYPEOF_AS( 161, TypeOfEntityKeyStringList,	"std::list<KeyString>" )
REGISTER_TYPEOF_AS( 162, TypeOfCameraKeyStringList,	"std::list<KeyString>" )

// Swapped to this so that adding chunks is less painful!
REGISTER_TYPEOF_AS( 33, 	TypeOfAnimationVoidStarList,	"std::list<void*>" )
REGISTER_TYPEOF_AS( 43, 	TypeOfAnimationIntList,			"std::list<int>" )
REGISTER_TYPEOF_AS( 53, 	TypeOfAnimationFloatList,		"std::list<float>" )
REGISTER_TYPEOF_AS( 63, 	TypeOfAnimationBoolList,		"std::list<bool>" )
REGISTER_TYPEOF_AS( 73, 	TypeOfAnimationGameGUIDList,	"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 83, 	TypeOfAnimationDataObjectList,	"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 93, 	TypeOfAnimationStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 103,	TypeOfAnimationUint32_tList,	"std::list<uint32>" )
REGISTER_TYPEOF_AS( 113,	TypeOfAnimationCVectorList,		"std::list<fvector>" )
REGISTER_TYPEOF_AS( 123,	TypeOfAnimationCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 163,	TypeOfAnimationKeyStringList,	"std::list<KeyString>" )


REGISTER_TYPEOF_AS( 124, 	TypeOfEffectsVoidStarList,		"std::list<void*>" )
REGISTER_TYPEOF_AS( 125, 	TypeOfEffectsIntList,			"std::list<int>" )
REGISTER_TYPEOF_AS( 126, 	TypeOfEffectsFloatList,			"std::list<float>" )
REGISTER_TYPEOF_AS( 127, 	TypeOfEffectsBoolList,			"std::list<bool>" )
REGISTER_TYPEOF_AS( 128, 	TypeOfEffectsGameGUIDList,		"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 129, 	TypeOfEffectsDataObjectList,	"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 130, 	TypeOfEffectsStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 131,	TypeOfEffectsUint32_tList,		"std::list<uint32>" )
REGISTER_TYPEOF_AS( 132,	TypeOfEffectsCVectorList,		"std::list<fvector>" )
REGISTER_TYPEOF_AS( 133,	TypeOfEffectsCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 164,	TypeOfEffectsKeyStringList,		"std::list<KeyString>" )

REGISTER_TYPEOF_AS( 134, 	TypeOfProceduralVoidStarList,	"std::list<void*>" )
REGISTER_TYPEOF_AS( 135, 	TypeOfProceduralIntList,		"std::list<int>" )
REGISTER_TYPEOF_AS( 136, 	TypeOfProceduralFloatList,		"std::list<float>" )
REGISTER_TYPEOF_AS( 137, 	TypeOfProceduralBoolList,		"std::list<bool>" )
REGISTER_TYPEOF_AS( 138, 	TypeOfProceduralGameGUIDList,	"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 139, 	TypeOfProceduralDataObjectList,	"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 140, 	TypeOfProceduralStringList,		"std::list<string>" )
REGISTER_TYPEOF_AS( 141,	TypeOfProceduralUint32_tList,	"std::list<uint32>" )
REGISTER_TYPEOF_AS( 142,	TypeOfProceduralCVectorList,	"std::list<fvector>" )
REGISTER_TYPEOF_AS( 143,	TypeOfProceduralCPointList,		"std::list<point>" )
REGISTER_TYPEOF_AS( 165,	TypeOfProceduralKeyStringList,	"std::list<KeyString>" )


REGISTER_TYPEOF_AS( 144, 	TypeOfHavokVoidStarList,		"std::list<void*>" )
REGISTER_TYPEOF_AS( 145, 	TypeOfHavokIntList,				"std::list<int>" )
REGISTER_TYPEOF_AS( 146, 	TypeOfHavokFloatList,			"std::list<float>" )
REGISTER_TYPEOF_AS( 147, 	TypeOfHavokBoolList,			"std::list<bool>" )
REGISTER_TYPEOF_AS( 148, 	TypeOfHavokGameGUIDList,		"std::list<GameGUID>" )
REGISTER_TYPEOF_AS( 149, 	TypeOfHavokDataObjectList,		"std::list< class DataObject*>" )
REGISTER_TYPEOF_AS( 150, 	TypeOfHavokStringList,			"std::list<string>" )
REGISTER_TYPEOF_AS( 151,	TypeOfHavokUint32_tList,		"std::list<uint32>" )
REGISTER_TYPEOF_AS( 152,	TypeOfHavokCVectorList,			"std::list<fvector>" )
REGISTER_TYPEOF_AS( 153,	TypeOfHavokCPointList,			"std::list<point>" )
REGISTER_TYPEOF_AS( 166,	TypeOfHavokKeyStringList,		"std::list<KeyString>" )

// always update NUM_TYPEOF_MACROS

#define NUM_TYPEOF_MACROS 166

// Franks dodgyness!!! he hasn't implemented the full set of typeof operators so 
// these have to go after NUM_TYPEOF_MACROS GRRRR!!!!! Speedtree is being rewritten
// once it is kill these
REGISTER_TYPEOF_AS( 200, Vec2, "Vec2" )
REGISTER_TYPEOF_AS( 201, Vec3, "Vec3" )
REGISTER_TYPEOF_AS( 202, Vec4, "Vec4" )


// this is a not fast at all convertsion from the string to the typeof number
extern int ConvertTypeOfStringToNum( const char* pName );
// returns a new object of typeof_number type
extern void* CreateTypeOfType( int typeof_number );
// returns the size in Bytes of a typeof_num type
extern unsigned int ReturnTypeOfSize( int typeof_number );

// this takes a void* pointer of the correct size, an input stream and a typeof_number
// and converts the string into native format stored the pointer. 
// returns the pointer
extern void* ConvertIStreamToType( ntstd::Istream &is, int typeof_number, void* data );

template<typename T>
inline int typeof_num_of_type()
{
	T dummy;
	return typeof_num(dummy);
}
#endif // CORE_TYPEOF_H
