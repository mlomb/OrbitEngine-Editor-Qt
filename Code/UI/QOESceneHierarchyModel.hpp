#ifndef qoescenehierarchymodel_hpp
#define qoescenehierarchymodel_hpp

#include <QAbstractItemModel>
#include <QTreeView>
#include <QMutex>

#include <OE/Engine/SceneObject.hpp>

#include "EditorInteraction.hpp"

struct QOESceneHierarchyItem {
	QMutex mutex;

	OrbitEngine::WeakPtr<OrbitEngine::Engine::SceneObject> ref;
	QOESceneHierarchyItem* parent;
	std::string icon;
	std::string text; // dyn
	std::vector<QOESceneHierarchyItem*> childrens; // dyn
	QOESceneHierarchyItem* childAt(unsigned int row) {
		if (row < 0 || row >= childrens.size())
			return 0;
		return childrens.at(row);
	}
};

class QOESceneHierarchyModel : public QAbstractItemModel {
    Q_OBJECT

public slots:
	void sync();

public:
	QOESceneHierarchyModel(EditorInteraction* editorInteraction, QTreeView* view);
    virtual ~QOESceneHierarchyModel();

	void sync(QOESceneHierarchyItem* item, const QModelIndex& index);

	// Helpers
	QOESceneHierarchyItem* getItemFromIndex(const QModelIndex& index) const;

private:
	EditorInteraction* m_EditorInteraction;
	QTreeView* m_View;
	QOESceneHierarchyItem* m_Root;

	// Qt Model
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	Qt::DropActions supportedDropActions() const override;
	QMimeData* mimeData(const QModelIndexList &indexes) const override;
	QStringList mimeTypes() const override;

	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex & parent)  override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};

#endif