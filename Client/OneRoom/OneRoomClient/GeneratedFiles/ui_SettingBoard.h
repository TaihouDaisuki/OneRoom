/********************************************************************************
** Form generated from reading UI file 'SettingBoard.ui'
**
** Created by: Qt User Interface Compiler version 5.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGBOARD_H
#define UI_SETTINGBOARD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_settingBoard
{
public:
    QTabWidget *tabWidget;
    QWidget *generalTab;
    QGridLayout *gridLayout;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_4;
    QSpinBox *spinBox;
    QLabel *label;
    QPushButton *changeButton;
    QPushButton *cancelButton;
    QWidget *securityTab;
    QGridLayout *gridLayout_2;
    QPushButton *changePasswordBtn;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *settingBoard)
    {
        if (settingBoard->objectName().isEmpty())
            settingBoard->setObjectName(QStringLiteral("settingBoard"));
        settingBoard->resize(426, 283);
        settingBoard->setMinimumSize(QSize(426, 283));
        settingBoard->setMaximumSize(QSize(426, 283));
        tabWidget = new QTabWidget(settingBoard);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(9, 9, 411, 271));
        generalTab = new QWidget();
        generalTab->setObjectName(QStringLiteral("generalTab"));
        gridLayout = new QGridLayout(generalTab);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 0, 0, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 0, 4, 1, 1);

        spinBox = new QSpinBox(generalTab);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setMinimum(10);
        spinBox->setValue(10);

        gridLayout->addWidget(spinBox, 0, 3, 1, 1);

        label = new QLabel(generalTab);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 2, 1, 1);

        changeButton = new QPushButton(generalTab);
        changeButton->setObjectName(QStringLiteral("changeButton"));

        gridLayout->addWidget(changeButton, 1, 4, 1, 1);

        cancelButton = new QPushButton(generalTab);
        cancelButton->setObjectName(QStringLiteral("cancelButton"));

        gridLayout->addWidget(cancelButton, 1, 3, 1, 1);

        tabWidget->addTab(generalTab, QString());
        securityTab = new QWidget();
        securityTab->setObjectName(QStringLiteral("securityTab"));
        gridLayout_2 = new QGridLayout(securityTab);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        changePasswordBtn = new QPushButton(securityTab);
        changePasswordBtn->setObjectName(QStringLiteral("changePasswordBtn"));

        gridLayout_2->addWidget(changePasswordBtn, 0, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 0, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 0, 2, 1, 1);

        tabWidget->addTab(securityTab, QString());

        retranslateUi(settingBoard);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(settingBoard);
    } // setupUi

    void retranslateUi(QWidget *settingBoard)
    {
        settingBoard->setWindowTitle(QApplication::translate("settingBoard", "Form", nullptr));
        label->setText(QApplication::translate("settingBoard", "\345\233\236\347\234\213\347\232\204\345\216\206\345\217\262\347\272\252\345\275\225\346\235\241\346\225\260\357\274\232", nullptr));
        changeButton->setText(QApplication::translate("settingBoard", "\346\233\264\346\224\271", nullptr));
        cancelButton->setText(QApplication::translate("settingBoard", "\345\217\226\346\266\210", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(generalTab), QApplication::translate("settingBoard", "\345\270\270\351\200\211\351\241\271", nullptr));
        changePasswordBtn->setText(QApplication::translate("settingBoard", "\344\277\256\346\224\271\345\257\206\347\240\201", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(securityTab), QApplication::translate("settingBoard", "\345\256\211\345\205\250", nullptr));
    } // retranslateUi

};

namespace Ui {
    class settingBoard: public Ui_settingBoard {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGBOARD_H
