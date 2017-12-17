#include <iostream>

#include <QApplication>
#include <QFile>
#include <QSettings>

#include <fstream>
#include <QTimer>
#include <QFile>
#include <QObject>

#include "UI/ProjectSelector.hpp"

#include <OE/Application/Window.hpp>
#include <OE/Application/Looper.hpp>
using namespace OrbitEngine;

#include <MetaCPP/Runtime.hpp>
#include <MetaCPP/TypeInfo.hpp>
#include <OE/Engine/SceneObject.hpp>


void ReplaceStringInPlace(std::string& subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}


int main(int argc, char *argv[])
{
	metacpp::generated::Load(metacpp::Runtime::GetStorage());

	metacpp::Storage* storage = metacpp::Runtime::GetStorage();
	storage->dump();

	auto tinfo = metacpp::TypeInfo<OrbitEngine::Engine::SceneObject>::TYPE;
	if(tinfo)
		tinfo->dump();

    QApplication app(argc, argv);
	
	/* Configuration */
	QApplication::setOrganizationName("TBD");
	QApplication::setApplicationName("OrbitEngine Editor");
	
	/* Apply global style */
	QFile globalQss(":/stylesheets/global.qss");
	globalQss.open(QFile::ReadOnly);
	app.setStyleSheet(QLatin1String(globalQss.readAll()));

	/* Project selection */
	ProjectSelector projectSelector;
	projectSelector.show();





	QTimer *timer = new QTimer();
	QObject::connect(timer, &QTimer::timeout, [&]() -> void {
		QFile globalQss("D:/Google Drive/Programming/C++/OrbitEngine-Editor/Resources/stylesheets/global.qss");
		globalQss.open(QFile::ReadOnly);
		std::string qss = globalQss.readAll();
		globalQss.close();

		std::map<std::string, std::string> vars;

		std::ifstream infile("D:/Google Drive/Programming/C++/OrbitEngine-Editor/Resources/stylesheets/variables.vars");
		std::string key;
		while (std::getline(infile, key))
		{
			std::string value;
			std::getline(infile, value);
			vars.insert(make_pair("$" + key, value));
		}

		for (auto& var : vars)
			ReplaceStringInPlace(qss, var.first, var.second);

		app.setStyleSheet(QString::fromStdString(qss));
	});
	timer->start(500);

    return app.exec();
}