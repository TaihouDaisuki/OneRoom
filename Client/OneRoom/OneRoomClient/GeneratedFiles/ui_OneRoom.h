/********************************************************************************
** Form generated from reading UI file 'OneRoom.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ONEROOM_H
#define UI_ONEROOM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OneRoomClass
{
public:
    QWidget *centralWidget;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QPushButton *pushButton;
    QLabel *label;

    void setupUi(QMainWindow *OneRoomClass)
    {
        if (OneRoomClass->objectName().isEmpty())
            OneRoomClass->setObjectName(QStringLiteral("OneRoomClass"));
        OneRoomClass->resize(276, 320);
        centralWidget = new QWidget(OneRoomClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(30, 210, 221, 31));
        lineEdit_2 = new QLineEdit(centralWidget);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(30, 260, 171, 31));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(210, 260, 41, 31));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(80, 40, 151, 131));
        OneRoomClass->setCentralWidget(centralWidget);

        retranslateUi(OneRoomClass);

        QMetaObject::connectSlotsByName(OneRoomClass);
    } // setupUi

    void retranslateUi(QMainWindow *OneRoomClass)
    {
        OneRoomClass->setWindowTitle(QApplication::translate("OneRoomClass", "OneRoom", nullptr));
        pushButton->setText(QApplication::translate("OneRoomClass", "->", nullptr));
        label->setText(QApplication::translate("OneRoomClass", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OneRoomClass: public Ui_OneRoomClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ONEROOM_H
