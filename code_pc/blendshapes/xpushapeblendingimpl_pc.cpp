//--------------------------------------------------
//!
//!	\file xpushapeblendingimpl_pc.cpp
//!	PC implementation of the blendshape manager
//!
//--------------------------------------------------




#include "blendshapes/xpushapeblendingimpl_pc.h"
#include "blendshapes/blendshapes.h"
#include "blendshapes/blendedmeshinstance_pc.h"


//! threshold for performing blending over a target or not
//! TODO: should be tweaked to a bigger number
#include <float.h>
#define W_EPSILON FLT_MIN

//! maximun time  we are willing to wait for the blenders to finish
//! their job during a flush
static const int MAX_WAIT = 20;	

//! used for asserting that the blenders are keeping up with the rest of the game
static const int SAFE_BLEND_QUEUE_SIZE = 10;


static const int NUM_OF_BLEND_THREADS = 1;

//-------------------------------------------------------------------------------------
//
//									XPUShapeBlendingImpl
//
//-------------------------------------------------------------------------------------
bool XPUShapeBlendingImpl::IsRegistered( BlendedMeshInstance* pMesh ) const
{
	return ( ntstd::find( m_meshRegistry.begin(), m_meshRegistry.end(), pMesh ) != m_meshRegistry.end() );
}

void XPUShapeBlendingImpl::Register( BlendedMeshInstance *pMesh )
{
	ntAssert( pMesh->IsShapeBlended() );
	
	bool bAlreadyRegistered = IsRegistered( pMesh );
	ntAssert_p( !bAlreadyRegistered, ("BlendedMeshInstance is already registered with XPUShapeBlender") );
	
	if ( !bAlreadyRegistered )
	{
		m_meshRegistry.push_back( pMesh );
	}
}

void XPUShapeBlendingImpl::Unregister( BlendedMeshInstance *pMesh )
{
	if ( IsRegistered( pMesh ) )
	{
		m_meshRegistry.remove( pMesh );
	}
	else
		user_warn_msg(("Unregistered BlendedMeshInstance detected\n"));
}

bool NeedsToBeBlended( BlendedMeshInstance* pMesh )
{
	return ( pMesh->IsRendering() && pMesh->HasBlendShapes() );
}

void XPUShapeBlendingImpl::BeginUpdate()
{
	if ( !IsEnabled() )
		return;

	for ( BlendedMeshRegistry_t::iterator it = m_meshRegistry.begin(); it != m_meshRegistry.end(); it++)
	{
		if ( NeedsToBeBlended( *it ) )
		{
            QueueForShapeBlending( *it );
		}
	}
}

void XPUShapeBlendingImpl::QueueForShapeBlending( BlendedMeshInstance* pMesh )
{
	//! make sure the blenders are keeping up with the game
	ntAssert_p( m_meshQueue.size() < SAFE_BLEND_QUEUE_SIZE, ("Shapeblending queue is too long") );

	EnterCriticalSection( &m_criticalSection );

	m_meshQueue.push_back( pMesh );
	//! tell blenders that there's work to do
	SetEvent( m_hBlendEvent );

	LeaveCriticalSection( &m_criticalSection );
}


void XPUShapeBlendingImpl::Reset( bool bDeleteRegisteredInstances )
{
	user_warn_p( m_meshRegistry.empty(), ("BSMeshInstance registry is not empty on singleton reset. There are dangling instances somewhere... \n") );

	DiscardQueuedBatches();

	if ( bDeleteRegisteredInstances )
	{
		while ( !m_meshRegistry.empty() )
		{
			// note that blendedmeshintance unregisters itself from this manager upon destruction
			NT_DELETE_CHUNK( Mem::MC_GFX, m_meshRegistry.back() );
		}
	}
	else
	{
		m_meshRegistry.clear();
	}	
}

