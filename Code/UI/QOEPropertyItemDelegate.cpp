#include "QOEPropertyItemDelegate.hpp"

#include <QSpinBox>
#include <QPainter>
#include <QStylePainter>
#include <QStyle>
#include <QApplication>
#include <QPixmapCache>

#include "QOEProperty.hpp"
#include <OE/Math/Vec4.hpp>

#include <OE/Misc/Property.hpp>
#include <OE/Engine/Transform.hpp>
#include <OE/Math/Vec2.hpp>
#include <OE/Math/Vec3.hpp>
#include <OE/Math/Vec4.hpp>

#include "QOEProperty.hpp"

#include <MetaCPP/TypeInfo.hpp>

QMap<metacpp::TypeID, QWidget*> QOEPropertyItemDelegate::s_Placeholders;

QOEPropertyItemDelegate::QOEPropertyItemDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
	QPixmapCache::setCacheLimit(10240 * 5); // 30MB
}

QOEPropertyItemDelegate::~QOEPropertyItemDelegate()
{
	QPixmapCache::clear();
}

void QOEPropertyItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	PropertiesModelNode* item = static_cast<PropertiesModelNode*>(index.internalPointer());
	if (!index.isValid() || index.column() != 1 || item->shouldSpan()) {
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}
	
	bool hover = option.state & QStyle::StateFlag::State_MouseOver;

	QRect targetRect = updateRect(option.rect);
	QString key = QString::number((ulong)item);
	QPixmap pixmap;

	if (!QPixmapCache::find(key, pixmap) || pixmap.size() != targetRect.size() || item->_value_expired || item->_hover_last != hover) {
		metacpp::TypeID typeID = item->getTypeID();
		QWidget* widget = 0;

		auto it = s_Placeholders.find(typeID);
		if (it != s_Placeholders.end())
			widget = it.value();
		else {
			widget = createEditorWidget(item->getTypeID(), nullptr);
			s_Placeholders.insert(typeID, widget);
		}

		if (widget == nullptr) { // unsupported property
			QStyledItemDelegate::paint(painter, option, index);
			return;
		}

		setEditorData(widget, index);
		updateEditorGeometry(widget, option, index);
		
		QPixmap new_pixmap(targetRect.size());
		QPainter p(&new_pixmap);

		widget->setAttribute(Qt::WA_UnderMouse, hover);
		widget->setAttribute(Qt::WA_Hover, hover);
		widget->setAttribute(Qt::WA_NoMouseReplay, true);
		widget->setAttribute(Qt::WA_NoMousePropagation, true);
		widget->render(&p);
		p.end();

		QPixmapCache::insert(key, new_pixmap);

		pixmap.swap(new_pixmap);

		item->_hover_last = hover;
		item->_value_expired = false;
	}

	painter->save();
	painter->drawPixmap(targetRect, pixmap);
	painter->restore();
}

QWidget* QOEPropertyItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	PropertiesModelNode* item = static_cast<PropertiesModelNode*>(index.internalPointer());

	QWidget* widget = createEditorWidget(item->getTypeID(), parent);
	return widget;
}

void QOEPropertyItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	PropertiesModelNode* item = static_cast<PropertiesModelNode*>(index.internalPointer());

	const metacpp::TypeID typeID = item->getTypeID();
	void* ptr = item->getProperty()->getPtr();

