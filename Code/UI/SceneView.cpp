#include "SceneView.hpp"

#include <iostream>

#include <OE/Graphics/3D/ForwardRenderer.hpp>
#include <OE/Graphics/3D/DeferredRenderer.hpp>
#include <OE/Graphics/2D/BatchRenderer.hpp>
#include <OE/Graphics/Font.hpp>
#include <OE/Misc/EulerCamera.hpp>
#include <OE/Graphics/MeshGenerator.hpp>
#include <OE/Graphics/3D/SurfaceShader.hpp>

SceneView::SceneView(OrbitEngine::Engine::Scene* scene)
	: m_Scene(scene)
{
}

static OrbitEngine::Graphics::BatchRenderer* br;
static float globalY;
static OrbitEngine::Graphics::Mesh* sphereMesh;
static OrbitEngine::Graphics::Material* mat;

void renderSceneHierarchyRecurvise(OrbitEngine::Engine::SceneObject* obj, float indent) {
	using namespace OrbitEngine;

	Graphics::Font* defFont = Graphics::Font::GetDefaultFont();
	defFont->drawString(obj->getName(), Math::Vec2f(indent, globalY), *br);

	globalY += 20;

	std::vector<OrbitEngine::Engine::SceneObject*> childs = obj->getChildrens();

	for (auto child : childs)
		renderSceneHierarchyRecurvise(child, indent + 20);
}

void SceneView::init()
{
	using namespace OrbitEngine;

	m_Camera = new Misc::EulerCamera();

	m_Renderer = new Graphics::ForwardRenderer();
	m_Renderer->useCamera(m_Camera);

	br = new Graphics::BatchRenderer(30000);


	sphereMesh = Graphics::MeshGenerator::SphereStandard();
	mat = new Graphics::Material(Graphics::SurfaceShader::Standard());
	Math::Color c1(0.0f, 0.0f, 1.0f, 1.0f);
	mat->setBaseColor(c1);
	mat->setMetallic(0.04f);
}

void SceneView::render()
{
	using namespace OrbitEngine;
	
	m_Renderer->begin();

	m_Renderer->submitMesh(sphereMesh, sphereMesh->getIBO()->getSize(), mat);

	m_Renderer->end();

	br->begin();

	Graphics::Font* defFont = Graphics::Font::GetDefaultFont();
	defFont->prepareFont(20.0f);


	globalY = 30;
	renderSceneHierarchyRecurvise(m_Scene->getRoot(), 30);
	
	
	
	br->end();
}

void SceneView::update(float delta)
{
	m_Camera->update(delta);
}

void SceneView::deinitialize()
{
	delete br;
	delete m_Renderer;
	delete m_Camera;
}
