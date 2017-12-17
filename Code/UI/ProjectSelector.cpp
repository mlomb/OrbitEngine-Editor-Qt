#include "ProjectSelector.hpp"
#include "ui_projectselector.h"

#include <iostream>

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include <QTimer>

#include "Editor.hpp"

ProjectSelector::ProjectSelector(QWidget *parent)
: QMainWindow(parent), ui(new Ui::ProjectSelector)
{
    ui->setupUi(this);

	QObject::connect(ui->btnBrowseProject, SIGNAL(clicked()), this, SLOT(btnBrowseProject_clicked()));
	QObject::connect(ui->btnCreateProject, SIGNAL(clicked()), this, SLOT(btnCreateProject_clicked()));
	QObject::connect(ui->btnOpenProject, SIGNAL(clicked()), this, SLOT(btnOpenProject_clicked()));
	QObject::connect(ui->listRecentProjects, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listRecentProjects_itemClicked(QListWidgetItem*)));
	QObject::connect(ui->txtProjectName, SIGNAL(textChanged(const QString&)), this, SLOT(newProjectButtonCheck()));
	QObject::connect(ui->txtLocationProject, SIGNAL(textChanged(const QString&)), this, SLOT(newProjectButtonCheck()));

	updateRecentProjects();

	QTimer::singleShot(50, [this]() {
		loadProject(QFile("D:/Google Drive/Programming/C++/OrbitEngine Workspace/Projects/Project.oep"));
	});
}

ProjectSelector::~ProjectSelector()
{

}

void ProjectSelector::btnBrowseProject_clicked()
{
	QString project_directory = QFileDialog::getExistingDirectory(this);
	if (project_directory.length() > 0)
		ui->txtLocationProject->setText(project_directory);
}

void ProjectSelector::btnCreateProject_clicked()
{
	QDir dir(ui->txtLocationProject->text());
	QFile project_file;
	project_file.setFileName(dir.absoluteFilePath(ui->txtProjectName->text() + ".oep"));

	if (project_file.exists()) {
		// A project already exists there!
		QMessageBox::critical(this, "Failed to create project!", "A project with that name already exists in that location.");
		return;
	}

	/* For now, just create an empty file */
	project_file.open(QIODevice::ReadWrite);
	project_file.close();

	loadProject(project_file);
}

void ProjectSelector::newProjectButtonCheck()
{
	QDir dir(ui->txtLocationProject->text());

	bool enabled = true;

	enabled &= ui->txtProjectName->text().length() > 0;
	enabled &= !dir.isRelative();
	enabled &= dir.exists();

	ui->btnCreateProject->setEnabled(enabled);
}

void ProjectSelector::listRecentProjects_itemClicked(QListWidgetItem* item)
{
	loadProject(item->data(Qt::UserRole).toString());
}

void ProjectSelector::btnOpenProject_clicked()
{
	QString project_file = QFileDialog::getOpenFileName(this, QString(), QString(), "*.oep");
	if (project_file.length() > 0)
		loadProject(project_file);
}

void ProjectSelector::updateRecentProjects()
{
	QSettings settings;
	QStringList projects = settings.value("recentProjects").toStringList();

	ui->listRecentProjects->clear();
	for (const QString& proj : projects) {
		QListWidgetItem* item = new QListWidgetItem();

		item->setText(QFileInfo(proj).baseName());
		item->setData(Qt::UserRole, proj);

		ui->listRecentProjects->addItem(item);
	}
}

void ProjectSelector::loadProject(const QFile& projectFile)
{
	if (!projectFile.exists()) {
		// The project does not exist
		QMessageBox::critical(this, "Failed to open project!", "The project does not exist!");
		return;
	}

	/* Update recent projects */
	QSettings settings;
	QStringList projects = settings.value("recentProjects").toStringList();
	projects.removeAll(projectFile.fileName());
	projects.prepend(projectFile.fileName());
	while (projects.size() > 100)
		projects.removeLast();
	settings.setValue("recentProjects", projects);

	/* Open the project */
	QMessageBox::information(this, "OPENING PROJECT: ", projectFile.fileName());
	
	Editor* editor = new Editor();
	editor->show();
	this->hide();
}
