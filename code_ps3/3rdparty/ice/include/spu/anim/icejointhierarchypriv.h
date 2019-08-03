/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_JOINTHIERARCHYPRIV_H
#define ICE_JOINTHIERARCHYPRIV_H

/*!
 * \file	icejointhierarchypriv.h
 * \brief	Internal structure of JointHierarchy data.
 */

namespace Ice 
{
    namespace Anim
    {
		const U32 kJointHierarchyMagicVersion1_03 = 0x4a480103;
		const U32 kJointHierarchyMagicVersion1_02 = 0x4a480102;
		const U32 kJointHierarchyMagicVersion1_01 = 0xd0db0101;
		const U32 kJointHierarchyMagicVersion1_00 = 0xd0dba11e;
		/*
		 * The JointHierarchy is a major structure for all animation-related data pertaining 
		 * to a specific model. It contains parenting data, but also set-driven key information,
		 * as well as constraint data. 
		 *
		 * Following is a complete description of the internal structure of the JointHierarchy data:
		 *
		struct JointHierarchy {
            U32           m_magic;                      // Magic word to identify structure version.
				//================================================================================
				//	Version 1.03	m_magic = 0x4a480103 = kJointHierarchyMagicVersion1_03
				//      Added support for float anim channels, extra animated transform 
				//      channels.  Added id tables for control inputs and outputs.
				//
				//	Version 1.02	m_magic = 0x4a480102 = kJointHierarchyMagicVersion1_02
				//		Added m_sdkControlsOffset for direct access to external sdk control
				//		default values.  Replaced unused m_defaultTableOffset (always 0).
				//		Fixed export of m_numScalars, m_numSdkControls (were always 0).
				//		Switched to new magic base 'JH\(version major)\(version minor)'.
				//
				//	Version 1.01	m_magic = 0xd0db0101 = kJointHierarchyMagicVersion1_01
				//		Inverse bind pose data is now stored as transposed 4x3 matrices, the 
				//		format read by the SPU task.
				//
				//	Version 1.00	m_magic = 0xd0dba11e = kJointHierarchyMagicVersion1_00
				//		Last version before adding version specific magic.
				//		Future versions should have the last two bytes of m_magic set to the major
				//		and minor version number of the JointHierarchy structure.
				//================================================================================
			U32           m_hierarchyId;				// Unique id assigned to this hierarchy.
            U32           m_totalSize;                  // Total size in bytes for this hierarchy.
            U16           m_numTotalJoints;             // Total number of joints in hierarchy.
			U16           m_numFloatChannels;			// Number of animateable float channels.

            U8            m_numSets;                    // Number of joint sets (max 2, currently).
			U8            pad;
			U16           m_numScalars;                 // Required number of scalar table entries to process sdk commands.
			U16           m_numSdkConstants;            // Number of sdk constants
			U16           m_numInputControls;			// Number of external input float controls.
			U16           m_numOutputControls;          // Number of external output float controls.
			U16           m_numInternalFloatChannels;	// Number of float channels that are consumed internally by sdks or constraints (which are always sorted to the front of the float channels)
			U8            m_numSdkCommands;             // Number of SDK command headers.
			U8            m_numConstraintCommands;      // Number of constraint command headers.
			U8            m_numChannelGroups;			// Number of blend processing groups == num joint groups + num float channel groups.  
														// Sets the number of ValidBits required to hold a pose and the number of 
														// JointFactors groups required for a blend operation.
			U8            m_numSdkToConstraintVecs;     // Number of extra qwords output at end of JointParam table for vector constraint inputs.

			U32           m_defaultPoseOffset;          // Offset from header to default pose for primary joint set (local SQTs to generate the bind pose).
			U32           m_defaultFloatChannelsOffset;	// Offset to default values for float channels; float[m_numFloatChannels]
			U32           m_secondaryDefaultPoseOffset; // Offset from header to packed default pose data for secondary joint set (local SQTs to generate the bind pose).
			U32           m_defaultInputControlsOffset; // Offset to default external input float control values; float[m_numInputControls]

			U32           m_sdkCommandHeaderOffset;     // Offset from header to SDK command headers; SdkCommandHeader[m_numSdkCommands]
			U32           m_constraintCommandHeaderOffset; // Offset from header to constraint command headers; ConstraintCommandHeader[m_numConstraintCommands]
			U32           m_inverseBindPoseOffset;      // Offset from header to inverse bind pose matrices; Mat4x3[m_numTotalJoints]
			U32           m_debugParentInfoOffset;		// (DEBUG) Offset from header to DebugJointParentInfo[m_numTotalJoints].  0 if no debug info.

			U32           m_jointIdTableOffset;			// Offset to ids for output controls; U32[m_numTotalJoints]
			U32           m_floatChannelIdTableOffset;	// Offset to ids for output controls; U32[m_numFloatChannels]
			U32           m_inputControlIdTableOffset;	// Offset to ids for input controls; U32[m_numInputControls]
			U32           m_outputControlIdTableOffset;	// Offset to ids for output controls; U32[m_numOutputControls]

			U16           m_numAnimatedJoints;          // Number of joints driven by keyframe animation data (as opposed to SDK or constraint driven procedural joints)
			U16           m_numPrimaryJoints;           // Number of joints in first parenting (primary) set
			U16           m_numParentingQuads[2];       // Number of parenting quads in (primary) set 0 and (secondary) set 1.
			U32           m_parentingDataOffset[2];     // Offset from header to the joint parenting data for (primary) set 0 and (secondary) set 1.
			//====================================================================================
			// Following is data:
			//		Structure and details follow below.
			//		The size and order of entries in memory may change, hence offsets are
			//		provided to access the data.

			// Qword aligned data (padded as necessary):
			JointParams					defaultPose[ m_numTotalJoints ];			// @ ((U8*)hierarchy + m_defaultPoseOffset)
			float						defaultFloatChannels[m_numFloatChannels]; 	// @ ((U8*)hierarchy + m_defaultFloatChannelsOffset)
			JointParentingQuad			parentInfo[ m_numParentingQuads[0] ];		// @ ((U8*)hierarchy + m_parentingDataOffset[0])
			float						defaultInputControls[m_numInputControls]; 	// @ ((U8*)hierarchy + m_defaultInputControlsOffset)
			<variable format>			sdkData[m_numConstraintCommands];			// @ ((U8*)hierarchy + SdkCommandHeader[i].m_dataOffset)
				//type dependent data: see below										// @ ((U8*)hierarchy + m_sdkHeader[i].m_dataOffset)
			<variable format>			constraintData[m_numConstraintCommands];	// @ ((U8*)hierarchy + ConstraintCommandHeader[i].m_dataOffset)
				//type dependent data: see below										// @ ((U8*)hierarchy + m_constraintHeader[i].m_dataOffset)
				//ConstraintPatchData		patchData[header->m_numPatchItems];			// @ ((U8*)hierarchy + m_constraintHeader[i].m_dataOffset + m_constraintHeader[i].m_dataSize)
			SecondaryDefaultPoseHeader  secondaryDefaultPose;						// @ ((U8*)hierarchy + m_secondaryDefaultPoseOffset)
				UnpackVec4DestOctet			destRefs[ secondaryDefaultPose.m_sizeDefaultValues/sizeof(Vec4) / 8 ];	// @ ((U8*)hierarchy + m_secondaryDefaultPoseOffset + sizeof(SecondaryDefaultPoseHeader))
				Vec4						defaultValues[ m_sizeDefaultValues/sizeof(Vec4) ];						// @ ((U8*)hierarchy + m_secondaryDefaultPoseOffset + sizeof(SecondaryDefaultPoseHeader) + m_secondaryDefaultPose.m_sizeDefaultValues/sizeof(Vec4)/8 * sizeof(UnpackVec4DestOctet))
			JointParentingQuad			parentInfo[ m_numParentingQuads[1] ];		// @ ((U8*)hierarchy + m_parentingDataOffset[1])
			SMath::Mat34				inverseBindPose[ m_numTotalJoints ];		// @ ((U8*)hierarchy + m_inverseBindPoseOffset)

			// Unaligned data:
			SdkCommandHeader			sdkHeader[ m_numSdkCommands ];				// @ ((U8*)hierarchy + m_sdkSdkCommandHeaderOffset)
			ConstraintCommandHeader		constraintHeader[ m_numConstraintData ];	// @ ((U8*)hierarchy + m_constraintSdkCommandHeaderOffset)

			U32							jointIdTable[m_numTotalJoints];				// @ ((U8*)hierarchy + m_jointIdTableOffset)
			U32							floatChannelIdTable[m_numFloatChannels];	// @ ((U8*)hierarchy + m_floatChannelIdTableOffset)
			U32							inputControlIdTable[m_numInputControls];	// @ ((U8*)hierarchy + m_inputControlIdTableOffset)
			U32							outputControlIdTable[m_numOutputControls];	// @ ((U8*)hierarchy + m_outputControlIdTableOffset)
			DebugJointParentInfo		debugParentInfo[ m_numTotalJoints ];		// @ ((U8*)hierarchy + m_debugParentInfoOffset)
			//====================================================================================
		};
		 *
		 */

