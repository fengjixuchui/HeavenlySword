

#include "debugshadergraph_ps3.h"
#include "core/fileio_ps3.h"

void CDebugShaderGraph::CreateGraph(const char* shaderAttribute)
{
	InitGraphDefinitions();

	m_graph = NT_NEW_CHUNK(m_memoryChunk) GraphNode(m_graphDef[0].m_len);

	const char* cgDir = "content_ps3/cg/rsx/";
	unsigned int len = strlen(shaderAttribute);

	FileEnumerator enumerator( cgDir, DEFAULT_MEDIA );
	while ( enumerator.AdvanceToNextFile() )
	{
		if ( enumerator.IsDirectory() )
		{
			continue;
		}

		const char *filename = enumerator.GetCurrentFilename();
		if ( strncmp( filename, shaderAttribute, len ) == 0 )
		{
			AddToGraph( filename + len, filename );
		}
	}

	int stringLen = sizeof(m_debugShaderAttribute)< len + 1 ? sizeof(m_debugShaderAttribute) : len + 1;
	NT_MEMCPY(m_debugShaderAttribute, shaderAttribute, stringLen);

	//CommandBaseInput<const int&>* pReloadShaders = CommandManager::Get().CreateCommand("Speedtree", this, &CSpeedTreeShading::CommandReloadShaders, "Reload speedtree shaders");
	//KeyBindManager::Get().RegisterKey("rendering", pReloadShaders, "Reload Speedtree shaders", int(), KEYS_PRESSED, KEYC_R, KEYM_ALT);
}

void CDebugShaderGraph::DestroyGraphLevel(IGraphNode* node, unsigned int level)
{
	if (level < m_graphDef.size())
	{
		unsigned int numEntities = m_graphDef[level].m_len;
		for (unsigned int elem = 0; elem < numEntities; ++ elem)
		{
			IGraphNode*	childNode = node -> GetChildNode(elem);
			if (childNode)
			{
				DestroyGraphLevel(childNode, level + 1);
			}
		}
	}

	NT_DELETE_CHUNK(m_memoryChunk, node);
}

CDebugShaderGraph::~CDebugShaderGraph()
{
	IGraphNode*	node = m_graph;

	DestroyGraphLevel(node, 0);
}

void CDebugShaderGraph::OnShaderNotFound(const unsigned int* graphLevels) const
{
	int numGraphLevels = (int)m_graphDef.size();

	ntPrintf("CDebugShaderGraph : Shader %s", m_debugShaderAttribute);

	for (int level = 0; level < numGraphLevels; ++ level)
	{
		int graphPart = graphLevels[level];
		if (0 == graphPart)
		{
			ntPrintf("[%s]", m_graphDef[level].m_defs[0]);
		}
		else
		{
			if (graphPart < (int)m_graphDef[level].m_len)
			{
				ntPrintf("%s", m_graphDef[level].m_defs[graphPart]);
			}
		}
	}

	ntPrintf(".sho not found. \n");
}
