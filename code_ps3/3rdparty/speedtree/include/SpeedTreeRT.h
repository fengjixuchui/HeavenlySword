///////////////////////////////////////////////////////////////////////  
//  SpeedTreeRT.h
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
//      http://www.idvinc.com
//
//  Release version 4.0 Beta 1


#pragma once
#ifdef PS3
#include <new>
#endif


// storage-class specification
#if (defined(WIN32) || defined(_XBOX)) && defined(SPEEDTREE_DLL_EXPORTS)
    #define ST_STORAGE_CLASS __declspec(dllexport)
#else
    #define ST_STORAGE_CLASS
#endif


// Macintosh export control
#ifdef __APPLE__
    #pragma export on
#endif


//  specify calling convention
#if defined(WIN32) || defined(_XBOX)
    #define CALL_CONV   __cdecl
#else
    #define CALL_CONV
#endif


//  forward references
class CIndexedGeometry;
class CTreeEngine;
class CLeafGeometry;
class CLightingEngine;
class CWindEngine;
class CTreeFileAccess;
class CSimpleBillboard;
class CFrondEngine;
struct STreeInstanceData;
struct SInstanceList;
struct SEmbeddedTexCoords;
struct SCollisionObjects;
class CProjectedShadow;
class CSpeedTreeAllocator;
class CMapBank;


///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeRT
//
//  In an effort to make the SpeedTreeRT.h header file dependency free
//  and easy to include into almost any project, a number of steps have
//  been taken:
//
//      1. No external header files need to be included by SpeedTreeRT.h
//         or by the application before including it.
//
//      2. Most of the implementation of the class is hidden by pointers
//         to the major sections of the library (the internal classes
//         can then just be forward-referenced)
//
//      3. Where possible, basic C++ datatypes are used to define the
//         member functions' parameters.
//
//  Because almost all of the implementation details are hidden, none of
//  the functions for CSpeedTreeRT are inlined.  However, inlined functions
//  were used copiously within the library.
    
class ST_STORAGE_CLASS CSpeedTreeRT
{
public:
        ///////////////////////////////////////////////////////////////////////  
        //  Enumerations

        enum EWindMethod
        {
            WIND_GPU, WIND_CPU, WIND_NONE
        };

        enum ELodMethod
        {
            LOD_POP, LOD_SMOOTH, LOD_NONE = 3
        };

        enum ELightingMethod
        {
            LIGHT_DYNAMIC, LIGHT_STATIC
        };

        enum EStaticLightingStyle
        {
            SLS_BASIC, SLS_USE_LIGHT_SOURCES, SLS_SIMULATE_SHADOWS
        };

        enum ECollisionObjectType
        {
            CO_SPHERE, CO_CAPSULE, CO_BOX
        };

        enum ETextureLayers
        {
            TL_DIFFUSE,        // diffuse layer
            TL_DETAIL,         // detail layer
            TL_NORMAL,         // normal-map layer
            TL_HEIGHT,         // height/displacement-map layer
            TL_SPECULAR,       // specular mask layer
            TL_USER1,          // user-defined #1
            TL_USER2,          // user-defined #2
            TL_SHADOW,         // shadow layer (used for texcoords only - the shadow map
                               // filename is stored separately in SMapBank in m_pSelfShadowMap)
            TL_NUM_TEX_LAYERS
        };

        enum EMemoryPreference
        {
            MP_FAVOR_SMALLER_FOOTPRINT,
            MP_FAVOR_LESS_FRAGMENTATION
        };


        ///////////////////////////////////////////////////////////////////////
        //  SGeometry bit vectors
        //
        //  Passed into GetGeometry() in order to mask out unneeded geometric elements

        #define                 SpeedTree_BranchGeometry            (1 << 0)
        #define                 SpeedTree_FrondGeometry             (1 << 1)
        #define                 SpeedTree_LeafGeometry              (1 << 2)
        #define                 SpeedTree_BillboardGeometry         (1 << 3)
        #define                 SpeedTree_AllGeometry               (SpeedTree_BranchGeometry + SpeedTree_FrondGeometry + SpeedTree_LeafGeometry + SpeedTree_BillboardGeometry)


