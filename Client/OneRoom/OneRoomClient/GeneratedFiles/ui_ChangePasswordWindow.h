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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_changePasswordWindow
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *oldPwLineEdit;
    QLabel *label_2;
    QLineEdit *newPwLineEdit;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *confirmButton;

    void setupUi(QWidget *changePasswordWindow)
    {
        if (changePasswordWindow->objectName().isEmpty())
            changePasswordWindow->setObjectName(QStringLiteral("changePasswordWindow"));
        changePasswordWindow->resize(300, 250);
        changePasswordWindow->setMinimumSize(QSize(300, 250));
        changePasswordWindow->setMaximumSize(QSize(300, 250));
        gridLayout = new QGridLayout(changePasswordWindow);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(changePasswordWindow);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 1, 1, 1);

        oldPwLineEdit = new QLineEdit(changePasswordWindow);
        oldPwLineEdit->setObjectName(QStringLiteral("oldPwLineEdit"));

        gridLayout->addWidget(oldPwLineEdit, 0, 2, 1, 1);

        label_2 = new QLabel(changePasswordWindow);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 1, 1, 1);

        newPwLineEdit = new QLineEdit(changePasswordWindow);
        newPwLineEdit->setObjectName(QStringLiteral("newPwLineEdit"));

        gridLayout->addWidget(newPwLineEdit, 1, 2, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 0, 3, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 0, 0, 1, 1);

        confirmButton = new QPushButton(changePasswordWindow);
        confirmButton->setObjectName(QStringLiteral("confirmButton"));

        gridLayout->addWidget(confirmButton, 2, 1, 1, 2);


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
