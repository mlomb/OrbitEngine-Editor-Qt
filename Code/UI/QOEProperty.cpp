#include "QOEProperty.hpp"

#include <qthread.h>
#include <QtConcurrent/qtconcurrentthreadengine.h>
#include <qmetaobject.h>

#include <MetaCPP/Runtime.hpp>
#include <MetaCPP/Field.hpp>
#include <MetaCPP/TypeInfo.hpp>

#include <OE/Misc/Property.hpp>
#include <OE/Engine/Transform.hpp>

QOEProperty::QOEProperty(OrbitEngine::Misc::PropertyBase* prop, QWidget* parent)
	: QWidget(parent), m_Property(prop)
{
}

void QOEProperty::init()
{
	QHBoxLayout* layout = new QHBoxLayout();

	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	buildLayout(layout);
	loadProperty();

	this->setLayout(layout);
}

//////////////////////////
// String
//////////////////////////
QOEPropertyString::QOEPropertyString(OrbitEngine::Misc::Property<std::string>* prop, QWidget* parent)
	: QOEProperty(prop, parent)
{
	init();
}

void QOEPropertyString::buildLayout(QLayout* layout)
{
	m_LineEdit = new QLineEdit(this);
	
	bindWidget(m_LineEdit, &QLineEdit::textEdited);

	layout->addWidget(m_LineEdit);
}

void QOEPropertyString::loadProperty()
{
	std::string text = *static_cast<OrbitEngine::Misc::Property<std::string>*>(m_Property);
	m_LineEdit->deselect();
	QMetaObject::invokeMethod(m_LineEdit, "setText", Q_ARG(QString, QString::fromStdString(text)));
}

void QOEPropertyString::storeProperty()
{
	auto prop = static_cast<OrbitEngine::Misc::Property<std::string>*>(m_Property);
	*prop = m_LineEdit->text().toStdString();
}

//////////////////////////
// Boolean
//////////////////////////
QOEPropertyBoolean::QOEPropertyBoolean(OrbitEngine::Misc::Property<bool>* prop, QWidget* parent)
	: QOEProperty(prop, parent)
{
	init();
}

void QOEPropertyBoolean::buildLayout(QLayout* layout)
{
	m_Checkbox = new QCheckBox(this);

	bindWidget(m_Checkbox, &QCheckBox::stateChanged);

	layout->addWidget(m_Checkbox);
}

void QOEPropertyBoolean::loadProperty()
{
	bool b = *static_cast<OrbitEngine::Misc::Property<bool>*>(m_Property);
	QMetaObject::invokeMethod(m_Checkbox, "setChecked", Q_ARG(bool, b));
}

void QOEPropertyBoolean::storeProperty()
{
	auto prop = static_cast<OrbitEngine::Misc::Property<bool>*>(m_Property);
	*prop = m_Checkbox->checkState() == Qt::CheckState::Checked;
}
