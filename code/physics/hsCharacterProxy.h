
#ifndef HS_CHARACTER_CONTROLLER_H
#define HS_CHARACTER_CONTROLLER_H

#include "Physics/havokincludes.h"
#include <hkutilities/charactercontrol/characterproxy/hkCharacterProxy.h>


/// Allows to modify contact point found by m_shapePhantom->setPositionAndLinearCast
class hsCharacterProxyContactListener
{
public:
	virtual ~hsCharacterProxyContactListener() {};

	virtual void contactsFoundCallback(hkAllCdPointCollector& castCollector, hkAllCdPointCollector& startPointCollector) = 0;
};

/// The character proxy class is used to represent a non penetrating shape that can move dynamically around 
/// the scene. It is called character proxy because it is usually used to represent game characters. It could
/// just as easily be used to represent any dynamic game object.
class hsCharacterProxy : public hkCharacterProxy
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CHARACTER);

		hsCharacterProxy(const hkCharacterProxyCinfo& info, float fUpdateEverySeconds);

		virtual ~hsCharacterProxy();

			/// Update from a given construction info.
		void updateFromCinfo( const hkCharacterProxyCinfo& cinfo );
	
			/// Update and move the character. To override the logic defining which bodies the character can collide
			/// with or capture this information for your own use (e.g. to deal with 'trigger' volumes), use ::integrateWithCollectors() instead.
			/// By default ::integrate() uses hkAllCdPointCollectors for the character's collision queries.
			/// The worldGravity parameter is only used when the character's mass is greater than 0, to apply downward impulses to dynamic objects.
		void integrate(const hkStepInfo& stepInfo, const hkVector4& worldGravity);

			/// Update and move the character. You must pass the two collectors used internally in the hkShapePhantom::setPositionAndLinearCast()
			/// calls. Implementing your own hkAllCdPointCollector will allow you to customize the character's movement behavior
			/// or extract information about objects it overlaps with or objects it linear casts through.
			/// The gravity parameter is only used when the character's mass is greater than 0, to apply downward impulses to dynamic objects.
		void integrateWithCollectors(const hkStepInfo& stepInfo, const hkVector4& gravity, hkAllCdPointCollector& castCollector, hkAllCdPointCollector& startPointCollector);

			/// Check and see if the character is supported in the given direction (and by what).
			/// This call checks the geometry immediately around the character, and does not take velocity into account.
			/// i.e. if the character is moving away from the ground, but is still touching the ground, this function
			/// will return SUPPORTED as the supported state.
		void checkSupport(const hkVector4& direction, hkSurfaceInfo& ground);

			/// This function is the same as the checkSupportFunction, except it allows you to pass your own collectors to filter
			/// collisions. You only need to call this if you are using the integrateWithCollectors call.
		void checkSupportWithCollector(const hkVector4& direction, hkSurfaceInfo& ground, hkAllCdPointCollector& startPointCollector);

			/// When hsCharacterProxy::integrate() is called, or hkWorld::stepDeltaTime() is called, the manifold
			/// information will become out of date.  If you use the manifold in your state machine, and need it to be
			/// up to date, you should call this function first. Note that this function is called by checkSupport automatically,
			/// so if you call checkSupport just before your state machine, you do not need to call this function.
		void refreshManifold( hkAllCdPointCollector& startPointCollector );
		
			/// Get current position
		const hkVector4& getPosition() const;

			/// Warp the character (through walls etc.)
		void setPosition(const hkVector4& position, bool bResetInterpolation = false);		

			/// The 3.0.0 version of check support
		void checkSupportDeprecated(const hkVector4& direction, hkSurfaceInfoDeprecated& ground) const;

		/// Add a hsCharacterProxyContactListener 
		void addCharacterProxyContactListener(hsCharacterProxyContactListener* listener);

			/// Remove a hsCharacterProxyContactListener
		void removeCharacterProxyContactListener(hsCharacterProxyContactListener* listener);

		//bool m_useMaxSlope;

		void forceUpdate() { m_bForceUpdate = true; };

	protected:
		float m_fUpdateEverySeconds; // How often we want to update our CC
		hkStepInfo m_obTimeSinceLastUpdate; // Used to accumulate time to integrate over when we finally do update
		hkVector4 m_obMyPsuedoPosition; // Interpolated position
		hkVector4 m_aobMyLastRealPosition[2]; // History of proper positions
		bool m_bHasBeenUpdatedOnce; // Have I been updated at all
		bool m_bForceUpdate; // Force an update?		

		bool m_bCurrentVelocityPrediction; 
		hkVector4 m_oldVelocity;		

		hkArray<hsCharacterProxyContactListener *> m_contactListeners;

			/// Update the manifold of contact points without calling collision detection
		virtual void updateManifold(const hkAllCdPointCollector& startPointCollector, const hkAllCdPointCollector& castCollector);

			/// Apply impulses to the object.		
		void applySurfaceInteractions( const hkStepInfo& info, const hkVector4& gravity );

			/// Build surface constraint information from the point returned by a cast
		virtual void extractSurfaceConstraintInfo(const hkRootCdPoint& hit, hkSurfaceConstraintInfo& surfaceOut, hkReal timeTravelled) const;	

			// Defines a distance metric for keeping and discarding planes
			// Used to update the mnaifold
		inline hkReal surfaceDistance(const hkRootCdPoint& p1, const hkRootCdPoint& p2) const;

			// Extract the surface velocity at a point returned by a cast
		inline void extractSurfaceVelocity(const hkRootCdPoint& hit, hkVector4& velocityOut) const;

			/// Converts the distance stored as a fraction in the cast collector to a proper euclidean distance.
			/// The cast collector returns distances as a fraction of the cast.
			///	We need to convert these to Euclidian distances, and project them onto the
			/// normal of the collision plane. This replaces the fraction by the
			/// correct distance and moves the fraction to the normal.w component.
		inline void convertFractionToDistance( const hkRootCdPoint* hits , int numHits, const hkVector4& displacement) const;

		void fireContactsFound(hkAllCdPointCollector& castCollector, hkAllCdPointCollector& startPointCollector) const; 

};

#endif //HS_CHARACTER_CONTROLLER_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20050420)
*
* Confidential Information of Havok.  (C) Copyright 1999-2005 
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
