#ifndef GFX_GPUREPORTS_PS3_H
#define GFX_GPUREPORTS_PS3_H


#include <Gc/Gc.h>
#include <Gc/GcKernel.h>

//--------------------------------------------------
//!
//!	GPUReport
//! 
//--------------------------------------------------
class GPUReport
{
public:

	enum GPUR_ID
	{
		GPRID_ShadowMapsPass = 0,
		GPRID_OcclusionGatheringPass,
		GPRID_IrradianceCachePass,
		GPRID_ZPrePass,
		GPRID_OpaquePass,
		GPRID_SkyPass,
		GPRID_NAO32ToHDR,
		GPRID_HDRAlphaPass,
		GPRID_LDRAlphaPass,
		GPRID_ExposurePass,
		GPRID_ToneMappingPass,
		GPRID_DOFPass,
		GPRID_LensEffectPass,
		GPRID_RadialBlurPass,

		GPRID_UnknownPass,
	};

public:
	GPUReport( uint32_t uiMaxPerViewReports, uint32_t uiMaxViews, uint32_t uiGameHz );
	~GPUReport();
	
	bool StartReports(void);
	bool CloseReports(void);
	bool CollectReports(void);

	bool StartReport( GPUR_ID eReportID, uint32_t uiViewID );
	bool CloseReport( GPUR_ID eReportID, uint32_t uiViewID );

	float GetReportFrameRelativeTime( GPUR_ID eReportID, uint32_t uiViewID = 0xffffffff );

	inline uint32_t GetAllocatedIOMem(void) { return m_uiRequiredMem; };

private:

	inline void AddReport( Ice::Render::ReportType eReportType, uint32_t uiReportIndex )
	{
		// get current ICE context
		Ice::Render::InlineUnsafe::CommandContext* pIceContext =  (Ice::Render::InlineUnsafe::CommandContext*)(&GcKernel::GetContext());
		//add GPU Report in our push buffer
		pIceContext->WriteReport( eReportType, Ice::Render::TranslateAddressToIoOffset((void*)&m_pIOStampsMemory[ uiReportIndex ]) );
	}

	// hardcoded to RSX final frequency (500 Mhz)
	inline float TicksToNanoSecs( U64 ui64Ticks ) { return ((((float)(ui64Ticks)) / 1000000000.0f) * 500000000.0f); }

static const uint32_t s_uiReservStamps = 8;
static const uint32_t s_uiEOFReportID = 0;

	Ice::Render::Report *m_pIOStampsMemory;
	Ice::Render::Report *m_pStampsMemory;
	
	uint32_t m_uiMaxPerViewReports;
	uint32_t m_uiMaxViews;
	uint32_t m_uiGameHz;
	uint32_t m_uiRequiredMem;

	bool m_bReportsEnabled;
};

#endif // GFX_GPUREPORTS_PS3_H
