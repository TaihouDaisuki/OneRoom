/********************************************************************************
** Form generated from reading UI file 'ChangePasswordWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHANGEPASSWORDWINDOW_H
#define UI_CHANGEPASSWORDWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_changePasswordWindow
{
public:
    QLabel *label;
    QLineEdit *oldPwLineEdit;
    QLabel *label_2;
    QLineEdit *newPwLineEdit;
    QPushButton *confirmButton;

    void setupUi(QWidget *changePasswordWindow)
    {
        if (changePasswordWindow->objectName().isEmpty())
            changePasswordWindow->setObjectName(QStringLiteral("changePasswordWindow"));
        changePasswordWindow->resize(300, 250);
        changePasswordWindow->setMinimumSize(QSize(300, 250));
        changePasswordWindow->setMaximumSize(QSize(300, 250));
        label = new QLabel(changePasswordWindow);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(23, 48, 101, 20));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        oldPwLineEdit = new QLineEdit(changePasswordWindow);
        oldPwLineEdit->setObjectName(QStringLiteral("oldPwLineEdit"));
        oldPwLineEdit->setGeometry(QRect(130, 48, 124, 20));
        label_2 = new QLabel(changePasswordWindow);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(23, 113, 101, 20));
        newPwLineEdit = new QLineEdit(changePasswordWindow);
        newPwLineEdit->setObjectName(QStringLiteral("newPwLineEdit"));
        newPwLineEdit->setGeometry(QRect(130, 113, 124, 20));
        confirmButton = new QPushButton(changePasswordWindow);
        confirmButton->setObjectName(QStringLiteral("confirmButton"));
        confirmButton->setGeometry(QRect(46, 178, 211, 23));

        retranslateUi(changePasswordWindow);

        QMetaObject::connectSlotsByName(changePasswordWindow);
    } // setupUi

    void retranslateUi(QWidget *changePasswordWindow)
    {
        changePasswordWindow->setWindowTitle(QApplication::translate("changePasswordWindow", "Form", nullptr));
        label->setText(QApplication::translate("changePasswordWindow", "Old Password:", nullptr));
        label_2->setText(QApplication::translate("changePasswordWindow", "New Password:", nullptr));
        confirmButton->setText(QApplication::translate("changePasswordWindow", "Confirm", nullptr));
    } // retranslateUi

};

namespace Ui {
    class changePasswordWindow: public Ui_changePasswordWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHANGEPASSWORDWINDOW_H
