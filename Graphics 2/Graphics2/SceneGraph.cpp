#include "SceneGraph.h"

bool SceneGraph::Initialise(void)
{
	for (SceneNodePointer snp : _children)
	{
		if (!snp->Initialise())
		{
			return false;
		}
	}
	return true;
}

void SceneGraph::Update(FXMMATRIX & currentWorldTransformation)
{
	for (SceneNodePointer snp : _children)
	{
		snp->Update(currentWorldTransformation);
	}
}

void SceneGraph::Render(void)
{
	for (SceneNodePointer snp : _children)
	{
		snp->Render();
	}
}

void SceneGraph::Shutdown(void)
{
	for (SceneNodePointer snp : _children)
	{
		snp->Shutdown();
	}
}

void SceneGraph::Add(SceneNodePointer node)
{
	_children.push_back(node);
}

void SceneGraph::Remove(SceneNodePointer node)
{
	for (SceneNodePointer snp : _children)
	{
		if (snp == node)
		{
			_children.remove(snp);
		}
	}
}

SceneNodePointer SceneGraph::Find(wstring name)
{
	for (SceneNodePointer snp : _children)
	{
		if (snp->Find(name) != nullptr)
		{
			return snp;
		}
	}
	return nullptr;
}
