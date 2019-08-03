#ifndef SHADERGRAPH_PS3_H
#define SHADERGRAPH_PS3_H

#include "gfx/debugshader_ps3.h"

#define DECLARE_CONSTANT(name, shader) static unsigned int name = (shader) -> GetConstantIndex(#name);
#define DECLARE_VS_CONSTANT(name) DECLARE_CONSTANT(name, VS)
#define DECLARE_PS_CONSTANT(name) DECLARE_CONSTANT(name, PS)

#define BEGIN_GRAPH_LEVEL { static const char* level[] = {
#define END_GRAPH_LEVEL ,NULL}; m_graphDef.push_back(GraphLevelDefinition()); m_graphDef.back().m_defs = level; m_graphDef.back().m_len = sizeof(level) / sizeof(char*); } 


class CSpeedTreeShaderGraph
{
	// graph nodes -----------------------------
	struct IGraphNode
	{
		virtual IGraphNode*		GetChildNode(unsigned int node) = 0;
		virtual void			SetChildNode(unsigned int node, IGraphNode* newNode) = 0;
		virtual DebugShader*	GetData() = 0;
		virtual ~IGraphNode()
		{
		}
	};

	class GraphNode : public IGraphNode
	{
		typedef ntstd::Vector<IGraphNode*>	NodeList;

	public:
		GraphNode(unsigned int numChildren) : m_list(numChildren, NULL)
		{
		}

		virtual IGraphNode* GetChildNode(unsigned int node)
		{
			return m_list[node];
		}

		virtual void SetChildNode(unsigned int node, IGraphNode* newNode)
		{
			m_list[node] = newNode;
		}

		virtual DebugShader*	GetData()
		{
			return NULL;
		}

		NodeList	m_list;
	};

	class LeafGraphNode : public IGraphNode
	{
	public:
		LeafGraphNode(const char* filename)
		{
			m_shader.SetCGFile(filename);
		}

		virtual IGraphNode* GetChildNode(unsigned int node)
		{
			return NULL;
		}
		virtual void SetChildNode(unsigned int, IGraphNode*)
		{
			ntAssert(0);
		}

		virtual DebugShader* GetData()
		{
			return &m_shader;
		}

		DebugShader	m_shader;
	};

	class ConnectingGraphNode : public IGraphNode
	{
		virtual IGraphNode* GetChildNode(unsigned int node)
		{
			return NULL;
		}

		virtual DebugShader* GetData()
		{
			if (m_targetNode)
			{
				return m_targetNode -> GetData();
			}

			return NULL;
		}

		virtual void SetChildNode(unsigned int, IGraphNode*)
		{
			ntAssert(0);
		}


	private:
		IGraphNode*		m_targetNode;
	};

	// ---------------------------------------------

protected:
	struct GraphLevelDefinition
	{
		unsigned int	m_len;
		const char**	m_defs;
	};

	typedef ntstd::Vector<GraphLevelDefinition> GraphDefinition;


public:
	template <int Levels>
	struct ShaderDescriptionBase
	{
		enum { numLevels = Levels };

		unsigned int m_levels[Levels];
	};

private:

	void DestroyGraphLevel(IGraphNode* node, unsigned int level);

	void AddToGraph(const char* fileName, const char* fullName)
	{
		const unsigned int numLevels = m_graphDef.size();

		IGraphNode* currNode = m_graph;

		for (unsigned int level = 0; level < numLevels; ++ level)
		{
			unsigned int namePart = 0;
			const char** nameParts = m_graphDef[level].m_defs;
			while(nameParts[namePart])
			{
				if (strncmp(fileName, nameParts[namePart], strlen(nameParts[namePart])) == 0)
				{
					fileName += strlen(nameParts[namePart]);
					break;
				}
				++ namePart;
			}

			if (!nameParts[namePart])
			{
				// use default
				namePart = 0;

			}
			//if (!currNode.m_list[namePart])
			if (!currNode -> GetChildNode(namePart))
			{
				if (level == numLevels - 1)
				{
					currNode -> SetChildNode(namePart, NT_NEW LeafGraphNode(fullName));
				}
				else
				{
					currNode -> SetChildNode(namePart, NT_NEW GraphNode(m_graphDef[level + 1].m_len)); 												
				}
			}

			currNode = currNode -> GetChildNode(namePart);


		}

	}

public:
	virtual ~CSpeedTreeShaderGraph();

	void CreateGraph(const char* shaderAttribute);

	template <class ShaderDescription>
	DebugShader* GetShader(const ShaderDescription& desc)
	{
		ntAssert(ShaderDescription::numLevels == m_graphDef.size());

		IGraphNode* node = m_graph;
		for (unsigned int level = 0; level < m_graphDef.size(); ++ level)
		{
			node = node -> GetChildNode(desc.m_levels[level]);
			if (!node)
			{
				ntAssert(!"SPEEDTREE : Shader not found\n");
				return NULL;
			}
		}

		DebugShader* shader = node -> GetData();
		ntAssert(shader);
		return shader;
	}



protected:
	//COMMAND_RESULT CommandReloadShaders(const int&)
	//{
	//	return CR_SUCCESS;
	//}

	virtual void InitGraphDefinitions() = 0;

	IGraphNode*	m_graph;
	GraphDefinition	m_graphDef;

};

class CSpeedTreeShaderBase
{
protected:
	CSpeedTreeShaderBase() : m_shader(NULL)
	{
	}

public:
	Shader*			operator->() const
	{
		ntAssert(m_shader);
		return m_shader;
	}

	Shader*			GetShader()	const
	{
		ntAssert(m_shader);
		return m_shader;
	}

protected:
	DebugShader*	m_shader;
};
		

#endif
