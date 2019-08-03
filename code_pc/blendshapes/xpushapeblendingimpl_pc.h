//--------------------------------------------------
//!
//!	\file xpushapeblendingimpl_pc.h
//!	PC implementation of XPUShapeBlending interface
//!
//--------------------------------------------------



#ifndef SHAPE_BLENDING_IMPL_PC_H
#define SHAPE_BLENDING_IMPL_PC_H

class BlendedMeshInstance;

// Forward declarations
class CPUBlender;
class XPUBlendBatch;


class XPUShapeBlendingImpl : public Singleton< XPUShapeBlendingImpl >
{
public:
	//! constants for local buffer allocation
	static const size_t MAX_BLEND_TARGETS	= 64;
	static const size_t MAX_VERT_COMPONENTS = 32;
	static const size_t MAX_VERTS_PER_BATCH = 1 << 16;

	XPUShapeBlendingImpl( unsigned int ignored );
	~XPUShapeBlendingImpl();

	//! register a blended mesh
	void Register( BlendedMeshInstance* pMesh );
	void Unregister( BlendedMeshInstance* pMesh );
	bool IsRegistered( BlendedMeshInstance* pMesh ) const;

	//! begin update (since it's not immediate) of all registered meshes
	void BeginUpdate();

	//! process all queued batches
	void Flush();
	//! flush all batches that belong to a particular instance
	void Flush( BlendedMeshInstance* pMesh);				// not implented yet
	//! empty blend queue without processing
	void DiscardQueuedBatches();

	//! enabling/disabling blenders
	void Enable();
	void Disable( bool bDiscardCurrentBatches = false );

	bool IsEnabled() const { return m_bEnabled; }

	void Reset( bool bDeleteRegisteredInstances = false );
	
private:

	//! queue a (blended) mesh instance for blending 
	void QueueForShapeBlending( BlendedMeshInstance* pMesh );
	
	typedef ntstd::List< BlendedMeshInstance*, Mem::MC_PROCEDURAL > BlendedMeshQueue_t;	
	typedef ntstd::List< BlendedMeshInstance*, Mem::MC_PROCEDURAL > BlendedMeshRegistry_t;	

	//! cpu blend thread wrapper
	class CPUBlender 
	{
	public:

		//CPUBlender( BlendBatchQueue_t* hBlendQueue );
		CPUBlender( BlendedMeshQueue_t* pMeshQueue );
		~CPUBlender();

		void Enable();
		void Disable();

	private:

		// annoying thread stuff
		static uint32_t Go( CPUBlender* ptr );
		void CreateCPUThread();
		static unsigned int GetMinStackSize();

	private:
		// blend queue to work on
		BlendedMeshQueue_t* m_pMeshQueue;

		// Win32 thread stuff
		DWORD m_wThreadId;
		HANDLE m_hThreadHandle;

		bool m_bThreadRunning;
	};

	typedef ntstd::Vector< CPUBlender*, Mem::MC_PROCEDURAL > BlenderPool_t;

private:

	//! a pool of blenders to avoid creationg/deletion when workload changes
	BlenderPool_t				m_blenders;	 
	//! used for updating blended meshes during component update
	BlendedMeshRegistry_t		m_meshRegistry;
	//! queued meshes
	BlendedMeshQueue_t			m_meshQueue;

	HANDLE						m_hBlendEvent;
	HANDLE						m_hQueueEmptyEvent;

	bool						m_bEnabled;

	mutable CRITICAL_SECTION m_criticalSection;
};


#endif //SHAPE_BLENDING_IMPL_PC_H
