#include "Graphics2.h"

Graphics2 app;

void Graphics2::CreateSceneGraph()
{
	SceneGraphPointer sceneGraph = GetSceneGraph();
	//SceneNodePointer cube = make_shared<CubeNode>(L"cube1");
	//sceneGraph->Add(cube);
	/*SceneNodePointer body = make_shared<CubeNode>(L"body", L"Concrete.png");
	SceneNodePointer lLeg = make_shared<CubeNode>(L"leftLeg", L"Woodbox.bmp");
	SceneNodePointer rLeg = make_shared<CubeNode>(L"rightLeg", L"Woodbox.bmp");
	SceneNodePointer head = make_shared<CubeNode>(L"head", L"Bricks.png");
	SceneNodePointer lArm = make_shared<CubeNode>(L"leftArm", L"Bricks.png");
	SceneNodePointer rArm = make_shared<CubeNode>(L"rightArm", L"Bricks.png");*/
	

	/*sceneGraph->Add(body);
	sceneGraph->Add(lLeg);
	sceneGraph->Add(rLeg);
	sceneGraph->Add(head);
	sceneGraph->Add(lArm);
	sceneGraph->Add(rArm);*/

	shared_ptr<TerrainNode> terrain = make_shared<TerrainNode>(L"Terrain1", L"rollinghills.bmp");
	shared_ptr<MeshNode> node = make_shared<MeshNode>(L"Plane1", L"airplane.x");
	sceneGraph->Add(terrain);
	sceneGraph->Add(node);

}

void Graphics2::UpdateSceneGraph()
{
	SceneGraphPointer sceneGraph = GetSceneGraph();

	// This is where you make any changes to the local world transformations to nodes
	// in the scene graph

	sceneGraph->Find(L"Plane1")->SetWorldTransform(/*XMMatrixTranslation(20.0f, 0.0f, 0.0f) **/ XMMatrixRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), _rotation * XM_PI / 180.0f));
	_rotation += 1.0f;
}
