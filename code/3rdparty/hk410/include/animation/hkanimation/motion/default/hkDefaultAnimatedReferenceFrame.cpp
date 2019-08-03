/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkanimation/hkAnimation.h>

#include <hkanimation/motion/default/hkDefaultAnimatedReferenceFrame.h>
#include <hkanimation/animation/interleaved/hkInterleavedSkeletalAnimation.h>

#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkDefaultAnimatedReferenceFrame);

hkDefaultAnimatedReferenceFrame::MotionExtractionOptions::MotionExtractionOptions()
:
	m_referenceFrameTransforms (HK_NULL),
	m_numReferenceFrameTransforms (0),
	m_referenceFrameDuration (0.0f),
	m_numSamples (2), 
	m_allowUpDown (false), 
	m_allowFrontBack (true), 
	m_allowRightLeft (false),
	m_allowTurning (false)
{
	m_up.set(0.0f,0.0f,1.0f);
	m_forward.set(1.0f,0.0f,0.0f);
}

hkReal hkDefaultAnimatedReferenceFrame::getDuration() const
{
	return m_duration;
}

inline hkQsTransform sampleReferenceFrame( hkReal sampleTime, const hkQsTransform* t, int numT, hkReal duration )
{
	if (sampleTime >= duration)
	{
		return t[ numT -1 ];
	}
	else
	{
		const hkReal timeSlice = duration / (numT - 1);
		int index = static_cast<int> (sampleTime / timeSlice);
		HK_ASSERT( 0x7e54e54e, index < numT - 1 );
		hkReal dt = sampleTime / timeSlice - index;
		hkQsTransform result;
		result.setInterpolate4( t[index] , t[index+1], dt);
		return result;
	}
}

hkDefaultAnimatedReferenceFrame::hkDefaultAnimatedReferenceFrame ( const MotionExtractionOptions& options )
{
	const int numPoses = (options.m_numSamples == -1) ? options.m_numReferenceFrameTransforms : options.m_numSamples;
 
	HK_ASSERT2(0x59a3aac5, numPoses >= 2, "Invalid number of samples for motion extraction");

	m_up = options.m_up;
	m_forward = options.m_forward;
	m_duration = options.m_referenceFrameDuration;

	const hkReal timeSlice =  m_duration / (numPoses - 1);

	// Start calculating the motion track
	const hkQsTransform rootReferenceFrame = options.m_referenceFrameTransforms[0];

	// EXP-482 : We always remember the last projection to ensure we pick the closest one next time
	hkVector4 previousProjection; previousProjection.setZero4();
	hkVector4 initialProjection;
	project(options, rootReferenceFrame, previousProjection, initialProjection);
	previousProjection = initialProjection;

	// Warn when motion extraction may produce bad results
	if (options.m_allowTurning)
	{
		hkVector4 sideDirection;
		sideDirection.setCross(options.m_up, options.m_forward);

		// EXP-482
		// Turning motion will affect translation components
		// So it is important that the non-vertical translation components of the root start at 0
		static const hkReal warnEpsilon = 1e-2f;
		const hkReal forwardError = hkMath::fabs(initialProjection.dot3(options.m_forward));
		const hkReal sideError = hkMath::fabs(initialProjection.dot3(sideDirection));
		if ( (forwardError > warnEpsilon )|| (sideError > warnEpsilon))
		{
			HK_WARN_ALWAYS(0xabba9245,"Extracting turning motion on an animation works best when the start frame is vertically aligned with the origin.");
			HK_WARN_ALWAYS(0xabba9245,"Some misalignment ("<<forwardError<<", "<<sideError<<") found.");
			HK_WARN_ALWAYS(0xabba9245,"For best results, try moving the animation to the origin in the Create Animations filter.");
		}
	}

	{
		m_numReferenceFrameSamples = numPoses;
		m_referenceFrameSamples = hkAllocateChunk<hkVector4>( m_numReferenceFrameSamples, HK_MEMORY_CLASS_ANIM_MOTION );

		for (int poseIt=0; poseIt < numPoses; poseIt++)
		{
			const hkReal time = poseIt * timeSlice;
			hkQsTransform rootPose = sampleReferenceFrame( time, options.m_referenceFrameTransforms, options.m_numReferenceFrameTransforms, options.m_referenceFrameDuration );

			// EXP-482 : One of the problems in the past was that we were calculating the motion by calculating the changes on the root
			// and then "projecting" them (into x,y,z and yaw). The right way is to first project them, and then calculate the changes in 
			// that projection
			hkVector4 newProjection;
			project(options, rootPose, previousProjection, newProjection);
			m_referenceFrameSamples[poseIt].setSub4(newProjection, initialProjection);
			previousProjection = newProjection;
		}
	}
}

