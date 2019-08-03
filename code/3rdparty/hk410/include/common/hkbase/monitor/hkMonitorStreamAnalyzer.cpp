/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/htl/hkObjectArray.h>
#include <hkbase/monitor/hkMonitorStreamAnalyzer.h>
#include <hkbase/config/hkConfigVersion.h>

const char* HK_CAPTURE_FRAME_STRING = "Fd";
const char* HK_CAPTURE_PARAMS_STRING = "Ii";


//
// Node constructors and destructor
//
hkMonitorStreamAnalyzer::Node::Node(Node* parent, const char* name, NodeType type )
: m_parent(parent), m_name( name ), m_userFlags(0), m_type( type )
{
	if (parent)
	{
		parent->m_children.pushBack(this);
	}
	for (int i = 0; i < NUM_VALUES; i++)
	{
		m_value[i] = 0; m_count[i] = 0;
	}
	m_absoluteStartTime = 0;
}

hkMonitorStreamAnalyzer::Node::~Node()
{
	for (int i=0; i< m_children.getSize();i++)
	{
		delete m_children[i];
	}
	m_children.clear();
}

char* hkMonitorStreamAnalyzer::getStreamBegin()
{
	// If there have been no calls to HK_TIMER_BEGIN, the size of m_data will
	// be 0. Calling m_data[0] would cause an assert in such a case.
	if (m_data.getSize() == 0)
	{
		return HK_NULL;
	}
	else
	{
		return &m_data[0];
	}
}



//
// Monitor stream analyzer constructor and destructor
//
hkMonitorStreamAnalyzer::hkMonitorStreamAnalyzer(int memorySize)
{
	m_data.reserve( memorySize );
	m_nodeIdForFrameOverview = "Simulate";
}

void hkMonitorStreamAnalyzer::reset()
{
	m_data.clear();
	m_frameInfos.clear();
	m_frameStartLocations.clear();
}


hkMonitorStreamFrameInfo::hkMonitorStreamFrameInfo() 
	: m_heading( "Unknown Heading" ),
	m_indexOfTimer0(0), 
	m_indexOfTimer1(1), 
	m_absoluteTimeCounter(ABSOLUTE_TIME_TIMER_0), 
	m_timerFactor0(1.0f), 
	m_timerFactor1(1.0f)
{
}

inline void _byteSwapUint32( hkUint32& v )
{
	union b4
	{
		hkUint8 b[4];
		hkUint32 d;
	} dataIn, dataOut;

	dataIn.d = v;
	dataOut.b[0] = dataIn.b[3];
	dataOut.b[1] = dataIn.b[2];
	dataOut.b[2] = dataIn.b[1];
	dataOut.b[3] = dataIn.b[0];
	v = dataOut.d;
} 

inline void _byteSwapFloat( hkReal& v )
{
	union b4
	{
		hkUint8 b[4];
		hkReal d;
	} dataIn, dataOut;

	dataIn.d = v;
	dataOut.b[0] = dataIn.b[3];
	dataOut.b[1] = dataIn.b[2];
	dataOut.b[2] = dataIn.b[1];
	dataOut.b[3] = dataIn.b[0];
	v = dataOut.d;
} 

void hkMonitorStreamAnalyzer::applyStringMap( const char* frameStart, const char* frameEnd, const hkPointerMap<void*, char*>& map, hkBool endianSwap )
{
//	hkPointerMap<void*, char*> map; 
//	for ( int i = 0; i < m_mappings.getSize(); ++i )
//	{
//		const void* ptr = reinterpret_cast<void*>( m_mappings[i].m_id );
//		map.insert(const_cast<void*>(ptr), const_cast<char*>(m_mappings[i].m_string));
//	}

	// if the data came from a different endian machine, we need to run through it and do a quick swap on
	// data that is > byte sized.

	char* current = const_cast<char*>( frameStart );
	char* end = const_cast<char*>( frameEnd );
	while(current < end) // for all frames
	{
		hkMonitorStream::Command* command = reinterpret_cast<hkMonitorStream::Command*>(current);

		// Replace char* with pointer to loaded string
		HK_ON_DEBUG( hkResult res = ) map.get( const_cast<char*>(command->m_commandAndMonitor), const_cast<char**>(&command->m_commandAndMonitor) ); 
		HK_ASSERT(0,res != HK_FAILURE);
		
		switch(command->m_commandAndMonitor[0])
		{
		case 'T': // timer begin
		case 'E': // timer end
		case 'S': // split list
		case 'l': // list end
			{
				hkMonitorStream::TimerCommand* timerCommand = reinterpret_cast<hkMonitorStream::TimerCommand*>( current );
				current = (char*)(timerCommand + 1);
				if (endianSwap)
				{
					_byteSwapUint32( timerCommand->m_time0 );
					_byteSwapUint32( timerCommand->m_time1 );
				}
				break;
			}

		case 'L': // timer list begin
			{
				hkMonitorStream::TimerBeginListCommand* timerCommand = reinterpret_cast<hkMonitorStream::TimerBeginListCommand*>( current );
				current = (char*)(timerCommand + 1);
				
				HK_ON_DEBUG( hkResult res2 = ) map.get( const_cast<char*>(timerCommand->m_nameOfFirstSplit), const_cast<char**>(&timerCommand->m_nameOfFirstSplit) ); 
				HK_ASSERT(0,res2 != HK_FAILURE);
				
				if (endianSwap)
				{
					_byteSwapUint32( timerCommand->m_time0 );
					_byteSwapUint32( timerCommand->m_time1 );
				}
				break;
			}

		case 'M':
			{
				hkMonitorStream::AddValueCommand* serializedCommand = reinterpret_cast<hkMonitorStream::AddValueCommand*>( current );
				current = (char*)(serializedCommand + 1);
				if (endianSwap)
				{
					_byteSwapFloat( serializedCommand->m_value );
				}
				break;
			}
		case 'P':
		case 'p':
			{
				hkMonitorStream::Command* serializedCommand = reinterpret_cast<hkMonitorStream::Command*>( current );
				current = (char*)(serializedCommand + 1);
				break;
			}

		case 'F':	// new frame
		case 'N':	// nop, skip command
			{
				hkMonitorStream::Command* com = reinterpret_cast<hkMonitorStream::Command*>(current);
				current = (char*)(com + 1);
				break;
			}
		
		default:
			HK_ASSERT2(0x3f2fecd9, 0, "Inconsistent Monitor capture data" ); return;
		}
	}
}

