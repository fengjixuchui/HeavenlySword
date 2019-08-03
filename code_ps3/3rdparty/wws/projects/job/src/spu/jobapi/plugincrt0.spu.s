/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */


/*
	This is the startup code for a plugin
	It calls the global constructors and returns the pointer to PluginStart
*/
	.section .uninitialized,"aw",@nobits
	.align 4
	.global g_jobContext

g_jobContext:
	.zero 16


	.text
	.align	3
	.global	PluginEntryPoint
	.global PluginStart
	.global __CTOR_LIST__
	.global __CTOR_END__
	.global __DTOR_LIST__
	.global __DTOR_END__
	.global __BSS_START__
	.global __BSS_END__

PluginEntryPoint:

	#Respect the ABI
	stqd			$80,		-16($1)
	#stqd			$81,		-32($1)						#Reclaim a qword of stack space here at some point
	stqd			$126,		-48($1)
	stqd			$0,			16($1)
	stqd			$1,			-80($1)
	ai				$1,			$1,				-80

	stqr			$3,			g_jobContext				#Store the jobCtxt param ready for the job to use


	#Calculate position independence offset
	ila				$2,			PluginCrt0PositionIndependenceLabel
	brsl			$126,		PluginCrt0PositionIndependenceLabel
PluginCrt0PositionIndependenceLabel:
	sf				$126,		$2,				$126


	#Zero out the bss
	ila				$3,			__BSS_START__
	ila				$4,			__BSS_END__
	clgt			$2,			$4,				$3			#Compare labels to see if there's anything to zero
	brz				$2,			noBssToZero
	il				$5,			0
zeroNextQword:
	stqx			$5,			$3,				$126		#Store relative to $126 to shift to the correct PIC run-time location
	ai				$3,			$3,				16			#Move to next qword
	clgt			$6,			$4,				$3			#Keep going until all zeroed
	brnz			$6,			zeroNextQword
noBssToZero:


	#Call global constructors
	ila				$80,		(__CTOR_END__ - 4)			#Pointer to the last ctor
	a				$80,		$80,			$126		#Offset last ctor pointer appropriately
	br				TestConstructorTerminator

CallConstructor:
	ai				$80,		$80,			-4			#Iterate backwards over ctors
	a				$3,			$3,				$126		#Offset ctor pointer appropriately
	bisl			$0,			$3							#Call ctor
TestConstructorTerminator:
	lqd				$7,			0($80)						#Load up ctor qword
	rotqby			$3,			$7,				$80			#Get ctor in preferred slot
	ceqi			$2,			$3,				-1			#Ctor lists is terminated by 0xFFFFFFFF
	brz				$2,			CallConstructor				#Branch back if not terminator



	#######################################
	#Return the pointer to the main function to the module that called the init
	#######################################
	ila				$3,			PluginStart
	a				$3,			$3,				$126
	ai				$1,			$1,				80			#Respect the ABI
	lqd				$126,		-48($1)
	lqd				$80,		-16($1)
	lqd				$0,			16($1)
	bi				$0





###################################################
###################################################
###################################################
###################################################
###################################################
#PluginShutdown:
#
#	ila				$80,		(__DTOR_LIST__ + 4)			#Pointer to first dtor
#	a				$80,		$80,			$126		#Offset first dtor pointer appropriately
#	br				TestDestructorTerminator
#CallDestructor:
#	a				$3,			$3,				$126		#Offset dtor pointer appropriately
#	bisl			$0,			$3							#Call dtor
#	ai				$80,		$80,			4			#Iterate forwards over dtors
#TestDestructorTerminator:
#	lqd				$3,			0($80)						#Load up dtor qword
#	rotqby			$3,			$3,				$80			#Get dtor in preferred slot
#	brnz			$3,			CallDestructor				#Branch back if not terminated by 0x00000000
#
#
#	ai				$1,			$1,				80			#Respect the ABI
#	lqd				$126,		-48($1)
#	lqd				$80,		-16($1)
#	lqd				$0,			16($1)
#	bi				$0										#Return to the Job Manager
