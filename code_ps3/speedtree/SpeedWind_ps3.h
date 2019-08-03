///////////////////////////////////////////////////////////////////////  
//  SpeedWind2.h
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

#pragma once
#include "SpeedTreeRT.h"

// Use SpeedTreeRT allocators if available
#ifdef SPEEDTREE_FRAMEWORK
	#include "stl_vector.h"
    #include "SpeedTreeMemory.h"
    #define wind_string st_string
    #define wind_float_vector st_vector_float
#else
    #define wind_string ntstd::String
    #define wind_float_vector ntstd::Vector<float>
#endif


/////////////////////////////////////////////////////////////////////////////
// Up Vector
//
//  SpeedTree defaults to using a positive Z up vector and all of the branch
//  and leaf computations are done in this orientation.  If you are using a
//  SpeedTree build with a different up vector, make sure you define the same
//  up vector here as the one used by SpeedTree in the file "UpVector.h"
//
//  One and only one of the following seven symbols should be defined:
//
//      SPEEDWIND_UPVECTOR_POS_Z
//      SPEEDWIND_UPVECTOR_NEG_Z
//      SPEEDWIND_UPVECTOR_POS_Y
//      SPEEDWIND_UPVECTOR_DIRECTX_RIGHT_HANDED_COORDINATE_SYSTEM
#ifndef SPEEDWIND_UPVECTOR_POS_Y
#define SPEEDWIND_UPVECTOR_POS_Y
#endif

#ifdef UPVECTOR_POS_Y
    #define SPEEDWIND_UPVECTOR_POS_Y
#else
    #define SPEEDWIND_UPVECTOR_POS_Z
#endif


/////////////////////////////////////////////////////////////////////////////
// class CSpeedWindMatrix2

class CSpeedWindMatrix2
{
public:
                                CSpeedWindMatrix2( );
                                ~CSpeedWindMatrix2( );

        void                    LoadIdentity(void);
        void                    SetRotation(float fAngle, float fX, float fY, float fZ);
        void                    RotateX(float fAngle);
        void                    RotateY(float fAngle);
        void                    RotateZ(float fAngle);
        CSpeedWindMatrix2       operator*(const CSpeedWindMatrix2& cTrans) const;

public:
        float                   m_afData[4][4];                             // matrix data
};


///////////////////////////////////////////////////////////////////////  
//  SpeedTree memory management
//
//  Used to capture allocations in STL containers to report back to
//  SpeedTreeRT's allocation mechanism

#ifdef SPEEDTREE_FRAMEWORK
    DefineAllocator(st_allocator_stmatrix2);
    typedef ntstd::Vector<CSpeedWindMatrix2, st_allocator_stmatrix2<CSpeedWindMatrix2> > st_vector_stmatrix2;
#endif


/////////////////////////////////////////////////////////////////////////////
// class CSpeedWind2

class CSpeedWind2
{
public:
                                        CSpeedWind2( );
                                        ~CSpeedWind2( );

        // parameter setting
        void                            Reset(void);
        void                            SetQuantities(int iNumWindMatrices, int iNumLeafAngles);
        void                            SetWindResponse(float fResponse, float fReponseLimit);
        void                            SetWindStrengthAndDirection(float fWindStrength, float fWindDirectionX, float fWindDirectionY, float fWindDirectionZ);
        void                            SetMaxBendAngle(float fMaxBendAngle);
        void                            SetExponents(float fBranchExponent, float fLeafExponent);
        void                            SetGusting(float fGustStrength, float fGustFrequency, float fGustDuration);
        void                            SetBranchHorizontal(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed);
        void                            SetBranchVertical(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed);
        void                            SetLeafRocking(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed);
        void                            SetLeafRustling(float fLowWindAngle, float fHighWindAngle, float fLowWindSpeed, float fHighWindSpeed);

        // parameter getting
        void                            GetQuantities(int& iNumWindMatrices, int& iNumLeafAngles);
        void                            GetWindResponse(float& fResponse, float& fReponseLimit);
        void                            GetWindStrengthAndDirection(float& fWindStrength, float& fWindDirectionX, float& fWindDirectionY, float& fWindDirectionZ);
        float                           GetMaxBendAngle(void);
        void                            GetExponents(float& fBranchExponent, float& fLeafExponent);
        void                            GetGusting(float& fGustStrength, float& fGustFrequency, float& fGustDuration);
        void                            GetBranchHorizontal(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed);
        void                            GetBranchVertical(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed);
        void                            GetLeafRocking(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed);
        void                            GetLeafRustling(float& fLowWindAngle, float& fHighWindAngle, float& fLowWindSpeed, float& fHighWindSpeed);

        // render interface
        void                            Advance(float fCurrentTime, bool bUpdateMatrices = true, bool bUpdateLeafAngleMatrices = false, float fCameraX = 0.0f, float fCameraY = 0.0f, float fCameraZ = 1.0f);
        void                            UpdateSpeedTreeRT(void);
        void                            UpdateTree(CSpeedTreeRT* pTree);
        float                           GetFinalStrength(void);
        unsigned int                    GetNumWindMatrices(void) const;
        float*                          GetWindMatrix(unsigned int uiIndex) const;
        unsigned int                    GetNumLeafAngleMatrices(void) const;
        float*                          GetLeafAngleMatrix(unsigned int uiIndex) const;
        bool                            GetRustleAngles(wind_float_vector& vAngles) const;  // assumes vector is already appropriately sized
        bool                            GetRockAngles(wind_float_vector& vAngles) const;    // assumes vector is already appropriately sized

        // file I/O
        bool                            Load(const wind_string& strFilename);
        bool                            Load(ntstd::Istream& isData);
        bool                            Save(const wind_string& strFilename) const;
        bool                            Save(ntstd::Ostream& osData) const;

        // blending SpeedWinds into this one
        void                            InterpolateParameters(CSpeedWind2* pWind1, CSpeedWind2* pWind2, float fInterpolation);
        void                            BlendParameters(CSpeedWind2** pWinds, float* pWeights, unsigned int uiNumWinds);
    
protected:

        struct SOscillationParams
        {
                                        SOscillationParams(float fLowWindAngle = 10.0f, float fHighWindAngle = 4.0f, float fLowWindSpeed = 0.01f, float fHighWindSpeed = 0.3f);

                void                    Advance(float fDeltaTime, float fWindStrendth);

                float                   m_fLowWindAngle;                    // The amount of movement at 0.0 wind strength
                float                   m_fHighWindAngle;                   // The amount of movement at 1.0 wind strength
                float                   m_fLowWindSpeed;                    // The speed of movement at 0.0m wind strength
                float                   m_fHighWindSpeed;                   // The speed of movement at 1.0 wind strength
                float                   m_fAdjustedTime;                    // Internal concept of time
                wind_float_vector       m_vOutputs;                         // Output angles for this oscillator
        };

        class CSpeedWindController
        {
        public:
                                        CSpeedWindController( );

                void                    Advance(float fDeltaTime, float fP, float fMaxA);
                void                    Reset(float fValue);

        public:
                float                   m_fWantedValue;                     // The target output value for this controller
                float                   m_fCurrentValue;                    // The current output value of this controller
                float                   m_fLastDelta;                       // The change made on the last time step
                float                   m_fLastDeltaTime;                   // The duration of the last time step
        };

        // SpeedWind2 Parameters
        SOscillationParams              m_sBranchHorizontal;                // Horizontal (to the wind direction) branch movement oscillator
        SOscillationParams              m_sBranchVertical;                  // Vertical (to the wind direction) branch movement oscillator
        SOscillationParams              m_sLeafRocking;                     // Leaf rocking oscillator
        SOscillationParams              m_sLeafRustling;                    // Leaf rustling oscillator
        float                           m_fMaxBendAngle;                    // The maximum bend angle of the tree
        float                           m_fWindResponse;                    // The speed of the wind strength controller
        float                           m_fWindResponseLimit;               // The acceleration limiter on the wind strength controller
        float                           m_fGustingStrength;                 // The maximum strength of wind gusts
        float                           m_fGustingFrequency;                // The approximate frequency of wind gusts (in gusts per second)
        float                           m_fGustingDuration;                 // The maximum duration of a gust once it has started
        float                           m_fBranchExponent;                  // The exponent placed on the wind strength before branch oscillation
        float                           m_fLeafExponent;                    // The exponent placed on the wind strength before leaf rocking/rustling

        // SpeedWind2 Internals
        float                           m_fLastTime;                        // The last time Advance was called
        float                           m_fWindStrength;                    // The current wind strength (no gusting)
        float                           m_fCurrentGustingStrength;          // The current gusting strength
        float                           m_fGustStopTime;                    // The time when the current gust will stop

        // SpeedWind2 Outputs
        CSpeedWindController            m_cFinalWindStrength;               // The wind strength controller
        CSpeedWindController            m_cFinalWindDirectionX;             // The wind direction X controller
        CSpeedWindController            m_cFinalWindDirectionY;             // The wind direction Y controller
        CSpeedWindController            m_cFinalWindDirectionZ;             // The wind direction Z controller
        float                           m_fFinalBendAngle;                  // The current bend angle of the tree

#ifdef SPEEDTREE_FRAMEWORK
        st_vector_stmatrix2             m_vWindMatrices;                    // The wind matrices
        st_vector_stmatrix2             m_vLeafAngleMatrices;               // The leaf angle matrices
#else
        ntstd::Vector<CSpeedWindMatrix2>  m_vWindMatrices;                    // The wind matrices
        ntstd::Vector<CSpeedWindMatrix2>  m_vLeafAngleMatrices;               // The leaf angle matrices
#endif
};

typedef CSpeedWind2 CSpeedWind;

