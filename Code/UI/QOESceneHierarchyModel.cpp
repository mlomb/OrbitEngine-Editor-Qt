#include "QOESceneHierarchyModel.hpp"

#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <qpixmapcache.h>

#include <iostream>
#include <thread>

#include "Icons.hpp"

#include "OE/Engine/MeshRenderer.hpp"

QOESceneHierarchyModel::QOESceneHierarchyModel(OrbitEngine::Engine::SceneObject* root)
	: m_Root(root)
{
	m_Root->internal_EditorIndex = new QPersistentModelIndex(QModelIndex());
	fillInternalCallbacks(m_Root);
	
	// TEST
	new std::thread([&]() {
		int i = 4;
		while (true) {
#define ttt 500
#if _WIN32
			Sleep(ttt);
#else
			usleep(ttt * 1000);
#endif
			OrbitEngine::Engine::SceneObject* toAdd = i % 2 ? m_Root : m_Root->childAt(1);
			OrbitEngine::Engine::SceneObject* added = toAdd->addChildren("Child" + std::to_string(i));
			if (added) {
				added->addComponent<OrbitEngine::Engine::MeshRenderer>();
				//std::cout << "Added " << i << "\n";
				i++;

				m_Root->childAt(1)->m_Name = "just added " + std::to_string(i);
			}
		}
	});
}

QOESceneHierarchyModel::~QOESceneHierarchyModel()
{
}

void QOESceneHierarchyModel::fillInternalCallbacks(OrbitEngine::Engine::SceneObject* obj)
{
	obj->internal_EditorBeginInsert = [&](int position, OrbitEngine::Engine::SceneObject* parent) {
		QPersistentModelIndex* pindex = static_cast<QPersistentModelIndex*>(parent->internal_EditorIndex);
		if (pindex)
			beginInsertRows(*pindex, position, position);
		else
			Q_ASSERT(false);
	};
	obj->internal_EditorEndInsert = std::bind(&QOESceneHierarchyModel::endInsertRows, this);
	
	for (OrbitEngine::Engine::SceneObject* child : obj->getChildrens())
		fillInternalCallbacks(child);
}

OrbitEngine::Engine::SceneObject* QOESceneHierarchyModel::getItemFromIndex(const QModelIndex& index) const
{
	if (index.isValid()) {
		OrbitEngine::Engine::SceneObject* item = static_cast<OrbitEngine::Engine::SceneObject*>(index.internalPointer());
		if (item)
			return item;
	}
	return m_Root;
}

QModelIndex QOESceneHierarchyModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return QModelIndex();

	OrbitEngine::Engine::SceneObject* parentItem = getItemFromIndex(parent);
	OrbitEngine::Engine::SceneObject* childItem = parentItem->childAt(row);

	if (childItem) {
		QModelIndex index = createIndex(row, column, childItem);
		// Is this a memory leak?
		if (childItem->internal_EditorIndex == 0)
			childItem->internal_EditorIndex = new QPersistentModelIndex(index);
		return index;
	}
	else
		return QModelIndex();
}

QModelIndex QOESceneHierarchyModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	OrbitEngine::Engine::SceneObject* childItem = getItemFromIndex(index);
	OrbitEngine::Engine::SceneObject* parentItem = childItem->getParent();

	if (parentItem == m_Root)
		return QModelIndex();

	return createIndex(parentItem->childCount(), 0, parentItem);
}

int QOESceneHierarchyModel::rowCount(const QModelIndex& parent) const
{
	OrbitEngine::Engine::SceneObject* parentItem = getItemFromIndex(parent);
	return parentItem->childCount();
}

int QOESceneHierarchyModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

QVariant QOESceneHierarchyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

	OrbitEngine::Engine::SceneObject* item = getItemFromIndex(index);

	switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
		return QString::fromStdString(item->getName());
	case Qt::DecorationRole:
	{
		std::string iconName = "cube";
		return Icons::GetIcon(iconName);
	}
	default:
		return QVariant();
	}

}
