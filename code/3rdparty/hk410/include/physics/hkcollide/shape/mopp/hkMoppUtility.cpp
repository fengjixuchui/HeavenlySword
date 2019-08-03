/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkinternal/collide/mopp/builder/hkbuilder.h>
#include <hkcollide/shape/mopp/hkMoppFitToleranceRequirements.h>

hkMoppCode* HK_CALL hkMoppUtility::buildCode(const hkShapeContainer* shapeContainer, const hkMoppFitToleranceRequirements& moppFtr)
{
	hkMoppCode* code;
	if ( moppFtr.m_cachePrimitiveExtents )
	{
		hkMoppCachedShapeMediator mediator( shapeContainer );
		code = hkMoppUtility::buildCodeInternal(mediator, moppFtr);
	}
	else
	{
		hkMoppShapeMediator mediator( shapeContainer );
		code = hkMoppUtility::buildCodeInternal(mediator, moppFtr);
	}

	return code;
}

hkMoppCode* HK_CALL hkMoppUtility::buildCodeInternal(hkMoppMediator& mediator, const hkMoppFitToleranceRequirements& moppFtr)
{
	HK_WARN(0x6e8d163b, "Building Mopp code at runtime can be slow. Mopp code \n" \
		"is a platform independent byte code. It can be preprocessed \n" \
		"and saved on PC and loaded on the required platform at runtime. \n");

	hkMoppCompiler compiler;

	//
	// set up user scaling struct for cost functions
	//
	{
		hkMoppCostFunction::hkMoppSplitCostParams costParams;
		costParams.m_weightPrimitiveSplit = 1.0f;
		if ( moppFtr.m_useShapeKeys == false )
		{
			costParams.m_weightPrimitiveIdSpread  = 0.0f;
		}
		compiler.setCostParams( costParams );
	}

	//
	// optionally control the assembler
	//
	{
		hkMoppAssembler::hkMoppAssemblerParams ap;

		ap.m_relativeFitToleranceOfInternalNodes = moppFtr.getRelativeFitToleranceOfInternalNodes();
		ap.m_absoluteFitToleranceOfInternalNodes = moppFtr.getAbsoluteFitToleranceOfInternalNodes();
		ap.m_absoluteFitToleranceOfTriangles = moppFtr.getAbsoluteFitToleranceOfTriangles();
		ap.m_absoluteFitToleranceOfAxisAlignedTriangles = moppFtr.getAbsoluteFitToleranceOfAxisAlignedTriangles();
		ap.m_interleavedBuildingEnabled = moppFtr.m_enableInterleavedBuilding;

		compiler.setAssemblerParams( ap );
	}

	{
		hkMoppSplitter::hkMoppSplitParams splitParams( HK_MOPP_MT_LANDSCAPE );
		if ( moppFtr.m_enablePrimitiveSplitting)
		{
			splitParams.m_maxPrimitiveSplitsPerNode = 50;
		}
		else
		{
			splitParams.m_maxPrimitiveSplits = 0;
			splitParams.m_maxPrimitiveSplitsPerNode = 0;
		}
		splitParams.m_minRangeMaxListCheck = 5;
		splitParams.m_interleavedBuildingEnabled = moppFtr.m_enableInterleavedBuilding;
		compiler.setSplitParams( splitParams );
	}

	// enable/disable interleaved building (depending on user settings)
	compiler.enableInterleavedBuilding(moppFtr.m_enableInterleavedBuilding);

	int bufferSize = compiler.calculateRequiredBufferSize(&mediator);
	char *buffer = hkAllocateStack<char>(bufferSize);

	//This is where the built code is assigned to the hkGeometry
	hkMoppCode *code = compiler.compile(&mediator, buffer, bufferSize);
	hkDeallocateStack<char>( buffer );
	return code;
}

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