        ///////////////////////////////////////////////////////////////////////
        //  struct SGeometry declaration
        
        struct ST_STORAGE_CLASS SGeometry
        {

            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SIndexed declaration
            //
            //  Used to store indexed geometry - branches and fronds in SpeedTree.

            struct ST_STORAGE_CLASS SIndexed
            {
                SIndexed( );
                ~SIndexed( );

                // indexed triangle strip data
                int                     m_nNumLods;             // total number of discrete LODs strip data represents
                const int*              m_pNumStrips;           // m_pNumStrips[L] == total number of strips in LOD L
                const int**             m_pStripLengths;        // m_pStripLengths[L][S] == lengths of strip S in LOD L
                const int***            m_pStrips;              // m_pStrips[L][S][I] == triangle strip index in LOD L, strip S, and index I

                // these vertex attributes are shared across all discrete LOD levels
                int                     m_nNumVertices;         // total vertex count in tables, referenced by all LOD levels
                const unsigned int*     m_pColors;              // RGBA values for each vertex - static lighting only (m_nNumVertices in length)
                const float*            m_pNormals;             // normals for each vertex (3 * m_nNumVertices in length)    
                const float*            m_pBinormals;           // binormals for each vertex (3 * m_nNumVertices in length)    
                const float*            m_pTangents;            // tangents for each vertex (3 * m_nNumVertices in length)        
                const float*            m_pCoords;              // coordinates for each vertex (3 * m_nNumVertices in length)
                const float*            m_pWindWeights[2];      // primary & secondary values from from 0.0 for rigid to 1.0 for flexible (m_nNumVertices in length)
                const unsigned char*    m_pWindMatrixIndices[2];// primary & secondary tables of wind matrix indices (m_nNumVertices in length)

                const float*            m_pTexCoords[TL_NUM_TEX_LAYERS];
            };


            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SLeaf declaration

            struct ST_STORAGE_CLASS SLeaf
            {
                SLeaf( );
                virtual ~SLeaf( );

                // active LOD level data
                int                     m_nDiscreteLodLevel;    // range: [0, GetNumLeafLodLevels( ) - 1]
                int                     m_nNumLeaves;           // number of leaves stored in this structure
                float                   m_fLeafRockScalar;      // passed to shader to control rocking motion
                float                   m_fLeafRustleScalar;    // passed to shader to control rustling motion

                // leaf attributes
                const unsigned char*    m_pLeafMapIndices;      // not for game/application engine use (SpeedTree toolset only)
                const unsigned char*    m_pLeafCardIndices;     // used to index m_pCards array in SLeaf (m_nNumLeaves in length)
                const float*            m_pCenterCoords;        // (x,y,z) values of the centers of leaf clusters (3 * m_nNumLeaves in length)
                const unsigned int*     m_pColors;              // RGBA values for each corner of each leaf (4 * m_nNumLeaves in length)
                const float*            m_pDimming;             // dimming factor for each leaf to use with dynamic lighting (m_nNumLeaves in length)
                const float*            m_pNormals;             // normals for each corner of each leaf (4 * 3 * m_nNumLeaves in length)
                const float*            m_pBinormals;           // binormals for each corner of each leaf (4 * 3 * m_nNumLeaves in length)
                const float*            m_pTangents;            // tangents for each corner of each leaf (4 * 3 * m_nNumLeaves in length)
                const float*            m_pWindWeights[2];      // primary & seconardy wind weights [0.0, 1.0] (m_nNumLeaves in length)
                const unsigned char*    m_pWindMatrixIndices[2];// primary & secondary wind matrix indices (m_nNumLeaves in length)

