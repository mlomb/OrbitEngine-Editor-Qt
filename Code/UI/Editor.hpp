#ifndef editor_hpp
#define editor_hpp

#include <QMainWindow>
#include <QTreeView>
#include <QEventLoop>

#include <OE/Engine/Scene.hpp>
#include <OE/Engine/EngineDomain.hpp>

#include "EditorLooper.hpp"
#include "EditorInteraction.hpp"
#include "QOESceneHierarchyModel.hpp"

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
	void looperInitialized();
	void sceneObjectSelection(const QItemSelection& selected, const QItemSelection& deselected);

private:
	QScopedPointer<Ui::Editor> ui;

	OrbitEngine::Engine::EngineDomain* m_EngineDomain;
	OrbitEngine::Engine::EditorLooper* m_EditorLooper;
	QOESceneHierarchyModel* m_HierarchyModel;
};

#endif