		//========================================================================================

		/*!
		 * Internal structure of parenting data used to pass driving data to Parenting().
		 *
		 * Stored in the JointHierarchy as hierarchy->m_numParentingQuads[0|1] sequential
		 * entries at hierarchy->m_parentingDataOffset[0|1].
		 *
		 * Stored as sets of 4 independent parenting operations (i.e. within a quad, no
		 * (m_jointRef &~0xF) is equal to any other (m_parentRef &~0xF).  In some circumstances,
		 * a hierarchy may not have enough branching to allow 4 parallel operations.  In these
		 * cases, a quad must be padded out with copies one of the other operations.
		 */
		struct JointParentingQuad {
			struct JointParentingData {
				U16 m_jointRef;	 //!< (joint index + 1) * 16
				U16 m_parentRef; //!< (parent joint index + 1) * 16 bit-wise or'd with the inheritScaleFlag.
			} m_elem[4];
		};

		//========================================================================================

		/*!
		 * This is a header for the data stored in the JointHierarchy at 
		 * hierarchy->m_secondaryDefaultPoseOffset, which defines the default values for joints in 
		 * the second joint set which are partially or fully unconstrained by SDKs or constraints.
		 */
		struct SecondaryDefaultPoseHeader {
			U32 m_numDefaultValues;
			U32 m_pad[3];
		//	UnpackVec4DestOctet		m_destRefs[ (m_numDefaultValues + 0x7) / 8 ];
		//	Vec4					m_defaultValues[ m_numDefaultValues ];
		};

