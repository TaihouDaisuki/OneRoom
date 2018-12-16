#include "OneRoomClient.h"
#include "OneRoom.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	OneRoomClient w;
	w.hide();
	return a.exec();
}