void hkMonitorStreamAnalyzer::extractStringMap( const char* frameStart, const char* frameEnd, hkPointerMap<void*, char*>& map )
{
	const char* current = frameStart;
	const char* end = frameEnd;

	while ( current < end )
	{
		const hkMonitorStream::Command* command = reinterpret_cast<const hkMonitorStream::Command*>( current );
		char* str = const_cast<char*>(command->m_commandAndMonitor);
		map.insert(str, str);

		switch( command->m_commandAndMonitor[0] )
		{
		case 'S':		// split list
		case 'E':		// timer end
		case 'l':		// list end
		case 'T':		// timer begin
			{
				const hkMonitorStream::TimerCommand* com = reinterpret_cast<const hkMonitorStream::TimerCommand*>( current );
				current = (const char *)(com+1);
				break;
			}
		
		case 'L':		// timer list begin
			{
				const hkMonitorStream::TimerBeginListCommand* com = reinterpret_cast<const hkMonitorStream::TimerBeginListCommand*>( current );
				map.insert(const_cast<char*>(com->m_nameOfFirstSplit), const_cast<char*>(com->m_nameOfFirstSplit) );
				current = (const char *)(com+1);
				break;
			}

		case 'M':
			{
				const hkMonitorStream::AddValueCommand* com = reinterpret_cast<const hkMonitorStream::AddValueCommand*>( current );
				current = (const char *)(com+1);
				break;
			}
		case 'P':
		case 'p':
		case 'N':
			{
				const hkMonitorStream::Command* com = reinterpret_cast<const hkMonitorStream::Command*>( current );
				current = (const char *)(com+1);
				break;
			}
		
			default:
			HK_ASSERT2(0x5120d10a, 0, "Inconsistent Monitor capture data" ); 	return;
		}
	}
}


void hkMonitorStreamAnalyzer::captureFrameDetails( const char* monitorStreamBegin, const char* monitorStreamEnd, hkMonitorStreamFrameInfo& info )
{
	// make sure there is enough capacity for the actual capture data (+params +end of frame)
	int size = static_cast<int>( monitorStreamEnd - monitorStreamBegin); // 2 Gig limit ;)
	if( (m_data.getCapacity() - m_data.getSize()) < size  )
	{
		return;
	}

	// allocate the space
	char* data = m_data.expandByUnchecked( size  );

	// copy the capture data
	hkString::memCpy( data , monitorStreamBegin, size );

	// copy the frame info
	hkMonitorStreamFrameInfo& newInfo = m_frameInfos.expandOne();
	newInfo = info;
	
	// Note the start point in the stream for the capture. 
	m_frameStartLocations.setSize(m_frameInfos.getSize() + 1);
	m_frameStartLocations[m_frameStartLocations.getSize() - 2] = m_data.getSize() - size;
	
	// The last element in the start locations array is always the end of the stream
	m_frameStartLocations[m_frameStartLocations.getSize() - 1] = m_data.getSize();
}


//
//  build tree
//

static hkMonitorStreamAnalyzer::Node* createNewNode( hkMonitorStreamAnalyzer::Node* parent, const char* name, hkMonitorStreamAnalyzer::Node::NodeType type, bool wantNodeReuse )
{
	// see if we have added this node already to this parent. If so reuse and augment
	// to prevent needless spliting.
	if(wantNodeReuse)
	{
		for (int c=0; parent && name && (c < parent->m_children.getSize()); ++c)
		{
			if (parent->m_children[c]->m_name && (hkString::strCmp(parent->m_children[c]->m_name, name) == 0) )
			{
				return parent->m_children[c];
			}
		}
	}

	return new hkMonitorStreamAnalyzer::Node( parent, name, type );
}



void hkMonitorStreamAnalyzer::Node::setTimers( const hkMonitorStreamFrameInfo& frameInfo, const hkMonitorStream::TimerCommand& start, const hkMonitorStream::TimerCommand& end)
{
	int id0 = frameInfo.m_indexOfTimer0;
	// note we use += here as we may be sharing a previous node.
	if ( id0 >= 0 && id0 < NUM_VALUES)
	{
		this->m_value[id0] += frameInfo.m_timerFactor0 * float( end.m_time0 - start.m_time0 );
		this->m_count[id0] += 1;
	}
	int id1 = frameInfo.m_indexOfTimer1;
	if ( id1 >= 0 && id1 < NUM_VALUES)
	{
		this->m_value[id1] += frameInfo.m_timerFactor1 * float(end.m_time1 - start.m_time1 );
		this->m_count[id1] += 1;
	}

	if (frameInfo.m_absoluteTimeCounter == hkMonitorStreamFrameInfo::ABSOLUTE_TIME_TIMER_0)
	{
		//HK_ASSERT(0, m_absoluteStartTime == 0); 
		m_absoluteStartTime = frameInfo.m_timerFactor0 * float( start.m_time0 );
	}
	else if (frameInfo.m_absoluteTimeCounter == hkMonitorStreamFrameInfo::ABSOLUTE_TIME_TIMER_1)
	{
		//HK_ASSERT(0, m_absoluteStartTime == 0);
		m_absoluteStartTime = frameInfo.m_timerFactor1 * float( start.m_time1 );
	}
}

static bool findChildWithHint(hkMonitorStreamAnalyzer::Node* mainNode, const char* childName, int& childId )
{
	if ( (mainNode->m_children.getSize() > childId) && (hkString::strCmp(mainNode->m_children[childId]->m_name, childName) == 0) )
	{
		return true;
	}

	for (int i = 0; i < mainNode->m_children.getSize(); ++i)
	{
		if ( hkString::strCmp(mainNode->m_children[i]->m_name, childName) == 0)
		{
			childId = i;
			return true;
		}
	}
	return false;
}

static void HK_CALL reduceMainTree( hkMonitorStreamAnalyzer::Node* node, int destTimerId, hkReal smoothingFactor )
{
	node->m_value[destTimerId] *= smoothingFactor;
	node->m_count[destTimerId] = 0;
	for ( int i = 0; i < node->m_children.getSize(); ++i)
	{
		reduceMainTree( node->m_children[i], destTimerId, smoothingFactor );
	}
}


static void HK_CALL mergeSubTree( hkMonitorStreamAnalyzer::Node* mainTree, hkMonitorStreamAnalyzer::Node* subTree, int destTimerId, int sourceTimerId, hkReal smoothingFactor )
{
	if (destTimerId > 6)
	{
		HK_WARN_ONCE(0x6945fade, "Only 6 threads are supported at the moment.");
		destTimerId = 6;
	}

	int childId = 0; // Also use this as a hint to hasChild - usually the children will be in the same order.

	for ( int i = 0; i < subTree->m_children.getSize(); ++i )
	{
		hkMonitorStreamAnalyzer::Node* matchedNode = HK_NULL;
		hkMonitorStreamAnalyzer::Node* subTreeNode = subTree->m_children[i];

		if ( !findChildWithHint( mainTree, subTree->m_children[i]->m_name, childId ) )
		{
			// No child exists in the main tree - add a new one
			matchedNode = new hkMonitorStreamAnalyzer::Node(mainTree, subTreeNode->m_name, subTreeNode->m_type);
		}
		else
		{
			matchedNode = mainTree->m_children[childId];
		}
		matchedNode->m_value[destTimerId] += (1.0f - smoothingFactor) * subTreeNode->m_value[sourceTimerId];
		matchedNode->m_count[destTimerId] = subTreeNode->m_count[sourceTimerId];
		mergeSubTree(matchedNode, subTreeNode, destTimerId, sourceTimerId, smoothingFactor );

		if (childId < subTree->m_children.getSize() - 1 )
		{
			childId++;
		}
	}
}		