		/*!
		 * Internal structure used to pass driving data to UnpackVec4Components().
		 *
		 * Considering m_dstRefs as one contiguous array of U16's, each m_dstRefs[i] is associated
		 * with m_defaultValues[i] and describes where to copy that Vec4 to and which components 
		 * to copy.
		 *
		 * If the number of default values is not a multiple of 8, the last UnpackVec4DestOctet in  
		 * m_dstRefs must be padded out with no-op values (0 in the lowest 4 bits).
		 * m_defaultValues[] does not need to be padded.
		 */
		struct UnpackVec4DestOctet
		{
			U16 m_dstRef[8];	// (dest_qword_index * 16) | (write_x << 3) | (write_y << 2) | (write_z << 1) | write_w
		};

		//========================================================================================

		/*!
		 * This is a header for sdk data stored in the JointHierarchy as hierarchy->m_numSdkCommands
		 * sequential entries at hierarchy->m_sdkCommandHeaderOffset.
		 */
		struct SdkCommandHeader {
			U16 m_cmd;						//!< The SDK command to run.
			U16 m_numItems;					//!< Number of SDK data items.
			U16 m_dataSize;					//!< Size in bytes for the data.
			U16 m_cmdData;					//!< Command specific data
			U32 m_dataOffset;				//!< Offset to the data.
		};

