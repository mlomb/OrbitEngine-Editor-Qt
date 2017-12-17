#ifndef sceneview_hpp
#define sceneview_hpp

#include <OE/Application/Loopeable.hpp>
#include <OE/Engine/Scene.hpp>
#include <OE/Graphics/3D/Renderer3D.hpp>
#include <OE/Misc/Camera.hpp>

class SceneView : public OrbitEngine::Application::Loopeable {
public:
	SceneView(OrbitEngine::Engine::Scene* scene);

	void init() override;
	void render() override;
	void update(float delta) override;
	void deinitialize() override;

private:
	OrbitEngine::Engine::Scene* m_Scene;
	OrbitEngine::Graphics::Renderer3D* m_Renderer;
	OrbitEngine::Misc::Camera* m_Camera;
};

#endif