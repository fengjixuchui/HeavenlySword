/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

/*
	JobBasePrintf( fmt, varargs...)
	{
		do
		{
			U32 interruptsEnabled = AreInterruptsEnabled();
			if ( interruptsEnabled )
			{
				DisableInterrupts();
			}
			_spu_call_event_va_arg(EVENT_PRINTF_PORT<<EVENT_PORT_SHIFT, fmt, ##varargs);
			if ( interruptsEnabled )
			{
				EnableInterrupts();
			}
		} while ( 0 )
	}
*/

	.text
	.align	3
	.global	JobBasePrintf
	.type	JobBasePrintf, @function

JobBasePrintf:
	rdch	$78,	$ch13					#Are interrupts enabled?
	andi	$78,	$78,		1

	brsl	$2,		.PositionIndependenceLabel
.PositionIndependenceLabel:
	ai		$2,		$2,			8
	bid		$2								#Disable interrupts (even if they were previously disabled)
.PrintfDisableInterrupts:

	rchcnt	$2,		$ch29
	stqd	$3,		-256($1)
	il      $3,		-10						#
	brnz	$2,		.PrintfExit				#Return -10
	stqd	$4,		-240($1)
	stqd	$5,		-224($1)
	stqd	$6,		-208($1)
	stqd	$7,		-192($1)
	stqd	$8,		-176($1)
	stqd	$9,		-160($1)
	stqd	$10,	-144($1)
	stqd	$11,	-128($1)
	stqd	$12,	-112($1)
	stqd	$13,	-96($1)
	stqd	$14,	-80($1)
	stqd	$15,	-64($1)
	stqd	$16,	-48($1)
	stqd	$17,	-32($1)
	stqd	$18,	-16($1)
	or		$2,		$2,			$1
	ai		$2,		$2,			-256
	wrch	$ch28,	$2
	dsync
	ilhu	$3,		0xF00
	wrch	$ch30,	$3
	rdch	$3,		$ch29
	brnz	$3,		.PrintfExit				#Return $3
	rdch	$3,		$ch29					#Return $3
.PrintfExit:

	biz		$78,	$0						#If interrupts were previously disabled return now
	bie		$0								#Otherwise re-enable interrupts on returning