                // leaf meshes
                struct ST_STORAGE_CLASS SMesh
                {
                    SMesh( );
                    ~SMesh( );

                    // mesh vertex attributes
                    int                 m_nNumVertices;         // number of vertices in mesh
                    const float*        m_pCoords;              // (x,y,z) coords for each vertex (3 * m_nNumVertices in length)
                    const float*        m_pTexCoords;           // texcoords for each vertex (2 * m_nNumVertices in length)
                    const float*        m_pNormals;             // normals for each vertex (3 * m_nNumVertices in length)    
                    const float*        m_pBinormals;           // binormals (bump mapping) for each vertex (3 * m_nNumVertices in length)    
                    const float*        m_pTangents;            // tangents (bump mapping) for each vertex (3 * m_nNumVertices in length)        
                                    
                    // mesh indexes
                    int                 m_nNumIndices;          // length of index array
                    const int*          m_pIndices;             // triangle indices (m_nNumIndices in length)
                };

                // leaf cards
                struct ST_STORAGE_CLASS SCard
                {
                    SCard( );
                    ~SCard( );

                    // cluster attributes
                    float               m_fWidth;               // width of the leaf cluster
                    float               m_fHeight;              // height of the leaf cluster
                    float               m_afPivotPoint[2];      // center point about which rocking/rustling occurs
                    float               m_afAngleOffsets[2];    // angular offset to help combat z-fighting among the leaves
                    const float*        m_pTexCoords;           // 4 pairs of (s,t) texcoords (8 floats total)
                    const float*        m_pCoords;              // 4 sets of (x,y,z,0) coords (16 floats total)

                    // cluster mesh
                    const SMesh*        m_pMesh;                // mesh leaf data (no mesh if NULL, use leaf cards)
                };
                
                SCard*                  m_pCards;               // to be accessed as m_pCards[nLodLevel][m_pLeafCardIndices[nLeaf]] where nLeaf
                                                                // is the current leaf being drawn or accessed
            };


            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::S360Billboard declaration

            struct ST_STORAGE_CLASS S360Billboard
            {
                S360Billboard( );

                // GPU-only
                float                   m_fWidth;               // width of billboard (based on 3D model size)
                float                   m_fHeight;              // height of billboard (based on 3D model size)
                int                     m_nNumImages;           // # of 360-degree images stored in CAD export
                const float*            m_pTexCoordTable;       // all of the texture coordinates for all 360-degree images
                
                // both GPU and CPU
                const float*            m_pCoords;              // pointer to billboarded coords (4 * 3 floats in all)
                const float*            m_pTexCoords[2];        // two sets of texcoords (one for each overlapping bb),
                                                                // eight texcoords in m_pTexCoords[0] and m_pTexCoords[1]
                const float*            m_pNormals;             // normal for each vertex (4 * 3 floats in all)
                const float*            m_pBinormals;           // binormal for each vertex (4 * 3 floats in all)
                const float*            m_pTangents;            // tangent for each vertex (4 * 3 floats in all)
                float                   m_afAlphaTestValues[2]; // alpha test values for each overlapping bb
            };


            ///////////////////////////////////////////////////////////////////////
            //  struct SGeometry::SHorzBillboard declaration

            struct ST_STORAGE_CLASS SHorzBillboard
            {
                SHorzBillboard( );

                const float*            m_pCoords;              // pointer to billboarded coords (4 * 3 floats)
                const float*            m_pTexCoords;           // pointer to texcoords (4 * 2 floats)
                float                   m_afNormals[4][3];      // normals for all four corners
                float                   m_afBinormals[4][3];    // binormals for all four corners
                float                   m_afTangents[4][3];     // tangents for all for corners
                float                   m_fAlphaTestValue;      // alpha test value used for fade-in
            };


            SGeometry( );
            ~SGeometry( );


            ///////////////////////////////////////////////////////////////////////
            //  branch geometry

            SIndexed                m_sBranches;                // holds the branch vertices and index buffers for all
                                                                // of the discrete LOD levels

            ///////////////////////////////////////////////////////////////////////
            //  frond geometry

            SIndexed                m_sFronds;                  // holds the frond vertices and index buffers for all
                                                                // of the discrete LOD levels

            ///////////////////////////////////////////////////////////////////////
            //  leaf geometry

            int                     m_nNumLeafLods;             // m_pLeaves contains m_nNumLeafLods elements
            SLeaf*                  m_pLeaves;                  // contains all of the LOD information in one dynamic array


            ///////////////////////////////////////////////////////////////////////
            //  billboard geometry

            S360Billboard           m_s360Billboard;
            SHorzBillboard          m_sHorzBillboard;
        };


        ///////////////////////////////////////////////////////////////////////
        //  struct SLodValues declaration

        struct ST_STORAGE_CLASS SLodValues
        {
            SLodValues( );

            // branches
            int                     m_nBranchActiveLod;         // 0 is highest LOD, -1 is inactive
            float                   m_fBranchAlphaTestValue;    // 0.0 to 255.0 alpha testing value, used for fading

            // fronds
            int                     m_nFrondActiveLod;          // 0 is highest LOD, -1 is inactive
            float                   m_fFrondAlphaTestValue;     // 0.0 to 255.0 alpha testing value, used for fading

            // leaves
            int                     m_anLeafActiveLods[2];      // 0 is highest LOD, -1 is inactive
            float                   m_afLeafAlphaTestValues[2]; // 0.0 to 255.0 alpha testing value, used for fading

            // billboard
            float                   m_fBillboardFadeOut;        // 0.0 = faded out, 1.0 = opaque
        };


        ///////////////////////////////////////////////////////////////////////  
        //  struct SMapBank declaration

        struct ST_STORAGE_CLASS SMapBank
        {
            SMapBank( );
            ~SMapBank( );

            // branches
            const char**            m_pBranchMaps;              // filename = m_pBranchMaps[ETextureLayers]

            // not for game/application engine use (SpeedTree toolset only)
            unsigned int            m_uiNumLeafMaps;
            const char***           m_pLeafMaps;
            unsigned int            m_uiNumFrondMaps;
            const char***           m_pFrondMaps;

            // composite & billboards
            const char**            m_pCompositeMaps;           // filename = m_pCompositeMaps[ETextureLayers]
            const char**            m_pBillboardMaps;           // filename = m_pBillboardMaps[ETextureLayers]

            // self-shadow
            const char*             m_pSelfShadowMap;           // only one map per tree, or NULL if none
        };


        ///////////////////////////////////////////////////////////////////////  
        //  struct SLightShaderParams declaration

        struct ST_STORAGE_CLASS SLightShaderParams
        {
            SLightShaderParams( );

            float       m_fGlobalLightScalar;
            float       m_fBranchLightScalar;
            float       m_fFrondLightScalar;
            float       m_fLeafLightScalar;
            float       m_fAmbientScalar;
            float       m_fBillboardDarkSideLightScalar;
            float       m_fBillboardBrightSideLightScalar;
            float       m_fBillboardAmbientScalar;
        };

        ///////////////////////////////////////////////////////////////////////  
        //  Constructor/Destructor

        CSpeedTreeRT( );
        ~CSpeedTreeRT( );


        ///////////////////////////////////////////////////////////////////////  
        //  Memory management

static  void          CALL_CONV SetAllocator(CSpeedTreeAllocator* pAllocator);
        void*                   operator new(size_t sBlockSize);
        void*                   operator new[](size_t sBlockSize);
        void                    operator delete(void* pBlock);
        void                    operator delete[](void* pBlock);

static  void          CALL_CONV SetMemoryPreference(EMemoryPreference eMemPref);
static  EMemoryPreference CALL_CONV GetMemoryPreference(void);


        ///////////////////////////////////////////////////////////////////////  
        //  Specifying a tree model

        bool                    Compute(const float* pTransform = 0, unsigned int nSeed = 1, bool bCompositeStrips = true);
        CSpeedTreeRT*           Clone(void) const;
        const CSpeedTreeRT*     InstanceOf(void) const;
        CSpeedTreeRT*           MakeInstance(void);
        void                    DeleteTransientData(void);

        bool                    LoadTree(const char* pFilename);
        bool                    LoadTree(const unsigned char* pBlock, unsigned int nNumBytes);
        unsigned char*          SaveTree(unsigned int& nNumBytes, bool bSaveLeaves = false) const;

        void                    GetTreeSize(float& fSize, float& fVariance) const;
        void                    SetTreeSize(float fNewSize, float fNewVariance = 0.0f);
        unsigned int            GetSeed(void) const;

        const float*            GetTreePosition(void) const;
        void                    SetTreePosition(float x, float y, float z);

        void                    SetLeafTargetAlphaMask(unsigned char ucMask = 0x54);


        ///////////////////////////////////////////////////////////////////////  
        //  Lighting

        //  lighting style
        ELightingMethod         GetBranchLightingMethod(void) const;
        void                    SetBranchLightingMethod(ELightingMethod eMethod);
        ELightingMethod         GetLeafLightingMethod(void) const;
        void                    SetLeafLightingMethod(ELightingMethod eMethod);
        ELightingMethod         GetFrondLightingMethod(void) const;
        void                    SetFrondLightingMethod(ELightingMethod eMethod);

        EStaticLightingStyle    GetStaticLightingStyle(void) const;
        void                    SetStaticLightingStyle(EStaticLightingStyle eStyle);
        float                   GetLeafLightingAdjustment(void) const;
        void                    SetLeafLightingAdjustment(float fScalar);

        //  global lighting state
static  bool          CALL_CONV GetLightState(unsigned int nLightIndex);
static  void          CALL_CONV SetLightState(unsigned int nLightIndex, bool bLightOn);
static  const float*  CALL_CONV GetLightAttributes(unsigned int nLightIndex);
static  void          CALL_CONV SetLightAttributes(unsigned int nLightIndex, const float* pLightAttributes);

        //  materials
        const float*            GetBranchMaterial(void) const;
        void                    SetBranchMaterial(const float* pMaterial);
        const float*            GetFrondMaterial(void) const;
        void                    SetFrondMaterial(const float* pMaterial);
        const float*            GetLeafMaterial(void) const;
        void                    SetLeafMaterial(const float* pMaterial);

        //  shader lighting support
        void                    GetLightShaderParams(SLightShaderParams& sParams) const;


        ///////////////////////////////////////////////////////////////////////  
        //  Camera

static  void          CALL_CONV GetCamera(float* pPosition, float* pDirection);
static  void          CALL_CONV SetCamera(const float* pPosition, const float* pDirection);
static  void          CALL_CONV GetCameraAngles(float& fAzimuth, float& fPitch); // values are in degrees


        ///////////////////////////////////////////////////////////////////////  
        //  Wind 

static  void          CALL_CONV SetTime(float fTime);
        void                    ComputeWindEffects(bool bBranches, bool bLeaves, bool bFronds = true);
        void                    ResetLeafWindState(void);

        bool                    GetLeafRockingState(void) const;
        void                    SetLeafRockingState(bool bFlag);
        void                    SetNumLeafRockingGroups(unsigned int nRockingGroups);
        
        EWindMethod             GetLeafWindMethod(void) const;
        void                    SetLeafWindMethod(EWindMethod eMethod);
        EWindMethod             GetBranchWindMethod(void) const;
        void                    SetBranchWindMethod(EWindMethod eMethod);
        EWindMethod             GetFrondWindMethod(void) const;
        void                    SetFrondWindMethod(EWindMethod eMethod);

        float                   GetWindStrength(void) const;
        float                   SetWindStrength(float fNewStrength, float fOldStrength = -1.0f, float fFrequencyTimeOffset = -1.0f);
        void                    SetWindStrengthAndLeafAngles(float fNewStrength, const float* pRockAngles = 0, const float* pRustleAngles = 0, unsigned int uiNumRockAngles = 0);

static  void          CALL_CONV SetNumWindMatrices(int nNumMatrices);
static  void          CALL_CONV SetWindMatrix(unsigned int nMatrixIndex, const float* pMatrix);
        void                    GetLocalMatrices(unsigned int& nStartingIndex, unsigned int& nMatrixSpan);
        void                    SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan);

        
        ///////////////////////////////////////////////////////////////////////  
        //  LOD

        void                    ComputeLodLevel(void);
        float                   GetLodLevel(void) const;
        void                    SetLodLevel(float fLodLevel);
static  void          CALL_CONV SetDropToBillboard(bool bFlag);
        void                    GetLodValues(SLodValues& sLodValues, float fLodLevel = -1.0f);

        void                    GetLodLimits(float& fNear, float& fFar) const;
        void                    SetLodLimits(float fNear, float fFar);

        int                     GetDiscreteBranchLodLevel(float fLodLevel = -1.0f) const;
        int                     GetDiscreteLeafLodLevel(float fLodLevel = -1.0f) const;
        int                     GetDiscreteFrondLodLevel(float fLodLevel = -1.0f) const;

        int                     GetNumBranchLodLevels(void) const;
        int                     GetNumLeafLodLevels(void) const;
        int                     GetNumFrondLodLevels(void) const;

static  void          CALL_CONV SetHorzBillboardFadeAngles(float fStart, float fEnd); // in degrees
static  void          CALL_CONV GetHorzBillboardFadeAngles(float& fStart, float& fEnd); // in degrees


        ///////////////////////////////////////////////////////////////////////  
        //  Geometry

        void                    DeleteBranchGeometry(void);
        void                    DeleteFrondGeometry(void);
        void                    DeleteLeafGeometry(void);
        unsigned char*          GetFrondGeometryMapIndexes(int nLodLevel) const;
        const float*            GetLeafBillboardTable(unsigned int& nNumEntries) const;
        void                    GetGeometry(SGeometry& sGeometry, unsigned int uiBitVector = SpeedTree_AllGeometry);
        void                    UpdateLeafCards(SGeometry& sGeometry);
        void                    UpdateBillboardGeometry(SGeometry& sGeometry);
static  void                    UpdateBillboardLighting(SGeometry::S360Billboard& sBillboard);


        ///////////////////////////////////////////////////////////////////////  
        //  Textures

        void                    GetMapBank(SMapBank& sMapBank) const;
static  const char*             GetTextureLayerName(ETextureLayers eLayer);
        void                    SetLeafTextureCoords(unsigned int nLeafMapIndex, const float* pTexCoords);
        void                    SetFrondTextureCoords(unsigned int nFrondMapIndex, const float* pTexCoords);
static  bool          CALL_CONV GetTextureFlip(void);
static  void          CALL_CONV SetTextureFlip(bool bFlag);
        void                    SetBranchTextureFilename(const char* pFilename);
        void                    SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename);
        void                    SetFrondTextureFilename(unsigned int nFrondMapIndex, const char* pFilename);


        ///////////////////////////////////////////////////////////////////////  
        //  Statistics & information

static  bool          CALL_CONV Authorize(const char* pKey);
static  bool          CALL_CONV IsAuthorized(void);
static  const char*   CALL_CONV GetCurrentError(void);
static  void          CALL_CONV ResetError(void);
static  const char*   CALL_CONV Version(void);

        void                    GetBoundingBox(float* pBounds) const;
        int                     GetNumLeafTriangles(float fLodLevel = -1.0f);
        int                     GetNumBranchTriangles(float fLodLevel = -1.0f);
        int                     GetNumFrondTriangles(float fLodLevel = -1.0f);


        ///////////////////////////////////////////////////////////////////////  
        //  Collision objects

        int                     GetNumCollisionObjects(void);
        void                    GetCollisionObject(unsigned int nIndex, ECollisionObjectType& eType, float* pPosition, float* pDimensions, float* pEulerAngles);
 
        
        ///////////////////////////////////////////////////////////////////////  
        //  User Data

        const char*             GetUserData(void) const;

