#ifndef qoewidget_hpp
#define qoewidget_hpp

#include <iostream>

#include <QWidget>
#include <QThread>
#include <QEvent>

#include <OE/Application/Window.hpp>
#include <OE/Application/Context.hpp>
#include <OE/Application/Looper.hpp>
#include <OE/Application/Loopeable.hpp>

class QOEWidget : public QWidget {
	Q_OBJECT

public:
	QOEWidget(QWidget *parent = 0);
	virtual ~QOEWidget();

	void setLoopeable(OrbitEngine::Application::Loopeable* loopeable);

	virtual QPaintEngine* paintEngine() const override { return 0; }

#if OE_WINDOWS
	WNDPROC _WNDPROC;
#endif

protected:
	void run();

	bool event(QEvent* event) override;
	void showEvent(QShowEvent* event) override;
	void focusInEvent(QFocusEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	OrbitEngine::Application::Window* m_Window;
	OrbitEngine::Application::Context* m_Context;
	OrbitEngine::Application::Looper* m_Looper;
	OrbitEngine::Application::Loopeable* m_Loopeable;

	QThread* m_Thread;
};

#endif