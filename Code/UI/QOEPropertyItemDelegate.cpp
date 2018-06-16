#include "QOEPropertyItemDelegate.hpp"

#include <QSpinBox>
#include <QPainter>
#include <QStylePainter>
#include <QStyle>
#include <QApplication>
#include <QPixmapCache>

#include <QCheckBox>
#include <QLineEdit>
#include "QNumeric.hpp"

#include <OE/Math/Vec4.hpp>

#include <OE/Engine/Transform.hpp>
#include <OE/Math/Vec2.hpp>
#include <OE/Math/Vec3.hpp>
#include <OE/Math/Vec4.hpp>

static QWidget* s_Placeholders[50];

QOEPropertyItemDelegate::QOEPropertyItemDelegate(EditorInteraction* editorInteraction, QObject* parent)
	: QStyledItemDelegate(parent), m_EditorInteraction(editorInteraction)
{
	QPixmapCache::setCacheLimit(10240 * 5);
}

QOEPropertyItemDelegate::~QOEPropertyItemDelegate()
{
	QPixmapCache::clear();
}

void QOEPropertyItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (!index.isValid() || index.column() != 1) {
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	QOEPropertiesModelItem* item = static_cast<QOEPropertiesModelItem*>(index.internalPointer());
	OrbitEngine::Meta::Kind kind = item->type->GetKind();
	QPixmap pixmap;

	bool hover = option.state & QStyle::StateFlag::State_MouseOver;
	QRect targetRect = updateRect(option.rect);

	bool regenerate = true;

	// item->pixmapKey.isValid() // Qt > 5.7

	if (!item->value_changed && item->last_hover == hover) {
		if (QPixmapCache::find(item->pixmapKey, &pixmap)) {
			if (pixmap.size() == targetRect.size())
				regenerate = false;
		}
	}

	if (regenerate) {
		OE_ASSERT(kind < 50);

		QWidget* widget = s_Placeholders[kind];

		if (!widget) {
			widget = createEditorWidget(kind);
			if (!widget) {
				// unsupported property
				QStyledItemDelegate::paint(painter, option, index);
				return;
			}
			s_Placeholders[kind] = widget;
		}

		updateEditorGeometry(widget, option, index);
		setEditorData(widget, index);

		// render
		QPixmap new_pixmap(targetRect.size());
		QPainter p(&new_pixmap);

		widget->setAttribute(Qt::WA_UnderMouse, hover);
		widget->setAttribute(Qt::WA_Hover, hover);
		widget->setAttribute(Qt::WA_NoMouseReplay, true);
		widget->setAttribute(Qt::WA_NoMousePropagation, true);
		widget->render(&p);
		p.end();

		QPixmapCache::remove(item->pixmapKey);
		item->pixmapKey = QPixmapCache::insert(new_pixmap);
		item->last_hover = hover;
		item->value_changed = false;

		painter->save();
		painter->drawPixmap(targetRect, new_pixmap);
		painter->restore();

		return;
	}

	painter->save();
	/*
	// Debug margins
	QStyleOptionViewItemV4 bg(option);
	bg.backgroundBrush = QBrush(QColor(255, 0, 0), Qt::BrushStyle::SolidPattern);
	QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &bg, painter);
	*/
	painter->drawPixmap(targetRect, pixmap);
	painter->restore();
}

QWidget* QOEPropertyItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QOEPropertiesModelItem* item = static_cast<QOEPropertiesModelItem*>(index.internalPointer());

	QWidget* widget = createEditorWidget(item->type->GetKind(), parent);

	if (widget)
		return widget;
	else
		return QStyledItemDelegate::createEditor(parent, option, index);
}

void QOEPropertyItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	QOEPropertiesModelItem* item = static_cast<QOEPropertiesModelItem*>(index.internalPointer());

	QMutexLocker locker(&item->mutex);
	const OrbitEngine::Meta::Variant& v = item->value;

#define LOAD_EDITOR_PROPERTY(kind, getter, editorClass) \
	case OrbitEngine::Meta::Kind::kind: \
	{ \
		const auto& value = v.getter(); \
		editorClass *editor_widget = static_cast< editorClass *>(editor); \

#define LOAD_EDITOR_PROPERTY_QT(kind, getter, editorClass) \
	LOAD_EDITOR_PROPERTY(kind, getter, editorClass) \
		editor_widget->setValue(value); \
		break; \
	} \

