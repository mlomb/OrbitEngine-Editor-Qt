#include "QOEComponentInspector.hpp"
#include "ui_qoecomponentinspector.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QTreeView>
#include <QTime>

#include "Icons.hpp"
#include "QOEPropertiesTreeView.hpp"

#include <OE/Math/Vec4.hpp>

#include "OE/Engine/Component.hpp"

QOEComponentInspector::QOEComponentInspector(OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> sceneObject, EditorInteraction* editorInteraction, QWidget *parent)
	: QWidget(parent), ui(new Ui::QOEComponentInspector), m_SceneObject(sceneObject)
{
	ui->setupUi(this);

	OrbitEngine::Meta::Type* t = OrbitEngine::Meta::NativeTypeResolver<OrbitEngine::Engine::SceneObject>::Get();
	auto name_member = t->GetMember("m_Name");

	//ui->layout_Name->addWidget(new QOEPropertyString(&m_SceneObject->m_Name, this));
	
	std::vector<void*> components;
	if (true) {
		components.push_back(sceneObject.Get());
		components.push_back(sceneObject.Get());
		components.push_back(sceneObject.Get());
	}
	else {
		OrbitEngine::Meta::ReflectionDatabase* rd = editorInteraction->GetEngineDomain()->GetReflectionDatabase();
		auto v3 = rd->GetMonoType("Plugin.CustomComponent");
		auto mt = v3->GetMonoType();

		auto cls = mono_type_get_class(mt);

		MonoClassField* mf = mono_class_get_field_from_name(cls, "b");

		MonoObject* mo = mono_object_new(mono_domain_get_by_id(0), cls);
		mono_runtime_object_init(mo);
		int val = 42;
		mono_field_set_value(mo, mf, &val);
		components.push_back(mo);
		components.push_back(mo);
		components.push_back(mo);
		components.push_back(mo);
		components.push_back(mo);
		t = v3;
	}
	/*
	auto& comps = m_SceneObject->getComponents();
	components.reserve(comps.size());
	for (auto& comp : comps)
		components.push_back(comp);
	*/
	/*
	if (sceneObject->m_Name.operator std::string() == "Child5") {
		for (int i = 0; i < 50; i++) {
			components.push_back(comps[0]);
		}
	}
	*/

	ui->scrollArea->setWidget(new QOEPropertiesTreeView(components, t, editorInteraction, ui->scrollArea));
}

QOEComponentInspector::~QOEComponentInspector()
{

}

OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> QOEComponentInspector::GetSceneObject()
{
	return m_SceneObject;
}