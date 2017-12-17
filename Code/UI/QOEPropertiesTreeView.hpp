#ifndef qoepropertiestreeview_hpp
#define qoepropertiestreeview_hpp

#include <vector>

#include <QTreeView>

#include <OE/Misc/OEObject.hpp>

#include "QOEPropertiesModel.hpp"

class QOEPropertiesTreeView : public QTreeView {
    Q_OBJECT

public:
	QOEPropertiesTreeView(std::vector<OrbitEngine::Misc::OEObject*>& objects, QWidget *parent = 0);
	virtual ~QOEPropertiesTreeView();

private:
	QOEPropertiesModel* m_Model;
};

#endif