/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkvisualize/hkVisualize.h>

// Serialization dependencies
#include <hkvisualize/serialize/hkDisplaySerializeOStream.h>

// Class definition
#include <hkvisualize/hkDebugDisplay.h>
#include <hkvisualize/hkProcessFactory.h>

#include <hkvisualize/process/hkDebugDisplayProcess.h>
#include <hkvisualize/hkVisualDebuggerDebugOutput.h>
#include <hkvisualize/shape/hkDisplayGeometry.h>
#include <hkvisualize/shape/hkDisplaySphere.h>
#include <hkvisualize/shape/hkDisplayBox.h>
#include <hkvisualize/shape/hkDisplayAABB.h>
#include <hkvisualize/shape/hkDisplayCone.h>
#include <hkvisualize/shape/hkDisplayPlane.h>
#include <hkvisualize/shape/hkDisplaySemiCircle.h>
#include <hkvisualize/shape/hkDisplayCapsule.h>

#include <hkbase/stream/hkStreamWriter.h>
#include <hkbase/stream/impl/hkArrayStreamWriter.h>

int hkDebugDisplayProcess::m_tag = 0;

hkProcess* HK_CALL hkDebugDisplayProcess::create(const hkArray<hkProcessContext*>& contexts)
{
	return new hkDebugDisplayProcess(); // doesn't need any context data
}

hkDebugDisplayProcess::hkDebugDisplayProcess()
	: hkProcess( true )
{
	if (m_tag == 0)
	{
		registerProcess();
	}
	hkDebugDisplay::getInstance().addDebugDisplayHandler(this);
}

void HK_CALL hkDebugDisplayProcess::registerProcess()
{
	m_tag = hkProcessFactory::getInstance().registerProcess( getName(), create );
}

hkDebugDisplayProcess::~hkDebugDisplayProcess()
{
	hkDebugDisplay::getInstance().removeDebugDisplayHandler(this);
}

hkResult hkDebugDisplayProcess::addGeometry(const hkArray<hkDisplayGeometry*>& geometries, const hkTransform& transform, hkUlong id, int tag)
{
	return m_displayHandler->addGeometry(geometries, transform, id, m_tag);
}

hkResult hkDebugDisplayProcess::displayGeometry(const hkArray<hkDisplayGeometry*>& geometries, const hkTransform& transform, int color, int tag)
{
	return m_displayHandler->displayGeometry(geometries, transform, color, m_tag);
}

hkResult hkDebugDisplayProcess::displayGeometry(const hkArray<hkDisplayGeometry*>& geometries, int color, int tag)
{
	return m_displayHandler->displayGeometry(geometries, color, m_tag);
}

hkResult hkDebugDisplayProcess::setGeometryColor(int color, hkUlong id, int tag)
{
	return m_displayHandler->setGeometryColor(color, id, m_tag);
}

hkResult hkDebugDisplayProcess::updateGeometry(const hkTransform& transform, hkUlong id, int tag)
{
	return m_displayHandler->updateGeometry(transform, id, m_tag);
}

hkResult hkDebugDisplayProcess::removeGeometry(hkUlong id, int tag)
{
	return m_displayHandler->removeGeometry(id, m_tag);
}

hkResult hkDebugDisplayProcess::updateCamera(const hkVector4& from, const hkVector4& to, const hkVector4& up, hkReal nearPlane, hkReal farPlane, hkReal fov, const char* name)
{
	return m_displayHandler->updateCamera(from, to, up, nearPlane, farPlane, fov, name);
}

hkResult hkDebugDisplayProcess::displayPoint(const hkVector4& position, int color, int tag)
{
	return m_displayHandler->displayPoint(position, color, m_tag);
}

hkResult hkDebugDisplayProcess::displayLine(const hkVector4& start, const hkVector4& end, int color, int tag)
{
	return m_displayHandler->displayLine(start, end, color, m_tag);
}

hkResult hkDebugDisplayProcess::displayText(const char* text, int color, int tag)
{
	return m_displayHandler->displayText(text, color, m_tag);
}
		
hkResult hkDebugDisplayProcess::sendMemStatsDump(const char* data, int length)
{
	return m_displayHandler->sendMemStatsDump(data, length);
}

hkResult hkDebugDisplayProcess::holdImmediate()
{
	return m_displayHandler->holdImmediate();
}

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
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