		/*!
		 * Internal constants used by header->m_sdkDataType to define which sdk operation the data
		 * that follows is formatted for.
		 */
		enum SdkDataType
		{
			//NOTE: These constants must match the same constants in the tools source file sdkwriter.cpp
			// and must match the order of kCmdSdk* commands in iceanimbatchpriv.h.
			kSdkType0 = 					0x10,
			kSdkDriversRot = kSdkType0,			// SdkQuaternionToEuler()	: quaternions -> Euler angles -> scalars
			kSdkDrivers,						// SdkLoadVecs()			: scales/translations -> vector components -> scalars
			kSdkCopyIn,							// SdkCopyInScalars()		: constants/runtime values -> scalars 
			kSdkDrivenRot,						// SdkEulerToQuaternion()	: scalars+defaults -> Euler angles -> quaternions
			kSdkDriven,							// SdkStoreVecs()			: scalars+defaults -> vectors -> scales/translations
			kSdkCopyOut,						// SdkCopyOutScalarTable()	: scalars -> output float table
			kSdkCopy,							// SdkCopyScalars()			: copies scalars around in scalar table
			kSdkRunBaked,						// SdkEvalBaked()			: scalar[i] = sampled_function( scalar[j] )
			kSdkRunSegs1,						// SdkEvalCurves()			: scalar[i] = piecewise_bezier_function( scalar(j] )
			kSdkRunSegs2,
			kSdkRunSegs3,
			kSdkRunSegs4,
			kSdkCopyIn_FromFloatChannels =	0x20 | kSdkCopyIn,	// SdkCopyInScalars from FloatChannels
		};

		/*!
		 * Internal enum used to specify Euler angle order, used by SDKs.
		 */
		enum RotationOrder
		{ 
			kRotOrderZXY, 
			kRotOrderZYX, 
			kRotOrderYXZ, 
			kRotOrderYZX, 
			kRotOrderXYZ,	// the default rotation order
			kRotOrderXZY,
		};

		/*!
		 * kSdkDriversRot data, to be passed to SdkQuaternionToEuler():
		 *
		 *	SdkDriversRotSourceData		sourceData[ header->m_numItems ];		// Driving data determining the source joint_index and which Euler values to extract
		 *	U16							destOffsets[ header->m_numItems ];		// The destination scalar_index * 0x10 (sizeof(Scalar))
		 *	Quat						jointOrients[ header->m_numItems ];		// The joint orientation of the source joint
		 *	Quat						rotationAxis[ header->m_numItems ];		// The rotation axis of the source joint
		 *
		 *	See the animation docs for a general description of the joint animation system and SDK principles.
		 *
		 *	header->m_numItems must be a multiple of 4.
		 *	sourceData must be padded out to a multiple of 4 with entries with writeMask 0.
		 *	destOffsets must be padded out to a multiple of 8 with any value (usually 0) to maintain quadword alignment of following data.
		 *  jointOrients and rotationAxis do not need padding.
		 */
		struct SdkDriversRotSourceQuad {
			struct Element {
				U8	m_rotOrder;				//!< (RotationOrder (one of kRotOrder*)) << 4
				U8	m_writeMask;			//!< ((write_x << 2) | (write_y << 1) | write_z) << 4
				U16 m_sourceOffset; 		//!< joint_index * 0x30 + 0x10
			} m_elem[4];
		};

		/*!
		 * kSdkDrivers data, to be passed to SdkLoadVecs():
		 *
		 *	SdkDriversQuad::Element		inputData[ header->m_numItems ];		// Driving data determining the source joint, destination scalar index, and which components to write
		 *
		 *	header->m_numItems must be a multiple of 4.
		 *	inputData should be padded with no-op elements (write_x == write_y == write_z == 0)
		 */
		struct SdkDriversQuad {
			struct Element {
				// Format: (srcIndex << 18) | (dstIndex << 4) | ((write_x << 2) | (write_y << 1) | write_z)
				//     Or: (srcOffset << 14) | dstOffset | ((write_x << 2) | (write_y << 1) | write_z)
				U32 m_srcDestAndMask;
			} m_elem[4];
		};

