/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 * Revision History:
 *  - Created 8/8/05
 */

#ifndef ICEDUMP_H
#define ICEDUMP_H

namespace Ice
{
	namespace Graphics
	{
		void DumpTransform(const SMath::Transform *transform, unsigned verbosity = 0, int indent = 0);
		void DumpDiscretePmObject(const DiscretePmObject *object, unsigned verbosity = 0, int indent = 0);
		void DumpContinuousPmObject(const ContinuousPmObject *object, unsigned verbosity = 0, int indent = 0);
		void DumpMaterialDescriptor(const MaterialDescriptor *materialDescriptor, unsigned verbosity = 0, int indent = 0);
	}
}

#endif // ICEDUMP_H
