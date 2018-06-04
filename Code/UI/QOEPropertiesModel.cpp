#include "QOEPropertiesModel.hpp"

#include "Icons.hpp"

#include "OE/Misc/Log.hpp"

QOEPropertiesModelItem::~QOEPropertiesModelItem()
{
	mutex.lock();
	QPixmapCache::remove(pixmapKey);
	for (auto i : childrens)
		delete i;
	mutex.unlock();
}

QOEPropertiesModel::QOEPropertiesModel(std::vector<void*>& objects, OrbitEngine::Meta::Type* objects_type, EditorInteraction* editorInteraction, QTreeView* view)
	: QAbstractItemModel(view), m_View(view)
{
	m_Root = new QOEPropertiesModelItem();

	connect(editorInteraction, SIGNAL(sync()), this, SLOT(sync()), Qt::ConnectionType::DirectConnection);

	for (void* object : objects) {
		QOEPropertiesModelItem* item = new QOEPropertiesModelItem();

		item->object = object;
		item->type = objects_type;
		item->parent = m_Root;

		m_Root->childrens.push_back(item);
	}
}

QOEPropertiesModel::~QOEPropertiesModel()
{
	delete m_Root;
}

void QOEPropertiesModel::sync()
{
	for (QOEPropertiesModelItem* item : m_Root->childrens) {
		sync(item, QModelIndex());
	}
}

void QOEPropertiesModel::sync(QOEPropertiesModelItem* item, const QModelIndex& index)
{
	{
		QMutexLocker locker(&item->mutex);

		if (!item->type)
			return;

		OrbitEngine::Meta::Kind kind = item->type->GetKind();

		if (item->parent != m_Root) {
			item->object = 0;

			OrbitEngine::Meta::Variant v = item->member->Get(item->parent->object);

			if (kind == OrbitEngine::Meta::Kind::CLASS) {
				item->object = v.GetPointer();
				item->value = OrbitEngine::Meta::Variant();
			}
			else {
				if (item->value != v) {
					item->value = v;
					item->value_changed = true;
					emit dataChanged(index, this->index(index.row(), 1, index.parent()), { Qt::DisplayRole, Qt::EditRole });
				}
			}
		}

		if (item->object == 0)
			return; // NULL param

		// Optimize the syncronization
		bool modification = false;
		if (item->childrens.size() != item->type->GetMembers().size()) {
			item->childrens.resize(item->type->GetMembers().size());
			modification = true;
		}

		int i = 0;
		for (OrbitEngine::Meta::Member* member : item->type->GetMembers()) {
			QOEPropertiesModelItem* child = nullptr;
			if (i < item->childrens.size())
				child = item->childrens[i];
			if (child == nullptr)
				item->childrens[i] = child = new QOEPropertiesModelItem();
			child->type = member->GetType();
			child->member = member;
			child->parent = item;

			i++;
		}

		if (modification) {
			QList<QPersistentModelIndex> a = { QPersistentModelIndex(index) };
			emit layoutChanged(a);
		}
	}

	int row = 0;
	for (QOEPropertiesModelItem* child : item->childrens) {
		sync(child, createIndex(row++, 0, item));
	}
}

QOEPropertiesModelItem* QOEPropertiesModel::getItemFromIndex(const QModelIndex& index) const
{
	if (index.isValid()) {
		QOEPropertiesModelItem* item = static_cast<QOEPropertiesModelItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return m_Root;
}

QModelIndex QOEPropertiesModel::index(int row, int column, const QModelIndex& parent) const
{
	if (row < 0 || column < 0)
		return QModelIndex();
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	QOEPropertiesModelItem* parentItem = getItemFromIndex(parent);

	if (row >= 0 && row < parentItem->childrens.size()) {
		QOEPropertiesModelItem* childItem = parentItem->childrens.at(row);

		if (childItem) {
			QModelIndex index = createIndex(row, column, childItem);
			return index;
		}
	}
	return QModelIndex();
}

QModelIndex QOEPropertiesModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	QOEPropertiesModelItem* childItem = getItemFromIndex(index);
	QOEPropertiesModelItem* parentItem = childItem->parent;

	if (parentItem == m_Root)
		return QModelIndex();

	return createIndex(std::find(parentItem->childrens.begin(), parentItem->childrens.end(), childItem) - parentItem->childrens.begin(), 0, parentItem);
}

int QOEPropertiesModel::rowCount(const QModelIndex& parent) const
{
	QOEPropertiesModelItem* parentItem = getItemFromIndex(parent);
	return int(parentItem->childrens.size());
}

int QOEPropertiesModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}

QVariant QOEPropertiesModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	QOEPropertiesModelItem* item = getItemFromIndex(index);


	QMutexLocker locker(&item->mutex);

	int column = index.column();

	switch (role) {
	case Qt::TextAlignmentRole:
		return QVariant(Qt::AlignVCenter | Qt::AlignLeft);
	case Qt::DisplayRole:
		if (column == 0) {
			return QString::fromStdString(item->member ? item->member->GetName() : item->type->GetName());
		}
		else { // column = 1
			return QString::fromStdString(item->type->GetName());
		}
	case Qt::EditRole:
		return "INVALID";
	case Qt::DecorationRole:
	{
		std::string icon = "";
		if (column == 0) {
			if (!item->member)
				icon = "cube";
		}
		else { // column = 1
			if (item->object)
				icon = "gear";
		}
		if (icon.size() > 0)
			return Icons::GetIcon(icon);
		else
			return QVariant();
	}
	default:
		return QVariant();
	}
}

Qt::ItemFlags QOEPropertiesModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);

	if (index.isValid()) {
		QOEPropertiesModelItem* item = getItemFromIndex(index);

		if (index.column() == 1 && item->member && !item->object) {
			flags |= Qt::ItemIsEditable;
		}
	}
	return flags;
}