//! needs revising
void XPUShapeBlendingImpl::Flush()
{
	if ( !IsEnabled() )
		return;

	//! send a wake-up call to all blenders in case they're slacking
	PulseEvent( m_hBlendEvent );
	//! wait a reasonable time for the queue to empy
	WaitForSingleObject( m_hQueueEmptyEvent, MAX_WAIT );
	//! give up after the wait
	DiscardQueuedBatches();
}

void XPUShapeBlendingImpl::DiscardQueuedBatches()
{
	//! cancel blending
	ResetEvent( m_hBlendEvent );

	EnterCriticalSection( &m_criticalSection );
	m_meshQueue.clear();
	LeaveCriticalSection( &m_criticalSection );
}

void XPUShapeBlendingImpl::Enable()
{
	ntstd::for_each( m_blenders.begin(), m_blenders.end(), ntstd::mem_fun( &XPUShapeBlendingImpl::CPUBlender::Enable ) );

	m_bEnabled = true;
}

//! needs revising. TODO: suspend them
void XPUShapeBlendingImpl::Disable( bool bDiscardCurrentBatches )
{
	if ( bDiscardCurrentBatches )
	{
		DiscardQueuedBatches();
	}
	else 
	{
		Flush();
	}

	ntstd::for_each( m_blenders.begin(), m_blenders.end(), ntstd::mem_fun( &XPUShapeBlendingImpl::CPUBlender::Disable ) );

	m_bEnabled = false;
}


XPUShapeBlendingImpl::XPUShapeBlendingImpl( uint32_t ignored )
	:
	//m_blendQueue(),
	m_meshQueue(),
	m_blenders(),
	m_bEnabled( false )
{
	UNUSED( ignored );

	m_hBlendEvent = CreateEvent( 0, TRUE,				// Manual reset
									FALSE,				// initialise with nothing to do
									0 );

	m_hQueueEmptyEvent = CreateEvent( 0, TRUE,			// Auto reset  TODO: set to auto?
									FALSE,				// no need to initialise this to true
									0 );

	InitializeCriticalSection( &m_criticalSection );

	// create blenders (cpu threads for now)
	for ( uint32_t i=0; i < NUM_OF_BLEND_THREADS; i++ )
	{
		m_blenders.push_back( NT_NEW_CHUNK (Mem::MC_PROCEDURAL)  CPUBlender( &m_meshQueue ) );
	}
	
	// let the things loose
	ntstd::for_each( m_blenders.begin(), m_blenders.end(), ntstd::mem_fun( &XPUShapeBlendingImpl::CPUBlender::Enable ) );
}

XPUShapeBlendingImpl::~XPUShapeBlendingImpl()
{
	//! disable all threads, discarding pending jobs
	Disable( true );

	//! release blender resources
	for ( BlenderPool_t::iterator it = m_blenders.begin(); it != m_blenders.end(); it++ )
	{
		NT_DELETE( *it );
	}

	//! by now these shouldn't be necessary
	CloseHandle( m_hBlendEvent );
	CloseHandle( m_hQueueEmptyEvent );

	//! finally, eliminate the critical section
	DeleteCriticalSection( &m_criticalSection );
}



//-------------------------------------------------------------------------------------
//
//									CPUBlender
//
//-------------------------------------------------------------------------------------



// for indexed deltas list
inline void BlendPos( u_char* pDst, 
			  const uint16_t* pIndices,
			  const blendshapes::delta_t* pDeltas,
			  float deltaScale,
			  float weight, 
			  u_int nVerts,
			  u_int nDeltas,
			  u_int stride )
{

	float scaleAndWeight = deltaScale * weight;

	//user_warn_p( nDeltas > 4, ("nDeltas is very small") );

	for( u_int iDelta = 0 ; iDelta < nDeltas; ++iDelta )
	{
		// real vertex displacement index
		uint16_t index = pIndices[ iDelta ];

		ntAssert_p( index < nVerts, ("delta index is bigger than number of vertices") );
		UNUSED( nVerts );

		float* pPos = (float*)( pDst + index*stride );
		const blendshapes::delta_t* pDelta = pDeltas + iDelta * 3;

        pPos[0] += static_cast<float>(pDelta[0]) * scaleAndWeight;
		pPos[1] += static_cast<float>(pDelta[1]) * scaleAndWeight;
		pPos[2] += static_cast<float>(pDelta[2]) * scaleAndWeight;
	}
}



