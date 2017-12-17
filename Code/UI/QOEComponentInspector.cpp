#include "QOEComponentInspector.hpp"
#include "ui_qoecomponentinspector.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QTreeView>
#include <QTime>

#include "QOEPropertyItemDelegate.hpp"
#include "QOEProperty.hpp"
#include "Icons.hpp"
#include "QOEPropertiesTreeView.hpp"

#include <OE/Math/Vec4.hpp>

QOEComponentInspector::QOEComponentInspector(OrbitEngine::Engine::SceneObject* sceneObject, QWidget *parent)
	: QWidget(parent), ui(new Ui::QOEComponentInspector), m_SceneObject(sceneObject)
{
	ui->setupUi(this);

	ui->layout_Name->addWidget(new QOEPropertyString(&m_SceneObject->m_Name, this));

	std::vector<OrbitEngine::Misc::OEObject*> components;
	auto& comps = m_SceneObject->getComponents();
	components.reserve(comps.size());
	for (auto& comp : comps)
		components.push_back(comp);

	if (sceneObject->m_Name.operator std::string() == "Child5") {
		for (int i = 0; i < 50; i++) {
			components.push_back(comps[1]);
		}
	}

	ui->scrollArea->setWidget(new QOEPropertiesTreeView(components, ui->scrollArea));
}

QOEComponentInspector::~QOEComponentInspector()
{

}

OrbitEngine::Engine::SceneObject* QOEComponentInspector::getSceneObject()
{
	return m_SceneObject;
}