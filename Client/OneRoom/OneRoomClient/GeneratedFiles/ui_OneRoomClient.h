/********************************************************************************
** Form generated from reading UI file 'OneRoomClient.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ONEROOMCLIENT_H
#define UI_ONEROOMCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OneRoomClientClass
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QPushButton *sendImgBtn;
    QTextEdit *msgTextEdit;
    QPushButton *sendMsgBtn;
    QPushButton *sendFileBtn;
    QPushButton *msgHistoryBtn;
    QPushButton *logOutBtn;
    QListWidget *userListWidget;
    QSpacerItem *horizontalSpacer;
    QListWidget *msgListWidget;
    QPushButton *settingBtn;

    void setupUi(QMainWindow *OneRoomClientClass)
    {
        if (OneRoomClientClass->objectName().isEmpty())
            OneRoomClientClass->setObjectName(QStringLiteral("OneRoomClientClass"));
        OneRoomClientClass->resize(780, 645);
        centralWidget = new QWidget(OneRoomClientClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        sendImgBtn = new QPushButton(centralWidget);
        sendImgBtn->setObjectName(QStringLiteral("sendImgBtn"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(sendImgBtn->sizePolicy().hasHeightForWidth());
        sendImgBtn->setSizePolicy(sizePolicy);

        gridLayout->addWidget(sendImgBtn, 2, 6, 1, 1);

        msgTextEdit = new QTextEdit(centralWidget);
        msgTextEdit->setObjectName(QStringLiteral("msgTextEdit"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(msgTextEdit->sizePolicy().hasHeightForWidth());
        msgTextEdit->setSizePolicy(sizePolicy1);
        msgTextEdit->setBaseSize(QSize(0, 0));
        msgTextEdit->viewport()->setProperty("cursor", QVariant(QCursor(Qt::IBeamCursor)));

        gridLayout->addWidget(msgTextEdit, 1, 2, 1, 6);

        sendMsgBtn = new QPushButton(centralWidget);
        sendMsgBtn->setObjectName(QStringLiteral("sendMsgBtn"));
        sizePolicy.setHeightForWidth(sendMsgBtn->sizePolicy().hasHeightForWidth());
        sendMsgBtn->setSizePolicy(sizePolicy);

        gridLayout->addWidget(sendMsgBtn, 2, 7, 1, 1);

        sendFileBtn = new QPushButton(centralWidget);
        sendFileBtn->setObjectName(QStringLiteral("sendFileBtn"));
        sizePolicy.setHeightForWidth(sendFileBtn->sizePolicy().hasHeightForWidth());
        sendFileBtn->setSizePolicy(sizePolicy);

        gridLayout->addWidget(sendFileBtn, 2, 5, 1, 1);

        msgHistoryBtn = new QPushButton(centralWidget);
        msgHistoryBtn->setObjectName(QStringLiteral("msgHistoryBtn"));
        sizePolicy.setHeightForWidth(msgHistoryBtn->sizePolicy().hasHeightForWidth());
        msgHistoryBtn->setSizePolicy(sizePolicy);

        gridLayout->addWidget(msgHistoryBtn, 2, 4, 1, 1);

        logOutBtn = new QPushButton(centralWidget);
        logOutBtn->setObjectName(QStringLiteral("logOutBtn"));
        sizePolicy.setHeightForWidth(logOutBtn->sizePolicy().hasHeightForWidth());
        logOutBtn->setSizePolicy(sizePolicy);

        gridLayout->addWidget(logOutBtn, 2, 0, 1, 1);

        userListWidget = new QListWidget(centralWidget);
        userListWidget->setObjectName(QStringLiteral("userListWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(userListWidget->sizePolicy().hasHeightForWidth());
        userListWidget->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(userListWidget, 0, 0, 2, 2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 2, 2, 1, 1);

        msgListWidget = new QListWidget(centralWidget);
        msgListWidget->setObjectName(QStringLiteral("msgListWidget"));
        msgListWidget->setMinimumSize(QSize(500, 400));

        gridLayout->addWidget(msgListWidget, 0, 2, 1, 6);

        settingBtn = new QPushButton(centralWidget);
        settingBtn->setObjectName(QStringLiteral("settingBtn"));

        gridLayout->addWidget(settingBtn, 2, 1, 1, 1);

        OneRoomClientClass->setCentralWidget(centralWidget);

        retranslateUi(OneRoomClientClass);

        QMetaObject::connectSlotsByName(OneRoomClientClass);
    } // setupUi

    void retranslateUi(QMainWindow *OneRoomClientClass)
    {
        OneRoomClientClass->setWindowTitle(QApplication::translate("OneRoomClientClass", "OneRoomClient", nullptr));
        sendImgBtn->setText(QApplication::translate("OneRoomClientClass", "Send Picture", nullptr));
        msgTextEdit->setHtml(QApplication::translate("OneRoomClientClass", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'SimSun'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p></body></html>", nullptr));
        sendMsgBtn->setText(QApplication::translate("OneRoomClientClass", "Send Message", nullptr));
        sendFileBtn->setText(QApplication::translate("OneRoomClientClass", "Send File", nullptr));
        msgHistoryBtn->setText(QApplication::translate("OneRoomClientClass", "Check History", nullptr));
        logOutBtn->setText(QApplication::translate("OneRoomClientClass", "Log Out", nullptr));
        settingBtn->setText(QApplication::translate("OneRoomClientClass", "Settings", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OneRoomClientClass: public Ui_OneRoomClientClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ONEROOMCLIENT_H
