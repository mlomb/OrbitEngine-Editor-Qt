#ifndef qoeproperty_hpp
#define qoeproperty_hpp

#include <QWidget>
#include <qformlayout.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qspinbox.h>

#include "OE/Misc/OEObject.hpp"
#include "OE/Misc/Property.hpp"

#include "QNumeric.hpp"

class QOEProperty : public QWidget {
	Q_OBJECT

public:
	QOEProperty(OrbitEngine::Misc::PropertyBase* prop, QWidget *parent = 0);

	void init();

protected:
	template<typename W, typename S>
	void bindWidget(W* widget, S editSignal);

	virtual void buildLayout(QLayout* layout) = 0;
	virtual void loadProperty() = 0;
	virtual void storeProperty() = 0;

	OrbitEngine::Misc::PropertyBase* m_Property;
	bool m_EditGuard;
};

template<typename W, typename S>
inline void QOEProperty::bindWidget(W* widget, S editSignal)
{
	connect(widget, editSignal, this, [&]() {
		m_EditGuard = true;
		storeProperty();
	});

	auto modificationEventPtr = m_Property->onModification.AddListener([&](OrbitEngine::Application::Event e)-> void {
		if (m_EditGuard) {
			m_EditGuard = false;
			return;
		}
		loadProperty();
	});

	// when the widget is destroyed, unbind
	connect(widget, &QObject::destroyed, this, [&, modificationEventPtr]() {
		m_Property->onModification.RemoveListener(modificationEventPtr);
	});
}

class QOEPropertyString : public QOEProperty {
public:
	QOEPropertyString(OrbitEngine::Misc::Property<std::string>* prop, QWidget *parent = 0);

	void buildLayout(QLayout* layout) override;
	void loadProperty() override;
	void storeProperty() override;

private:
	QLineEdit* m_LineEdit;
};

class QOEPropertyBoolean : public QOEProperty {
public:
	QOEPropertyBoolean(OrbitEngine::Misc::Property<bool>* prop, QWidget *parent = 0);

	void buildLayout(QLayout* layout) override;
	void loadProperty() override;
	void storeProperty() override;

private:
	QCheckBox* m_Checkbox;
};

template<typename T>
class QOEPropertyNumeric : public QOEProperty {
public:

	QOEPropertyNumeric(OrbitEngine::Misc::Property<T>* prop, QWidget *parent = 0)
		: QOEProperty(prop, parent)
	{
		init();
	}

	void buildLayout(QLayout* layout) override {
		m_SpinBox = new QNumeric<T>(this);

		bindWidget(m_SpinBox, &QAbstractSpinBox::windowIconChanged /* Just connect to a dummy signal */);
		m_SpinBox->setValueChangedCallback([&](T) -> void {
			m_EditGuard = true;
			storeProperty();
		});

		layout->addWidget(m_SpinBox);
	}

	void loadProperty() override {
		auto value = *static_cast<OrbitEngine::Misc::Property<T>*>(m_Property);
		m_SpinBox->setValue(value);
	}

	void storeProperty() override {
		auto prop = static_cast<OrbitEngine::Misc::Property<T>*>(m_Property);
		*prop = m_SpinBox->value();
	}

private:
	QNumeric<T>* m_SpinBox;
};


// <3, float, Vec3<float>>
template<typename T, typename V, unsigned int N>
class QOEPropertyVector : public QOEProperty {
public:

	QOEPropertyVector(OrbitEngine::Misc::Property<V>* prop, QWidget *parent = 0)
		: QOEProperty(prop, parent)
	{
		init();
	}

	void buildLayout(QLayout* layout) override {
		static const std::string alias[]{ "X", "Y", "Z", "W" };
		static const QString color[]{ "red", "green", "blue", "yellow" };

		for (int i = 0; i < N; i++) {
			QLabel* label = new QLabel(QString::fromStdString(alias[i] + ":"), this);
			label->setStyleSheet("QLabel { color: " + color[i] + "; }");
			label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
			layout->addWidget(label);

			QNumeric<T>* numeric = new QNumeric<T>(this);
			numeric->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			bindWidget(numeric, &QAbstractSpinBox::windowIconChanged /* Just connect to a dummy signal */);
			numeric->setValueChangedCallback([&](T) -> void {
				m_EditGuard = true;
				storeProperty();
			});

			layout->addWidget(numeric);

			m_SpinBoxes.push_back(numeric);
		}
	}

	void loadProperty() override {
		V prop = *static_cast<OrbitEngine::Misc::Property<V>*>(m_Property);

		for (int i = 0; i < N; i++)
			m_SpinBoxes[i]->setValue(prop.data[i]);
	}

	void storeProperty() override {
		auto prop = static_cast<OrbitEngine::Misc::Property<V>*>(m_Property);

		V vec;
		for (int i = 0; i < N; i++) {
			vec.data[i] = m_SpinBoxes[i]->value();
		}
		*prop = vec;
	}

private:
	std::vector<QNumeric<T>*> m_SpinBoxes;
};

#endif