private:
        CSpeedTreeRT(const CSpeedTreeRT* pOrig);

        void                    ComputeLeafStaticLighting(void);
        void                    ComputeSelfShadowTexCoords(void);
static  void          CALL_CONV NotifyAllTreesOfEvent(int nMessage);
static  void          CALL_CONV SetError(const char* pError);


        ///////////////////////////////////////////////////////////////////////  
        //  File I/O

        void                    ParseLodInfo(CTreeFileAccess* pFile);
        void                    ParseWindInfo(CTreeFileAccess* pFile);
        void                    ParseTextureCoordInfo(CTreeFileAccess* pFile);
        void                    ParseCollisionObjects(CTreeFileAccess* pFile);
        void                    SaveTextureCoords(CTreeFileAccess* pFile) const;
        void                    SaveCollisionObjects(CTreeFileAccess* pFile) const;
        void                    ParseShadowProjectionInfo(CTreeFileAccess* pFile);
        void                    SaveUserData(CTreeFileAccess* pFile) const;
        void                    ParseUserData(CTreeFileAccess* pFile);
        void                    SaveSupplementalTexCoordInfo(CTreeFileAccess* pFile) const;
        void                    ParseSupplementalTexCoordInfo(CTreeFileAccess* pFile);
        void                    SaveStandardShaderInfo(CTreeFileAccess* pFile) const;
        void                    ParseStandardShaderInfo(CTreeFileAccess* pFile);
        void                    SaveStandardShaderInfo(CTreeFileAccess& cFile) const;
        void                    ParseStandardShaderInfo(CTreeFileAccess& cFile);
        void                    SaveSupplementalCollisionObjectsInfo(CTreeFileAccess& cFile) const;
        void                    ParseSupplementalCollisionObjectsInfo(CTreeFileAccess& cFile);

        void                    RecoverDeprecatedMaps(void);
static  char*         CALL_CONV CopyUserData(const char* pData);


        ///////////////////////////////////////////////////////////////////////  
        //  Geometry
        
        void                    GetBranchGeometry(SGeometry& sGeometry);
        void                    GetFrondGeometry(SGeometry& sGeometry);
        void                    GetLeafGeometry(SGeometry& sGeometry);
        void                    GetBillboardGeometry(SGeometry& sGeometry);
        void                    SetupHorizontalBillboard(void);
        float                   ComputeLodCurve(float fStart, float fEnd, float fPercent, bool bConcaveUp);


        ///////////////////////////////////////////////////////////////////////  
        //  Member variables

        // general
        CTreeEngine*            m_pEngine;                  // core tree-generating engine
        CIndexedGeometry*       m_pBranchGeometry;          // abstraction mechanism for branch geometry
        CLeafGeometry*          m_pLeafGeometry;            // abstraction mechanism for leaf geometry
        SGeometry::SLeaf*       m_pLeafLods;
        CLightingEngine*        m_pLightingEngine;          // engine for computing static/dynamic lighting data
        CWindEngine*            m_pWindEngine;              // engine for computing CPU/GPU wind effects
        CSimpleBillboard*       m_pSimpleBillboard;
