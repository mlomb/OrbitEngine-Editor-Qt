#ifndef qoepropertyitemdelegate_hpp
#define qoepropertyitemdelegate_hpp

#include <QAbstractItemDelegate>
#include <QStyledItemDelegate>
#include <QItemDelegate>

#include "QOEPropertiesModel.hpp"

class QOEPropertyItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	QOEPropertyItemDelegate(EditorInteraction* editorInteraction, QObject* parent = 0);
	~QOEPropertyItemDelegate();

	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
	QRect updateRect(const QRect& rect) const;

private:
	EditorInteraction* m_EditorInteraction;

	QWidget* createEditorWidget(const OrbitEngine::Meta::Kind kind, QWidget* parent = 0) const;
};

#endif