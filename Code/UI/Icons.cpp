#include "Icons.hpp"

#include <QPixmapCache>

QPixmap Icons::GetIcon(const std::string& iconName)
{
	std::string path = ":/icons/" + iconName + ".png";
	QString icon(QString::fromStdString(path));
	QPixmap pm;
	if (!QPixmapCache::find(icon, &pm)) {
		pm.load(icon);
		if (pm.size().height() > 24)
			pm = pm.scaled(QSize(24, 24), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QPixmapCache::insert(icon, pm);
	}
	return pm;
}
