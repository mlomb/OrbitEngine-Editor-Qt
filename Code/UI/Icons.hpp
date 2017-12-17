#ifndef qoicons_hpp
#define qoicons_hpp

#include <string>

#include <QPixmap>

class Icons {
public:
	static QPixmap GetIcon(const std::string& iconName);
};

#endif