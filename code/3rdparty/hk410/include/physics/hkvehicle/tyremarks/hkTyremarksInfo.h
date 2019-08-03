/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKVEHICLE_TYREMARKS_hkTyremarksInfo_XML_H
#define HKVEHICLE_TYREMARKS_hkTyremarksInfo_XML_H


#include <hkmath/hkMath.h>

class hkVehicleInstance;
class hkVehicleData;
	
/// A tyremark point is defined by two points (left and right and the strength of
/// the tyremark). Having two points instead of one allows for thickness in
/// tyremarks. The strength is a user value that can be used, for example, to
/// shade tyremarks depending on the amount of skidding 	
struct hkTyremarkPoint
{
	public:
	
		HK_DECLARE_REFLECTION();

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VEHICLE, hkTyremarkPoint);

			/// Default constructor
		hkTyremarkPoint();
		
		//
		// Methods
		//
		
			/// The strength of a tyremarkPoint is stored in the w-component
			/// of the vectors.  The strength is in the range 0.0f to 255.0f.
		hkReal getTyremarkStrength() const;
		
		//
		// Members
		//
	public:
		
			/// The left position of the tyremark.
		hkVector4 m_pointLeft;
		
			/// The right position of the tyremark.
		hkVector4 m_pointRight;

	public: 
		hkTyremarkPoint(hkFinishLoadedObjectFlag f) { }

};


/// hkTyremarksWheel stores a list of tyremarks associated with a particular wheel.
/// This is a circular array, so old tyremarks eventually get replaced by new
/// tyremarks.
class hkTyremarksWheel : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VEHICLE);
		HK_DECLARE_REFLECTION();

			/// Default constructor
		hkTyremarksWheel();

		virtual ~hkTyremarksWheel() {}
		
		//
		// Methods
		//
		
			/// Sets the number of tyre mark points that can be stored.
		void setNumPoints(int num_points);
		
			/// Add a tyreMarkPoint to the array.
		void addTyremarkPoint( hkTyremarkPoint& point);
		
			/// Returns the i-th stored tyremark point in the object.
		const hkTyremarkPoint& getTyremarkPoint(int point) const;
		
		//
		// Members
		//
	public:
		
			/// Current position in the array of tyremarkPoints.
		int m_currentPosition;
		
			/// The number of points in the array
		int m_numPoints;
		
			/// Circular array of tyreMarkPoints.
		hkArray<struct hkTyremarkPoint> m_tyremarkPoints;

	public: 
		hkTyremarksWheel(hkFinishLoadedObjectFlag f) : m_tyremarkPoints(f) { }

};


/// hkTyremarksInfo stores a list of hkTyremarksWheel for a particular vehicle
class hkTyremarksInfo : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VEHICLE);
		HK_DECLARE_REFLECTION();

		//
		// Methods
		//
		
			/// Updates Tyremark information
		virtual void updateTyremarksInfo(hkReal timestep, const hkVehicleInstance* vehicle);
		
			/// Retrieves the Tyremark information in the form of a strip
		virtual void getWheelTyremarksStrips(const hkVehicleInstance* vehicle, int wheel, hkVector4* strips_out) const;
		
			/// The constructor takes a hkVehicleData object and the number of skidmark
			/// points to store.
		hkTyremarksInfo(const hkVehicleData& data, int num_points);
		
		virtual ~hkTyremarksInfo();
		//
		// Members
		//
	public:
		
			/// The minimum energy a tyremark should have.  The actual strength of a point will
			/// be scaled to be between 0.0f and 255.0f.
		hkReal m_minTyremarkEnergy;
		
			/// The maximum energy a tyremark should have.  The actual strength of a point will
			/// be scaled to be between 0.0f and 255.0f.
		hkReal m_maxTyremarkEnergy;
		
			/// There is a hkTyremarksWheel for each wheel.
		hkArray<class hkTyremarksWheel*> m_tyremarksWheel;

	public: 
		void calcStatistics( hkStatisticsCollector* collector) const;

		hkTyremarksInfo(hkFinishLoadedObjectFlag f) : m_tyremarksWheel(f) { }

};

#endif // HKVEHICLE_TYREMARKS_hkTyremarksInfo_XML_H

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
