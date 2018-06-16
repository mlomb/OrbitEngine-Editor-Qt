#include "Editor.hpp"
#include "ui_editor.h"

#include <iostream>
#include <map>
#include <fstream>
#include <QTimer>
#include <QFile>

#include "QOESceneHierarchyModel.hpp"
#include "QOEComponentInspector.hpp"
#include "EditorLooper.hpp"

Q_DECLARE_METATYPE(OrbitEngine::Engine::SceneObject*);
Q_DECLARE_METATYPE(OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject>);
Q_DECLARE_METATYPE(OrbitEngine::Meta::Member*);
Q_DECLARE_METATYPE(OrbitEngine::Meta::Variant);

Editor::Editor(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::Editor)
{
	qRegisterMetaType<OrbitEngine::Engine::SceneObject*>("OrbitEngine::Engine::SceneObject*");
	qRegisterMetaType<OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject>>("OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject>");
	qRegisterMetaType<OrbitEngine::Meta::Member*>("OrbitEngine::Meta::Member*");
	qRegisterMetaType<OrbitEngine::Meta::Variant>("OrbitEngine::Meta::Variant");
	 
	ui->setupUi(this);

	ui->dock_inspector->setMinimumSize(QSize(350, ui->dock_inspector->minimumHeight()));

	ui->sceneHierarchy->setUniformRowHeights(true);
	ui->sceneHierarchy->setDragEnabled(true);
	ui->sceneHierarchy->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
	ui->sceneHierarchy->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	ui->sceneHierarchy->setDropIndicatorShown(true);
	ui->sceneHierarchy->header()->close();

	m_EditorLooper = new OrbitEngine::Engine::EditorLooper();
	ui->mainView->setLoopeable(m_EditorLooper);

	connect(m_EditorLooper, SIGNAL(initialized()), this, SLOT(looperInitialized()));
}

void Editor::looperInitialized() {
	m_HierarchyModel = new QOESceneHierarchyModel(m_EditorLooper->GetEditorInteraction(), ui->sceneHierarchy);
	ui->sceneHierarchy->setModel(m_HierarchyModel);
	connect(ui->sceneHierarchy->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(sceneObjectSelection(const QItemSelection&, const QItemSelection&)));
}

void Editor::sceneObjectSelection(const QItemSelection& selected, const QItemSelection& deselected) {
	if (selected.indexes().isEmpty())
		return;

	OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> currentSceneObject = OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject>();
	auto currentWidget = ui->dock_inspector->widget();

	if (currentWidget) {
		auto currentComponentInspector = dynamic_cast<QOEComponentInspector*>(currentWidget);
		if (currentComponentInspector)
			currentSceneObject = currentComponentInspector->GetSceneObject();
	}

	QOESceneHierarchyItem* i = m_HierarchyModel->getItemFromIndex(selected.indexes().first());
	if (i == nullptr || i->parent == nullptr) {
		// Nothing should be shown
		if (currentWidget)
			delete currentWidget;
		return;
	}

	OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> sceneObject = i->ref;

	if (currentSceneObject == sceneObject)
		return;

	delete currentWidget;

	ui->dock_inspector->setWidget(new QOEComponentInspector(sceneObject, m_EditorLooper->GetEditorInteraction(), this));
}

Editor::~Editor()
{

}