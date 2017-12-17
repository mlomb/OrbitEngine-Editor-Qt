#ifndef qoepropertiesmodel_hpp
#define qoepropertiesmodel_hpp

#include <OE/Misc/OEObject.hpp>
#include <OE/Misc/Property.hpp>

#include <MetaCPP/TypeID.hpp>
#include <MetaCPP/Field.hpp>

#include <QAbstractItemModel>
#include <QTreeView>

class PropertiesModelNode {
public:
	PropertiesModelNode(OrbitEngine::Misc::OEObject* object, QAbstractItemModel* model, PropertiesModelNode* parent = 0);
	PropertiesModelNode(const metacpp::Field* field, OrbitEngine::Misc::PropertyBase* prop, QAbstractItemModel* model, PropertiesModelNode* parent = 0);

	~PropertiesModelNode();

	PropertiesModelNode* getParent();
	PropertiesModelNode* childAt(int row) const;

	int childCount() const;
	void addChild(PropertiesModelNode* node);
	void setIndex(QPersistentModelIndex* persistentIndex);

	bool isPopulated() const;
	bool shouldSpan() const;
	void populate();
	QPersistentModelIndex* getIndex() const;
	metacpp::TypeID getTypeID() const;
	OrbitEngine::Misc::PropertyBase* getProperty() const;

	QVariant data(int column, int role) const;

	// internal use
	bool _value_expired = true;
	bool _hover_last = false;

private:
	void initObject();

private:
	PropertiesModelNode* m_Parent;
	std::vector<PropertiesModelNode*> m_Childs;
	QPersistentModelIndex* m_Index = 0;
	QAbstractItemModel* m_Model;

	OrbitEngine::Misc::OEObject* m_Object;
	const metacpp::Field* m_Field;
	OrbitEngine::Misc::PropertyBase* m_Property;
	metacpp::TypeID m_PropertyTypeID; // Property<int> => typeidof(int)

	OrbitEngine::Application::EventHandler<>::EventCallbackPtr m_CallbackPtr;

	bool m_Loaded = false;
	QString m_ObjectName;
	QString m_FieldName;
	std::string m_PropertyIcon;
};

class QOEPropertiesModel : public QAbstractItemModel {
    Q_OBJECT

public:
	QOEPropertiesModel(std::vector<OrbitEngine::Misc::OEObject*>& objects, QObject* parent = Q_NULLPTR);
	virtual ~QOEPropertiesModel();
	
	// Qt Model
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QSize span(const QModelIndex &index) const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
	void fetchMore(const QModelIndex &parent) override;
	bool canFetchMore(const QModelIndex &parent) const override;
private:
	std::vector<OrbitEngine::Misc::OEObject*> m_Objects;
	PropertiesModelNode* m_Root;
	QTreeView* m_View;

	void buildRoot();
	PropertiesModelNode* getItemFromIndex(const QModelIndex& index) const;
};

#endif