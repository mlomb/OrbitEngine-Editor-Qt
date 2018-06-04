#ifndef qoepropertiesmodel_hpp
#define qoepropertiesmodel_hpp

#include <QAbstractItemModel>
#include <QTreeView>
#include <QMutex>
#include <QPixmapCache>

#include "OE/Meta/Type.hpp"

#include "EditorInteraction.hpp"

struct QOEPropertiesModelItem {
public:
	~QOEPropertiesModelItem();

	// sync
	QMutex mutex;

	// render
	QPixmapCache::Key pixmapKey;
	bool last_hover;
	bool value_changed;

	// meta
	OrbitEngine::Meta::Type* type = 0;
	OrbitEngine::Meta::Member* member = 0;

	// data
	void* object = 0;
	OrbitEngine::Meta::Variant value;

	// hierarchy
	QOEPropertiesModelItem* parent = 0;
	std::vector<QOEPropertiesModelItem*> childrens;
};

class QOEPropertiesModel : public QAbstractItemModel {
	Q_OBJECT
public:
	QOEPropertiesModel(std::vector<void*>& objects, OrbitEngine::Meta::Type* objects_type, EditorInteraction* editorInteraction, QTreeView* view);
	virtual ~QOEPropertiesModel();

public slots:
	void sync();

public:
	QOEPropertiesModelItem* getItemFromIndex(const QModelIndex& index) const;

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
	void sync(QOEPropertiesModelItem* item, const QModelIndex& index);

	QTreeView* m_View;
	QOEPropertiesModelItem* m_Root;
};

#endif