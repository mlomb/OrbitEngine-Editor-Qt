#ifndef EDITOR_LOOPER_HPP
#define EDITOR_LOOPER_HPP

#include <QObject>

#include "EditorInteraction.hpp"

#include "OE/Engine/EngineDomain.hpp"
#include "OE/Engine/Scene.hpp"
#include "OE/Engine/SceneObject.hpp"
#include "OE/Application/Loopeable.hpp"

#include "OE/Graphics/2D/Renderer2D.hpp"
#include "OE/Graphics/3D/Renderer3D.hpp"

namespace OrbitEngine { namespace Engine {

	class EditorLooper : public QObject, public Application::Loopeable {
		Q_OBJECT
	public:
		EditorLooper();
		virtual ~EditorLooper();

		void init(OrbitEngine::Application::Context* context) override;
		void render() override;
		void update(float delta) override;
		void deinitialize() override;

		EditorInteraction* GetEditorInteraction();

	signals:
		void initialized();

	protected:

	private:
		EngineDomain* m_EngineDomain;
		EditorInteraction* m_EditorInteraction;

		Graphics::Renderer2D* m_Renderer2D;
		Graphics::Renderer3D* m_Renderer3D;
	};
} }

#endif