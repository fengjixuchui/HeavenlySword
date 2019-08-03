//------------------------------------------------------------------------------------------
//!
//! globalwind.cpp
//! TO BE TIDIED UP POST E3!
//!
//------------------------------------------------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "physics/globalwind.h"
#include "core/visualdebugger.h"
#include "physics/verletmanager.h"


#define s_curve(t) ( t * t * (3.0f - 2.0f * t) )
#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.0f;

int Perlin::m_gHash[B + B + 2];
float Perlin::m_gGradient1[B + B + 2];
float Perlin::m_gGradient2[B + B + 2][2];
bool Perlin::m_bInitialised = false;

void Perlin::init()
{
	if (!m_bInitialised)
	{
		int i, j, k;

		for (i = 0 ; i < B ; i++)
		{
			m_gHash[i] = i;
			m_gGradient1[i] = (float)((rand() % (B + B)) - B) / B;
			for (j = 0 ; j < 2 ; j++)
				m_gGradient2[i][j] = (float)((rand() % (B + B)) - B) / B;
			normalize2(m_gGradient2[i]);
		}

		while (--i)
		{
			k = m_gHash[i];
			m_gHash[i] = m_gHash[j = rand() % B];
			m_gHash[j] = k;
		}

		for (i = 0 ; i < B + 2 ; i++)
		{
			m_gHash[B + i] = m_gHash[i];
			m_gGradient1[B + i] = m_gGradient1[i];
			for (j = 0 ; j < 2 ; j++)
				m_gGradient2[B + i][j] = m_gGradient2[i][j];
		}
		m_bInitialised = true;
	}
}

float Perlin::noise1(float arg)
{
	init();
	assert(m_bInitialised);
	int bx0, bx1;
	float rx0, rx1, sx, t, u, v, vec[1];

	vec[0] = arg;

	setup(0, bx0,bx1, rx0,rx1);

	sx = s_curve(rx0);

	u = rx0 * m_gGradient1[ m_gHash[ bx0 ] ];
	v = rx1 * m_gGradient1[ m_gHash[ bx1 ] ];

	return lerp(sx, u, v);
}

float Perlin::noise2(float vec[2])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	int i, j;
	init();
	setup(0,bx0,bx1,rx0,rx1);
	setup(1,by0,by1,ry0,ry1);

	i = m_gHash[bx0];
	j = m_gHash[bx1];

	b00 = m_gHash[i + by0];
	b10 = m_gHash[j + by0];
	b01 = m_gHash[i + by1];
	b11 = m_gHash[j + by1];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

  #define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = m_gGradient2[b00];
	u = at2(rx0,ry0);
	q = m_gGradient2[b10];
	v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = m_gGradient2[b01];
	u = at2(rx0,ry1);
	q = m_gGradient2[b11];
	v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}

void Perlin::normalize2(float v[2])
{
	float s;

	s = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
	s = 1.0f/s;
	v[0] = v[0] * s;
	v[1] = v[1] * s;
}


// Here are the default settings for global wind.
// It can be overridden on a system basis. 

START_STD_INTERFACE	( GlobalWind )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obConstantWind,		CVector(1.0f,-0.6f,0.0f,0.0f),	ConstantWind )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obTurbulenceFrequency,	CVector(0.0f,0.0f,0.0f,0.0f), TurbulenceFrequency )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obTurbulenceVelocity,	CVector(0.0f,0.0f,0.0f,0.0f), TurbulenceVelocity )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fPower, 1.0f, Power )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//! GlobalWind::GlobalWind()
//! Constructor
//!
//------------------------------------------------------------------------------------------
GlobalWind::GlobalWind()
:	m_obConstantWind(1.0f, -0.6f, 0.0f, 0.0f),
	m_obTurbulenceFrequency(0.01f, 0.02f, 0.03f, 0.0f),
	m_obTurbulenceVelocity(0.1f, 0.2f, 0.3f, 0.0f),
	m_fPower(0.0f),
	m_fCurrentTime(0.0f)
{
	// Register this wind with the manager
	if ( Physics::VerletManager::Exists() && Physics::VerletManager::Get().IsEnabled() ) 
		Physics::VerletManager::Get().AddWind(this);
}

GlobalWind::~GlobalWind()
{
	// Unregister this wind with the manager
	if ( Physics::VerletManager::Exists() && Physics::VerletManager::Get().IsEnabled()  ) 
		Physics::VerletManager::Get().RemoveWind(this);
}

void GlobalWind::Update(float fDeltaTime)
{
	m_fCurrentTime += fDeltaTime;
}

CVector GlobalWind::GetForce(const CPoint& obPos)
{
	CVector obResult = m_obConstantWind;
	float vec[2];
	vec[0] = obPos[0]* m_obTurbulenceFrequency[0] + m_fCurrentTime * m_obTurbulenceVelocity[0];
	vec[1] = obPos[2]* m_obTurbulenceFrequency[2] + m_fCurrentTime * m_obTurbulenceVelocity[2];
	obResult[0] += Perlin::noise2(vec)*m_fPower;

	vec[0] += 50.0f; // Create an offset in the perlin vector field
	vec[1] += 77.0f;
	obResult[2] += Perlin::noise2(vec)*m_fPower;
	return obResult;
}

void GlobalWind::DebugRender()
{
#ifndef _RELEASE
	if (m_fPower > 0.0f)
	{
		float fXScale = 0.5f;
		float fZScale = 0.5f;
		float fXMax = 15.0f;
		float fZMax = 15.0f;
		for (float x = 0.0f; x < fXMax; x += fXScale)
		{
			for (float z = 0.0f; z < fZMax; z += fZScale)
			{
				// Query the wind
				CPoint obPos(x, 0.0f, z);
				CVector obForce = GetForce(obPos);
				
				// display the wind
				CPoint obEnd = obPos;
				obEnd += (CPoint)obForce;
				g_VisualDebug->RenderLine(obPos, obEnd, 0x00ffff00); 
			}
		}
	}
#endif
}


START_STD_INTERFACE	( SoftMaterial )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obLocalTurbulenceFreq,	CVector(0.0f, 0.0f, 0.0f, 0.0f),	LocalTurbulenceFreq )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obLocalTurbulenceVel,	CVector(0.0f, 0.0f, 0.0f, 0.0f),	LocalTurbulenceVel )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fLocalPower,		0.0f,		LocalPower )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fDrag,				0.03f,		Drag )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fMass,				0.1f,		Mass )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_iIterations,		3,			Iterations )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!
//! SoftMaterial::SoftMaterial()
//! Constructor
//!
//------------------------------------------------------------------------------------------
SoftMaterial::SoftMaterial()
:	
	m_obLocalTurbulenceFreq(0.0f, 0.0f, 0.0f, 0.0f),
	m_obLocalTurbulenceVel(0.0f, 0.0f, 0.0f, 0.0f),
	m_fLocalPower(0.0f),
	m_fDrag(0.03f),
	m_fMass(0.1f),
	m_iIterations(3)
{
}