void hkMonitorStreamAnalyzer::mergeTreesForRuntimeDisplay( hkMonitorStreamAnalyzer::Node* mainTree, hkMonitorStreamAnalyzer::Node* subTree, int destTimerId, int sourceTimerId, hkReal smoothingFactor )
{
	reduceMainTree( mainTree, destTimerId, smoothingFactor );
	mergeSubTree( mainTree, subTree, destTimerId, sourceTimerId, smoothingFactor );
}


hkMonitorStreamAnalyzer::Node* hkMonitorStreamAnalyzer::navigateMonitors( const hkMonitorStreamAnalyzer::CursorKeys& keys, hkMonitorStreamAnalyzer::Node* activeNodeIn )
{
	hkMonitorStreamAnalyzer::Node* activeNode = activeNodeIn;

	if ( activeNode == HK_NULL )
	{
		return HK_NULL;
	}

	if ( keys.m_upPressed )
	{
		hkMonitorStreamAnalyzer::Node* f = activeNode->m_parent->m_children[0];
		// If there is a previous child to go to, move to it
		if ( f != activeNode )
		{
			// find the previous child
			for (int i= 0; i < activeNode->m_parent->m_children.getSize(); ++i)
			{
				if ( activeNode->m_parent->m_children[i] == activeNode)
				{
					f = activeNode->m_parent->m_children[i - 1];
					break;
				}
			}

			// if this value is unfolded, go into it
			while (true)
			{
				if ( ( f->m_children.getSize() > 0) && f->m_userFlags & 1 )
				{
					f = f->m_children[f->m_children.getSize() - 1];
					continue;
				}
				break;
			}
			activeNode = f;
		}
		else
		{
			if ( activeNode->m_parent->m_parent )
			{
				activeNode = activeNode->m_parent;
			}	
		}
	}

	// test for down
	if ( keys.m_downPressed )
	{
		hkMonitorStreamAnalyzer::Node* f  = activeNode;
		if ( ( f->m_children.getSize() > 0 ) && f->m_userFlags & 1 )
		{
			activeNode = f->m_children[0];
		}
		else
		{
			bool foundChild = false;
			while (!foundChild)
			{
				for (int i = 0; i < f->m_parent->m_children.getSize(); ++i)
				{
					if ( (f->m_parent->m_children[i] == f) && i < (f->m_parent->m_children.getSize() - 1) )
					{
						activeNode = f->m_parent->m_children[i + 1];
						foundChild = true;
						break;
					}
				}
				if ( !foundChild && f->m_parent->m_parent )
				{
					f = f->m_parent;
					continue;
				}
				break;
			}
		}
	}

	// test for left
	if ( keys.m_leftPressed )
	{
		if ( activeNode->m_userFlags & 1 )
		{
			activeNode->m_userFlags &= ~1;
		}
		else
		{
			if ( activeNode->m_parent->m_parent )
			{
				activeNode = activeNode->m_parent;
				activeNode->m_userFlags &= ~1;
			}
		}
	}

	// test for right
	if ( keys.m_rightPressed )
	{
		if ( 0 == (activeNode->m_userFlags & 1))
		{
			activeNode->m_userFlags |= 1;
		}
		{
			// unfold the whole subtree
			activeNode->m_userFlags |= 1;
		}
	}

	return activeNode;
}	
void hkMonitorStreamAnalyzer::printMonitorsToRuntimeDisplay( hkOstream& os, hkMonitorStreamAnalyzer::Node* rootNode, hkMonitorStreamAnalyzer::Node* activeNode, hkObjectArray<hkString>& names, int numFields, char rightArrow, char downArrow )
{
	os.printf("Timer Name	    Count   "); 
	if (numFields > 1)
	{
		for ( int i = 0; i < numFields; ++i )
		{
			os.printf(names[i].cString() );
		}
		os.printf("Total CPU" );
	}
	os.printf("\n\n");

	printMonitorsToRuntimeDisplay( os, rootNode, 0, activeNode, numFields, rightArrow, downArrow);
}

void hkMonitorStreamAnalyzer::printMonitorsToRuntimeDisplay( hkOstream& os, hkMonitorStreamAnalyzer::Node* node, int recursionDepth, hkMonitorStreamAnalyzer::Node* activeNode, int numFields, char rightArrow, char downArrow)
{
	if (numFields > NUM_VALUES)
	{
		HK_WARN_ONCE(0x6945fade, "Too many threads for timer output.");
		numFields = NUM_VALUES;
	}

	if (recursionDepth != 0)
	{
		os << char((activeNode == node) ? rightArrow : ' ');
		{
		    for(int i=0; i < recursionDepth; ++i)
		    {
			    os << ' ';
		    }
		}
		
		if( node->m_children.getSize() > 0 )
		{
			os << ((node->m_userFlags & 1) ? downArrow : rightArrow);
		}
		else
		{
			os << ' ';
		}

		int count = 0;
		{
			for ( int i = 0; i < numFields; ++i )
			{
				count += node->m_count[i];
			}
		}

		os.printf("%-12s(%4i): ", node->m_name, count );
		{
			hkReal total = 0;
		    for ( int i = 0; i < numFields; ++i )
		    {
			    os.printf("%-6.3f ", node->m_value[i] );
				total += node->m_value[i];
		    }
			if (numFields > 1)
			{
				os.printf("%-6.3f ", total );
			}
		}

		os << '\n';
	}

	if( node->m_userFlags & 1 )
	{
		for ( int i = 0; i < node->m_children.getSize(); ++i )
		{
			printMonitorsToRuntimeDisplay( os, node->m_children[i], recursionDepth + 1, activeNode, numFields, rightArrow, downArrow );
		}
	}
}


