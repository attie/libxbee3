/********************************************************************************
** Form generated from reading UI file 'window.ui'
**
** Created: Sun Jul 14 12:53:47 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_window
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_2;
    QLabel *lbl1;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *lbl2;
    QLineEdit *textIdent;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *btnGet;
    QPushButton *btnSet;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *window)
    {
        if (window->objectName().isEmpty())
            window->setObjectName(QString::fromUtf8("window"));
        window->resize(400, 300);
        verticalLayout = new QVBoxLayout(window);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        lbl1 = new QLabel(window);
        lbl1->setObjectName(QString::fromUtf8("lbl1"));
        QFont font;
        font.setPointSize(16);
        lbl1->setFont(font);

        horizontalLayout_2->addWidget(lbl1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        lbl2 = new QLabel(window);
        lbl2->setObjectName(QString::fromUtf8("lbl2"));
        QFont font1;
        font1.setPointSize(11);
        lbl2->setFont(font1);

        horizontalLayout_3->addWidget(lbl2);

        textIdent = new QLineEdit(window);
        textIdent->setObjectName(QString::fromUtf8("textIdent"));

        horizontalLayout_3->addWidget(textIdent);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        btnGet = new QPushButton(window);
        btnGet->setObjectName(QString::fromUtf8("btnGet"));
        btnGet->setMinimumSize(QSize(60, 30));
        btnGet->setFont(font1);

        horizontalLayout->addWidget(btnGet);

        btnSet = new QPushButton(window);
        btnSet->setObjectName(QString::fromUtf8("btnSet"));
        btnSet->setEnabled(false);
        btnSet->setMinimumSize(QSize(60, 30));
        btnSet->setFont(font1);

        horizontalLayout->addWidget(btnSet);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(window);

        QMetaObject::connectSlotsByName(window);
    } // setupUi

    void retranslateUi(QDialog *window)
    {
        window->setWindowTitle(QApplication::translate("window", "window", 0, QApplication::UnicodeUTF8));
        lbl1->setText(QApplication::translate("window", "libxbee3 sample application #15 - Qt", 0, QApplication::UnicodeUTF8));
        lbl2->setText(QApplication::translate("window", "Node Identifier:", 0, QApplication::UnicodeUTF8));
        btnGet->setText(QApplication::translate("window", "Get", 0, QApplication::UnicodeUTF8));
        btnSet->setText(QApplication::translate("window", "Set", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class window: public Ui_window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WINDOW_H
