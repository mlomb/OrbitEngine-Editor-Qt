#ifndef projectselector_hpp
#define projectselector_hpp

#include <QMainWindow>
#include <QFile>
#include <QScopedPointer>
#include <QListWidget>

namespace Ui
{
    class ProjectSelector;
}

class ProjectSelector : public QMainWindow {
    Q_OBJECT

public:
    ProjectSelector(QWidget *parent = 0);
    virtual ~ProjectSelector();

private slots:
	void btnBrowseProject_clicked();
	void btnCreateProject_clicked();
	void btnOpenProject_clicked();
	void newProjectButtonCheck();
	void listRecentProjects_itemClicked(QListWidgetItem* item);

private:
    QScopedPointer<Ui::ProjectSelector> ui;

	void updateRecentProjects();
	void loadProject(const QFile& projectFile);
};

#endif