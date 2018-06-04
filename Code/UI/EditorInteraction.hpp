#ifndef EDITOR_INTERACTION_HPP
#define EDITOR_INTERACTION_HPP

#include <QObject>

#include "OE/Engine/SceneObject.hpp"
#include "OE/Engine/EngineDomain.hpp"
#include "OE/Misc/Log.hpp"

class EditorInteraction : public QObject {
	Q_OBJECT

signals:
	void sync();

public slots:
	void parent(OrbitEngine::Engine::SceneObject* child, OrbitEngine::Engine::SceneObject* parent, int position = 9999999);
	void rename(OrbitEngine::Engine::SceneObject* obj, const QString new_name);
	void setProperty(OrbitEngine::Meta::Member* member, void* object, OrbitEngine::Meta::Variant value);

public:
	EditorInteraction(OrbitEngine::Engine::EngineDomain* engineDomain);

	void syncEngine();

	OrbitEngine::Engine::EngineDomain* GetEngineDomain();

private:
	long long m_LastSync;
	bool m_ForceSync;
	OrbitEngine::Engine::EngineDomain* m_EngineDomain;
};

#endif