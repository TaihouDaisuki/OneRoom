#include "OneRoomClient.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	OneRoomClient w;
	return a.exec();
}
