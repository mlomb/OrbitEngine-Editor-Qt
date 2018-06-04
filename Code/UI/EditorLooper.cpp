#include "EditorLooper.hpp"

#include <QTimer>
#include <QThread>

#include "OE/Graphics/2D/BatchRenderer.hpp"
#include "OE/Graphics/Font.hpp"

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

	void t(Engine::SceneObject* so, Graphics::Renderer2D* r2d, Math::Vec2f& off) {
		//Graphics::BatchRenderer* brd = static_cast<Graphics::BatchRenderer*>(r2d);

		Graphics::Font::GetDefaultFont()->prepareFont(25);
		Graphics::Font::GetDefaultFont()->drawString(so->getName(), Math::Vec2f(100, 100) + off, *r2d);

		off.x += 25;
		for (Engine::SceneObject* c : so->getChildrens()) {
			off.y += 25;
			t(c, r2d, off);
		}
		off.x -= 25;
	}

	void EditorLooper::render()
	{
		static int i = 0;
		i++;
		Engine::SceneObject* root = m_EngineDomain->GetActiveScene()->GetRoot();
		Engine::SceneObject* k = root->getChildrens().at(1);
		k->setName("Passed " + std::to_string(i) + " frames");
		if (i % 60 == 0) {
			k->addChildren("Added " + std::to_string(i));
		}

		m_Renderer2D->begin();

		Math::Vec2f d;
		t(m_EngineDomain->GetActiveScene()->GetRoot(), m_Renderer2D, d);

		m_Renderer2D->end();
	}
	void EditorLooper::update(float delta)
	{


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