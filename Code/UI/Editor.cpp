#include "Editor.hpp"
#include "ui_editor.h"

#include <iostream>
#include <map>
#include <fstream>
#include <QTimer>
#include <QFile>

#include "QOESceneHierarchyModel.hpp"
#include "QOEComponentInspector.hpp"

Editor::Editor(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::Editor)
{
	ui->setupUi(this);

	/* Scene Load Procedure */
	m_Scene = new OrbitEngine::Engine::Scene();
	m_SceneView = new SceneView(m_Scene);
	/* --- */

	ui->dock_inspector->setMinimumSize(QSize(350, ui->dock_inspector->minimumHeight()));

	ui->sceneHierarchy->setUniformRowHeights(true);
	ui->sceneHierarchy->header()->close();
	ui->sceneHierarchy->setModel(new QOESceneHierarchyModel(m_Scene->getRoot()));

	ui->mainView->setLoopeable(m_SceneView);

	connect(ui->sceneHierarchy->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(sceneObjectSelection(const QItemSelection&, const QItemSelection&)));
}

void Editor::sceneObjectSelection(const QItemSelection& selected, const QItemSelection& deselected) {
	if (selected.indexes().isEmpty())
		return;

	OrbitEngine::Engine::SceneObject* currentSceneObject = 0;
	auto currentWidget = ui->dock_inspector->widget();

	if (currentWidget) {
		auto currentComponentInspector = dynamic_cast<QOEComponentInspector*>(currentWidget);
		if (currentComponentInspector)
			currentSceneObject = currentComponentInspector->getSceneObject();
	}

	void* ptr = selected.indexes().first().internalPointer();
	if (!ptr) {
		// Nothing should be shown
		if (currentWidget)
			delete currentWidget;
		return;
	}

	OrbitEngine::Engine::SceneObject* sceneObject = static_cast<OrbitEngine::Engine::SceneObject*>(ptr);

	if (currentSceneObject == sceneObject)
		return;

	delete currentWidget;

	ui->dock_inspector->setWidget(new QOEComponentInspector(sceneObject));
}

Editor::~Editor()
{

}