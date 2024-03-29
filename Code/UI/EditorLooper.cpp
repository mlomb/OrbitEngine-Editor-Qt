#include "EditorLooper.hpp"

#include <QTimer>
#include <QThread>

#include "OE/Graphics/2D/Renderer2D.hpp"
#include "OE/Graphics/3D/Renderer3D.hpp"

#include "OE/Graphics/2D/BatchRenderer.hpp"
#include "OE/Graphics/Font.hpp"

#include "OE/Engine/TestComponent.hpp"

namespace OrbitEngine { namespace Engine {
	EditorLooper::EditorLooper()
	{
	}

	EditorLooper::~EditorLooper()
	{
	}

	void EditorLooper::init(Application::Context* context)
	{
		m_EngineDomain = new EngineDomain();
		m_EditorInteraction = new EditorInteraction(m_EngineDomain);
		
		m_Renderer2D = new Graphics::BatchRenderer();

		emit initialized();
	}

	void t(WeakPtr<Engine::SceneObject> so, Graphics::Renderer2D* r2d, Math::Vec2f& off) {
		//Graphics::BatchRenderer* brd = static_cast<Graphics::BatchRenderer*>(r2d);

		Graphics::Font::GetDefaultFont()->prepareFont(25);
		Graphics::Font::GetDefaultFont()->drawString(so->GetName(), Math::Vec2f(100, 100) + off, *r2d);

		off.x += 25;
		for (int i = 0; i < so->GetChildCount(); i++) {
			WeakPtr<Engine::SceneObject> c = so->GetChild(i);
			off.y += 25;
			t(c, r2d, off);
		}
		off.x -= 25;
	}

	void EditorLooper::render()
	{
		m_Renderer2D->begin();

		Math::Vec2f d;
		t(m_EngineDomain->GetActiveScene()->GetRoot(), m_Renderer2D, d);

		m_Renderer2D->end();
	}
	void EditorLooper::update(float delta)
	{
		WeakPtr<Engine::SceneObject> root = m_EngineDomain->GetActiveScene()->GetRoot();
		WeakPtr<Engine::SceneObject> k = root->GetChild(1);
		static int l = 0;
		l++;
		k->SetName("Passed " + std::to_string(l) + " frames");
		if (l % 60 == 0) {
			WeakPtr<Engine::SceneObject> c = k->AddChildren();
			if (c)
				c->SetName("Added " + std::to_string(l));
		}

		auto sceneObjects = m_EngineDomain->GetMemoryDomain()->GetAll<SceneObject>();
		auto trans = m_EngineDomain->GetMemoryDomain()->GetAll<Transform>();
		auto test_ob = m_EngineDomain->GetMemoryDomain()->GetAll<TestComponent>();
		auto comps = m_EngineDomain->GetMemoryDomain()->GetAll<Component>();

		int i = 0;
		for (WeakPtr<SceneObject> so : sceneObjects) {
			so->SetName("Object #" + std::to_string(i++));
		}

		// Engine UI Sync
		m_EditorInteraction->syncEngine();
	}

	void EditorLooper::deinitialize()
	{
	}

	EditorInteraction* EditorLooper::GetEditorInteraction()
	{
		return m_EditorInteraction;
	}

} }