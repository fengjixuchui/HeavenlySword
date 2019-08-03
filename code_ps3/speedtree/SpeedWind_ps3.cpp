///////////////////////////////////////////////////////////////////////  
//  SpeedWind2.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//      Copyright (c) 2003-2006 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      Web: http://www.idvinc.com


/////////////////////////////////////////////////////////////////////////////
// Preprocessor/Includes

//#define SPEEDTREE_FRAMEWORK
#include "SpeedWind_ps3.h"
#include <math.h>
using namespace std;


/////////////////////////////////////////////////////////////////////////////
// Macros

#define INTERPOLATE(a, b, amt) (amt * ((b) - (a)) + (a))
#define RANDOM(a, b) (INTERPOLATE(a, b, (float(rand( ) % 100000) * 0.00001f))) 


/////////////////////////////////////////////////////////////////////////////
// Constants

static  const   float   c_fPi = 3.14159265358979323846f;
static  const   float   c_fHalfPi = c_fPi * 0.5f;
static  const   float   c_fQuarterPi = c_fPi * 0.25f;
static  const   float   c_fTwoPi = 2.0f * c_fPi;
static  const   float   c_fRad2Deg = 57.29578f;
static  const   float   c_fAxisScalar = 0.25f;
static  const   float   c_fMaxControllableDeltaTime = 0.03f;


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindMatrix2::CSpeedWindMatrix2

