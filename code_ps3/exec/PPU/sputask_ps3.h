//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\sputask_ps3.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( EXEC_PPU_SPUTASK_PS3_H )
#define EXEC_PPU_SPUTASK_PS3_H

#include "jobapi/joblist.h"
#include "exec/ppu/spuargument_ps3.h"

// forward decl
class DMABuffer;
class SPUProgram;

class SPUTask
{
	public:
		//
		//	Ctor.
		//
		explicit SPUTask( const SPUProgram *pProgram );

	public:
		//
		//	Public interface.
		//

		//! Set an argument in the given slot.
		//! Slot can be from 0 to SPUArgumentList::MaxNumArguments-1.
		void	SetArgument( const SPUArgument &arg, int32_t slot ) { m_Arguments.Set( arg, slot ); }

		//! Get the argument in slot n, if there is no argument for slot n then NULL is returned.
		SPUArgument *GetArgument( int32_t slot ) { return m_Arguments.Get( slot ); }

		const SPUArgumentList *GetArgumentList() const { return &m_Arguments; }
	
		const SPUProgram* GetProgram() const 
		{
			return m_pProgram;
		}

		void SetMarker( JobListMarker marker ) { m_JobListMarker = marker; }

		void RequestDependencyDecrement( DependencyCounter* pCounter ) { m_pDependency = pCounter; }

		void StallForJobToFinish	()	const	{ m_JobListMarker.StallForJobMarker(); }
		bool HasJobBeenStarted		()	const	{ return m_JobListMarker.IsJobMarkerPassed(); }
		DependencyCounter* GetDependency() const { return m_pDependency; }
	private:
		//
		//	Dont' copy tasks around.
		//
		SPUTask( const SPUTask & ) NOT_IMPLEMENTED;
		SPUTask &operator = ( const SPUTask & ) NOT_IMPLEMENTED;

	private:
		//
		//	Aggregated members.
		//
		const SPUProgram *	m_pProgram;
		JobListMarker		m_JobListMarker;
		SPUArgumentList		m_Arguments __attribute__((aligned( 16 )));
		DependencyCounter*	m_pDependency;

} __attribute__((aligned( 16 )));

#endif // EXEC_PPU_SPUTASK_PS3_H