void ProcessBlendedMesh( BlendedMeshInstance& mesh )
{
	//using namespace blendshapes;

	static const int maxVertices = 1 << 16;
	static float vertexBuffer[ maxVertices * 3 * 5 ];
	
	MeshBSSetPtr_t pBlendShapes = mesh.GetBlendShapes();
	const CMeshHeader* pMeshHeader = mesh.GetMeshHeader(); 

	ntAssert( pMeshHeader );
	ntAssert( pBlendShapes );

	u_int numVerts = pMeshHeader->m_iNumberOfVertices;
	u_int vertexStride = pMeshHeader->m_iVertexStride;
	u_int vertexBufferSize = numVerts * vertexStride;


	// reset to original vertex buffer ( static in clump for now )
	memcpy( vertexBuffer, pMeshHeader->m_pvVertexBufferData, vertexBufferSize );

	for ( u_int iTarget = 0; iTarget < pBlendShapes->GetNumOfBSTargets(); iTarget++ )
	{
		float weight = pBlendShapes->GetTargetWeightByIndex( iTarget );
		if ( weight > EPSILON )
		{
			BlendPos( (u_char*)vertexBuffer, 
				pBlendShapes->GetTargetIndices( iTarget ), 
				pBlendShapes->GetTargetDeltas( iTarget ), 
				pBlendShapes->GetTargetDeltaScale( iTarget ),
				weight, 
				numVerts , 
				pBlendShapes->GetTargetNumOfDeltas( iTarget ) , 
				vertexStride );
		}
	}

	//copy into mesh's vbuffer
	VBHandle hVB = mesh.GetVBHandle();
	
	void* pDst;
	HRESULT hr = hVB->Lock( 0, vertexBufferSize, &pDst, D3DLOCK_DISCARD );
	UNUSED(hr);
	ntError_p( SUCCEEDED(hr), ( __FUNCTION__ ": Lock failed") );

	memcpy( pDst, vertexBuffer, vertexBufferSize );

	hVB->Unlock();

}




uint32_t XPUShapeBlendingImpl::CPUBlender::Go( XPUShapeBlendingImpl::CPUBlender* ptr )
{
	while ( ptr->m_bThreadRunning )
	{
		//! Wait for some work to do
 		WaitForSingleObject( XPUShapeBlendingImpl::Get().m_hBlendEvent, INFINITE );

		//! make sure no other thread is updating the queue
		EnterCriticalSection( &(XPUShapeBlendingImpl::Get().m_criticalSection) );
		if ( ptr->m_pMeshQueue->empty() )
		{
			//! notify that the queue is empty
			PulseEvent( XPUShapeBlendingImpl::Get().m_hQueueEmptyEvent );

			//! free for others to check/update
			LeaveCriticalSection( &(XPUShapeBlendingImpl::Get().m_criticalSection) );
		} 
		else
		{
			BlendedMeshInstance* pMesh = ptr->m_pMeshQueue->front();
			ptr->m_pMeshQueue->pop_front();

			//! first we release the section and let others do their job
			LeaveCriticalSection( &(XPUShapeBlendingImpl::Get().m_criticalSection) );
			
			//! the batch is ours. Time to blend!
			ProcessBlendedMesh( *pMesh );
		}
	}

	return 0;
}



void XPUShapeBlendingImpl::CPUBlender::CreateCPUThread()
{
	m_hThreadHandle = CreateThread( 0,										/* Security */
									GetMinStackSize(),						/* Stack for this thread */
									(LPTHREAD_START_ROUTINE)&(this->Go),	/* Work horse function */
									this,									/* Parameter to pass to the work horse */
									CREATE_SUSPENDED,						/* Flags : Start suspended */
									&m_wThreadId );							/* Thread ID */

	ntError_p( m_hThreadHandle != 0, ("Eeek! Win32 failed shape-blending thread creation") );
}


