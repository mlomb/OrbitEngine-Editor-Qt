#include "QOESceneHierarchyModel.hpp"

#include <chrono>
#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <qpixmapcache.h>
#include <qtimer.h>
#include <qmetaobject.h>
#include <QMimeData>

#include <iostream>
#include <thread>
#include <set>

#include "Icons.hpp"

#include "OE/Engine/MeshRenderer.hpp"

const QString SceneObjectMIME = "application/SceneObject";

QOESceneHierarchyModel::QOESceneHierarchyModel(EditorInteraction* editorInteraction, QTreeView* view)
	: m_EditorInteraction(editorInteraction), m_View(view)
{
	qRegisterMetaType<QVector<int>>();
	qRegisterMetaType<QList<QPersistentModelIndex>>();
	qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>();

	m_Root = new QOESceneHierarchyItem();

	connect(m_EditorInteraction, SIGNAL(sync()), this, SLOT(sync()), Qt::ConnectionType::DirectConnection);
}

QOESceneHierarchyModel::~QOESceneHierarchyModel()
{
}

void QOESceneHierarchyModel::sync() {
	m_Root->ref = m_EditorInteraction->GetEngineDomain()->GetActiveScene()->GetRoot();
	sync(m_Root, QModelIndex());
}

void QOESceneHierarchyModel::sync(QOESceneHierarchyItem* item, const QModelIndex& index)
{
	if (!item)
		return;

	{
		QMutexLocker locker(&item->mutex);

		if (item->text != item->ref->GetName()) {
			item->text = item->ref->GetName();
			emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
			//QMetaObject::invokeMethod(this, "dataChanged", Qt::QueuedConnection, Q_ARG(const QModelIndex&, index), Q_ARG(const QModelIndex&, index), Q_ARG(const QVector<int>&, { Qt::DisplayRole }));
		}

		// Optimize the syncronization
		bool modification = false;
		if (item->childrens.size() != item->ref->GetChildCount()) {
			item->childrens.resize(item->ref->GetChildCount());
			modification = true;
		}

		for (int i = 0; i < item->ref->GetChildCount(); i++) {
			OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> child = item->ref->GetChild(i);
			QOESceneHierarchyItem* sel = item->childrens[i];
			if (!sel) {
				// TODO FIX MEMORY LAKE
				sel = item->childrens[i] = new QOESceneHierarchyItem();
				sel->icon = "cube";
				sel->ref = child;
				sel->parent = item;
			}
			else {
				if (sel->ref != child) {
					sel->ref = child;
				}
			}
		}

		if (modification) {
			QList<QPersistentModelIndex> a = { QPersistentModelIndex(index) };
			emit layoutChanged(a);
		}
	}

	int row = 0;
	for (QOESceneHierarchyItem* child : item->childrens) {
		sync(child, this->index(row++, 0, index));
	}
}

QOESceneHierarchyItem* QOESceneHierarchyModel::getItemFromIndex(const QModelIndex& index) const
{
	if (index.isValid()) {
		QOESceneHierarchyItem* item = static_cast<QOESceneHierarchyItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return m_Root;
}

QModelIndex QOESceneHierarchyModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	QOESceneHierarchyItem* parentItem = getItemFromIndex(parent);

	if (row >= 0 && row < parentItem->childrens.size()) {
		QOESceneHierarchyItem* childItem = parentItem->childrens[row];

		if (childItem) {
			QModelIndex index = createIndex(row, column, childItem);
			return index;
		}
	}
	return QModelIndex();
}

QModelIndex QOESceneHierarchyModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	QOESceneHierarchyItem* childItem = getItemFromIndex(index);
	QOESceneHierarchyItem* parentItem = childItem->parent;

	if (parentItem == m_Root)
		return QModelIndex();

	return createIndex(int(parentItem->childrens.size()), 0, parentItem);
}

int QOESceneHierarchyModel::rowCount(const QModelIndex& parent) const
{
	QOESceneHierarchyItem* parentItem = getItemFromIndex(parent);
	return int(parentItem->childrens.size());
}

int QOESceneHierarchyModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

Qt::DropActions QOESceneHierarchyModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

QMimeData* QOESceneHierarchyModel::mimeData(const QModelIndexList& indexes) const
{
	QVector<QOESceneHierarchyItem*>* items = new QVector<QOESceneHierarchyItem*>();
	for (auto i : indexes)
		(*items) << getItemFromIndex(i);

	QByteArray encodedData(QString::number((quintptr)items).toUtf8());

	QMimeData* md = new QMimeData();
	md->setData(SceneObjectMIME, encodedData);
	return md;
}

QStringList QOESceneHierarchyModel::mimeTypes() const
{
	QStringList list;
	list << SceneObjectMIME;
	return list;
}

Qt::ItemFlags QOESceneHierarchyModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);

	if (index.isValid())
		return flags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
	else
		return flags | Qt::ItemIsDropEnabled;
}

QVariant QOESceneHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

	QOESceneHierarchyItem* item = getItemFromIndex(index);

	QMutexLocker locker(&item->mutex);

	switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
		return QString::fromStdString(item->text);
	case Qt::DecorationRole:
	{
		return Icons::GetIcon(item->icon);
	}
	default:
		return QVariant();
	}

}

bool QOESceneHierarchyModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
	if (action == Qt::DropAction::MoveAction) {
		QOESceneHierarchyItem* parent_item = getItemFromIndex(parent);

		QByteArray encodedData = data->data(SceneObjectMIME);
		QVector<QOESceneHierarchyItem*>* items = (QVector<QOESceneHierarchyItem*>*)encodedData.toULongLong();
		if (!items) return false;
		
		for (QOESceneHierarchyItem* child_item : (*items)) {
			QMetaObject::invokeMethod(m_EditorInteraction, "parent", Qt::QueuedConnection, Q_ARG(OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject>, child_item->ref), Q_ARG(OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject>, parent_item->ref), Q_ARG(int, row));
		}

		delete items;
		return false;
	}
	return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

bool QOESceneHierarchyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (index.isValid()) {
		QOESceneHierarchyItem* item = getItemFromIndex(index);
		OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> obj = item->ref;

		if (role == Qt::EditRole) {
			QMetaObject::invokeMethod(m_EditorInteraction, "rename", Qt::QueuedConnection, Q_ARG(OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject>, obj), Q_ARG(const QString, value.toString()));
			item->text = value.toString().toStdString();
			return true;
		}
	}
	return false;
}
