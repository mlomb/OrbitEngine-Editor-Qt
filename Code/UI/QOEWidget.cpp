#include "QOEWidget.hpp"

#include <OE/Platform/OpenGL/OpenGL.hpp>

QOEWidget::QOEWidget(QWidget *parent)
	: QWidget(parent), m_Loopeable(0), m_Looper(0), m_Context(0), m_Window(0)
#if OE_WINDOWS
	, _WNDPROC(0)
#endif
{
	setAttribute(Qt::WA_NoBackground);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_MSWindowsUseDirect3D);
	setAttribute(Qt::WA_OpaquePaintEvent);

	setAutoFillBackground(false);
	setFocusPolicy(Qt::StrongFocus);
}

QOEWidget::~QOEWidget()
{
	if (m_Looper) {
		m_Looper->stop();
		m_Thread->wait();
		delete m_Looper;
	}
	if (m_Context)
		delete m_Context;
	delete m_Window;
}

void QOEWidget::run() {
	m_Context = new OrbitEngine::Application::Context(OrbitEngine::RenderAPI::OPENGL, m_Window);
	m_Looper = new OrbitEngine::Application::Looper(m_Context, m_Loopeable);
	m_Looper->loop();
}

#if OE_WINDOWS
LRESULT CALLBACK WinHostProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	QOEWidget* widget = qobject_cast<QOEWidget*>(QWidget::find((WId)::GetParent(hwnd)));

	if (widget) {
		switch (msg) {
		case WM_LBUTTONDOWN:
			if (::GetFocus() != hwnd && (widget->focusPolicy() & Qt::ClickFocus))
				widget->setFocus(Qt::MouseFocusReason);
			break;
		default:
			break;
		}

		if (widget->_WNDPROC)
			return CallWindowProc(widget->_WNDPROC, hwnd, msg, wParam, lParam);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif

bool QOEWidget::event(QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Polish:
		if (!m_Window) {
			OrbitEngine::Application::WindowProperties props;
			props.displayMode = OrbitEngine::Application::OVERLAPPED;
			props.parent = (OrbitEngine::Application::WindowNativeHandle)winId();

			m_Window = new OrbitEngine::Application::Window(props);
			m_Thread = new QThread();

			connect(m_Thread, &QThread::started, [this]() -> void
			{
				run();
			});

			m_Thread->start();
		}

#if OE_WINDOWS
		HWND hwnd = m_Window->getWindowNativeHandle();
		if (m_Window && !_WNDPROC && GetParent(hwnd) == (HWND)winId()) {
			_WNDPROC = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WinHostProc);
		}
#endif
		break;
	}
	return QWidget::event(event);
}

void QOEWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	if (m_Window) {
#if OE_WINDOWS
		::SetWindowPos(m_Window->getWindowNativeHandle(), HWND_TOP, 0, 0, width(), height(), SWP_SHOWWINDOW);
#endif
	}
}

void QOEWidget::focusInEvent(QFocusEvent* event)
{
	QWidget::focusInEvent(event);

	if (m_Window) {
#if OE_WINDOWS
		::SetFocus(m_Window->getWindowNativeHandle());
#endif
	}
}

void QOEWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	if (m_Window) {
#if OE_WINDOWS
		::SetWindowPos(m_Window->getWindowNativeHandle(), HWND_TOP, 0, 0, width(), height(), 0);
#endif
	}
}

void QOEWidget::setLoopeable(OrbitEngine::Application::Loopeable* loopeable)
{
	m_Loopeable = loopeable;
	if(m_Looper)
		m_Looper->setLoopeable(loopeable);
}