XPUShapeBlendingImpl::CPUBlender::CPUBlender( XPUShapeBlendingImpl::BlendedMeshQueue_t* pMeshQueue ) :
	m_pMeshQueue( pMeshQueue ),
	m_wThreadId( 0xFFFFFFFF ),
	m_hThreadHandle ( 0 ),
	m_bThreadRunning( false )
{
	CreateCPUThread();
	//ResumeThread( m_hThreadHandle ); moved to enabled. Plus, blenders need to be enabled by XPUShapeBlender
}


XPUShapeBlendingImpl::CPUBlender::~CPUBlender()
{
	//Enable();

	while ( true )
	{
		DWORD dwExitCode = 0;
		bool res = GetExitCodeThread( m_hThreadHandle, &dwExitCode );
		UNUSED(res);
		ntError_p( res, ("Could not get exit code for thread %d", m_hThreadHandle) );
		
		if ( dwExitCode != STILL_ACTIVE )
		{
			break;
		}

		PulseEvent( XPUShapeBlendingImpl::Get().m_hBlendEvent );
		Sleep( MAX_WAIT );
	}

	Disable();

	CloseHandle( m_hThreadHandle );
}


void XPUShapeBlendingImpl::CPUBlender::Enable()
{
	m_bThreadRunning = true;
	ResumeThread( m_hThreadHandle );
}

void XPUShapeBlendingImpl::CPUBlender::Disable()
{
	m_bThreadRunning = false;
	//SuspendThread( m_hThreadHandle );
}


uint32_t XPUShapeBlendingImpl::CPUBlender::GetMinStackSize()
{
	const uint32_t deltaSize = sizeof( blendshapes::delta_t );
	const uint32_t floatSize = sizeof( float );
	
	return MAX_BLEND_TARGETS * floatSize +							// weight buffer
		MAX_VERTS_PER_BATCH * MAX_VERT_COMPONENTS * floatSize +		// vertex buffer
		MAX_VERTS_PER_BATCH * 3 * deltaSize +						// 1-target-deltas buffer
		1024 * 4;													// additional variables... TODO: tweak
}


//-------------------------------------------------------------------------------------
//
//									XPUBlendBatch
//
//-------------------------------------------------------------------------------------

//XPUShapeBlendingImpl::XPUBlendBatch::XPUBlendBatch( BlendedMeshInstance* pMesh ) :
//	m_pMesh( pMesh ), 
//	m_weightThreshold( W_EPSILON ),
//	m_pVerts( pMesh->GetMeshHeader()->m_pvVertexBufferData ),
//	m_pWeights( pMesh->GetBlendShapes()->GetTargetWeights() ),
//	m_pDeltas( pMesh->GetBlendShapes()->GetTargetDeltas(0) ),
//	m_vertexCount( pMesh->GetMeshHeader()->m_iNumberOfVertices ),
//	m_targetCount( pMesh->GetBlendShapes()->GetNumOfBSTargets() ),
//	m_vertexStride( pMesh->GetMeshHeader()->m_iVertexStride )//,
////	m_targetStride( pMesh->GetBlendShapes()->GetDeltaStride() * m_vertexCount )
//{
//	// nothing
//}





//-------------------------------------------------------------------------------------
//
//									OLD STUFF (pre-bsclump header)
//
//-------------------------------------------------------------------------------------

