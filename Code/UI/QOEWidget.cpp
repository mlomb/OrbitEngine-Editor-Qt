#include "QOEWidget.hpp"

#include <QEvent>
#include <QKeyEvent>

#include <OE/Platform/OpenGL/OpenGL.hpp>
#include <OE/Application/InputManager.hpp>

QOEWidget::QOEWidget(QWidget *parent)
	: QWidget(parent), m_Loopeable(0), m_Looper(0), m_Context(0)
{
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_MSWindowsUseDirect3D);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_NoSystemBackground);
	setAutoFillBackground(false);
	setFocusPolicy(Qt::StrongFocus);

	WId wid = this->winId();
	OrbitEngine::Application::WindowNativeHandle handle = (OrbitEngine::Application::WindowNativeHandle)wid;
	m_Window = new OrbitEngine::Application::Window(handle);

	m_Thread = new std::thread(&QOEWidget::run, this);
}

void QOEWidget::run() {
	m_Context = new OrbitEngine::Application::Context(OrbitEngine::RenderAPI::OPENGL, m_Window);
	m_Looper = new OrbitEngine::Application::Looper(m_Context, m_Loopeable);
	m_Looper->loop();
}

bool QOEWidget::event(QEvent* event)
{
	if (event->type() == QEvent::KeyPress ||
		event->type() == QEvent::KeyRelease) {
		QKeyEvent *ke = static_cast<QKeyEvent*>(event);

		bool down = event->type() == QEvent::KeyPress;
		m_Window->getInputManager()->onInputKey(OrbitEngine::Application::Key(ke->key()), down);
	}
	return true;
}

QOEWidget::~QOEWidget()
{
	if (m_Looper) {
		m_Looper->stop();
		m_Thread->join();
		delete m_Looper;
	}
	if (m_Context)
		delete m_Context;
	delete m_Window;
}

void QOEWidget::setLoopeable(OrbitEngine::Application::Loopeable* loopeable)
{
	m_Loopeable = loopeable;
	if(m_Looper)
		m_Looper->setLoopeable(loopeable);
}
