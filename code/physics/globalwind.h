//------------------------------------------------------------------------------------------
//!
//! globalwind.h
//! 
//!
//------------------------------------------------------------------------------------------
#ifndef _GLOBAL_WIND_H
#define _GLOBAL_WIND_H


class Perlin
{
public:
	Perlin() {}
	~Perlin() {}

	static void init();
	static float noise1(float arg);
	static float noise2(float vec[2]);
	static void normalize2(float v[2]);

private:
	static const int B = 0x100;
	static const int BM = 0xFF;
	static const int N = 0x1000;
	static const int NP = 12;
	static const int NM = 0xfff;
	static int m_gHash[B + B + 2];
	static float m_gGradient1[B + B + 2];
	static float m_gGradient2[B + B + 2][2];
	static bool m_bInitialised;
};


//------------------------------------------------------------------------------------------
//!
//! GlobalWind
//! Description here...
//!
//------------------------------------------------------------------------------------------ 
class GlobalWind
{
public:
	// Declare interface
	HAS_INTERFACE( GlobalWind );

	// Constructor
	GlobalWind();

	// Destructor
	~GlobalWind();

	// Update the wind with new delta time value
	void Update(float fDeltaTime);

	// Get the wind force at a particular point
	CVector GetForce(const CPoint& obPos);

	// Current time accessor
	float GetCurrentTime() {return m_fCurrentTime;}

	// Debug render mechanism
	void DebugRender();

private:
	// Constant wind parameter
	CVector	m_obConstantWind;

	// Turbulence frequency
	CVector m_obTurbulenceFrequency;

	// Turbulence velocity
	CVector m_obTurbulenceVelocity;

	// Wind power
	float	m_fPower;

	// The current time of this wind field
	float m_fCurrentTime;
};


class SoftMaterial
{
public:
	HAS_INTERFACE( SoftMaterial );

	SoftMaterial();
	~SoftMaterial() {};

	float GetParticleMass() {return m_fMass;}

	// Params
	CVector m_obLocalTurbulenceFreq;
	CVector m_obLocalTurbulenceVel;
	float	m_fLocalPower;
	float	m_fDrag;
	float	m_fMass;
	int		m_iIterations;
};





#endif //_GLOBAL_WIND_H
