#ifndef qoescenehierarchymodel_hpp
#define qoescenehierarchymodel_hpp

#include <OE/Engine/SceneObject.hpp>

#include <QAbstractItemModel>

class QOESceneHierarchyModel : public QAbstractItemModel {
    Q_OBJECT

public:
	QOESceneHierarchyModel(OrbitEngine::Engine::SceneObject* root);
    virtual ~QOESceneHierarchyModel();
	
private:
	OrbitEngine::Engine::SceneObject* m_Root;

	// Helpers
	void fillInternalCallbacks(OrbitEngine::Engine::SceneObject* obj);
	OrbitEngine::Engine::SceneObject* getItemFromIndex(const QModelIndex& index) const;

	// Qt Model
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

#endif