#define LOAD_EDITOR_PROPERTY_VECTOR_QT(kind, getter, type, vN) \
	case OrbitEngine::Meta::Kind::kind: \
	{ \
		const auto& value = v.getter(); \
		QNumericVector< type, vN > *editor_widget = static_cast< QNumericVector< type, vN > *>(editor); \
		for(int i = 0; i < vN; i++) { \
			editor_widget->setValue(value.data[i], i); \
		} \
		break; \
	} \

	if (v.IsValid() && !v.IsNull()) {
		switch (item->type->GetKind()) {
		LOAD_EDITOR_PROPERTY_QT(CHAR, GetChar, QNumeric<char>)
		LOAD_EDITOR_PROPERTY_QT(INT, GetInt, QNumeric<int>)
		LOAD_EDITOR_PROPERTY_QT(UINT, GetInt, QNumeric<unsigned int>)
		LOAD_EDITOR_PROPERTY_QT(LONG, GetLong, QNumeric<long>)
		LOAD_EDITOR_PROPERTY_QT(LONGLONG, GetLongLong, QNumeric<long long>)
		LOAD_EDITOR_PROPERTY_QT(ULONG, GetULong, QNumeric<unsigned long>)
		LOAD_EDITOR_PROPERTY_QT(ULONGLONG, GetULongLong, QNumeric<unsigned long long>)
		LOAD_EDITOR_PROPERTY_QT(DOUBLE, GetDouble, QNumeric<double>)
		LOAD_EDITOR_PROPERTY_QT(FLOAT, GetFloat, QNumeric<float>)

		LOAD_EDITOR_PROPERTY_VECTOR_QT(VEC2_INT, GetVec2i, int, 2)
		LOAD_EDITOR_PROPERTY_VECTOR_QT(VEC3_INT, GetVec3i, int, 3)
		LOAD_EDITOR_PROPERTY_VECTOR_QT(VEC4_INT, GetVec4i, int, 4)
		LOAD_EDITOR_PROPERTY_VECTOR_QT(VEC2_FLOAT, GetVec2f, float, 2)
		LOAD_EDITOR_PROPERTY_VECTOR_QT(VEC3_FLOAT, GetVec3f, float, 3)
		LOAD_EDITOR_PROPERTY_VECTOR_QT(VEC4_FLOAT, GetVec4f, float, 4)

		LOAD_EDITOR_PROPERTY(BOOL, GetBool, QCheckBox)
			editor_widget->setCheckState(value ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
			break;
		}
		LOAD_EDITOR_PROPERTY(STRING, GetString, QLineEdit)
			editor_widget->setText(QString::fromStdString(value));
			break;
		}
		default:
			// nope
			break;
		}
	}
}

void QOEPropertyItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	QOEPropertiesModelItem* item = static_cast<QOEPropertiesModelItem*>(index.internalPointer());

	QMutexLocker locker(&item->mutex);
	OrbitEngine::Meta::Variant v;

#define STORE_EDITOR_PROPERTY(kind, editorClass) \
	case OrbitEngine::Meta::Kind::kind: \
	{ \
		editorClass *editor_widget = static_cast< editorClass *>(editor); \

#define STORE_EDITOR_PROPERTY_QT(kind, editorClass) \
	STORE_EDITOR_PROPERTY(kind, editorClass) \
		v = OrbitEngine::Meta::Variant(editor_widget->value()); \
		break; \
	} \

