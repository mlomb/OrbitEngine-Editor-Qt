#include "EditorInteraction.hpp"

#include <QCoreApplication>

#include "OE/System/System.hpp"

EditorInteraction::EditorInteraction(OrbitEngine::Engine::EngineDomain* engineDomain)
	: m_EngineDomain(engineDomain), m_LastSync(0)
{
}

void EditorInteraction::parent(OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> child, OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> parent, int position) {
	OE_LOG_DEBUG("PARENT " << child->GetName() << " TO " << parent->GetName());
	child->SetParent(parent, position);
	m_ForceSync = true;
}

void EditorInteraction::rename(OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> obj, const QString new_name) {
	OE_LOG_DEBUG("RENAME " << obj->GetName() << " TO " << new_name.toStdString());
	obj->SetName(new_name.toStdString());
}

void EditorInteraction::setProperty(OrbitEngine::Meta::Member* member, void* object, OrbitEngine::Meta::Variant value)
{
	OE_LOG_DEBUG("SET PROPERTY " << member->GetName() << " TO " << value.GetString());
	member->Set(value, object);
}

void EditorInteraction::syncEngine()
{
	// We are between frames, no data should be modified
	// Use this time to sync the engine data with the UI
	// This is NOT the Qt's main thread, so no direct UI updates

	QCoreApplication::processEvents();

	long long current = OrbitEngine::System::System::Instance()->currentNano();

	if (m_ForceSync || current - m_LastSync > 1000000000 / 10) { // 10 updates per second
		emit sync();
		m_LastSync = current;
		m_ForceSync = false;
	}

	QCoreApplication::processEvents();
}

OrbitEngine::Engine::EngineDomain* EditorInteraction::GetEngineDomain()
{
	return m_EngineDomain;
}