hkDefaultAnimatedReferenceFrame::~hkDefaultAnimatedReferenceFrame()
{
	if (m_memSizeAndFlags)
	{
		hkDeallocateChunk<hkVector4>( m_referenceFrameSamples, m_numReferenceFrameSamples, HK_MEMORY_CLASS_ANIM_MOTION );
	}
}


void hkDefaultAnimatedReferenceFrame::getReferenceFrame (hkReal time, hkQsTransform& motionOut) const
{
	hkVector4 motionOut_v;

	const hkReal timeSlice =  m_duration / (m_numReferenceFrameSamples - 1);

	if ( time >= m_duration )
	{
		motionOut_v = m_referenceFrameSamples [m_numReferenceFrameSamples - 1];
	}
	else
	{
		if ( time < 0.0f )
		{
			motionOut_v = m_referenceFrameSamples [0];
		}
		else
		{
			const int frame	= static_cast<int>( time / timeSlice );
			const hkReal delta = ( time / timeSlice ) - frame;

			const hkVector4 motionA = m_referenceFrameSamples[frame];
			const hkVector4 motionB = m_referenceFrameSamples[frame+1];

			motionOut_v.setInterpolate4(motionA, motionB, delta);
		}
	}

	// Construct bone transform
	motionOut.m_translation=motionOut_v; // ignores w component
	motionOut.m_rotation.setAxisAngle(m_up, motionOut_v(3));
	motionOut.m_scale.setAll(1.0f);

}

// orig = planarMovement * rest
void hkDefaultAnimatedReferenceFrame::project (const MotionExtractionOptions& options, const hkQsTransform& transform, const hkVector4& previousProjection, hkVector4& projectionOut)
{
	// CALCULATE DIFFERENT COMPONENTS IN THE MOTION OF THE ROOT
	hkVector4 upComponent;
	hkVector4 sideComponent;
	hkVector4 forwardComponent;
	{
		const hkVector4& rootTranslation = transform.getTranslation();
		upComponent.setMul4(rootTranslation.dot3(options.m_up), options.m_up);

		hkVector4 sideDirection;
		sideDirection.setCross(options.m_up, options.m_forward);
		sideComponent.setMul4(rootTranslation.dot3(sideDirection), sideDirection);

		forwardComponent.setMul4(rootTranslation.dot3(options.m_forward), options.m_forward);
	}

	// ADD DIFFERENT COMPONENTS DEPENDING ON USER CHOICE
	projectionOut.setZero4();

	if (options.m_allowFrontBack)
	{
		projectionOut.add4(forwardComponent);
	}

	if (options.m_allowUpDown)
	{
		projectionOut.add4(upComponent);
	}

	if (options.m_allowRightLeft)
	{
		projectionOut.add4(sideComponent);
	}

	if (options.m_allowTurning)
	{
		const hkQuaternion& rootRotation = transform.getRotation();
		hkQuaternion rest;
		hkReal angle;
		rootRotation.decomposeRestAxis(options.m_up, rest, angle);

		const hkReal previousAngle = previousProjection(3);

		// EXP-482
		// Avoid sudden flips
		{
			while (angle-previousAngle > HK_REAL_PI)
			{
				angle-= 2*HK_REAL_PI;
			}
			while (angle-previousAngle < -HK_REAL_PI)
			{
				angle+= 2*HK_REAL_PI;
			}
		}

		projectionOut(3) = angle;
	}
	else
	{
		projectionOut(3) = 0.0f;
	}

}

void hkDefaultAnimatedReferenceFrame::getDeltaReferenceFrame( hkReal time, hkReal nextTime, int loops, hkQsTransform& output ) const
{
	// Grab motion at the start
	hkQsTransform curMotion;
	getReferenceFrame(time, curMotion);

	// Motion at the end
	hkQsTransform futMotion; 
	getReferenceFrame(nextTime, futMotion);

	// Handle complete loops
	// This assumes that the number of loops will usually be small
	// i.e. it will take longer to convert the quaternion to axis angle
	if (loops != 0)
	{
		hkQsTransform fullCycle; 	
		getReferenceFrame( m_duration, fullCycle );
		const hkUint32 fabsloops = (loops < 0) ? -loops : loops;
		for (hkUint32 l=0; l < fabsloops; l++)
		{
			if (loops < 0)
			{
				// Underflow
				hkQsTransform temp;
				temp.setMulInverseMul(fullCycle, futMotion);
				futMotion = temp;
			}
			else
			{
				// Overflow
				hkQsTransform temp;
				temp.setMul(fullCycle, futMotion);
				futMotion = temp;
			}
		}
	}

	//Compute output
	output.setMulInverseMul ( curMotion, futMotion );
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
