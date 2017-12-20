#ifndef editor_hpp
#define editor_hpp

#include <QMainWindow>
#include <QTreeView>

#include "OE/Engine/Scene.hpp"
#include "SceneView.hpp"

#include <OE/Engine/SceneRunner.hpp>

namespace Ui
{
    class Editor;
}

class Editor : public QMainWindow {
    Q_OBJECT

public:
	Editor(QWidget *parent = 0);
    virtual ~Editor();

private slots:
	void sceneObjectSelection(const QItemSelection& selected, const QItemSelection& deselected);

private:
	QScopedPointer<Ui::Editor> ui;
	OrbitEngine::Engine::Scene* m_Scene;
	OrbitEngine::Engine::SceneRunner* m_SceneRunner;
};

#endif