#include <QtGui/QApplication>
#include "window.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	window w;
	w.show();
	w.adjustSize();
	w.triggerGet(false);
	
	return a.exec();
}
