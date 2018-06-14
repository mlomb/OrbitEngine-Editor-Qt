#ifndef qoecomponentinspector_hpp
#define qoecomponentinspector_hpp

#include <QWidget>
#include <qlineedit.h>

#include "EditorInteraction.hpp"

#include "OE/Engine/SceneObject.hpp"

namespace Ui {
	class QOEComponentInspector;
};

class QOEComponentInspector : public QWidget {
    Q_OBJECT

public:
	QOEComponentInspector(OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> sceneObject, EditorInteraction* editorInteraction, QWidget *parent = 0);
    virtual ~QOEComponentInspector();

	OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> GetSceneObject();

private:
	QScopedPointer<Ui::QOEComponentInspector> ui;
	OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> m_SceneObject;
};

#endif