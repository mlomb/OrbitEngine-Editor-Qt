#ifndef qoewidget_hpp
#define qoewidget_hpp

#include <QWidget>

#include <thread>

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

protected:
	void run();
	
	bool event(QEvent* event) override;

private:
	OrbitEngine::Application::Window* m_Window;
	OrbitEngine::Application::Context* m_Context;
	OrbitEngine::Application::Looper* m_Looper;
	OrbitEngine::Application::Loopeable* m_Loopeable;

	std::thread* m_Thread;
};

#endif