#define LOAD_EDITOR_PROPERTY(type, editorClass) \
	else if (typeID == metacpp::TypeInfo< ##type >::ID) { \
		##type *value = static_cast< ##type *>(ptr); \
		##editorClass *editor_widget = static_cast< ##editorClass *>(editor); \

#define LOAD_EDITOR_PROPERTY_QT(type, editorClass, editorSetValue) \
	LOAD_EDITOR_PROPERTY(type, editorClass) \
		editor_widget->editorSetValue((##type)(*value)); \
	} \

#define LOAD_EDITOR_PROPERTY_VECTOR(type, vType, vN) \
	else if (typeID == metacpp::TypeInfo< ##type >::ID) { \
		##type *value = static_cast< ##type *>(ptr); \
		QNumericVector< ##vType, ##vN > *editor_widget = static_cast< QNumericVector< ##vType, ##vN > *>(editor); \
		for(int i = 0; i < vN; i++) { \
			editor_widget->setValue(value->data[i], i); \
		} \
	} \

	if (false) {}
	LOAD_EDITOR_PROPERTY_QT(char, QNumeric<char>, setValue)
	LOAD_EDITOR_PROPERTY_QT(short, QNumeric<short>, setValue)
	LOAD_EDITOR_PROPERTY_QT(int, QNumeric<int>, setValue)
	LOAD_EDITOR_PROPERTY_QT(long, QNumeric<long>, setValue)
	LOAD_EDITOR_PROPERTY_QT(long long, QNumeric<long long>, setValue)
	LOAD_EDITOR_PROPERTY_QT(unsigned char, QNumeric<unsigned char>, setValue)
	LOAD_EDITOR_PROPERTY_QT(unsigned short, QNumeric<unsigned short>, setValue)
	LOAD_EDITOR_PROPERTY_QT(unsigned int, QNumeric<unsigned int>, setValue)
	LOAD_EDITOR_PROPERTY_QT(unsigned long, QNumeric<unsigned long>, setValue)
	LOAD_EDITOR_PROPERTY_QT(unsigned long long, QNumeric<unsigned long long>, setValue)
	LOAD_EDITOR_PROPERTY_QT(double, QNumeric<double>, setValue)
	LOAD_EDITOR_PROPERTY_QT(float, QNumeric<float>, setValue)
	LOAD_EDITOR_PROPERTY(std::string, QLineEdit)
		editor_widget->setText(QString::fromStdString(*value));
	}
	LOAD_EDITOR_PROPERTY(bool, QCheckBox)
		editor_widget->setCheckState(*value ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
	}
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec2<float>, float, 2)
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec3<float>, float, 3)
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec4<float>, float, 4)
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec2<int>, int, 2)
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec3<int>, int, 3)
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec4<int>, int, 4)
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec2<double>, double, 2)
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec3<double>, double, 3)
	LOAD_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec4<double>, double, 4)

}

void QOEPropertyItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	PropertiesModelNode* item = static_cast<PropertiesModelNode*>(index.internalPointer());

	const metacpp::TypeID typeID = item->getTypeID();
	void* ptr = item->getProperty()->getPtr();