hkMonitorStreamAnalyzer::Node* hkMonitorStreamAnalyzer::makeStatisticsTree( const char* frameStart, 
																						const char* frameEnd, 
																						const hkMonitorStreamFrameInfo& frameInfo, 
																						const char* rootNodeName,
																						hkBool reuseNodesIfPossible )
{
	const char* currentStreamPtr = frameStart;
	Node* currentNode = new Node(HK_NULL, rootNodeName, hkMonitorStreamAnalyzer::Node::NODE_TYPE_DIRECTORY );
	Node* rootNode = currentNode;
	hkInplaceArray<hkMonitorStream::TimerCommand,16> timerStack;

	while( currentStreamPtr < frameEnd ) 
	{
		const char* string = reinterpret_cast<const hkMonitorStream::Command*>(currentStreamPtr)->m_commandAndMonitor;

		switch(string[0])
		{
		case 'T': // timer begin
			{
				currentNode = createNewNode( currentNode, string + 2, hkMonitorStreamAnalyzer::Node::NODE_TYPE_TIMER, reuseNodesIfPossible );
				const hkMonitorStream::TimerCommand* com = reinterpret_cast<const hkMonitorStream::TimerCommand*>( currentStreamPtr );
				timerStack.pushBack( *com );
				currentStreamPtr = (const char *)(com+1);
				break;
			}
		case 'E': // timer end
			{
				HK_ASSERT2(0xfafe7975, timerStack.getSize() > 0, "Unmatched HK_TIMER_END() macro (with no HK_TIMER_BEGIN()) in timed code");
				const hkMonitorStream::TimerCommand& start = timerStack[timerStack.getSize() - 1];
				const hkMonitorStream::TimerCommand* com = reinterpret_cast<const hkMonitorStream::TimerCommand*>( currentStreamPtr );

				if ( string[2])
				{
					HK_ASSERT2( 0xf03edefe, hkString::strCmp( start.m_commandAndMonitor+2, string+2) ==0, "Unmatched timercommand: '" << start.m_commandAndMonitor+1 << "' =! '" << string+1 );
				}

				currentNode->setTimers( frameInfo, start, *com );
				currentNode = currentNode->m_parent;
				timerStack.popBack();
				currentStreamPtr = (const char *)(com + 1);
				break;
			}

		case 'L': // timer list begin
			{
				const hkMonitorStream::TimerBeginListCommand* com = reinterpret_cast<const hkMonitorStream::TimerBeginListCommand*>( currentStreamPtr );
				{
					timerStack.pushBack( *com );
					currentNode = createNewNode( currentNode, string + 2, hkMonitorStreamAnalyzer::Node::NODE_TYPE_TIMER, reuseNodesIfPossible );
				}
				{
					hkMonitorStream::TimerCommand com2 = *com;
					com2.m_commandAndMonitor = com->m_nameOfFirstSplit;
					timerStack.pushBack( com2 );
					currentNode = createNewNode( currentNode, com2.m_commandAndMonitor+2, hkMonitorStreamAnalyzer::Node::NODE_TYPE_TIMER, reuseNodesIfPossible );
				}
				currentStreamPtr = (const char*)(com + 1);
				break;
			}
		case 'S': // split list
			{
				hkMonitorStream::TimerCommand& start = timerStack[ timerStack.getSize()- 1 ];
				const hkMonitorStream::TimerCommand* com = reinterpret_cast<const hkMonitorStream::TimerCommand*>( currentStreamPtr );
				currentNode->setTimers( frameInfo, start, *com );
				currentNode = createNewNode( currentNode->m_parent, string + 2, hkMonitorStreamAnalyzer::Node::NODE_TYPE_TIMER, reuseNodesIfPossible );
				start = *com;
				currentStreamPtr = (const char*)(com + 1);
				break;
			}
		case 'l': // list end
			{
				const hkMonitorStream::TimerCommand* com = reinterpret_cast<const hkMonitorStream::TimerCommand*>( currentStreamPtr );
				{
					const hkMonitorStream::TimerCommand& start = timerStack[ timerStack.getSize()- 1 ];
					HK_ASSERT2( 0xf0323454, start.m_commandAndMonitor[0] == 'S', "HK_TIMER_LIST_END without HK_TIMER_LIST_BEGIN() found");
					currentNode->setTimers( frameInfo, start, *com );
					currentNode = currentNode->m_parent;
					timerStack.popBack();
				}
				{
					const hkMonitorStream::TimerCommand& start = timerStack[ timerStack.getSize()- 1 ];
					HK_ASSERT2( 0xf0323454, start.m_commandAndMonitor[0] == 'L', "HK_TIMER_LIST_END without HK_TIMER_LIST_BEGIN() found");
					currentNode->setTimers( frameInfo, start, *com );
					currentNode = currentNode->m_parent;
					timerStack.popBack();
				}
				currentStreamPtr = (const char*)(com + 1);
				break;
			}
		case 'M':
			{
				const hkMonitorStream::AddValueCommand* com = reinterpret_cast<const hkMonitorStream::AddValueCommand*>( currentStreamPtr );
				Node *node = createNewNode( currentNode, string + 2, hkMonitorStreamAnalyzer::Node::NODE_TYPE_SINGLE, reuseNodesIfPossible);
				node->m_value[0] += com->m_value;
				node->m_count[0] += 1;
				currentStreamPtr = (const char*)(com + 1);
				break;
			}
		case 'P':
			{
				const hkMonitorStream::Command* com = reinterpret_cast<const hkMonitorStream::Command*>( currentStreamPtr );
				currentNode = createNewNode( currentNode, string + 2, hkMonitorStreamAnalyzer::Node::NODE_TYPE_DIRECTORY, reuseNodesIfPossible);
				currentStreamPtr = (const char*)(com + 1);
				break;
			}
		case 'p':
			{
				const hkMonitorStream::Command* com = reinterpret_cast<const hkMonitorStream::Command*>( currentStreamPtr );
				currentNode = currentNode->m_parent;
				currentStreamPtr = (const char*)(com + 1);
				break;
			}
		case 'F':	// new frame
			{
				HK_ASSERT(0,0);
				break;
			}
		case 'N': // nop
			{
				const hkMonitorStream::Command* com = reinterpret_cast<const hkMonitorStream::Command*>( currentStreamPtr );
				currentStreamPtr = (const char*)(com + 1);
				break;
			}

		default:
			HK_ASSERT2(0x3d7745e3, 0, "Inconsistent Monitor capture data" ); return HK_NULL;
		}
	}

	return rootNode;
}


hkMonitorStreamAnalyzer::Node* hkMonitorStreamAnalyzer::makeStatisticsTreeForMultipleFrames(	const char* streamStart, 
																								const hkArray<hkMonitorStreamFrameInfo>& frameInfos,
																								const hkArray<int>& frameStartLocations,
																								hkBool reuseNodesIfPossible )
{
	Node* rootNode = new Node( HK_NULL, "/", hkMonitorStreamAnalyzer::Node::NODE_TYPE_DIRECTORY );
	rootNode->m_children.setSize(frameInfos.getSize());


	for (int i = 0; i < frameInfos.getSize(); ++i)
	{
		const char* start = streamStart + frameStartLocations[i];
		const char* end   = streamStart + frameStartLocations[i+1];

		rootNode->m_children[i] = makeStatisticsTree(	start, 	end, frameInfos[i], frameInfos[i].m_heading,	reuseNodesIfPossible );
	}

	return rootNode;
}



/////////////////////////////////////////////////////////////////////////////////
//
// Text output utilities
//
/////////////////////////////////////////////////////////////////////////////////

// The nodes array is arranged as follows:
// For each thread, there is one element in the list
// The root node has the frame info heading
// T

