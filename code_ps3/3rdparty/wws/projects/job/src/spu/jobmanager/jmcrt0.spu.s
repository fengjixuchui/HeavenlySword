/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */


	.section .fillsect.entrypoint,"ax"
	.global	cellSpursPolicyEntry
	.global WwsJob_Main
	.extern g_stackSetupValue

cellSpursPolicyEntry:
	lqa		$1,		g_stackSetupValue
	ai		$2,		$1,		0x20
	il		$5,		0
	stqd	$2,		0($1)
	stqd	$5,		16($1)
	stqd	$5,		32($1)
	br		WwsJob_Main
