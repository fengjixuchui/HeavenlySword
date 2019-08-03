/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

/*
	This is the startup code for a job
	It calls the global constructors before calling JobMain
	On return to this function it calls the global destructors before returning to the job manager
*/
	.section .uninitialized,"aw",@nobits
	.align 4
	.global g_jobContext
	.global g_nextJobHasStartedLoading

g_jobContext:
	.zero 16

g_nextJobHasStartedLoading:
	.zero 16


	.text
	.align	3
	.global	JobEntryPoint
	.global JobMain
	.global __CTOR_LIST__
	.global __CTOR_END__
	.global __DTOR_LIST__
	.global __DTOR_END__
	.global __BSS_START__
	.global __BSS_END__

JobEntryPoint:

	#Respect the ABI
	stqd			$80,		-16($1)						#Used for iterating over ctors/dtors.
	stqd			$81,		-32($1)						#Used for storing the breakpointEnabled flag during init.  Used for tracking timing start across calls to dtors.
	stqd			$82,		-48($1)						#Used for tracking timing start across calls to ctors.  Preserved across job and accumulated with time for dtors.
	stqd			$126,		-64($1)						#Used for position independence offset.
	stqd			$0,			16($1)
	stqd			$1,			-96($1)
	ai				$1,			$1,				-96

	#Move the breakpoint parameter into $81 for temporary storage
	ai				$81,		$4,				0

	#Get time at start of this init
	rdch			$82,		SPU_RdDec


	#Store the jobCtxt param ready for the job to use
	stqr			$3,			g_jobContext


	#Store 0 (ie. false) to the g_nextJobHasStartedLoading boolean
	il				$5,			0
	stqr			$5,			g_nextJobHasStartedLoading


	#Calculate position independence offset
	ila				$2,			JobCrt0PositionIndependenceLabel
	brsl			$126,		JobCrt0PositionIndependenceLabel
JobCrt0PositionIndependenceLabel:
	sf				$126,		$2,				$126


	#Zero out the bss
	ila				$3,			__BSS_START__
	ila				$4,			__BSS_END__
	clgt			$2,			$4,				$3			#Compare labels to see if there's anything to zero
	brnz			$2,			ZeroBss
FinishedZeroingBss:

	#Call global constructors
	ila				$80,		(__CTOR_END__ - 4)			#Pointer to the last ctor
	a				$80,		$80,			$126		#Offset last ctor pointer appropriately
	lqd				$7,			0($80)						#Load up ctor qword
	rotqby			$3,			$7,				$80			#Get ctor in preferred slot
	ceqi			$2,			$3,				-1			#Ctor lists is terminated by 0xFFFFFFFF
	brz				$2,			CallConstructor				#Branch to constructors if not terminator
FinishedCallingGlobalConstructors:


	hbrr			BranchToJobMain,	JobMain				#Hint the branch to main

	#Pass in the 8 U32 params to the job
	lqr				$2,			g_jobContext				#Load pDataForJob
	lqd				$2,			32($2)						#Load pDataForJob->m_pParameters
	lqd				$3,			0($2)						#Load the first two qwords of the params
	lqd				$7,			16($2)
	rotqbyi			$4,			$3,				4			#Rotate the params into their correct positions for calling JobMain
	rotqbyi			$5,			$3,				8
	rotqbyi			$6,			$3,				12
	rotqbyi			$8,			$7,				4
	rotqbyi			$9,			$7,				8
	rotqbyi			$10,		$7,				12


	#Track time spent in init
	rdch			$79,		SPU_RdDec					#Get time at end of init
	sf				$82,		$79,			$82			#Time spent in init


	#######################################
	#
	# Now call the job
	#
	# Effectively we're doing "brsl $0, JobMain" here,
	# but we do it in this style so that we can optionally
	# have a breakpoint just before the call and without any
	# additional branching overhead
	#
	#######################################

	#Setup the link register $0 with the correct return point
	ila				$0,				ReturnFromJob
	a				$0,				$0,			$126
	#And branch to the job
BranchToJobMain:
	brz				$81,			JobMain
	stopd			$0,				$0,			$0
	br				JobMain
ReturnFromJob:

	#######################################
	#Shutdown
	#######################################
	
	#Get time at start of shutdown
	rdch			$81,		SPU_RdDec


	ila				$80,		(__DTOR_LIST__ + 4)			#Pointer to first dtor
	a				$80,		$80,			$126		#Offset first dtor pointer appropriately
	lqd				$3,			0($80)						#Load up dtor qword
	rotqby			$3,			$3,				$80			#Get dtor in preferred slot
	brnz			$3,			CallDestructor				#Branch to destructors if not terminated by 0x00000000
FinishedCallingGlobalDestructors:

	#Track time spent in shutdown
	rdch			$79,		SPU_RdDec					#Get time at end of shutdown
	sf				$78,		$79,			$81			#Time spent in shutdown


	#Return combined time spent in init and shutdown
	a				$3,			$82,			$78


	#Respect the ABI
	ai				$1,			$1,				96
	lqd				$126,		-60($1)
	lqd				$82,		-48($1)
	lqd				$81,		-32($1)
	lqd				$80,		-16($1)
	lqd				$0,			16($1)

	bid				$0										#Return to the Job Manager



############################################################
ZeroBss:
	il				$5,			0
zeroNextQword:
	stqx			$5,			$3,				$126		#Store relative to $126 to shift to the correct PIC run-time location
	ai				$3,			$3,				16			#Move to next qword
	clgt			$6,			$4,				$3			#Keep going until all zeroed
	brnz			$6,			zeroNextQword
	br			FinishedZeroingBss


############################################################
CallConstructor:
	ai				$80,		$80,			-4			#Iterate backwards over ctors
	a				$3,			$3,				$126		#Offset ctor pointer appropriately
	bisl			$0,			$3							#Call ctor
TestConstructorTerminator:
	lqd				$7,			0($80)						#Load up ctor qword
	rotqby			$3,			$7,				$80			#Get ctor in preferred slot
	ceqi			$2,			$3,				-1			#Ctor lists is terminated by 0xFFFFFFFF
	brz				$2,			CallConstructor				#Branch back if not terminator
	br			FinishedCallingGlobalConstructors

############################################################
CallDestructor:
	a				$3,			$3,				$126		#Offset dtor pointer appropriately
	bisl			$0,			$3							#Call dtor
	ai				$80,		$80,			4			#Iterate forwards over dtors
TestDestructorTerminator:
	lqd				$3,			0($80)						#Load up dtor qword
	rotqby			$3,			$3,				$80			#Get dtor in preferred slot
	brnz			$3,			CallDestructor				#Branch back if not terminated by 0x00000000
	br			FinishedCallingGlobalDestructors