		/*!
		 * kSdkDrivenRot data, to be passed to SdkEulerToQuaternion().
		 *
		 *	SdkDrivenRotSourceQuad::Element		sourceData[ header->m_numItems ];		// driving data specifying source scalar and conversion to euler angles
		 *	SdkDrivenRotDestQuad::Element		destData[ header->m_numItems ];			// driving data specifying destination joint and default euler values
		 *	Quat						jointOrients[ header->m_numItems ];		// The joint orientation of the destination joint
		 *	Quat						rotationAxis[ header->m_numItems ];		// The rotation axis of the destination joint
		 *	Vec4						defaultValues[ ];						// default Euler angles - w component not used.
		 *
		 *	See the animation docs for a general description of the joint animation system and SDK principles.
		 *
		 *  header->m_numItems must be a multiple of 4.
		 *	Data must be padded with copies of another operation from the same quad.
		 *	jointOrients and rotationAxis do not need to be padded.
		 */
		struct SdkDrivenRotSourceQuad {
			struct Element {
				U8  m_rotOrder;			//!< (RotationOrder (one of kRotOrder*)) << 4
				U8  m_readMask;			//!< ((read_x << 2) | (read_y << 1) | read_z) << 4	- unread components will use defaultValues[ defaultOffset>>4 ]
				U16 m_sourceOffset;		//!< scalar_index * 0x10 (sizeof(Scalar))
			} m_elem[4];
		};
		struct SdkDrivenRotDestQuad {
			struct Element {
				U16 m_destOffset;		//!< dest_joint_index * 0x30 (sizeof(JointParams)) + 0x10 (m_quat offset)
				U16 m_defaultOffset;	//!< default_index * 0x10 (sizeof(Vec4))
			} m_elem[4];
		};

		/*!
		 * kSdkDriven data, to be passed to SdkStoreVecs().
		 *
		 *	SdkDrivenPair::Element		inputData[ header->m_numItems ];		// driving data specifying source scalar, read mask, destination vector, and default vector
		 *	Vec4						defaultValues[ ];						// default vector components - w component not used.
		 *
		 *	header->m_numItems must be a multiple of 2.
		 *	inputData should be padded with a copy of the other operation in the same quad.
		 */
		struct SdkDrivenPair {
			struct Element {
				//! Format: ((default_x<<22) | (default_y<<21) | (default_z<<20)) | (scalarIndex << 4)
				//!     Or: ((default_mask<<4) << 16) | scalarOffset
				U32 m_selectAndScalar;
				//! Format: (destIndex << 18) | (defaultIndex << 4)
				//!     Or: (destOffset << 14) | defaultOffset
				U32 m_destAndDefaultIndex;
			} m_elem[2];
		};

		/*! kSdkCopy data, to be passed to SdkCopyScalars():
		 *
		 *	A sequential list of source and destination offsets within the scalar table.
		 *
		 *	header->m_numItems must be a multiple of 4, with data padded out with copies
		 *	of the last operation.
		 */
		struct SdkCopyScalarsQuad {
			struct Element {
				U16 m_dstOffset;		//!< destination scalar index * 0x10
				U16 m_srcOffset;		//!< source scalar index * 0x10
			} m_elem[4];
		};


		/*!
		 * kSdkRunBaked data, to be passed to SdkEvalBaked():
		 *	Not implemented yet.
		 */

		/*!
		 * kSdkRunSegs* data, to be passed to SdkEvalCurves().
		 *
		 *	SdkRunSegsQuad::Element		inputData[ header->m_numItems ];		// driving data specifying input and output scalars, and modifiers on the operation
		 *	float						minInput[ header->m_numItems ];			// input scalar is clamped to this minimum value
		 *	float						maxInput[ header->m_numItems ];			// input scalar is clamped to this maximum value
		 *	float						keyValues[ header->m_numItems ][ numSegments-1 ];			// division values between piecewise bezier function segments - no data here if numSegments==1
		 *	Vec4						bezierCoefficients[ numSegments ][ header->m_numItems ];	// bezier coefficients for each segment of each function
		 *
		 *	For an input x, if x is between keyValues[ iSegment-1 ] and keyValues[ iSegment ] 
		 *	where we use minInput for keyValues[-1] and maxInput for keyValues[numSegments-1],
		 *	we have y = f(x) = ( ( c[iSegment].X() * x + c[iSegment].Y() ) * x + c[iSegment].Z() ) * x + c[iSegment].W().
		 *	We can optionally specify a sum flag:  y += f(x)
		 *	Or an accumulate flag: y = f_prev(x) + f(x) where f_prev(x) is the result of the evaluation of the 
		 *		function 4 data earlier (i.e. from inputData[i-4]).
		 *
		 *	header->m_numItems must be a multiple of 4.
		 *	inputData should be padded with safe entries, which apply a function with output to the safeScalar, 
		 *	scalar_index hierarchy->m_numScalars.
		 *	minInput, maxInput, and keyValues tables must be padded with any value (usually 0) for alignment.
		 *	bezierCoefficients does not need to be padded.
		 */
		struct SdkRunSegsQuad {
			struct Element {
				U16	m_srcOffset;			//!< source_scalar_index * 0x10 (sizeof(Scalar))
				U16	m_destOffsetAndFlags;	//!< dest_scalar_index * 0x10 (sizeof(Scalar))  | ((accumulate << 1) | sum)
			} m_elem[4];
		};