// TODO_OZZ: needs rewriting. Unnecessary copies everywhere!
//void XPUShapeBlendingImpl::CPUBlender::ProcessBatch( XPUShapeBlendingImpl::XPUBlendBatch& hBlendBatch )
//{
//	// local buffers
//	static float weightBuffer[ MAX_BLEND_TARGETS ];
//	static float vertexBuffer[ MAX_VERTS_PER_BATCH * MAX_VERT_COMPONENTS ];
//	static float deltaBuffer[ MAX_VERTS_PER_BATCH * 3 ];
//
//	// tmp refs
//	const uint32_t numTargets = min( MAX_BLEND_TARGETS, hBlendBatch.m_targetCount );
//	const uint32_t numVerts = min( MAX_VERTS_PER_BATCH, hBlendBatch.m_vertexCount );
//	const uint32_t targetStride = hBlendBatch.m_targetStride;
//	const uint32_t vertexStride = hBlendBatch.m_vertexStride;
//	BlendedMeshInstance* pMesh = hBlendBatch.m_pMesh;
//
//	UNUSED( pMesh );
//
//	//! fill buffers
//	memcpy( weightBuffer, hBlendBatch.m_pWeights, numTargets * sizeof(float) );
//	memcpy( vertexBuffer, hBlendBatch.m_pVerts, numVerts * vertexStride );
//	
//	//! process each target by the target deltas only if necessary
//	for ( uint32_t targetIndex = 0; targetIndex < numTargets; targetIndex++ )
//	{
//		// TODO: tweak W_EPSILON
//		float weight = weightBuffer[ targetIndex ] ;
//		if ( weight > hBlendBatch.m_weightThreshold )
//		{
//			const uint32_t offset = targetIndex * targetStride;
//			const byte* pDeltas = static_cast<const byte*>( hBlendBatch.m_pDeltas );
//			memcpy( deltaBuffer, pDeltas + offset , numVerts * 3 * sizeof(float) );
//
//			BlendTarget( vertexBuffer, deltaBuffer, weight, numVerts, vertexStride );
//		}
//	}
//
//	//CopyToVertexBuffer( pMesh, vertexBuffer, numVerts, vertexStride );
////	pMesh->GetBSVertexBuffer()->Write( vertexBuffer, numVerts * vertexStride );
//}
//
//uint32_t XPUShapeBlendingImpl::CPUBlender::Go( XPUShapeBlendingImpl::CPUBlender* ptr )
//{
//
//	XPUShapeBlendingImpl::XPUBlendBatch blendBatch;
//
//	while ( ptr->m_bThreadRunning )
//	{
//		//! Wait for some work to do
// 		WaitForSingleObject( XPUShapeBlendingImpl::Get().m_hBlendEvent, INFINITE );
//
//		//! make sure no other thread is updating the queue
//		EnterCriticalSection( &(XPUShapeBlendingImpl::Get().m_criticalSection) );
//		if ( ptr->m_hBlendQueue->empty() )
//		{
//			//! notify that the queue is empty
//			PulseEvent( XPUShapeBlendingImpl::Get().m_hQueueEmptyEvent );
//
//			//! free for others to check/update
//			LeaveCriticalSection( &(XPUShapeBlendingImpl::Get().m_criticalSection) );
//		} 
//		else
//		{
//			//! get the first batch and unqueue it
//			blendBatch = ptr->m_hBlendQueue->front();		
//			ptr->m_hBlendQueue->pop_front();
//
//			//! first we release the section and let others do their job
//			LeaveCriticalSection( &(XPUShapeBlendingImpl::Get().m_criticalSection) );
//			
//			//! the batch is ours. Time to blend!
//			ptr->ProcessBatch( blendBatch );
//		}
//	}
//
//	return 0;
//}

//inline 
//void BlendTarget( float* pVBuffer, 
//						const float* pDBuffer, 
//						float weight, 
//						uint32_t nVerts, 
//						uint32_t vertexStride )
//{
//	for ( uint32_t i = 0; i < nVerts; i++ )
//	{
//		pVBuffer[0] += weight * pDBuffer[3*i + 0]; 
//		pVBuffer[1] += weight * pDBuffer[3*i + 1];
//		pVBuffer[2] += weight * pDBuffer[3*i + 2];
//
//		pVBuffer += vertexStride/sizeof(float);
//	}
//}



//eof