// This function produces several output lists:
//		- a total hierarchical average
//		- a per frame single node
//		- a summary per frame
//		- the detailed view

// (reordered and with these protos for SN compiler:)
static void HK_CALL hkWriteRec( hkOstream& outstream, hkMonitorStreamAnalyzer::Node* node, int RecDepth, float factor );
static void HK_CALL hkMakeSum( hkMonitorStreamAnalyzer::Node* sum, hkMonitorStreamAnalyzer::Node* tree );
static hkMonitorStreamAnalyzer::Node* HK_CALL hkFindChildByName( hkMonitorStreamAnalyzer::Node* parent, const char* childName, hkBool searchAnyChild );
static hkMonitorStreamAnalyzer::Node* HK_CALL hkFindNextChildByName( hkMonitorStreamAnalyzer::Node* parent, const char* childName, const hkMonitorStreamAnalyzer::Node* oldChild );

void hkMonitorStreamAnalyzer::writeStatisticsDetails( hkOstream& outstream, hkArray<Node*>& nodes, int reportLevel, const char* nodeIdForFrameOverview )
{
	// Print the version of Havok that created these statistics.
	outstream.printf("%s\n", HAVOK_SDK_VERSION_STRING);

	// The first two analyses only work for one thread at the moment.
	if (nodes.getSize() == 1 )
	{
		hkArray<Node*>& childNodes = nodes[0]->m_children;

		// the average value
		hkReal avgValue = 0.0f;
		//
		//	summarize everything
		//

		{
			Node sum( 0, "Sum", hkMonitorStreamAnalyzer::Node::NODE_TYPE_DIRECTORY ); // sum over all frames
			float f = 1.0f / childNodes.getSize();
			for ( int i = 0; i < childNodes.getSize();i++ )
			{
				hkMakeSum( &sum, childNodes[i] );
			}
			for ( Node* node = hkFindChildByName( &sum, nodeIdForFrameOverview, true ); node; node = hkFindNextChildByName( &sum, nodeIdForFrameOverview, node ) )
			{
				avgValue += node->m_value[0] * f;
			}
			if ( avgValue <= 0.0f ) { avgValue = 1000.0f; }

			if( reportLevel & REPORT_TOTAL )
			{
				outstream.printf("\n\n");
				outstream.printf("*********************************\n" );
				outstream.printf("********** Total Times    *******\n" );
				outstream.printf("*********************************\n" );
				outstream.printf("Timers are added together\n");
				hkWriteRec( outstream, &sum, 0, f );
			}
		}

		//
		// print each frame in a single line
		//
		if( reportLevel & REPORT_PERFRAME_TIME )
		{
			outstream.printf("\n\n");
			outstream.printf("*********************************\n" );
			outstream.printf("********** Per Frame Time *******\n" );
			outstream.printf("*********************************\n" );
			outstream.printf("Ascii Art all frames overview\n" );

			const int GRAPH_SIZE = 40;
			char buffer[GRAPH_SIZE+10];
			hkString::memSet( buffer, ' ', GRAPH_SIZE );
			buffer[GRAPH_SIZE] = 0;

			for ( int i = 0; i < childNodes.getSize();i++ )
			{
				Node* frameRoot = childNodes[i];
				if ( frameRoot->m_children.getSize() == 0 )
				{
					continue;
				}

				hkReal val = 0.0f;
				const char* nodeName = "Unknown";
				{
					for ( Node* node = hkFindChildByName( frameRoot, nodeIdForFrameOverview, true ); node; node = hkFindNextChildByName( frameRoot, nodeIdForFrameOverview, node ) )
					{
						nodeName = node->m_name;
						val  += node->m_value[0];
					}
				}
				//
				// draw graph
				//
				{
					hkReal relVal = 0.5f * GRAPH_SIZE * val / avgValue;
					int index = int(relVal);
					if ( index < 0) index = 0;
					if (index >=GRAPH_SIZE ) index = GRAPH_SIZE-1;
					char *p = buffer;
					int j = 0;
					for (; j +4 < index; j+=4){ *(p++) = '\t'; }
					int j2 = j;
					for (; j2 < index; j2+=1){ *(p++) = ' '; }
					*(p++) = '#';
					j2++;
					j += 4;
					for (; j2 < j; j2+=1){ *(p++) = ' '; }
					for (; j < GRAPH_SIZE; j+=4){ *(p++) = '\t'; }
					*(p) = 0;
					outstream.printf(buffer);
				}
				outstream.printf("%i %-12s %f\n" , i, nodeName, val );
			}
		}
	}

	//
	//	summary per frame
	//

	if( reportLevel & REPORT_PERFRAME_SUMMARY )
	{
		// For each frame
		for ( int i = 0; i < nodes[0]->m_children.getSize(); i++ )
		{
			// For each thread
			for (int j = 0; j < nodes.getSize(); ++j )
			{
				Node* node = nodes[j];
				if ( i < node->m_children.getSize() )
				{
					Node sum(0, "Sum", hkMonitorStreamAnalyzer::Node::NODE_TYPE_DIRECTORY ); // sum over all frames
					hkMakeSum( &sum, node->m_children[i] );
					outstream.printf("\n");
					outstream.printf("****************************************\n" );
					outstream.printf("****** Summary Frame:%i Thread:%i ******\n",i, j );
					outstream.printf("****************************************\n" );
					outstream.printf("%s\n", node->m_children[i]->m_name ); // should be the heading from the frameinfo
					hkWriteRec( outstream, &sum, 0, 1.0f );
				}
			}
		}
	}

	//
	//	detailed view
	//
	if( reportLevel & REPORT_PERFRAME_DETAIL )
	{
		// For each frame
		for ( int i = 0; i < nodes[0]->m_children.getSize(); i++ )
		{
			// For each thread
			for (int j = 0; j < nodes.getSize(); ++j )
			{
				Node* node = nodes[j];
				if ( i < node->m_children.getSize() )
				{
					outstream.printf("\n\n");
					outstream.printf("***************************************\n" );
					outstream.printf("***** Details Frame-%i Thread:%i ******\n", i, j );
					outstream.printf("***************************************\n" );
					outstream.printf("%s\n", node->m_children[i]->m_name );
					hkWriteRec( outstream, node->m_children[i], 0, 1.0f );
				}
			}
		}
	}
}