#define STORE_EDITOR_PROPERTY_VECTOR_QT(kind, vec_type, type, vN) \
	case OrbitEngine::Meta::Kind::kind: \
	{ \
		QNumericVector< type, vN > *editor_widget = static_cast< QNumericVector< type, vN > *>(editor); \
		vec_type vec; \
		for(int i = 0; i < vN; i++) { \
			vec.data[i] = editor_widget->value(i); \
		} \
		v = OrbitEngine::Meta::Variant(vec); \
		break; \
	} \


	switch (item->type->GetKind()) {
		STORE_EDITOR_PROPERTY_QT(CHAR, QNumeric<char>)
		STORE_EDITOR_PROPERTY_QT(INT, QNumeric<int>)
		STORE_EDITOR_PROPERTY_QT(UINT, QNumeric<unsigned int>)
		STORE_EDITOR_PROPERTY_QT(LONG, QNumeric<long>)
		STORE_EDITOR_PROPERTY_QT(LONGLONG, QNumeric<long long>)
		STORE_EDITOR_PROPERTY_QT(ULONG, QNumeric<unsigned long>)
		STORE_EDITOR_PROPERTY_QT(ULONGLONG, QNumeric<unsigned long long>)
		STORE_EDITOR_PROPERTY_QT(DOUBLE, QNumeric<double>)
		STORE_EDITOR_PROPERTY_QT(FLOAT, QNumeric<float>)
			
		STORE_EDITOR_PROPERTY(BOOL, QCheckBox)
			v = OrbitEngine::Meta::Variant(editor_widget->checkState() == Qt::CheckState::Checked);
			break;
		}
		STORE_EDITOR_PROPERTY(STRING, QLineEdit)
			v = OrbitEngine::Meta::Variant(editor_widget->text().toStdString());
			break;
		}

		STORE_EDITOR_PROPERTY_VECTOR_QT(VEC2_INT, OrbitEngine::Math::Vec2i, int, 2)
		STORE_EDITOR_PROPERTY_VECTOR_QT(VEC3_INT, OrbitEngine::Math::Vec3i, int, 3)
		STORE_EDITOR_PROPERTY_VECTOR_QT(VEC4_INT, OrbitEngine::Math::Vec4i, int, 4)
		STORE_EDITOR_PROPERTY_VECTOR_QT(VEC2_FLOAT, OrbitEngine::Math::Vec2f, float, 2)
		STORE_EDITOR_PROPERTY_VECTOR_QT(VEC3_FLOAT, OrbitEngine::Math::Vec3f, float, 3)
		STORE_EDITOR_PROPERTY_VECTOR_QT(VEC4_FLOAT, OrbitEngine::Math::Vec4f, float, 4)
	}
	
	if (v.IsValid()) {
		if (item->value != v) {
			item->value = v;
			item->value_changed = true;
			QMetaObject::invokeMethod(m_EditorInteraction, "setProperty", Qt::QueuedConnection, Q_ARG(OrbitEngine::Meta::Member*, item->member), Q_ARG(void*, item->parent->object), Q_ARG(OrbitEngine::Meta::Variant, v));
		}
	}
}

QWidget* QOEPropertyItemDelegate::createEditorWidget(const OrbitEngine::Meta::Kind kind, QWidget* parent) const
{
	QWidget* widget = 0;

#define SUPPORTED_PROPERTY(kind, widgetType) \
	case OrbitEngine::Meta::Kind::kind: widget = new widgetType(parent); break;

	switch (kind)
	{
	SUPPORTED_PROPERTY(BOOL, QCheckBox)
	SUPPORTED_PROPERTY(CHAR, QNumeric<char>)
	SUPPORTED_PROPERTY(INT, QNumeric<int>)
	SUPPORTED_PROPERTY(UINT, QNumeric<unsigned int>)
	SUPPORTED_PROPERTY(LONG, QNumeric<long>)
	SUPPORTED_PROPERTY(LONGLONG, QNumeric<long long>)
	SUPPORTED_PROPERTY(ULONG, QNumeric<unsigned long>)
	SUPPORTED_PROPERTY(ULONGLONG, QNumeric<unsigned long long>)
	SUPPORTED_PROPERTY(DOUBLE, QNumeric<double>)
	SUPPORTED_PROPERTY(FLOAT, QNumeric<float>)
	SUPPORTED_PROPERTY(STRING, QLineEdit)

	SUPPORTED_PROPERTY(VEC2_INT, (QNumericVector<int, 2>))
	SUPPORTED_PROPERTY(VEC3_INT, (QNumericVector<int, 3>))
	SUPPORTED_PROPERTY(VEC4_INT, (QNumericVector<int, 4>))
	SUPPORTED_PROPERTY(VEC2_FLOAT, (QNumericVector<float, 2>))
	SUPPORTED_PROPERTY(VEC3_FLOAT, (QNumericVector<float, 3>))
	SUPPORTED_PROPERTY(VEC4_FLOAT, (QNumericVector<float, 4>))
	}

	if (widget != nullptr)
		widget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

	return widget;
}

void QOEPropertyItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	editor->setGeometry(updateRect(option.rect));
}

QRect QOEPropertyItemDelegate::updateRect(const QRect & rect) const
{
	QRect targetRect = rect;
	targetRect.setHeight(targetRect.height() - 2);
	targetRect.setRight(targetRect.right() - 5);
	targetRect.setTop(targetRect.top() + 2);
	return targetRect;
}
