#ifndef qoecomponentinspector_hpp
#define qoecomponentinspector_hpp

#include <QWidget>
#include <qlineedit.h>

#include "OE/Engine/SceneObject.hpp"
#include "OE/Misc/Property.hpp"

#include "QOEPropertiesModel.hpp"

namespace Ui {
	class QOEComponentInspector;
};

class QOEComponentInspector : public QWidget {
    Q_OBJECT

public:
	QOEComponentInspector(OrbitEngine::Engine::SceneObject* sceneObject, QWidget *parent = 0);
    virtual ~QOEComponentInspector();

	OrbitEngine::Engine::SceneObject* getSceneObject();

private:
	QScopedPointer<Ui::QOEComponentInspector> ui;
	OrbitEngine::Engine::SceneObject* m_SceneObject;
};

#endif