static void HK_CALL hkWriteRec( hkOstream& outstream, hkMonitorStreamAnalyzer::Node* node, int RecDepth, float factor )
{
	if( RecDepth )
	{
		for( int j = 1; j < RecDepth; j++ )
		{
			outstream.printf("\t" );
		}

		//
		//	Find maximum count
		//  and the number of columns used
		//
		int maxCount = 0;
		int columns = 0;
		{
			for (int i = 0; i < hkMonitorStreamAnalyzer::NUM_VALUES; i++)
			{
				if ( !node->m_count[i] )
				{
					continue;
				}
				columns ++;
				if ( node->m_count[i] > maxCount){ maxCount = node->m_count[i]; }
			}
		}

		
		//
		//	print name
		//
		if ( maxCount <= 1 )
		{
			outstream.printf("%-12s    ", node->m_name);
		}
		else if ( factor == 1.0f)
		{
			outstream.printf("%-12s(%i) ", node->m_name, maxCount);
		}
		else
		{
			outstream.printf("%-12s(%4.1f) ", node->m_name, maxCount * factor);
		}


		//
		//	rescale and print values 
		//
		if ( columns >0 )
		{
			int columnsToPrint = columns;
			if (1)
			{
				for ( int i = 0; i < hkMonitorStreamAnalyzer::NUM_VALUES; i++)
				{
					int c = node->m_count[i];
					if ( !c )
					{
						continue;
					}
					hkReal val = node->m_value[i] * factor;
					if ( c < maxCount)
					{
						val *= hkReal(maxCount)/hkReal(c);
					}
					columnsToPrint--;
					if (columnsToPrint)
					{
						outstream.printf("%4.3f: ", val);
					}
					else
					{
						outstream.printf("%4.3f\n", val);
					}
				}
			}
		}
		else
		{
			outstream.printf("%4.3f\n", 0.0f);
		}
	}

	for ( int i = 0; i < node->m_children.getSize(); i++ )
	{
		hkWriteRec( outstream, node->m_children[i], RecDepth + 1, factor );
	}
}

static hkMonitorStreamAnalyzer::Node* HK_CALL hkFindChildByName( hkMonitorStreamAnalyzer::Node* parent, const char* childName, hkBool searchAnyChild )
{

	// search child
	for( int j = 0; j < parent->m_children.getSize(); j++ )
	{
		if( hkString::strCmp( childName, parent->m_children[j]->m_name ) == 0 )
		{
			return parent->m_children[j];
		}
	}
	if ( searchAnyChild && parent->m_children.getSize()>0 )
	{
		return parent->m_children[0];
	}
	return HK_NULL;
}

static hkMonitorStreamAnalyzer::Node* HK_CALL hkFindNextChildByName( hkMonitorStreamAnalyzer::Node* parent, const char* childName, const hkMonitorStreamAnalyzer::Node* oldChild )
{
	// search child
	int j;
	for( j = 0; j < parent->m_children.getSize(); j++ )
	{
		if ( parent->m_children[j] == oldChild )
		{
			break;
		}
	}
	j++;
	for( ; j < parent->m_children.getSize(); j++ )
	{
		if( hkString::strCmp( childName, parent->m_children[j]->m_name ) == 0 )
		{
			return parent->m_children[j];
		}
	}
	return HK_NULL;
}

// This duplicates the structure of tree into sum (combining childen with the same parent of the same name)
static void HK_CALL hkMakeSum( hkMonitorStreamAnalyzer::Node* sum, hkMonitorStreamAnalyzer::Node* tree )
{
	for( int i = 0; i < tree->m_children.getSize(); i++ )
	{
		hkMonitorStreamAnalyzer::Node* childIn = tree->m_children[i];
		hkMonitorStreamAnalyzer::Node* childOut = hkFindChildByName( sum, childIn->m_name, false);

		if( !childOut )
		{
			childOut = new hkMonitorStreamAnalyzer::Node( sum, childIn->m_name, childIn->m_type );
		}
		for (int j = 0; j < hkMonitorStreamAnalyzer::NUM_VALUES; j++)
		{
			childOut->m_value[j]  += childIn->m_value[j];
			childOut->m_count[j]  += childIn->m_count[j];
		}

		hkMakeSum( childOut, childIn );
	}
}

// Multiple write statistics
void hkMonitorStreamAnalyzer::writeMultiThreadedStatistics( const hkArray<hkMonitorStreamAnalyzer*>& streamCaptures, hkOstream& outStream, int reportLevel, const char* nodeIdForFrameOverview )
{
	hkArray<Node*> nodes;

	{
	    for (int i = 0; i < streamCaptures.getSize(); ++i )
	    {
	    
		    Node* node =  hkMonitorStreamAnalyzer::makeStatisticsTreeForMultipleFrames(	streamCaptures[i]->getStreamBegin(),
																					    streamCaptures[i]->m_frameInfos,
																					    streamCaptures[i]->m_frameStartLocations,
																					    false );
			nodes.pushBack(node);
	   	}
	}
	
	hkMonitorStreamAnalyzer::writeStatisticsDetails( outStream, nodes, reportLevel, nodeIdForFrameOverview );

	{
	    for ( int i = 0; i < nodes.getSize();i++ )
	    {
		    delete nodes[i];
	    }
    }
}


// Single write statistics
void hkMonitorStreamAnalyzer::writeStatistics( hkOstream& outStream, int reportLevel )
{
	hkArray<hkMonitorStreamAnalyzer*> streamCapture;
	streamCapture.pushBack(this);
	if (m_data.getSize() > 0)
	{
		writeMultiThreadedStatistics( streamCapture, outStream, reportLevel, m_nodeIdForFrameOverview);
	}
	return;
}


//
// Drawing utilities
//

inline hkMonitorStreamAnalyzer::Node* findChildNode( hkMonitorStreamAnalyzer::Node* currentNode, float sampleTime, int absoluteTimeIndex )
{
	for (int i = 0; i < currentNode->m_children.getSize(); ++i )
	{
		hkMonitorStreamAnalyzer::Node* child = currentNode->m_children[i];
		hkReal endTime = child->m_absoluteStartTime + child->m_value[absoluteTimeIndex];
		if ( (sampleTime > child->m_absoluteStartTime) && ( sampleTime < endTime ) )
		{
			return child;
		}
	}
	return HK_NULL;
}



hkMonitorStreamAnalyzer::Node* getNodeAtSample( hkMonitorStreamAnalyzer::Node* currentNode, float sampleTime, int absoluteTimeIndex )
{
	HK_ASSERT(0, currentNode->m_absoluteStartTime <= sampleTime);
	hkReal endTime = currentNode->m_absoluteStartTime + currentNode->m_value[absoluteTimeIndex];
	if ( sampleTime > endTime )
	{
		return getNodeAtSample( currentNode->m_parent, sampleTime, absoluteTimeIndex );
	}
	else
	{
		hkMonitorStreamAnalyzer::Node* child = findChildNode( currentNode, sampleTime, absoluteTimeIndex );
		if ( child != HK_NULL )
		{
			return getNodeAtSample(child, sampleTime, absoluteTimeIndex );
		}
	}
	return currentNode;
}

#include <hkbase/htl/hkPointerMap.h>



