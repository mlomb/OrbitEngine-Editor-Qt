#ifndef qoepropertiestreeview_hpp
#define qoepropertiestreeview_hpp

#include <vector>

#include <QTreeView>

#include <OE/Meta/Type.hpp>

#include "QOEPropertiesModel.hpp"
#include "EditorInteraction.hpp"

class QOEPropertiesTreeView : public QTreeView {
    Q_OBJECT

public:
	QOEPropertiesTreeView(std::vector<void*>& objects, OrbitEngine::Meta::Type* objects_type, EditorInteraction* editorInteraction, QWidget *parent = 0);
	virtual ~QOEPropertiesTreeView();

private:
	QOEPropertiesModel* m_Model;
};

#endif