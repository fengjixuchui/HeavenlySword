/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

/*
	This is the startup code for a batch job plugin.
        It exposes three functions:
        void BatchJobPluginLoad(Ice::BatchJob::DispatcherFunction *pBatchJobFunctionTable)
                Sets g_batchJobFunctionTable to pBatchJobFunctionTable.
                Clears the BSS section.
                Calls all global constructors for the plugin.
                Returns BatchJobPluginCall in $3, BatchJobPluginUnload in $4
        void BatchJobPluginExecute(VU16 args, VU32 memoryMap)
                Sets up $126 and calls BatchJobPluginMain()
        void BatchJobPluginUnload()
                Calls all global destructors.

        The plugin is expected to contain a global function:
        void BatchJobPluginMain(VU16 args, VU32 memoryMap)
*/

	.section .uninitialized,"aw",@nobits
	.align 4
	.global g_batchJobFunctionTable

                                #32 byte SN, JobCheckData header
g_batchJobFunctionTable:        #@0x00020
	.zero 16

	.text
	.align	3
	.global	BatchJobPluginLoad
	.global BatchJobPluginExecute
        .global BatchJobPluginUnload
        .global BatchJobPluginMain
        .global _exit
	.global __CTOR_LIST__
	.global __CTOR_END__
	.global __DTOR_LIST__
	.global __DTOR_END__
	.global __BSS_START__
	.global __BSS_END__

##################################################################################################
BatchJobPluginLoad:     #void BatchJobPluginLoad(pBatchJobFunctionTable)                        #@0x00030
                                        stqd    $80,-16($1)     # Create a stack with one non-volatile ($80) for iteration
                                        stqd    $1, -48($1)
                                        stqd    $0,  16($1)
        ai      $1, $1, -48

	ila     $2, BatchJobPluginLoadPositionIndependenceLabel
                                        stqr    $3, g_batchJobFunctionTable     # store g_batchJobFunctionTable
	                                brsl    $126, BatchJobPluginLoadPositionIndependenceLabel
BatchJobPluginLoadPositionIndependenceLabel:

	ila     $3, __BSS_START__
        ila	$4, __BSS_END__
        il      $5, 0
	sf      $126, $2, $126                  #Calculate position independence offset
                                        brsl    $6, ZeroBss     #Call ZeroBss(__BSS_START__, __BSS_END__, 0) to zero the BSS section

	ila	$80, (__CTOR_END__ - 4)	        #Pointer to the last ctor
	a	$80, $80, $126		        #Offset last ctor pointer appropriately
	lqd	$7, 0($80)		        #Load up ctor qword
	rotqby	$3, $7, $80		        #Get ctor in preferred word
	ceqi	$2, $3,	-1		        #Ctor list is terminated by 0xFFFFFFFF
	brz	$2, BatchJobPluginCallConstructor	#Branch to constructors if first is not a terminator
BatchJobPluginCallConstructorEnd:
                                        lqd     $1,   0($1)     #restore stack
                                        lqd     $0,  16($1)
                                        lqd     $80,-16($1)
                                        bi      $0              #return

##################################################################################################
BatchJobPluginExecute:  #void BatchJobPluginExecute(VU16 args, VU32 memoryMap)                  #@0x00088
                                        stqd    $1, -32($1)     # Create a stack with no non-volatiles
                                        stqd    $0,  16($1)
        ai      $1, $1, -32

	ila     $2, BatchJobPluginExecutePositionIndependenceLabel
	                                brsl    $126, BatchJobPluginExecutePositionIndependenceLabel
BatchJobPluginExecutePositionIndependenceLabel:

        ila     $79, BatchJobPluginMain
	sf      $126, $2, $126                  #Calculate position independence offset
        a       $79, $79, $126                  #Offset BatchJobPluginMain appropriately
                                        bisl    $0, $79         #call BatchJobPluginMain($3,$4,...)

                                        lqd     $1,   0($1)     #restore stack
                                        lqd     $0,  16($1)
                                        bi      $0              #return