static void HK_CALL outputStatsForFrame(	hkMonitorStreamAnalyzer::Node* root, 
																	hkReal startTime, 
																	hkReal timeInc, 
																	hkArray<hkMonitorStreamAnalyzer::Node*>& timerNodesAtTicks, 
																	int absoluteTimeIndex )
{
	hkMonitorStreamAnalyzer::Node* firstNode = root->m_children[0];
	hkMonitorStreamAnalyzer::Node* lastNode  = root->m_children.back();

	// Wait until the current time starts
	// adding null names makes the entry in the texture the background texture
	float currentTime = firstNode->m_absoluteStartTime;
	float sampleTime = startTime;
	while ( currentTime > sampleTime )
	{
		sampleTime += timeInc;
		timerNodesAtTicks.pushBack(HK_NULL);
	}

	hkReal endTime = lastNode->m_absoluteStartTime + lastNode->m_value[absoluteTimeIndex];

	// reset root
	root->m_absoluteStartTime = firstNode->m_absoluteStartTime;
	root->m_value[absoluteTimeIndex] = endTime - root->m_absoluteStartTime + 1.0f;

	while ( endTime > sampleTime ) // more to sample
	{
		hkMonitorStreamAnalyzer::Node* node = getNodeAtSample( firstNode, sampleTime, absoluteTimeIndex );
		timerNodesAtTicks.pushBack(node);
		sampleTime += timeInc;
	}

}



int hkMonitorStreamAnalyzer::ColorTable::findColor( const char* color )
{
	for (int i = 0; i < m_colorPairs.getSize(); ++i)
	{
		if ( hkString::strCasecmp(m_colorPairs[i].m_colorName, color) == 0)
		{
			return m_colorPairs[i].m_color;
		}
	}
	return 0xffffffff;
}


struct TargaHeader2
{
	unsigned char  IDLength;
	unsigned char  ColormapType;
	unsigned char  ImageType;
	unsigned char  ColormapSpecification[5];
	unsigned short XOrigin;
	unsigned short YOrigin;
	unsigned short ImageWidth;
	unsigned short ImageHeight;
	unsigned char  PixelDepth;
	unsigned char  ImageDescriptor;
};

#define GET_ENDIAN_SWAPPED_16(x)  ((((x) & 0xff) << 8) | (( (x) & 0xff00) >> 8))

bool saveToTGA(int* data, hkOstream& s, int width, int height)
{
	// Header
	TargaHeader2 tga;
	hkString::memSet(&tga, 0, sizeof(tga));
	tga.ImageType  = 2; // raw

#if HK_ENDIAN_BIG
	tga.ImageHeight = (unsigned short)GET_ENDIAN_SWAPPED_16(height);
	tga.ImageWidth = (unsigned short)GET_ENDIAN_SWAPPED_16(width);
	for (int h=0; h< height;++h)
	{
		for (int w=0;w<width; ++w)
		{
			char* datac = (char*)( &data[h*width + w] );
			char r = datac[0];
			char g = datac[1];
			char b = datac[2];
			char a = datac[3];
			datac[0] = a;
			datac[1] = b;
			datac[2] = g;
			datac[3] = r;
		}
	}
#else
	tga.ImageHeight = (unsigned short)height;
	tga.ImageWidth = (unsigned short)width;
#endif

	tga.PixelDepth = (unsigned char)32;
	s.write((char*)&tga, sizeof(tga));

	s.write((char*)data, height * width * 4);
	return true;
}


static hkUint32 number_0[7] = 
{
	0x0000800,
	0x0008080,
	0x0080008,
	0x0080008,
	0x0080008,
	0x0008080,
	0x0000800,
};
static hkUint32 number_1[7] = 
{
	0x0008800,
	0x0080800,
	0x0000800,
	0x0000800,
	0x0000800,
	0x0000800,
	0x0088888,
};
static hkUint32 number_2[7] = 
{
	0x0008880,
	0x0080008,
	0x0000008,
	0x0000080,
	0x0000800,
	0x0008000,
	0x0088888,
};
static hkUint32 number_3[7] = 
{
	0x0088880,
	0x0000008,
	0x0000008,
	0x0000888,
	0x0000008,
	0x0000008,
	0x0088880,
};
static hkUint32 number_4[7] = 
{
	0x0000080,
	0x0000880,
	0x0008080,
	0x0008080,
	0x0088888,
	0x0000080,
	0x0000080,
};
static hkUint32 number_5[7] = 
{
	0x0008888,
	0x0008000,
	0x0008000,
	0x0008880,
	0x0000008,
	0x0000008,
	0x0008880,
};
static hkUint32 number_6[7] = 
{
	0x0000880,
	0x0008000,
	0x0080000,
	0x0080880,
	0x0088008,
	0x0080008,
	0x0008880,
};
static hkUint32 number_7[7] = 
{
	0x0088888,
	0x0000008,
	0x0000080,
	0x0000800,
	0x0008000,
	0x0008000,
	0x0008000,
};

static hkUint32 number_8[7] = 
{
	0x0008880,
	0x0080008,
	0x0080008,
	0x0008880,
	0x0080008,
	0x0080008,
	0x0008880,
};

static hkUint32 number_9[7] = 
{
	0x0008880,
	0x0080008,
	0x0080008,
	0x0008888,
	0x0000008,
	0x0000080,
	0x0088800,
};

static hkUint32* numbers[10] = { &number_0[0],&number_1[0],&number_2[0],&number_3[0],&number_4[0],&number_5[0],&number_6[0],&number_7[0],&number_8[0],&number_9[0]};


static void HK_CALL drawDigit( int nr, int currentY, int outputPixelWidth, int* texture )
{
	hkUint32* pattern = numbers[nr];
	for ( int x =0; x < 8; x++)
	{
		for (int y=0; y < 7; y++)
		{
			if ( (pattern[6-y]<<(4*x))&0xf0000000 )
			{
				texture[ x + y * outputPixelWidth] = 0xFF000000;
			}
		}
	}
}

static void HK_CALL drawNumber( int nr, int currentY, int outputPixelWidth, int* texture )
{
	int x = 0;
	for (int i=1000; i >= 1; i=i/10)
	{
		int digit = (nr/i)%10;
		drawDigit( digit, currentY, outputPixelWidth, texture + x);
		x += 7;
	}
}