static  EMemoryPreference       m_eMemoryPreference;

        // leaf lod
        ELodMethod              m_eLeafLodMethod;           // which leaf wind method is currently being used
        float                   m_fLeafLodTransitionRadius; // determines how much blending occurs between two separate leaf LOD levels
        float                   m_fLeafLodCurveExponent;    // exponent value used in the leaf LOD blending equation
        float                   m_fLeafSizeIncreaseFactor;  // value that controls how much larger leaf clusters get as LOD decreases
        float                   m_fLeafTransitionFactor;    // value that controls the intersection point of SMOOTH_1 transitions
        float*                  m_pLeafLodSizeFactors;      // array, GetNumLeafLodLevels()'s in size, containing leaf LOD scale factors

        // instancing & ref counting
        unsigned int*           m_pInstanceRefCount;        // single value shared among instances - number of active instances
        STreeInstanceData*      m_pInstanceData;            // if instance, differentiating data is stored here
        SInstanceList*          m_pInstanceList;            // each tree contains a list of its instances
static  unsigned int            m_uiAllRefCount;            // single value shared by all CSpeedTreeRT instances

        // other
        int                     m_nFrondLevel;              // from SpeedTreeCAD - branch level where fronds begin
        float*                  m_pTreeSizes;               // contains all tree extents, including billboard sizes
        float                   m_fTargetAlphaValue;        // value used for leaf alpha mask function
        bool                    m_bTreeComputed;            // some operations are not valid once the geometry has been computed
        int                     m_nBranchWindLevel;         // from SpeedTreeCAD - branch level where wind effects are active

        // texture coords
        SEmbeddedTexCoords*     m_pEmbeddedTexCoords;       // embedded leaf and billboard texture coords
static  bool                    m_bTextureFlip;             // used to flip coordinates for DirectX, Gamebryo, etc.

        // shadow projection
        CProjectedShadow*       m_pProjectedShadow;         // self-shadow projection

        // billboard
static  bool                    m_bDropToBillboard;         // flag specifying if last LOD will be simple single billboard
static  float                   m_fCameraAzimuth;
static  float                   m_fCameraPitch;

        // collision objects
        SCollisionObjects*      m_pCollisionObjects;        // collision objects

        // fronds
        CFrondEngine*           m_pFrondEngine;             // engine for computing fronds based on branch geometry
        CIndexedGeometry*       m_pFrondGeometry;           // abstraction mechanism for frond geometry

        // user data
        char*                   m_pUserData;                // user specified data

        // horizontal billboard
        bool                    m_b360Billboard;            // indicates that a 360 degree billboard sequence is present
        bool                    m_bHorizontalBillboard;     // indicates that a horizontal billboard is present in the embedded texcoordss
        float                   m_afHorizontalCoords[12];   // vertices of the horizontal billboard
static  float                   m_fHorizontalFadeStartAngle;// in degrees
static  float                   m_fHorizontalFadeEndAngle;  // in degrees
static  float                   m_fHorizontalFadeValue;

        // standard shader support
        float                   m_fBranchLightScalar;       // branch light scalar used by standard SpeedTree pixel shaders
        float                   m_fFrondLightScalar;        // frond light scalar used by standard SpeedTree pixel shaders
        float                   m_fLeafLightScalar;         // leaf light scalar used by standard SpeedTree pixel shaders
        float                   m_fGlobalLightScalar;       // global value used to multiply branch, frond, and leaf light scalars
        float                   m_fAmbientScalar;           // value used to scale the ambient material of branches, fronds, and leaves
        float                   m_fBillboardDarkSideLightScalar;    // billboard light scalar used on the dark side of the billboards
        float                   m_fBillboardBrightSideLightScalar;  // billboard light scalar used on the bright side of the billboards
        float                   m_fBillboardAmbientScalar;          // scales the ambient value of the billboard material

        // lod parameters
        int                     m_nNumBranchLodLevels;
        int                     m_nNumFrondLodLevels;
        int                     m_nNumLeafLodLevels;
        float                   m_fTransitionWidth;
        float                   m_fCrestWidth;
        float                   m_fCrestWidthBB;
        float                   m_fCycleLength;
        float                   m_fCycleLengthBB;

        // maps
        CMapBank*               m_pMapBank;
};


// Macintosh export control
#ifdef __APPLE__
#pragma export off
#endif