##################################################################################################
BatchJobPluginUnload:   #void BatchJobPluginUnload()                                            #@0x000b8
                                        stqd    $80,-16($1)     # Create a stack with one non-volatile ($80) for iteration
                                        stqd    $1, -48($1)
                                        stqd    $0,  16($1)
        ai      $1, $1, -48

	ila     $2, BatchJobPluginUnloadPositionIndependenceLabel
	                                brsl    $126, BatchJobPluginUnloadPositionIndependenceLabel 
BatchJobPluginUnloadPositionIndependenceLabel:

	sf      $126, $2, $126          #Calculate position independence offset

	ila     $80, (__DTOR_LIST__ + 4)	#Pointer to first dtor
	a	$80, $80, $126  		#Offset first dtor pointer appropriately
	lqd	$3, 0($80)			#Load up dtor qword
	rotqby	$3, $3,	$80			#Get dtor in preferred slot
	brnz	$3, BatchJobPluginCallDestructor        #Branch to destructors if first is not terminated by 0x00000000
BatchJobPluginCallDestructorEnd:
                                        lqd     $1,   0($1)     #restore stack
                                        lqd     $0,  16($1)
                                        lqd     $80,-16($1)
                                        bi      $0              #return

##################################################################################################
ZeroBss:# void ZeroBss(pStart, pEnd, clearValue)                                                #@0x000f8
	# All stores relative to $126 to shift to the correct PIC run-time location.
        # In our preamble, we zero qwords  pEnd-0x10, pEnd-0x20, pEnd-0x30 conditionally
        # then subtract pEnd-=0x30 (the three qwords) + 0x40 (so we can compare before incrementing)
        # so that our loop can set 4 qwords at a time.
        sf      $8, $3, $4              
                                        hbrr    ZeroBssBranch, ZeroBssLoop
        clgt    $10, $4, $3             
        a       $4, $4, $126
        clgti   $11, $8, 16             
                                        biz     $10, $6         #Return if no qwords to zero
        clgti   $12, $8, 32             
                                        stqd    $5, -16($4)
        clgti   $13, $8, 48             
                                        biz     $11, $6         #Return if only 1 qword
                                        stqd    $5, -32($4)
                                        biz     $12, $6         #Return if only 2 qwords
        a       $3, $3, $126                    
                                        stqd    $5, -48($4)
        ai      $4, $4,-112              
                                        biz     $13, $6         #Return if only 3 qwords 
ZeroBssLoop:
	ai      $13, $3, 0              
                                        stqd	$5,  0($3)
	clgt    $10, $4, $3             
                                        stqd	$5, 16($3)
	ai      $3, $3, 0x40            
                                        stqd	$5, 32($13)
	                                stqd	$5, 48($13)
ZeroBssBranch:
                                        brnz    $10, ZeroBssLoop
                                        bi      $6              #Return when ZeroBssLoop terminates

##################################################################################################
_exit:												#@0x0015c
					stopd	$0, $0, $0                             
##################################################################################################
BatchJobPluginCallConstructor:                                                                  #@0x00160
	ai      $80, $80, -4			#Iterate backwards over ctors
	a	$3, $3, $126		        #Offset ctor pointer appropriately
	bisl	$0, $3				#Call ctor
	lqd	$7, 0($80)			#Load up ctor qword
	rotqby	$3, $7, $80			#Get ctor in preferred word
	ceqi	$2, $3,	-1			#Ctor list is terminated by 0xFFFFFFFF
	brz	$2, BatchJobPluginCallConstructor		#Loop if not terminator
	br	BatchJobPluginCallConstructorEnd                #Return
##################################################################################################
BatchJobPluginCallDestructor:                                                                   #@0x00180
	a	$3, $3,	$126    		#Offset dtor pointer appropriately
	bisl	$0, $3				#Call dtor
	ai	$80, $80, 4			#Iterate forwards over dtors
	lqd	$3, 0($80)			#Load up dtor qword
        nop
	rotqby	$3, $3, $80			#Get dtor in preferred word
	brnz	$3, BatchJobPluginCallDestructor        	#Loop if not terminator
	br	BatchJobPluginCallDestructorEnd                 #Return
##################################################################################################
                                                                                                #@0x001a0