static void HK_CALL drawStatistics(	hkMonitorStreamFrameInfo& info, 
									int frameIndex,
									hkArray<hkMonitorStreamAnalyzer::Node*>& nodes, 
									int* texture, 
									int height, 
									hkMonitorStreamAnalyzer::ColorTable& colorTable, 
									int pixelWidth, 
									int maxX, 
									hkReal frameTime, 
									hkReal absoluteFrameStartTimes,
									hkPointerMap<const char*, int>& unknownColorMap)
{
	hkPointerMap<const char*, int> colorMap;

	hkReal timeIncrement = 1000000 * frameTime / (hkReal)maxX;

	hkArray<hkMonitorStreamAnalyzer::Node*> timerNodesAtTicks;
	timerNodesAtTicks.reserveExactly( maxX );

	{
 		if (nodes[frameIndex]->m_children.getSize() > 0)
		{
			timerNodesAtTicks.clear();

			HK_ASSERT2(0x8f258165, info.m_absoluteTimeCounter != hkMonitorStreamFrameInfo::ABSOLUTE_TIME_NOT_TIMED, \
					"You cannot draw statistics unless one of your timers is absolute time");

			int absoluteTimeIndex = (info.m_absoluteTimeCounter == hkMonitorStreamFrameInfo::ABSOLUTE_TIME_TIMER_0) ? info.m_indexOfTimer0 : info.m_indexOfTimer1;
			
			outputStatsForFrame(nodes[frameIndex], absoluteFrameStartTimes, timeIncrement, timerNodesAtTicks, absoluteTimeIndex );
			
			int numSamplesThisFrame = timerNodesAtTicks.getSize();

			for (int j = 0; (j < numSamplesThisFrame) && (j < maxX); j++ )
			{
				int color = 0xffffffff; // White is "Unknown" timer

				if (timerNodesAtTicks[j] != HK_NULL )
				{
					// For this node - try to match the name of the deepest known timer against
					// the color table.  Use 2 maps to speed the process.
					hkMonitorStreamAnalyzer::Node* node = timerNodesAtTicks[j];
					while (node != HK_NULL)
					{
						const char* name = node->m_name;
						if ( colorMap.get(name, &color) == HK_FAILURE )
						{
							if (unknownColorMap.get(name, &color) != HK_FAILURE )
							{
								// If the color is unknown try the parent
								node = node->m_parent;
								continue;
							}

							// Color not in cached map yet - look it up in the table
							bool colorFound = false;
							for (int i = 0; i < colorTable.m_colorPairs.getSize(); ++i)
							{
								if ( hkString::strCasecmp(colorTable.m_colorPairs[i].m_colorName, name) == 0)
								{
									color = colorTable.m_colorPairs[i].m_color;
									colorFound = true;
									break;
								}
							}
							if ( colorFound )
							{
								colorMap.insert(name, color );
								break; // Found color
							}
							else
							{
								unknownColorMap.insert(name, color);
								node = node->m_parent;
							}
						}
						else
						{
							break; // Found color
						}
					}
				}

				for (int k = 0; k < height; k++)
				{
					texture[ k * pixelWidth + j ] = color;
				}
			}
		}
	}
}

static inline hkReal hkMin( hkReal a, hkReal b)
{
	return (a<b)?a:b;
}

void HK_CALL hkMonitorStreamAnalyzer::drawThreadsToTga( const hkMonitorStreamAnalyzer::ThreadDrawInput& input, hkOstream& outStream )
{
	hkInplaceArray<Node*, 6> nodeList;
	const int numThreads = input.m_streamCaptures.getSize();
	if (!numThreads)
	{
		return;
	}
	nodeList.setSize(numThreads);

	{
	    for (int i = 0; i < numThreads; ++i)
	    {
		    nodeList[i] = makeStatisticsTreeForMultipleFrames(	input.m_streamCaptures[i]->getStreamBegin(), 
															    input.m_streamCaptures[i]->m_frameInfos,
															    input.m_streamCaptures[i]->m_frameStartLocations,
															    false );
	    }
	}

	int frameEnd = input.m_frameStart + input.m_numFrames;
	int numFramesInStream = nodeList[0]->m_children.getSize();
	int numFrames = input.m_numFrames;
	if (frameEnd > numFramesInStream )
	{
		numFrames = numFramesInStream - input.m_frameStart;
		frameEnd  = numFramesInStream;
	}
	if ( numFrames <= 0)
	{
		return;
	}

	int pixelHeightPerThread = input.m_heightPerThread + input.m_gapBetweenThreads;
	int pixelHeightPerFrame  = pixelHeightPerThread * numThreads + input.m_gapBetweenFrames;
	int pixelHeight          = pixelHeightPerFrame * numFrames;

	// Allocate texture
	int numTotalPixels = pixelHeight * input.m_outputPixelWidth;

	int* texture = hkAllocate<int>(numTotalPixels, HK_MEMORY_CLASS_DEMO);
	hkString::memSet(texture, 0xff, numTotalPixels * 4);

	// Get first start time for the thread so that they are calibrated to 
	// each other. Can be very noticeable on Xbox360 for instance.
	hkArray<hkReal> startTimes;	startTimes.setSize(input.m_numFrames, 0.0f);
	{
		for (int j = input.m_frameStart; j < frameEnd; ++j)
		{
			int fzero = j -input.m_frameStart;
			startTimes[fzero] = 1e30f; // some large number
			for (int i = 0; i < numThreads; ++i )
			{
				hkArray<Node*>& threadIframeJ = nodeList[i]->m_children[j]->m_children;
				if (threadIframeJ.getSize() > 0)
				{
					startTimes[fzero] = hkMin( threadIframeJ[0]->m_absoluteStartTime, startTimes[fzero]);
				}
			}
		}
	}
	int currentY = pixelHeight;
	int startX   = 32;
	int maxX = input.m_outputPixelWidth-startX;
	//
	//	Draw 1 msec lines
	//
	{
		int pixelsPerMs = int(0.001f * maxX/input.m_frameTime);
		for (int x = startX; x < input.m_outputPixelWidth; x+= pixelsPerMs)
		for (int y=0; y < pixelHeight;y++)
		{
			texture[y * input.m_outputPixelWidth+x] = 0xffd0d0d0;
		}
	}
	hkPointerMap<const char*, int> unknownColorMap;
	{
		for (int f = input.m_frameStart; f < frameEnd; f++ )
		{
			currentY -= input.m_gapBetweenFrames;

			drawNumber( f, currentY, input.m_outputPixelWidth, &texture[ (currentY-pixelHeightPerThread) * input.m_outputPixelWidth] );

			for (int i = 0; i < numThreads; ++i )
			{
				currentY -= pixelHeightPerThread;

				int* output = &texture[currentY * input.m_outputPixelWidth+startX];
				drawStatistics( input.m_streamCaptures[i]->m_frameInfos[0], f, nodeList[i]->m_children, output, 
								input.m_heightPerThread, *input.m_colorTable, input.m_outputPixelWidth, maxX, input.m_frameTime,
								startTimes[f-input.m_frameStart], unknownColorMap );
			}
		}
	}
	HK_ASSERT( 0xf0212343, currentY == 0);

	if (input.m_warnAboutMissingTimers)
	{
		// Warn about unknown timers
		for (hkPointerMap<const char*, int>::Iterator itr = unknownColorMap.getIterator(); unknownColorMap.isValid(itr); itr = unknownColorMap.getNext( itr ) )
		{
			HK_WARN(0x94696eee, "Unknown timer when drawing monitor output: " << unknownColorMap.getKey(itr));
		}
	}

	saveToTGA( texture, outStream, input.m_outputPixelWidth, pixelHeight );
	hkDeallocate(texture);

	{
	    for (int i = 0; i < numThreads; ++i)
	    {
		    delete nodeList[i];
	    }
    }
}


/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
