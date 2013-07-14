#ifndef WINDOW_H
#define WINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

#include "xbeeqt.h"

namespace Ui {
	class window;
}

class window: public QDialog {
		Q_OBJECT

	public:
		explicit window(QWidget *parent = 0);
		~window();

	private:
		Ui::window *ui;

		QLineEdit *ui_textIdent;
		QPushButton *ui_btnGet;
		QPushButton *ui_btnSet;

		enum {
			STATE_READY1,
			STATE_READY2,
			STATE_RETRIEVING,
			STATE_SENDING,
			STATE_ERROR,
		} state;

		libxbee::XBee *t_xbee;
		libxbee::ConQt *t_con;

		void setEnabled(bool enabled);

	public slots:
		void triggerGet(bool checked);

	private slots:
		void triggerSet(bool checked);
		void textIdentChanged(const QString &text);
		void response(libxbee::Pkt *pkt);
};

#endif // WINDOW_H