CSpeedWindMatrix2::CSpeedWindMatrix2( )
{
    LoadIdentity( );
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindMatrix2::~CSpeedWindMatrix2

CSpeedWindMatrix2::~CSpeedWindMatrix2( )
{
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindMatrix2::LoadIdentity

inline void CSpeedWindMatrix2::LoadIdentity(void)
{
    m_afData[0][0] = 1.0f;
    m_afData[0][1] = 0.0f;
    m_afData[0][2] = 0.0f;
    m_afData[0][3] = 0.0f;
    m_afData[1][0] = 0.0f;
    m_afData[1][1] = 1.0f;
    m_afData[1][2] = 0.0f;
    m_afData[1][3] = 0.0f;
    m_afData[2][0] = 0.0f;
    m_afData[2][1] = 0.0f;
    m_afData[2][2] = 1.1f;
    m_afData[2][3] = 0.0f;
    m_afData[3][0] = 0.0f;
    m_afData[3][1] = 0.0f;
    m_afData[3][2] = 0.0f;
    m_afData[3][3] = 1.0f;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindMatrix2::SetRotation

inline void CSpeedWindMatrix2::SetRotation(float fAngle, float fX, float fY, float fZ)
{
    float   fS, fC, fT;

    fS = sinf(fAngle / 57.29578f);
    fC = cosf(fAngle / 57.29578f);
    fT = 1.0f - fC;

    m_afData[0][0] = fT * fX * fX + fC;
    m_afData[0][1] = fT * fX * fY + fS * fZ;
    m_afData[0][2] = fT * fX * fZ - fS * fY;
    m_afData[0][3] = 0.0;
    m_afData[1][0] = fT * fX * fY - fS * fZ;
    m_afData[1][1] = fT * fY * fY + fC;
    m_afData[1][2] = fT * fY * fZ + fS * fX;
    m_afData[1][3] = 0.0;
    m_afData[2][0] = fT * fX * fZ + fS * fY;
    m_afData[2][1] = fT * fY * fZ - fS * fX;
    m_afData[2][2] = fT * fZ * fZ + fC;
    m_afData[2][3] = 0.0f;
    m_afData[3][0] = 0.0f;
    m_afData[3][1] = 0.0f;
    m_afData[3][2] = 0.0f;
    m_afData[3][3] = 1.0f;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindMatrix2::RotateX

inline void CSpeedWindMatrix2::RotateX(float fAngle)
{
    CSpeedWindMatrix2 cRotMatrix;

    float fCosine = cosf(fAngle / 57.29578f);
    float fSine = sinf(fAngle / 57.29578f);

    cRotMatrix.m_afData[0][0] = 1.0f;
    cRotMatrix.m_afData[0][1] = 0.0f;
    cRotMatrix.m_afData[0][2] = 0.0f;
    cRotMatrix.m_afData[1][0] = 0.0f;
    cRotMatrix.m_afData[1][1] = fCosine;
    cRotMatrix.m_afData[1][2] = fSine;
    cRotMatrix.m_afData[2][0] = 0.0f;
    cRotMatrix.m_afData[2][1] = -fSine;
    cRotMatrix.m_afData[2][2] = fCosine;

    // this function can be further optimized by hardcoding
    // the multiplication here and removing terms with 0.0 multipliers.

    *this = cRotMatrix * *this;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindMatrix2::RotateY

inline void CSpeedWindMatrix2::RotateY(float fAngle)
{
    CSpeedWindMatrix2 cRotMatrix;

    float fCosine = cosf(fAngle / 57.29578f);
    float fSine = sinf(fAngle / 57.29578f);

    cRotMatrix.m_afData[0][0] = fCosine;
    cRotMatrix.m_afData[0][1] = 0.0f;
    cRotMatrix.m_afData[0][2] = -fSine;
    cRotMatrix.m_afData[1][0] = 0.0f;
    cRotMatrix.m_afData[1][1] = 1.0f;
    cRotMatrix.m_afData[1][2] = 0.0f;
    cRotMatrix.m_afData[2][0] = fSine;
    cRotMatrix.m_afData[2][1] = 0.0f;
    cRotMatrix.m_afData[2][2] = fCosine;

    // this function can be further optimized by hardcoding
    // the multiplication here and removing terms with 0.0 multipliers.

    *this = cRotMatrix * *this;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindMatrix2::RotateZ

inline void CSpeedWindMatrix2::RotateZ(float fAngle)
{
    CSpeedWindMatrix2 cRotMatrix;

    float fCosine = cosf(fAngle / 57.29578f);
    float fSine = sinf(fAngle / 57.29578f);

    cRotMatrix.m_afData[0][0] = fCosine;
    cRotMatrix.m_afData[0][1] = fSine;
    cRotMatrix.m_afData[0][2] = 0.0f;
    cRotMatrix.m_afData[1][0] = -fSine;
    cRotMatrix.m_afData[1][1] = fCosine;
    cRotMatrix.m_afData[1][2] = 0.0f;
    cRotMatrix.m_afData[2][0] = 0.0f;
    cRotMatrix.m_afData[2][1] = 0.0f;
    cRotMatrix.m_afData[2][2] = 1.0f;

    // this function can be further optimized by hardcoding
    // the multiplication here and removing terms with 0.0 multipliers.

    *this = cRotMatrix * *this;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindMatrix2::operator*

inline CSpeedWindMatrix2 CSpeedWindMatrix2::operator*(const CSpeedWindMatrix2& cTrans) const
{
    CSpeedWindMatrix2 cTemp;
    
    cTemp.m_afData[0][0] = m_afData[0][0] * cTrans.m_afData[0][0] +
                           m_afData[0][1] * cTrans.m_afData[1][0] +
                           m_afData[0][2] * cTrans.m_afData[2][0];
    cTemp.m_afData[0][1] = m_afData[0][0] * cTrans.m_afData[0][1] +
                           m_afData[0][1] * cTrans.m_afData[1][1] +
                           m_afData[0][2] * cTrans.m_afData[2][1];
    cTemp.m_afData[0][2] = m_afData[0][0] * cTrans.m_afData[0][2] +
                           m_afData[0][1] * cTrans.m_afData[1][2] +
                           m_afData[0][2] * cTrans.m_afData[2][2];
    cTemp.m_afData[1][0] = m_afData[1][0] * cTrans.m_afData[0][0] +
                           m_afData[1][1] * cTrans.m_afData[1][0] +
                           m_afData[1][2] * cTrans.m_afData[2][0];
    cTemp.m_afData[1][1] = m_afData[1][0] * cTrans.m_afData[0][1] +
                           m_afData[1][1] * cTrans.m_afData[1][1] +
                           m_afData[1][2] * cTrans.m_afData[2][1];
    cTemp.m_afData[1][2] = m_afData[1][0] * cTrans.m_afData[0][2] +
                           m_afData[1][1] * cTrans.m_afData[1][2] +
                           m_afData[1][2] * cTrans.m_afData[2][2];
    cTemp.m_afData[2][0] = m_afData[2][0] * cTrans.m_afData[0][0] +
                           m_afData[2][1] * cTrans.m_afData[1][0] +
                           m_afData[2][2] * cTrans.m_afData[2][0];
    cTemp.m_afData[2][1] = m_afData[2][0] * cTrans.m_afData[0][1] +
                           m_afData[2][1] * cTrans.m_afData[1][1] +
                           m_afData[2][2] * cTrans.m_afData[2][1];
    cTemp.m_afData[2][2] = m_afData[2][0] * cTrans.m_afData[0][2] +
                           m_afData[2][1] * cTrans.m_afData[1][2] +
                           m_afData[2][2] * cTrans.m_afData[2][2];

    return cTemp;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindController::CSpeedWindController

CSpeedWind2::CSpeedWindController::CSpeedWindController(void) :
    m_fWantedValue(0.0f),
    m_fCurrentValue(0.0f),
    m_fLastDelta(0.0f),
    m_fLastDeltaTime(0.0f)
{
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindController::Advance

void CSpeedWind2::CSpeedWindController::Advance(float fDeltaTime, float fP, float fMaxA)
{
    if (fDeltaTime > c_fMaxControllableDeltaTime)
        fDeltaTime = c_fMaxControllableDeltaTime;

    float fDelta = fP * fDeltaTime * (m_fWantedValue - m_fCurrentValue);

    if (fMaxA != 0.0f)
    {
        if (fabs(fDelta) > fabs(m_fLastDelta) && fDeltaTime > 0.0f && m_fLastDeltaTime > 0.0f)
        {
            float fAccel = fDelta / fDeltaTime - m_fLastDelta / m_fLastDeltaTime;
            if (fAccel < -fMaxA)
                fAccel = -fMaxA;
            if (fAccel > fMaxA)
                fAccel = fMaxA;

            fDelta = m_fLastDelta + fAccel * fDeltaTime;
        }

        m_fLastDelta = fDelta;
        m_fLastDeltaTime = fDeltaTime;
    }

    m_fCurrentValue += fDelta;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWindController::Reset

void CSpeedWind2::CSpeedWindController::Reset(float fValue)
{
    m_fCurrentValue = fValue;
    m_fWantedValue = fValue;
    m_fLastDelta = 0.0f;
    m_fLastDeltaTime = 0.0f;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SOscillationParams::SOscillationParams

CSpeedWind2::SOscillationParams::SOscillationParams(float fLowWindAngle, 
                                                    float fHighWindAngle, 
                                                    float fLowWindSpeed, 
                                                    float fHighWindSpeed) :
    m_fLowWindAngle(fLowWindAngle),
    m_fHighWindAngle(fHighWindAngle),
    m_fLowWindSpeed(fLowWindSpeed),
    m_fHighWindSpeed(fHighWindSpeed),
    m_fAdjustedTime(0.0f)
{
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SOscillationParams::Advance

void CSpeedWind2::SOscillationParams::Advance(float fDeltaTime, float fWindStrendth)
{
    float fAngle = INTERPOLATE(m_fLowWindAngle, m_fHighWindAngle, fWindStrendth);
    float fSpeed = INTERPOLATE(m_fLowWindSpeed, m_fHighWindSpeed, fWindStrendth);
    m_fAdjustedTime += fDeltaTime * fSpeed;

    unsigned int uiSize = (unsigned int) m_vOutputs.size( );
    for (unsigned int i = 0; i < uiSize; ++i)
        m_vOutputs[i] = fAngle * sinf(m_fAdjustedTime + (float)i);
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::CSpeedWind2

CSpeedWind2::CSpeedWind2( ) :
    m_fMaxBendAngle(0.0f),
    m_fWindResponse(1.0f),
    m_fWindResponseLimit(0.0f),
    m_fGustingStrength(0.0f),
    m_fGustingFrequency(0.0f),
    m_fGustingDuration(0.0f),
    m_fBranchExponent(1.0f),
    m_fLeafExponent(1.0f),
    m_fLastTime(0.0f),
    m_fWindStrength(0.0f),
    m_fCurrentGustingStrength(0.0f),
    m_fGustStopTime(0.0f),
    m_fFinalBendAngle(0.0f)
{
    SetQuantities(1, 1);
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::~CSpeedWind2

CSpeedWind2::~CSpeedWind2( )
{
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetQuantities

void CSpeedWind2::SetQuantities(int iNumWindMatrices, int iNumLeafAngles)
{
    if (iNumWindMatrices < 1)
        iNumWindMatrices = 1;

    if (iNumLeafAngles < 1)
        iNumLeafAngles = 1;

    m_vWindMatrices.clear( );
    m_vWindMatrices.resize(iNumWindMatrices);

    m_sBranchHorizontal.m_vOutputs.clear( );
    m_sBranchVertical.m_vOutputs.clear( );
    m_sBranchHorizontal.m_vOutputs.resize(iNumWindMatrices);
    m_sBranchVertical.m_vOutputs.resize(iNumWindMatrices);

    m_sLeafRocking.m_vOutputs.clear( );
    m_sLeafRustling.m_vOutputs.clear( );
    m_vLeafAngleMatrices.clear( );
    m_sLeafRocking.m_vOutputs.resize(iNumLeafAngles);
    m_sLeafRustling.m_vOutputs.resize(iNumLeafAngles);
    m_vLeafAngleMatrices.resize(iNumLeafAngles);
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetWindResponse

void CSpeedWind2::SetWindResponse(float fResponse, float fReponseLimit)
{
    m_fWindResponse = fResponse;
    m_fWindResponseLimit = fReponseLimit;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetWindStrengthAndDirection

void CSpeedWind2::SetWindStrengthAndDirection(float fWindStrength, float fWindDirectionX, float fWindDirectionY, float fWindDirectionZ)
{
    if (fWindStrength > 1.0f)
        fWindStrength = 1.0f;
    if (fWindStrength < 0.0f)
        fWindStrength = 0.0f;
    m_fWindStrength = fWindStrength;

    float fSum = fWindDirectionX * fWindDirectionX + fWindDirectionY * fWindDirectionY + fWindDirectionZ * fWindDirectionZ;
    if (fSum != 0.0f)
    {
        fSum = sqrt(fSum);
        fWindDirectionX /= fSum;
        fWindDirectionY /= fSum;
        fWindDirectionZ /= fSum;
    }
    m_cFinalWindDirectionX.m_fWantedValue = fWindDirectionX;
    m_cFinalWindDirectionY.m_fWantedValue = fWindDirectionY;
    m_cFinalWindDirectionZ.m_fWantedValue = fWindDirectionZ;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetMaxBendAngle

void CSpeedWind2::SetMaxBendAngle(float fMaxBendAngle)
{
    m_fMaxBendAngle = fMaxBendAngle;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetExponents

void CSpeedWind2::SetExponents(float fBranchExponent, float fLeafExponent)
{
    m_fBranchExponent = fBranchExponent;
    m_fLeafExponent = fLeafExponent;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetGusting

void CSpeedWind2::SetGusting(float fGustStrength, float fGustFrequency, float fGustDuration)
{
    m_fGustingStrength = fGustStrength;
    m_fGustingFrequency = fGustFrequency;
    m_fGustingDuration = fGustDuration;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetBranchHorizontal

void CSpeedWind2::SetBranchHorizontal(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed)
{
    m_sBranchHorizontal.m_fLowWindAngle = fLowWindAngle;
    m_sBranchHorizontal.m_fHighWindAngle = fHighWindAngle;
    m_sBranchHorizontal.m_fLowWindSpeed = fLowWindSpeed;
    m_sBranchHorizontal.m_fHighWindSpeed = fHighWindSpeed;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetBranchVertical

void CSpeedWind2::SetBranchVertical(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed)
{
    m_sBranchVertical.m_fLowWindAngle = fLowWindAngle;
    m_sBranchVertical.m_fHighWindAngle = fHighWindAngle;
    m_sBranchVertical.m_fLowWindSpeed = fLowWindSpeed;
    m_sBranchVertical.m_fHighWindSpeed = fHighWindSpeed;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetLeafRocking

void CSpeedWind2::SetLeafRocking(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed)
{
    m_sLeafRocking.m_fLowWindAngle = fLowWindAngle;
    m_sLeafRocking.m_fHighWindAngle = fHighWindAngle;
    m_sLeafRocking.m_fLowWindSpeed = fLowWindSpeed;
    m_sLeafRocking.m_fHighWindSpeed = fHighWindSpeed;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::SetLeafRustling

void CSpeedWind2::SetLeafRustling(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed)
{

    m_sLeafRustling.m_fLowWindAngle = fLowWindAngle;
    m_sLeafRustling.m_fHighWindAngle = fHighWindAngle;
    m_sLeafRustling.m_fLowWindSpeed = fLowWindSpeed;
    m_sLeafRustling.m_fHighWindSpeed = fHighWindSpeed;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::Advance

void CSpeedWind2::Advance(float fCurrentTime, bool bUpdateMatrices, bool bUpdateLeafAngleMatrices, float fCameraX, float fCameraY, float fCameraZ)
{
    float fDeltaTime = fCurrentTime - m_fLastTime;
    m_fLastTime = fCurrentTime;

    // stop current gust if needed
    if (fCurrentTime > m_fGustStopTime)
    {
        m_fCurrentGustingStrength = 0.0f;
    
        // make new gusts
        if (m_fGustingFrequency * fDeltaTime > (float)rand( ) / (float)RAND_MAX)
        {
            m_fCurrentGustingStrength = m_fGustingStrength * ((float)rand( ) / (float)RAND_MAX + 0.25f);
            m_fGustStopTime = fCurrentTime + m_fGustingDuration * 0.5f * ((float)rand( ) / (float)RAND_MAX + 1.0f);
        }
    }

    // compute the actual wind strength due to gusting
    m_cFinalWindStrength.m_fWantedValue = m_fWindStrength + m_fCurrentGustingStrength;
    if (m_cFinalWindStrength.m_fWantedValue > 1.0f)
        m_cFinalWindStrength.m_fWantedValue = 1.0f;
    if (m_cFinalWindStrength.m_fWantedValue < 0.0f)
        m_cFinalWindStrength.m_fWantedValue = 0.0f;
    m_cFinalWindStrength.Advance(fDeltaTime, m_fWindResponse, m_fWindResponseLimit);
    if (m_cFinalWindStrength.m_fCurrentValue > 1.0f)
        m_cFinalWindStrength.m_fCurrentValue = 1.0f;
    if (m_cFinalWindStrength.m_fCurrentValue < 0.0f)
        m_cFinalWindStrength.m_fCurrentValue = 0.0f;

    // smooth out the direction
    m_cFinalWindDirectionX.Advance(fDeltaTime, m_fWindResponse, m_fWindResponseLimit);
    m_cFinalWindDirectionY.Advance(fDeltaTime, m_fWindResponse, m_fWindResponseLimit);
    m_cFinalWindDirectionZ.Advance(fDeltaTime, m_fWindResponse, m_fWindResponseLimit);

    // compute the branch and leaf wind strengths due to exponents
    float fBranchStrength = powf(m_cFinalWindStrength.m_fCurrentValue, m_fBranchExponent);
    float fLeafStrength = powf(m_cFinalWindStrength.m_fCurrentValue, m_fLeafExponent);

    // update the main tree bend angle (and compensate for unnormalized, over the pole movement)
    float fSum = m_cFinalWindDirectionX.m_fCurrentValue * m_cFinalWindDirectionX.m_fCurrentValue + 
                    m_cFinalWindDirectionY.m_fCurrentValue * m_cFinalWindDirectionY.m_fCurrentValue + 
                    m_cFinalWindDirectionZ.m_fCurrentValue * m_cFinalWindDirectionZ.m_fCurrentValue;
    if (fSum != 0.0f)
        fSum = sqrt(fSum);
    m_fFinalBendAngle = fBranchStrength * m_fMaxBendAngle * fSum;

    // update the oscillating parts
    m_sBranchHorizontal.Advance(fDeltaTime, fBranchStrength);
    m_sBranchVertical.Advance(fDeltaTime, fBranchStrength);
    m_sLeafRocking.Advance(fDeltaTime, fLeafStrength);
    m_sLeafRustling.Advance(fDeltaTime, fLeafStrength);

    if (bUpdateMatrices)
    {
        // build wind matrices
        float fWindAngle = -static_cast<float>(atan2(m_cFinalWindDirectionX.m_fCurrentValue, m_cFinalWindDirectionY.m_fCurrentValue));
        float fXRot = -cosf(fWindAngle);
        float fYRot = -sinf(fWindAngle);

        CSpeedWindMatrix2 cTemp;
        unsigned int uiSize = (unsigned int)m_vWindMatrices.size( );
        for (unsigned int i = 0; i < uiSize; ++i)
        {
            // vertical oscillation and bend angle are in the same direction, so do them at the same time
            m_vWindMatrices[i].SetRotation(m_fFinalBendAngle + m_sBranchVertical.m_vOutputs[i], fXRot, fYRot, 0.0f);

            // handle horizontal oscillation
            cTemp.SetRotation(m_sBranchHorizontal.m_vOutputs[i], -fYRot, fXRot, 0.0f);

            // final matrix
            m_vWindMatrices[i] = m_vWindMatrices[i] * cTemp;
        }
    }

    if (bUpdateLeafAngleMatrices)
    {
        // build leaf angle matrices
        float afAdjustedCameraDir[3];

        #ifdef SPEEDWIND_UPVECTOR_POS_Z
            afAdjustedCameraDir[0] = fCameraX;
            afAdjustedCameraDir[1] = fCameraY;
            afAdjustedCameraDir[2] = fCameraZ;
        #endif
        #ifdef SPEEDWIND_UPVECTOR_NEG_Z
            afAdjustedCameraDir[0] = -fCameraX;
            afAdjustedCameraDir[1] = fCameraY;
            afAdjustedCameraDir[2] = -fCameraZ;
        #endif
        #ifdef SPEEDWIND_UPVECTOR_POS_Y
            afAdjustedCameraDir[0] = -fCameraX;
            afAdjustedCameraDir[1] = fCameraZ;
            afAdjustedCameraDir[2] = fCameraY;
        #endif
        #ifdef SPEEDWIND_UPVECTOR_DIRECTX_RIGHT_HANDED_COORDINATE_SYSTEM
            afAdjustedCameraDir[0] = fCameraY;
            afAdjustedCameraDir[1] = fCameraX;
            afAdjustedCameraDir[2] = fCameraZ;
        #endif

        float fAzimuth = atan2f(afAdjustedCameraDir[1], afAdjustedCameraDir[0]) * c_fRad2Deg;
        float fPitch = -asinf(afAdjustedCameraDir[2]) * c_fRad2Deg;

        unsigned int uiSize = (unsigned int)m_vLeafAngleMatrices.size( );
        for (unsigned int i = 0; i < uiSize; ++i)
        {
            CSpeedWindMatrix2& cMatrix = m_vLeafAngleMatrices[i];

            cMatrix.LoadIdentity( );
            #ifdef SPEEDWIND_UPVECTOR_POS_Z
                cMatrix.RotateZ(fAzimuth);
                cMatrix.RotateY(fPitch);
                cMatrix.RotateZ(m_sLeafRustling.m_vOutputs[i]);
                cMatrix.RotateX(m_sLeafRocking.m_vOutputs[i]);
            #endif
            #ifdef SPEEDWIND_UPVECTOR_NEG_Z
                cMatrix.RotateZ(-fAzimuth);
                cMatrix.RotateY(fPitch);
                cMatrix.RotateZ(m_sLeafRustling.m_vOutputs[i]);
                cMatrix.RotateX(m_sLeafRocking.m_vOutputs[i]);
            #endif
            #ifdef SPEEDWIND_UPVECTOR_POS_Y
                cMatrix.RotateY(fAzimuth);
                cMatrix.RotateZ(fPitch);
                cMatrix.RotateY(-m_sLeafRustling.m_vOutputs[i]);
                cMatrix.RotateX(m_sLeafRocking.m_vOutputs[i]);
            #endif
            #ifdef SPEEDWIND_UPVECTOR_DIRECTX_RIGHT_HANDED_COORDINATE_SYSTEM
                cMatrix.RotateZ(-fAzimuth);
                cMatrix.RotateX(-fPitch);
                cMatrix.RotateZ(m_sLeafRustling.m_vOutputs[i]);
                cMatrix.RotateY(m_sLeafRocking.m_vOutputs[i]);
            #endif
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::UpdateTree

void CSpeedWind2::UpdateTree(CSpeedTreeRT* pTree)
{
    pTree->SetWindStrengthAndLeafAngles(m_cFinalWindStrength.m_fCurrentValue, 
                                        &(m_sLeafRocking.m_vOutputs[0]), 
                                        &(m_sLeafRustling.m_vOutputs[0]), 
                                        int(m_sLeafRocking.m_vOutputs.size( )));
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::UpdateSpeedTreeRT

void CSpeedWind2::UpdateSpeedTreeRT(void)
{
    unsigned int uiSize = (unsigned int) m_vWindMatrices.size( );
    for (unsigned int i = 0; i < uiSize; ++i)
        CSpeedTreeRT::SetWindMatrix(i, (const float*)m_vWindMatrices[i].m_afData);
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetFinalStrength

float CSpeedWind2::GetFinalStrength(void)
{
    return m_cFinalWindStrength.m_fCurrentValue;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetNumWindMatrices

unsigned int CSpeedWind2::GetNumWindMatrices(void) const
{
    return (unsigned int) m_vWindMatrices.size( );
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetWindMatrix

float* CSpeedWind2::GetWindMatrix(unsigned int uiIndex) const
{
    float* pReturn = NULL;

    if (uiIndex < m_vWindMatrices.size( ))
        pReturn = ((float*)m_vWindMatrices[uiIndex].m_afData);

    return pReturn;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetNumWindMatrices

unsigned int CSpeedWind2::GetNumLeafAngleMatrices(void) const
{
    return (unsigned int) m_vLeafAngleMatrices.size( );
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetLeafAngleMatrix

float* CSpeedWind2::GetLeafAngleMatrix(unsigned int uiIndex) const
{
    float* pReturn = NULL;

    if (uiIndex < m_vWindMatrices.size( ))
        pReturn = ((float*)m_vLeafAngleMatrices[uiIndex].m_afData);

    return pReturn;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::Load

bool CSpeedWind2::Load(const wind_string& strFilename)
{
    bool bSuccess = false;

	ntstd::Ifstream isData(strFilename.c_str( ));
    if (isData.is_open( ))
    {
        bSuccess = Load(isData);
        isData.close( );
    }

    return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::Load

bool CSpeedWind2::Load(ntstd::Istream& isData)
{
    bool bSuccess = false;

    isData.exceptions(ntstd::Istream::failbit | ntstd::Istream::badbit | ntstd::Istream::eofbit);

    //try
    {
        wind_string strToken;
        float fTemp0, fTemp1, fTemp2, fTemp3;
        fTemp0 = fTemp1 = fTemp2 = fTemp3 = 0.0f;

        for (unsigned int i = 0; i < 10; ++i)
        {
            isData >> strToken;

            if (strToken == "NumWindMatricesAndLeafAngles")
            {
                isData >> fTemp0 >> fTemp1;
                SetQuantities((int)fTemp0, (int)fTemp1);
            }
            else if (strToken == "WindResponseAndLimit")
            {
                isData >> fTemp0 >> fTemp1;
                SetWindResponse(fTemp0, fTemp1);
            }
            else if (strToken == "MaxBendAngle")
            {
                isData >> m_fMaxBendAngle;
            }
            else if (strToken == "BranchExponent")
            {
                isData >> m_fBranchExponent;
            }
            else if (strToken == "LeafExponent")
            {
                isData >> m_fLeafExponent;
            }
            else if (strToken == "GustStrengthFreqDuration")
            {
                isData >> fTemp0 >> fTemp1 >> fTemp2;
                SetGusting(fTemp0, fTemp1, fTemp2);
            }
            else if (strToken == "BranchOscillationX_LaLsHaHs")
            {
                isData >> fTemp0 >> fTemp1 >> fTemp2 >> fTemp3;
                SetBranchHorizontal(fTemp0, fTemp2, fTemp1, fTemp3);
            }
            else if (strToken == "BranchOscillationY_LaLsHaHs")
            {
                isData >> fTemp0 >> fTemp1 >> fTemp2 >> fTemp3;
                SetBranchVertical(fTemp0, fTemp2, fTemp1, fTemp3);
            }
            else if (strToken == "LeafRocking_LaLsHaHs")
            {
                isData >> fTemp0 >> fTemp1 >> fTemp2 >> fTemp3;
                SetLeafRocking(fTemp0, fTemp2, fTemp1, fTemp3);
            }
            else if (strToken == "LeafRustling_LaLsHaHs")
            {
                isData >> fTemp0 >> fTemp1 >> fTemp2 >> fTemp3;
                SetLeafRustling(fTemp0, fTemp2, fTemp1, fTemp3);
            }
        }

        bSuccess = true;
    }
    //catch (istream::failure e)
    //{
    //    bSuccess = false;
    //}

    return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::Save

bool CSpeedWind2::Save(const wind_string& strFilename) const
{
    bool bSuccess = false;

    ofstream osData(strFilename.c_str( ));
    if (osData.is_open( ))
    {
        bSuccess = Save(osData);
        osData.close( );
    }

    return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::Save

bool CSpeedWind2::Save(ntstd::Ostream& osData) const
{
    bool bSuccess = false;

    osData.exceptions(ntstd::Ostream::failbit | ntstd::Ostream::badbit);

    //try
    {
        osData << "NumWindMatricesAndLeafAngles " << (unsigned int) m_vWindMatrices.size( ) << " " << (unsigned int) m_sLeafRocking.m_vOutputs.size( ) << endl;
        osData << "WindResponseAndLimit " << m_fWindResponse << " " << m_fWindResponseLimit << endl;
        osData << "MaxBendAngle " << m_fMaxBendAngle << endl;
        osData << "BranchExponent " << m_fBranchExponent << endl;
        osData << "LeafExponent " << m_fLeafExponent << endl;
        osData << "GustStrengthFreqDuration " << m_fGustingStrength << " " << m_fGustingFrequency << " " << m_fGustingDuration << endl;
        osData << "BranchOscillationX_LaLsHaHs " << m_sBranchHorizontal.m_fLowWindAngle << " "
                                                    << m_sBranchHorizontal.m_fLowWindSpeed << " "
                                                    << m_sBranchHorizontal.m_fHighWindAngle  << " "
                                                    << m_sBranchHorizontal.m_fHighWindSpeed << endl;
        osData << "BranchOscillationY_LaLsHaHs " << m_sBranchVertical.m_fLowWindAngle << " "
                                                    << m_sBranchVertical.m_fLowWindSpeed << " "
                                                    << m_sBranchVertical.m_fHighWindAngle  << " "
                                                    << m_sBranchVertical.m_fHighWindSpeed << endl;
        osData << "LeafRocking_LaLsHaHs " << m_sLeafRocking.m_fLowWindAngle << " "
                                            << m_sLeafRocking.m_fLowWindSpeed << " "
                                            << m_sLeafRocking.m_fHighWindAngle  << " "
                                            << m_sLeafRocking.m_fHighWindSpeed << endl;
        osData << "LeafRustling_LaLsHaHs " << m_sLeafRustling.m_fLowWindAngle << " "
                                            << m_sLeafRustling.m_fLowWindSpeed << " "
                                            << m_sLeafRustling.m_fHighWindAngle  << " "
                                            << m_sLeafRustling.m_fHighWindSpeed << endl;

        bSuccess = true;
    }
    //catch (ostream::failure e)
    //{
    //    bSuccess = false;
    //}

    return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::Reset

void CSpeedWind2::Reset(void)
{
    unsigned int i = 0;
    unsigned int uiSize = (unsigned int) m_vWindMatrices.size( );
    for (i = 0; i < uiSize; ++i)
        m_vWindMatrices[i].LoadIdentity( );

    uiSize = (unsigned int) m_vLeafAngleMatrices.size( );
    for (i = 0; i < uiSize; ++i)
        m_vLeafAngleMatrices[i].LoadIdentity( );

    m_cFinalWindStrength.Reset(m_fWindStrength);
    m_cFinalWindDirectionX.Reset(m_cFinalWindDirectionX.m_fWantedValue);
    m_cFinalWindDirectionY.Reset(m_cFinalWindDirectionY.m_fWantedValue);
    m_cFinalWindDirectionZ.Reset(m_cFinalWindDirectionZ.m_fWantedValue);
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetQuantities

void CSpeedWind2::GetQuantities(int& iNumWindMatrices, int& iNumLeafAngles)
{
    iNumWindMatrices = (int)m_vWindMatrices.size( );
    iNumLeafAngles = (int)m_vLeafAngleMatrices.size( );
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetWindResponse

void CSpeedWind2::GetWindResponse(float& fResponse, float& fReponseLimit)
{
    fResponse = m_fWindResponse;
    fReponseLimit = m_fWindResponseLimit;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetWindStrengthAndDirection

void CSpeedWind2::GetWindStrengthAndDirection(float& fWindStrength, float& fWindDirectionX, float& fWindDirectionY, float& fWindDirectionZ)
{
    fWindStrength = m_fWindStrength;
    fWindDirectionX = m_cFinalWindDirectionX.m_fWantedValue;
    fWindDirectionY = m_cFinalWindDirectionY.m_fWantedValue;
    fWindDirectionZ = m_cFinalWindDirectionZ.m_fWantedValue;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetMaxBendAngle

float CSpeedWind2::GetMaxBendAngle(void)
{
    return m_fMaxBendAngle;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetExponents

void CSpeedWind2::GetExponents(float& fBranchExponent, float& fLeafExponent)
{
    fBranchExponent = m_fBranchExponent;
    fLeafExponent = m_fLeafExponent;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetGusting

void CSpeedWind2::GetGusting(float& fGustStrength, float& fGustFrequency, float& fGustDuration)
{
    fGustStrength = m_fGustingStrength;
    fGustFrequency = m_fGustingFrequency;
    fGustDuration = m_fGustingDuration;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetBranchHorizontal

void CSpeedWind2::GetBranchHorizontal(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed)
{
    fLowWindAngle = m_sBranchHorizontal.m_fLowWindAngle;
    fHighWindAngle = m_sBranchHorizontal.m_fHighWindAngle;
    fLowWindSpeed = m_sBranchHorizontal.m_fLowWindSpeed;
    fHighWindSpeed = m_sBranchHorizontal.m_fHighWindSpeed;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetBranchVertical

void CSpeedWind2::GetBranchVertical(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed)
{
    fLowWindAngle = m_sBranchVertical.m_fLowWindAngle;
    fHighWindAngle = m_sBranchVertical.m_fHighWindAngle;
    fLowWindSpeed = m_sBranchVertical.m_fLowWindSpeed;
    fHighWindSpeed = m_sBranchVertical.m_fHighWindSpeed;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetLeafRocking

void CSpeedWind2::GetLeafRocking(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed)
{
    fLowWindAngle = m_sLeafRocking.m_fLowWindAngle;
    fHighWindAngle = m_sLeafRocking.m_fHighWindAngle;
    fLowWindSpeed = m_sLeafRocking.m_fLowWindSpeed;
    fHighWindSpeed = m_sLeafRocking.m_fHighWindSpeed;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetLeafRustling

void CSpeedWind2::GetLeafRustling(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed)
{
    fLowWindAngle = m_sLeafRustling.m_fLowWindAngle;
    fHighWindAngle = m_sLeafRustling.m_fHighWindAngle;
    fLowWindSpeed = m_sLeafRustling.m_fLowWindSpeed;
    fHighWindSpeed = m_sLeafRustling.m_fHighWindSpeed;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetRustleAngles

bool CSpeedWind2::GetRustleAngles(wind_float_vector& vAngles) const
{
    bool bSuccess = false;

    if (vAngles.size( ) == m_sLeafRustling.m_vOutputs.size( ))
    {
        NT_MEMCPY(&vAngles[0], &m_sLeafRustling.m_vOutputs[0], vAngles.size( ) * sizeof(float));
        bSuccess = true;
    }

    return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::GetRockAngles

bool CSpeedWind2::GetRockAngles(wind_float_vector& vAngles) const
{
    bool bSuccess = false;

    if (vAngles.size( ) == m_sLeafRocking.m_vOutputs.size( ))
    {
        NT_MEMCPY(&vAngles[0], &m_sLeafRocking.m_vOutputs[0], vAngles.size( ) * sizeof(float));
        bSuccess = true;
    }

    return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::InterpolateParameters

void CSpeedWind2::InterpolateParameters(CSpeedWind2* pWind1, CSpeedWind2* pWind2, float fInterpolation)
{
#define INTERPOLATE_WIND_PARAM(a) a = INTERPOLATE(pWind1->a, pWind2->a, fInterpolation)

    if (pWind1 != NULL && pWind2 != NULL)
    {
        INTERPOLATE_WIND_PARAM(m_fWindStrength);
        INTERPOLATE_WIND_PARAM(m_cFinalWindDirectionX.m_fWantedValue);
        INTERPOLATE_WIND_PARAM(m_cFinalWindDirectionY.m_fWantedValue);
        INTERPOLATE_WIND_PARAM(m_cFinalWindDirectionZ.m_fWantedValue);

        // normalize direction
        float fSum = m_cFinalWindDirectionX.m_fWantedValue * m_cFinalWindDirectionX.m_fWantedValue + 
                    m_cFinalWindDirectionY.m_fWantedValue * m_cFinalWindDirectionY.m_fWantedValue + 
                    m_cFinalWindDirectionZ.m_fWantedValue * m_cFinalWindDirectionZ.m_fWantedValue;
        if (fSum != 0.0f)
        {
            fSum = sqrt(fSum);
            m_cFinalWindDirectionX.m_fWantedValue /= fSum;
            m_cFinalWindDirectionY.m_fWantedValue /= fSum;
            m_cFinalWindDirectionZ.m_fWantedValue /= fSum;
        }

        INTERPOLATE_WIND_PARAM(m_sBranchHorizontal.m_fLowWindAngle);
        INTERPOLATE_WIND_PARAM(m_sBranchHorizontal.m_fHighWindAngle);
        INTERPOLATE_WIND_PARAM(m_sBranchHorizontal.m_fLowWindSpeed);
        INTERPOLATE_WIND_PARAM(m_sBranchHorizontal.m_fHighWindSpeed);

        INTERPOLATE_WIND_PARAM(m_sBranchVertical.m_fLowWindAngle);
        INTERPOLATE_WIND_PARAM(m_sBranchVertical.m_fHighWindAngle);
        INTERPOLATE_WIND_PARAM(m_sBranchVertical.m_fLowWindSpeed);
        INTERPOLATE_WIND_PARAM(m_sBranchVertical.m_fHighWindSpeed);

        INTERPOLATE_WIND_PARAM(m_sLeafRocking.m_fLowWindAngle);
        INTERPOLATE_WIND_PARAM(m_sLeafRocking.m_fHighWindAngle);
        INTERPOLATE_WIND_PARAM(m_sLeafRocking.m_fLowWindSpeed);
        INTERPOLATE_WIND_PARAM(m_sLeafRocking.m_fHighWindSpeed);

        INTERPOLATE_WIND_PARAM(m_sLeafRustling.m_fLowWindAngle);
        INTERPOLATE_WIND_PARAM(m_sLeafRustling.m_fHighWindAngle);
        INTERPOLATE_WIND_PARAM(m_sLeafRustling.m_fLowWindSpeed);
        INTERPOLATE_WIND_PARAM(m_sLeafRustling.m_fHighWindSpeed);

        INTERPOLATE_WIND_PARAM(m_fMaxBendAngle);
        INTERPOLATE_WIND_PARAM(m_fWindResponse);
        INTERPOLATE_WIND_PARAM(m_fWindResponseLimit);
        INTERPOLATE_WIND_PARAM(m_fGustingStrength);
        INTERPOLATE_WIND_PARAM(m_fGustingFrequency);
        INTERPOLATE_WIND_PARAM(m_fGustingDuration);
        INTERPOLATE_WIND_PARAM(m_fBranchExponent);
        INTERPOLATE_WIND_PARAM(m_fLeafExponent);
        
    }

#undef INTERPOLATE_WIND_PARAM
}


/////////////////////////////////////////////////////////////////////////////
// CSpeedWind2::BlendParameters

void CSpeedWind2::BlendParameters(CSpeedWind2** pWinds, float* pWeights, unsigned int uiNumWinds)
{
    // zero out our data
    m_fWindStrength = 0.0f;
    m_cFinalWindDirectionX.m_fWantedValue = 0.0f;
    m_cFinalWindDirectionY.m_fWantedValue = 0.0f;
    m_cFinalWindDirectionZ.m_fWantedValue = 0.0f;

    m_sBranchHorizontal.m_fLowWindAngle = 0.0f;
    m_sBranchHorizontal.m_fHighWindAngle = 0.0f;
    m_sBranchHorizontal.m_fLowWindSpeed = 0.0f;
    m_sBranchHorizontal.m_fHighWindSpeed = 0.0f;

    m_sBranchVertical.m_fLowWindAngle = 0.0f;
    m_sBranchVertical.m_fHighWindAngle = 0.0f;
    m_sBranchVertical.m_fLowWindSpeed = 0.0f;
    m_sBranchVertical.m_fHighWindSpeed = 0.0f;

    m_sLeafRocking.m_fLowWindAngle = 0.0f;
    m_sLeafRocking.m_fHighWindAngle = 0.0f;
    m_sLeafRocking.m_fLowWindSpeed = 0.0f;
    m_sLeafRocking.m_fHighWindSpeed = 0.0f;

    m_sLeafRustling.m_fLowWindAngle = 0.0f;
    m_sLeafRustling.m_fHighWindAngle = 0.0f;
    m_sLeafRustling.m_fLowWindSpeed = 0.0f;
    m_sLeafRustling.m_fHighWindSpeed = 0.0f;

    m_fMaxBendAngle = 0.0f;
    m_fWindResponse = 0.0f;
    m_fWindResponseLimit = 0.0f;
    m_fGustingStrength = 0.0f;
    m_fGustingFrequency = 0.0f;
    m_fGustingDuration = 0.0f;
    m_fBranchExponent = 0.0f;
    m_fLeafExponent = 0.0f;

    
#define SUM_WIND_PARAM(a) a = pWinds[i]->a * pWeights[i]

    // fill data based on winds and weights
    for (unsigned int i = 0; i < uiNumWinds; ++i)
    {
        SUM_WIND_PARAM(m_fWindStrength);
        SUM_WIND_PARAM(m_cFinalWindDirectionX.m_fWantedValue);
        SUM_WIND_PARAM(m_cFinalWindDirectionY.m_fWantedValue);
        SUM_WIND_PARAM(m_cFinalWindDirectionZ.m_fWantedValue);

        SUM_WIND_PARAM(m_sBranchHorizontal.m_fLowWindAngle);
        SUM_WIND_PARAM(m_sBranchHorizontal.m_fHighWindAngle);
        SUM_WIND_PARAM(m_sBranchHorizontal.m_fLowWindSpeed);
        SUM_WIND_PARAM(m_sBranchHorizontal.m_fHighWindSpeed);

        SUM_WIND_PARAM(m_sBranchVertical.m_fLowWindAngle);
        SUM_WIND_PARAM(m_sBranchVertical.m_fHighWindAngle);
        SUM_WIND_PARAM(m_sBranchVertical.m_fLowWindSpeed);
        SUM_WIND_PARAM(m_sBranchVertical.m_fHighWindSpeed);

        SUM_WIND_PARAM(m_sLeafRocking.m_fLowWindAngle);
        SUM_WIND_PARAM(m_sLeafRocking.m_fHighWindAngle);
        SUM_WIND_PARAM(m_sLeafRocking.m_fLowWindSpeed);
        SUM_WIND_PARAM(m_sLeafRocking.m_fHighWindSpeed);

        SUM_WIND_PARAM(m_sLeafRustling.m_fLowWindAngle);
        SUM_WIND_PARAM(m_sLeafRustling.m_fHighWindAngle);
        SUM_WIND_PARAM(m_sLeafRustling.m_fLowWindSpeed);
        SUM_WIND_PARAM(m_sLeafRustling.m_fHighWindSpeed);

        SUM_WIND_PARAM(m_fMaxBendAngle);
        SUM_WIND_PARAM(m_fWindResponse);
        SUM_WIND_PARAM(m_fWindResponseLimit);
        SUM_WIND_PARAM(m_fGustingStrength);
        SUM_WIND_PARAM(m_fGustingFrequency);
        SUM_WIND_PARAM(m_fGustingDuration);
        SUM_WIND_PARAM(m_fBranchExponent);
        SUM_WIND_PARAM(m_fLeafExponent);
    }

#undef SUM_WIND_PARAM

    // normalize direction
    float fSum = m_cFinalWindDirectionX.m_fWantedValue * m_cFinalWindDirectionX.m_fWantedValue + 
                m_cFinalWindDirectionY.m_fWantedValue * m_cFinalWindDirectionY.m_fWantedValue + 
                m_cFinalWindDirectionZ.m_fWantedValue * m_cFinalWindDirectionZ.m_fWantedValue;
    if (fSum != 0.0f)
    {
        fSum = sqrt(fSum);
        m_cFinalWindDirectionX.m_fWantedValue /= fSum;
        m_cFinalWindDirectionY.m_fWantedValue /= fSum;
        m_cFinalWindDirectionZ.m_fWantedValue /= fSum;
    }
}
