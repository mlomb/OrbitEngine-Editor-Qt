#include "QOEPropertiesModel.hpp"

#include <MetaCPP/Runtime.hpp>
#include <MetaCPP/Type.hpp>

#include "Icons.hpp"

PropertiesModelNode::PropertiesModelNode(OrbitEngine::Misc::OEObject* object, QAbstractItemModel* model, PropertiesModelNode* parent)
	: m_Model(model), m_Parent(parent), m_Object(object), m_Field(0), m_Property(0)
{
	initObject();
}

PropertiesModelNode::PropertiesModelNode(const metacpp::Field* field, OrbitEngine::Misc::PropertyBase* prop, QAbstractItemModel* model, PropertiesModelNode* parent)
	: m_Model(model), m_Parent(parent), m_Object(0), m_Field(field), m_Property(prop)
{
	metacpp::Storage* metaStorage = metacpp::Runtime::GetStorage();

	m_PropertyTypeID = metaStorage->getType(field->getType()->getTypeID())->getTemplateArguments()[0]->getTypeID();

	if (metaStorage->isDerived(m_PropertyTypeID, metacpp::TypeInfo<OrbitEngine::Misc::OEObject>::ID)) {
		m_Object = static_cast<OrbitEngine::Misc::OEObject*>(prop->getPtr());
		initObject();
	}
	else {
		m_Loaded = true;

		m_CallbackPtr = prop->onModification.AddListener([&](OrbitEngine::Application::Event e)-> void {
			_value_expired = true;
			if(m_Index)
				emit m_Model->dataChanged(*m_Index, *m_Index);
		});
	}

	/*
	// Necessary?
	if (false) {}
	else if (m_PropertyTypeID == metacpp::TypeInfo<std::string>::ID)
		m_PropertyIcon = "text";
	else if (m_PropertyTypeID == metacpp::TypeInfo<int>::ID)
		m_PropertyIcon = "integer";
	else if (m_PropertyTypeID == metacpp::TypeInfo<float>::ID)
		m_PropertyIcon = "decimal";
	*/

	// Generate field name
	std::string name = field->getQualifiedName().getName();

	if (name.size() > 0) {
		if (name.size() >= 2) {
			if (name[1] == '_')
				name = name.substr(2);
		}

		name[0] = toupper(name[0]);
	}

	m_FieldName = QString::fromStdString(name);
}

PropertiesModelNode::~PropertiesModelNode() {
	if (m_Property)
		m_Property->onModification.RemoveListener(m_CallbackPtr);
	if (m_Index)
		delete m_Index;
	for (PropertiesModelNode* child : m_Childs)
		delete child;
	m_Childs.clear();
}

void PropertiesModelNode::populate() {
	m_Loaded = true;

	if (m_Object == nullptr)
		return;

	metacpp::Storage* metaStorage = metacpp::Runtime::GetStorage();

	for (const metacpp::Field* f : metaStorage->getAllFields(m_Object->getType())) {
		metacpp::Type* fieldType = metaStorage->getType(f->getType()->getTypeID());

		if (fieldType->getQualifiedName().getName() == "Property") {
			OrbitEngine::Misc::PropertyBase* prop = f->get<OrbitEngine::Misc::PropertyBase>(m_Object);

			addChild(new PropertiesModelNode(f, prop, m_Model, this));
		}
	}
}

QVariant PropertiesModelNode::data(int column, int role) const {
	switch (role) {
	case Qt::TextAlignmentRole:
		return Qt::AlignVCenter | Qt::AlignLeft;
	case Qt::EditRole:
	case Qt::DisplayRole:
	{
		if (column == 0) {
			if (!m_Property)
				return m_ObjectName;
			else
				return m_FieldName;
		}
		else { // column == 1
			if (m_Object) {
				if (m_Property)
					return m_ObjectName;
				else
					return "*should span*";
			}

			return "Unsupported type";
		}
	}
	case Qt::DecorationRole:
	{
		std::string iconName;
		if (m_Object) {
			if ((!m_Property && column == 0) ||
				(m_Property && column == 1))
				iconName = "gear";
		}
		else if (column == 0 && m_Property)
			iconName = m_PropertyIcon;
		else if (column == 1)
			iconName = "cross";

		if (iconName.size() > 0)
			return Icons::GetIcon(iconName);
		return QVariant();
	}
	default:
		return QVariant();
	}
}

void PropertiesModelNode::initObject() {
	if (m_Object == nullptr) // root
		return;

	metacpp::Storage* metaStorage = metacpp::Runtime::GetStorage();

	m_ObjectName = QString::fromStdString(m_Object->getType()->getQualifiedName().getName());
}

PropertiesModelNode* PropertiesModelNode::getParent() {
	return m_Parent;
}

PropertiesModelNode* PropertiesModelNode::childAt(int row) const {
	if (row >= 0 && row < m_Childs.size())
		return m_Childs[row];
	return 0;
}