		//========================================================================================

		/*!
		 * This is a header for constraint data stored in the JointHierarchy as hierarchy->m_numConstraintCommands
		 * sequential entries at hierarchy->m_constraintCommandHeaderOffset.
		 */
		struct ConstraintCommandHeader {
			U16 m_cmd;						//!< The constraint command to run.
			U16 m_numItems;					//!< Number of constraint drivers/sources.
			U16 m_dataSize;					//!< Size in bytes of the input data.
			U16 m_cmdData;					//!< Command specific data
			U32 m_dataOffset;				//!< Offset to the data and patch data.
			U16 m_numPatchItems;			//!< Number of patch command items.
			U16 m_patchDataSize;			//!< Size in bytes of the patch data.
		};

		/*!
		 * Internal constants used by header->m_cmd to define which constraint 
		 * operation the data that follows is formatted for.
		 */
		enum ConstraintType
		{
			//NOTE: These constants must match the same constants in the tools source file sdkwriter.cpp
			// and must match the order of kCmdConstraint* commands in iceanimbatchpriv.h.
			kConstraintType0		= 0x50,
			kConstraintParentPos = kConstraintType0,
			kConstraintParentRot,
			kConstraintPoint,
			kConstraintOrient,
			kConstraintAim,
		};

		/*!
		 * kConstraintParentPos data:
		 *
		 *	ConstraintInfoQuad::Element	inputData[header->m_numItems];			// driving data specifying the source and destination joints
		 *	Vec4						localOffsets[header->m_numItems];		// local offset vector, with weight in w component
		 *
		 *	header->m_numItems must be a multiple of 4.
		 *	inputData should be padded with firstSource == 0
		 *	localOffsets should be padded with weight == 0.0f
		 */
		struct ConstraintInfoQuad
		{
			struct Element {
				U16 m_srcRef; 		//!< joint index * 0x40
				U16 m_dstRef; 		//!< (joint index * 0x30 + 0x20 (point) or 0x10 (orient)) | firstSource
			} m_elem[4];
		};

		/*!
		 * kConstraintParentRot data:
		 *
		 *	ConstraintInfoQuad::Element	inputData[header->m_numItems];			// driving data specifying the source and destination joints
		 *	TransposedOrientation		offsets[header->m_numItems/4];			// transposed (joint_orient * offset) quaternions
		 *	float						weights[header->m_numItems];			// constraint weights
		 *	U32							numDestQuads;							// number of constraint destination joints / 4
		 *	U32							pad[3];
		 *	U16							destOffsets[ numDestQuads * 4 ];		// destination joints matching those in inputData but without repeats
		 *	U16							pad[ (numDestQuads & 0x1) * 4 ];
		 *	TransposedOrientation		rotAxis[ numDestQuads ];				// transposed rotation axis quaternions for destination joints
		 *
		 *	header->m_numItems must be a multiple of 4.
		 *	inputData should be padded with firstSource == 0.
		 *	weights should be padded with value 0.0f.
		 *	destOffsets should be padded with copies of another destOffset.
		 *	rotAxis and offsets should be padded with identity quaternions (0, 0, 0, 1).
		 */
		struct TransposedOrientation
		{
			SMath::Vec4 qx;		// q[0].x, q[1].x, q[2].x, q[3].x
			SMath::Vec4 qy;		// q[0].y, q[1].y, q[2].y, q[3].y
			SMath::Vec4 qz;		// q[0].z, q[1].z, q[2].z, q[3].z
			SMath::Vec4 qw;		// q[0].w, q[1].w, q[2].w, q[3].w
		};

