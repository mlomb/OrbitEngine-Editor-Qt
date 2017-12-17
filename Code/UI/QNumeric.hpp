#ifndef qonumeric_hpp
#define qonumeric_hpp

#include <QWidget>
#include <qspinbox.h>
#include <iomanip>
#include <sstream>
#include <QPainter>
#include <QStyleOption>

template<typename T>
class QNumeric : public QAbstractSpinBox {
public:
	QNumeric(QWidget *parent = 0);

	T value() const;

	void setValue(T val);

	void setValueChangedCallback(std::function<void(T)> cb);

protected:
	QValidator::State validate(QString &input, int &pos) const override;
	void stepBy(int steps) override;
	QAbstractSpinBox::StepEnabled stepEnabled() const override;

private:
	T m_Value;
	T m_Min, m_Max;
	std::function<void(T)> m_ValueChanged;

	T convert(const std::string& str, bool& ok) const;
	void updateLineEdit();
	void apply();

	const bool T_is_signed = std::numeric_limits<T>::is_signed;
	const bool T_is_char = std::is_same<T, char>::value || std::is_same<T, unsigned char>::value;
	const bool T_is_decimal = std::is_same<T, float>::value || std::is_same<T, double>::value;
};


template<typename T>
QNumeric<T>::QNumeric(QWidget* parent)
	: QAbstractSpinBox(parent), m_Value(0)
{
	m_Min = std::numeric_limits<T>::lowest();
	m_Max = std::numeric_limits<T>::max();

	this->setButtonSymbols(ButtonSymbols::NoButtons);
	this->setFocusPolicy(Qt::StrongFocus);

	connect(this, &QAbstractSpinBox::editingFinished, this, &QNumeric<T>::apply);

	updateLineEdit();
}

template<typename T>
inline T QNumeric<T>::value() const
{
	return m_Value;
}

template<typename T>
inline void QNumeric<T>::setValue(T val)
{
	if (val != m_Value) {
		m_Value = val;
		updateLineEdit();
		// emit valueChanged(val); no Q_OBJECT no Q_FUN
		if(m_ValueChanged)
			m_ValueChanged(m_Value);
	}
}

template<typename T>
inline void QNumeric<T>::setValueChangedCallback(std::function<void(T)> cb)
{
	m_ValueChanged = cb;
}

template<typename T>
inline QValidator::State QNumeric<T>::validate(QString& input, int& pos) const
{
	return QValidator::State::Acceptable;
}

template<typename T>
inline void QNumeric<T>::stepBy(int steps)
{
	setValue(m_Value + steps);
}

template<typename T>
inline QAbstractSpinBox::StepEnabled QNumeric<T>::stepEnabled() const
{
	QAbstractSpinBox::StepEnabled flags;

	if (m_Value > m_Min)
		flags |= QAbstractSpinBox::StepDownEnabled;
	if (m_Value < m_Max)
		flags |= QAbstractSpinBox::StepUpEnabled;

	return flags;
}

template<typename T>
inline T QNumeric<T>::convert(const std::string& str, bool& ok) const
{
	std::istringstream ss(str);
	T val = -42; // boi

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

	if (T_is_decimal) {
		double temp;
		ss >> temp;
		temp = MAX(m_Min, temp);
		temp = MIN(m_Max, temp);
		val = temp;
	}
	else if (T_is_signed) {
		long long temp;
		ss >> temp;
		temp = MAX(m_Min, temp);
		temp = MIN(m_Max, temp);
		val = temp;
	}
	else {
		unsigned long long temp;
		ss >> temp;
		if (str[0] == '-')
			temp = 0;
		temp = MAX(m_Min, temp);
		temp = MIN(m_Max, temp);
		val = temp;
	}

#undef MIN
#undef MAX

	ok = !ss.fail() && ss.eof();
	return val;
}

template<typename T>
inline void QNumeric<T>::updateLineEdit()
{
	std::stringstream ss;

	if (T_is_decimal) {
		ss.precision(4); // std::numeric_limits<T>::max_digits10
		ss << std::fixed;
	}

	if (T_is_char)
		ss << (int)m_Value;
	else
		ss << (T)m_Value;

	std::string val = ss.str();

	if (T_is_decimal) {
		char c;
		while (val.size() > 0) {
			c = val[val.size() - 1];
			if (c == '0' || c == '.') {
				val.erase(val.end() - 1);
				if (c == '.')
					break;
			}
			else break;
		}
	}

	lineEdit()->deselect();
	QMetaObject::invokeMethod(lineEdit(), "setText", Q_ARG(QString, QString::fromStdString(val)));
}

template<typename T>
inline void QNumeric<T>::apply()
{
	bool ok;
	std::string str = lineEdit()->text().toStdString();
	T val = convert(str, ok);
	if (ok)
		setValue(val);
	else
		updateLineEdit();
}

///////////////////////
// QNumericVector
///////////////////////
template<typename T, unsigned int N>
class QNumericVector : public QWidget {
public:
	QNumericVector(QWidget* parent = 0);
	~QNumericVector();

	T value(const int index) const;
	void setValue(const T& value, const int index);

private:
	QNumeric<T>* m_SpinBoxes[N];
};

#endif

template<typename T, unsigned int N>
inline QNumericVector<T, N>::QNumericVector(QWidget* parent)
	: QWidget(parent)
{
	QHBoxLayout* layout = new QHBoxLayout(this);

	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	static const QString alias[]{ "X", "Y", "Z", "W" };
	static const QString color[]{ "red", "green", "blue", "yellow" };

	for (int i = 0; i < N; i++) {
		QLabel* label = new QLabel(alias[i], this);
		label->setStyleSheet("QLabel { color: " + color[i] + "; }");
		label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
		layout->addWidget(label);

		QNumeric<T>* numeric = new QNumeric<T>(this);
		numeric->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

		layout->addWidget(numeric);

		m_SpinBoxes[i] = numeric;
	}

	this->setLayout(layout);
}

template<typename T, unsigned int N>
inline QNumericVector<T, N>::~QNumericVector()
{
}

template<typename T, unsigned int N>
inline T QNumericVector<T, N>::value(const int index) const
{
	return m_SpinBoxes[index]->value();
}

template<typename T, unsigned int N>
inline void QNumericVector<T, N>::setValue(const T& value, const int index)
{
	return m_SpinBoxes[index]->setValue(value);
}