int PropertiesModelNode::childCount() const {
	return m_Childs.size();
}

void PropertiesModelNode::addChild(PropertiesModelNode* node) {
	m_Childs.push_back(node);
}

bool PropertiesModelNode::isPopulated() const {
	return m_Loaded;
}

bool PropertiesModelNode::shouldSpan() const {
	return m_Object && !m_Property;
}

void PropertiesModelNode::setIndex(QPersistentModelIndex* persistentIndex) {
	m_Index = persistentIndex;
}

QPersistentModelIndex* PropertiesModelNode::getIndex() const {
	return m_Index;
}

metacpp::TypeID PropertiesModelNode::getTypeID() const {
	return m_PropertyTypeID;
}

OrbitEngine::Misc::PropertyBase* PropertiesModelNode::getProperty() const {
	return m_Property;
}

QOEPropertiesModel::QOEPropertiesModel(std::vector<OrbitEngine::Misc::OEObject*>& objects, QObject* parent)
	: QAbstractItemModel(parent), m_Objects(objects), m_Root(0), m_View(static_cast<QTreeView*>(parent))
{
	qRegisterMetaType<QVector<int>>("QVector<int>");

	buildRoot();
}

QOEPropertiesModel::~QOEPropertiesModel()
{
	delete m_Root;
}

void QOEPropertiesModel::buildRoot()
{
	metacpp::Storage* metaStorage = metacpp::Runtime::GetStorage();

	m_Root = new PropertiesModelNode(nullptr, this);

	for (OrbitEngine::Misc::OEObject* object : m_Objects)
		m_Root->addChild(new PropertiesModelNode(object, this, m_Root));
}

PropertiesModelNode* QOEPropertiesModel::getItemFromIndex(const QModelIndex& index) const
{
	if (index.isValid()) {
		PropertiesModelNode* item = static_cast<PropertiesModelNode*>(index.internalPointer());
		if (item)
			return item;
	}
	return m_Root;
}

QModelIndex QOEPropertiesModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	PropertiesModelNode* parentItem = getItemFromIndex(parent);
	PropertiesModelNode* childItem = parentItem->childAt(row);

	if (childItem) {
		QModelIndex index = createIndex(row, column, childItem);
		if (column == 1 && childItem->getIndex() == 0) {
			childItem->setIndex(new QPersistentModelIndex(index));

			// ugly
			bool shouldSpan = childItem->shouldSpan();
			if (m_View->isFirstColumnSpanned(index.row(), index.parent()) != shouldSpan)
				m_View->setFirstColumnSpanned(index.row(), index.parent(), shouldSpan);
		}

		return index;
	}
	else
		return QModelIndex();
}

QModelIndex QOEPropertiesModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	PropertiesModelNode* childItem = getItemFromIndex(index);
	PropertiesModelNode* parentItem = childItem->getParent();

	if (parentItem == m_Root)
		return QModelIndex();

	return createIndex(parentItem->childCount(), 0, parentItem);
}

int QOEPropertiesModel::rowCount(const QModelIndex& parent) const
{
	PropertiesModelNode* parentItem = getItemFromIndex(parent);
	return parentItem->isPopulated() ? parentItem->childCount() : 0;
}

int QOEPropertiesModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

QSize QOEPropertiesModel::span(const QModelIndex& index) const
{
	PropertiesModelNode* item = getItemFromIndex(index);
	return index.column() == 0 ? QSize(item->shouldSpan() ? 2 : 1, 1) : QAbstractItemModel::span(index);
}

Qt::ItemFlags QOEPropertiesModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return 0;

	Qt::ItemFlags flags = QAbstractItemModel::flags(index);

	if (index.column() == 1) {
		flags |= Qt::ItemIsEditable;
		flags |= Qt::ItemIsEnabled;
	}

	return flags;
}

QVariant QOEPropertiesModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	PropertiesModelNode* item = getItemFromIndex(index);
	return item->data(index.column(), role);
}

bool QOEPropertiesModel::hasChildren(const QModelIndex& parent) const
{
	PropertiesModelNode* parentItem = getItemFromIndex(parent);
	if (!parentItem->isPopulated())
		return true;

	return QAbstractItemModel::hasChildren(parent);
}

void QOEPropertiesModel::fetchMore(const QModelIndex& parent)
{
	PropertiesModelNode* parentItem = getItemFromIndex(parent);
	parentItem->populate();
	/*
	beginInsertRows(parent, 0, parentItem->childs.size() - 1);
	insertRows(0, parentItem->childs.size() - 1, parent);
	endInsertRows();
	*/
}

bool QOEPropertiesModel::canFetchMore(const QModelIndex& parent) const
{
	PropertiesModelNode* parentItem = getItemFromIndex(parent);
	return !parentItem->isPopulated();
}