#define STORE_EDITOR_PROPERTY(type, editorClass) \
	else if (typeID == metacpp::TypeInfo< ##type >::ID) { \
		OrbitEngine::Misc::Property< ##type *> prop = static_cast< ##type *>(ptr); \
		##editorClass *editor_widget = static_cast< ##editorClass *>(editor); \

#define STORE_EDITOR_PROPERTY_QT(type, editorClass, editorGetValue) \
	STORE_EDITOR_PROPERTY(type, editorClass) \
		*prop = (##type)editor_widget->editorGetValue(); \
	} \

#define STORE_EDITOR_PROPERTY_VECTOR(type, vType, vN) \
	else if (typeID == metacpp::TypeInfo< ##type >::ID) { \
		OrbitEngine::Misc::Property< ##type *> prop = static_cast< ##type *>(ptr); \
		QNumericVector< ##vType, ##vN > *editor_widget = static_cast< QNumericVector< ##vType, ##vN > *>(editor); \
		##type vec; \
		for(int i = 0; i < vN; i++) { \
			vec.data[i] = editor_widget->value(i); \
		} \
		*prop = vec; \
	} \

	if (false) {}
	STORE_EDITOR_PROPERTY_QT(char, QNumeric<char>, value)
	STORE_EDITOR_PROPERTY_QT(short, QNumeric<short>, value)
	STORE_EDITOR_PROPERTY_QT(int, QNumeric<int>, value)
	STORE_EDITOR_PROPERTY_QT(long, QNumeric<long>, value)
	STORE_EDITOR_PROPERTY_QT(long long, QNumeric<long long>, value)
	STORE_EDITOR_PROPERTY_QT(unsigned char, QNumeric<unsigned char>, value)
	STORE_EDITOR_PROPERTY_QT(unsigned short, QNumeric<unsigned short>, value)
	STORE_EDITOR_PROPERTY_QT(unsigned int, QNumeric<unsigned int>, value)
	STORE_EDITOR_PROPERTY_QT(unsigned long, QNumeric<unsigned long>, value)
	STORE_EDITOR_PROPERTY_QT(unsigned long long, QNumeric<unsigned long long>, value)
	STORE_EDITOR_PROPERTY_QT(double, QNumeric<double>, value)
	STORE_EDITOR_PROPERTY_QT(float, QNumeric<float>, value)
	STORE_EDITOR_PROPERTY(std::string, QLineEdit)
		*prop = editor_widget->text().toStdString();
	}
	STORE_EDITOR_PROPERTY(bool, QCheckBox)
		*prop = editor_widget->checkState() == Qt::CheckState::Checked;
	}
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec2<float>, float, 2)
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec3<float>, float, 3)
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec4<float>, float, 4)
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec2<int>, int, 2)
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec3<int>, int, 3)
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec4<int>, int, 4)
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec2<double>, double, 2)
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec3<double>, double, 3)
	STORE_EDITOR_PROPERTY_VECTOR(OrbitEngine::Math::Vec4<double>, double, 4)

	item->_value_expired = true;
}

QWidget* QOEPropertyItemDelegate::createEditorWidget(const metacpp::TypeID typeID, QWidget* parent) const
{
	QWidget* widget = 0;

#define SUPPORTED_PROPERTY(type, propertyClass) \
	else if (typeID == metacpp::TypeInfo< ##type >::ID) { widget = new propertyClass(parent); } \

	if (false) {}
	SUPPORTED_PROPERTY(std::string, QLineEdit)
	SUPPORTED_PROPERTY(bool, QCheckBox)
	SUPPORTED_PROPERTY(char, QNumeric<char>)
	SUPPORTED_PROPERTY(short, QNumeric<short>)
	SUPPORTED_PROPERTY(int, QNumeric<int>)
	SUPPORTED_PROPERTY(long, QNumeric<long>)
	SUPPORTED_PROPERTY(long long, QNumeric<long long>)
	SUPPORTED_PROPERTY(unsigned char, QNumeric<unsigned char>)
	SUPPORTED_PROPERTY(unsigned short, QNumeric<unsigned short>)
	SUPPORTED_PROPERTY(unsigned int, QNumeric<unsigned int>)
	SUPPORTED_PROPERTY(unsigned long, QNumeric<unsigned long>)
	SUPPORTED_PROPERTY(unsigned long long, QNumeric<unsigned long long>)
	SUPPORTED_PROPERTY(double, QNumeric<double>)
	SUPPORTED_PROPERTY(float, QNumeric<float>)
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec2<float>, (QNumericVector<float, 2>))
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec3<float>, (QNumericVector<float, 3>))
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec4<float>, (QNumericVector<float, 4>))
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec2<int>, (QNumericVector<int, 2>))
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec3<int>, (QNumericVector<int, 3>))
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec4<int>, (QNumericVector<int, 4>))
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec2<double>, (QNumericVector<double, 2>))
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec3<double>, (QNumericVector<double, 3>))
	SUPPORTED_PROPERTY(OrbitEngine::Math::Vec4<double>, (QNumericVector<double, 4>))

	if (widget != nullptr)
		widget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

	return widget;
}

void QOEPropertyItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	editor->setGeometry(updateRect(option.rect));
}

QRect QOEPropertyItemDelegate::updateRect(const QRect& rect) const
{
	QRect targetRect = rect;
	targetRect.setHeight(targetRect.height() - 5);
	targetRect.setRight(targetRect.right() - 5);
	return targetRect;
}
