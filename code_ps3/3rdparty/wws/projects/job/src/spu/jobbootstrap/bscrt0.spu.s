/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

	.data
	.align	4
	.global g_stackSetupValue

g_stackSetupValue:
	.word 0x3FFD0, 0x1FC0, 0, 0



	.text
	.align	3
	.global	cellSpursPolicyEntry
	.global BootStrapMain

cellSpursPolicyEntry:
	lqa		$1,		g_stackSetupValue
	ai		$2,		$1,		0x20
	il		$5,		0
	stqd	$2,		0($1)
	stqd	$5,		16($1)
	stqd	$5,		32($1)
	br		BootStrapMain