		/*!
		 * kConstraintPoint data:
		 *
		 *	ConstraintInfoQuad::Element	inputData[header->m_numItems];			// driving data specifying the source and destination joints
		 *	float						weights[header->m_numItems]; 			// constraint weights
		 *	Vec4						offsets[];								// world space offset per constraint (i.e. per m_dstRef & firstsource)
		 *
		 *	header->m_numItems must be a multiple of 4.
		 *	inputData should be padded with firstSource == 0.
		 *	weights should be padded with value 0.0f.
		 *	offsets does not need to be padded.
		 */

		/*!
		 * kConstraintOrient data:
		 *
		 *	ConstraintInfoQuad::Element	inputData[header->m_numItems];			// driving data specifying the source and destination joints
		 *	float						weights[header->m_numItems]; 			// constraint weights
		 *	U32							numDestQuads;							// number of constraint destination joints / 4
		 *	U32							pad[3];
		 *	U16							destOffsets[ numDestQuads * 4 ];		// destination joints matching those in inputData but without repeats
		 *	U16							pad[ (numDestQuads & 0x1) * 4 ];
		 *	TransposedOrientation		offsets[ numDestQuads ];				// transposed (joint_orient * offset * rot_axis) quaternions for destination joints
		 *
		 *	header->m_numItems must be a multiple of 4.
		 *	inputData should be padded with firstSource == 0.
		 *	weights should be padded with value 0.0f.
		 *	destOffsets should be padded with copies of another destOffset.
		 *	offsets should be padded with identity quaternions (0, 0, 0, 1).
		 */

		/*!
		 * kConstraintAim data:
		 *
		 *	ConstraintInfoQuad::Element	inputData[header->m_numItems];			// driving data specifying the source and destination joints
		 *	float						weights[header->m_numItems]; 			// constraint weights
		 *	U32							numDestQuads;							// number of constraint destination joints / 4
		 *	U32							pad[3];
		 *	AimConstraintDestQuad		destAndUpData[ numDestQuads ];			// destination joints matching those in inputData but without repeats, plus up vector source data
		 *	TransposedOrientation		offsets[ numDestQuads ];				// transposed (joint_orient * offset * rot_axis) quaternions for destination joints
		 *	Vec4						upVectors[ numDestQuads/4 ];			// up vector table (size depends on
		 *
		 *	header->m_numItems must be a multiple of 4.
		 *	inputData should be padded with firstSource == 0.
		 *	weights should be padded with value 0.0f.
		 *	destOffsets should be padded with copies of another destOffset.
		 *	worldUpVectors should be padded with copies of another worldUpVector.
		 *	offsets should be padded with identity quaternions (0, 0, 0, 1).
		 */
		struct AimConstraintDestQuad {
			struct Element {
				U16 m_dstRef;		//!< (joint index * 0x30 + 0x10)
				U16 m_upSrcRef;		//!< 0x0 (y vector) _or_ transformIdx*0x40 | 0x1 (joint rel pos) _or_ transformIdx*0x40 | 0x2 (joint orientation) _or_ 0x3 (up vector)
			} m_elem[4];
		};

		/*!
		 * kPatchConstraintData data:
		 *
		 *	ConstraintPatchQuad::Element	inputData[header->m_numPatchItems];		// driving data specifying source and destination for word patches
		 *
		 *	All constraint patches are word aligned float patches from either the scalar table or the sdkToConstraintVec vector table.
		 *	Patches from the scalar table are always quadword aligned, while patches from the sdkToConstraintVec vector table typically
		 *	consist of a set of 4 entries patching 4 sequential words from the table into 4 often non-sequential words in the constraint data.
		 */
		struct ConstraintPatchQuad {
			struct Element {
				U16 m_dstRef;		//!< destination byte offset (word aligned - i.e. (m_dstOffset & 0x3) == 0)
				U16 m_srcRef;		//!< source byte offset (word aligned) | 0x0001 (vector flag)
			} m_elem[4];
		};
	}
};

#endif //ICE_JOINTHIERARCHYPRIV_H
