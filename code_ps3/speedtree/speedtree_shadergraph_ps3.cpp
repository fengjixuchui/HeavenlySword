#include <cell/cell_fs.h>
#include "speedtree_shadergraph_ps3.h"

void CSpeedTreeShaderGraph::CreateGraph(const char* shaderAttribute)
{
	InitGraphDefinitions();

	m_graph = NT_NEW GraphNode(m_graphDef[0].m_len);

	const char* cgDir = "/app_home/content_ps3/cg/rsx";
	int dirDesc = 0;
	CellFsErrno	ret = cellFsOpendir(cgDir, &dirDesc);
	if (CELL_FS_SUCCEEDED != ret)
	{
		return;
	}


	uint64_t		size;
	CellFsDirent	dirEntry;

	while (CELL_FS_SUCCEEDED == cellFsReaddir(dirDesc, &dirEntry, &size) && size != 0)
	{
		if (CELL_FS_TYPE_REGULAR == dirEntry.d_type)
		{
			unsigned int len = strlen(shaderAttribute);
			if (strncmp(dirEntry.d_name, shaderAttribute, len) == 0)
			{
				AddToGraph(dirEntry.d_name + len, dirEntry.d_name);
			}
		}
	}  

	//CommandBaseInput<const int&>* pReloadShaders = CommandManager::Get().CreateCommand("Speedtree", this, &CSpeedTreeShading::CommandReloadShaders, "Reload speedtree shaders");
	//KeyBindManager::Get().RegisterKey("rendering", pReloadShaders, "Reload Speedtree shaders", int(), KEYS_PRESSED, KEYC_R, KEYM_ALT);
}

void CSpeedTreeShaderGraph::DestroyGraphLevel(IGraphNode* node, unsigned int level)
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

	NT_DELETE(node);
}

CSpeedTreeShaderGraph::~CSpeedTreeShaderGraph()
{
	IGraphNode*	node = m_graph;

	DestroyGraphLevel(node, 0);
}
