#include "QOEPropertiesTreeView.hpp"

#include <QHeaderView>

#include "QOEPropertyItemDelegate.hpp"

QOEPropertiesTreeView::QOEPropertiesTreeView(std::vector<OrbitEngine::Misc::OEObject*>& objects, QWidget *parent)
	: QTreeView(parent)
{
	setStyleSheet("QTreeView::item { margin: 3px 0; padding: 2px 0; } QTreeView::item:hover { background: transparent; } QTreeView::item { border: 0; outline: 0; }");
	header()->hide();
	setIndentation(15);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
	setEditTriggers(QAbstractItemView::EditTrigger::AllEditTriggers);
	setUniformRowHeights(true);

	m_Model = new QOEPropertiesModel(objects, this);
	
	setItemDelegateForColumn(1, new QOEPropertyItemDelegate(this));
	setModel(m_Model);

	setColumnWidth(0, 150);

	m_Model->fetchMore(QModelIndex());

	for (int row = 0; row < m_Model->rowCount(); row++) {
		setFirstColumnSpanned(row, QModelIndex(), true);
		setExpanded(m_Model->index(row, 0), true);
	}
}

QOEPropertiesTreeView::~QOEPropertiesTreeView()
{
}
