/********************************************************************************
** Form generated from reading UI file 'GLInfoDialog.ui'
**
** Created: Tue 6. May 10:33:56 2014
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef GLINFODIALOG_H
#define GLINFODIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_Dialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label;
    QLabel *labelVendor;
    QLabel *label_2;
    QLabel *labelVersion;
    QLabel *label_3;
    QLabel *labelRenderer;
    QLabel *label_7;
    QListWidget *listExtensions;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QString::fromUtf8("Dialog"));
        Dialog->resize(325, 410);
        verticalLayout_2 = new QVBoxLayout(Dialog);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(Dialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        labelVendor = new QLabel(groupBox);
        labelVendor->setObjectName(QString::fromUtf8("labelVendor"));

        gridLayout->addWidget(labelVendor, 0, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        labelVersion = new QLabel(groupBox);
        labelVersion->setObjectName(QString::fromUtf8("labelVersion"));

        gridLayout->addWidget(labelVersion, 1, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        labelRenderer = new QLabel(groupBox);
        labelRenderer->setObjectName(QString::fromUtf8("labelRenderer"));

        gridLayout->addWidget(labelRenderer, 2, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        verticalLayout->addWidget(label_7);

        listExtensions = new QListWidget(groupBox);
        listExtensions->setObjectName(QString::fromUtf8("listExtensions"));

        verticalLayout->addWidget(listExtensions);


        verticalLayout_2->addWidget(groupBox);

        buttonBox = new QDialogButtonBox(Dialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);


        retranslateUi(Dialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), Dialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), Dialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QDialog *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "OpenGL Info", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("Dialog", "Info", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Dialog", "Vendor", 0, QApplication::UnicodeUTF8));
        labelVendor->setText(QApplication::translate("Dialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("Dialog", "Version", 0, QApplication::UnicodeUTF8));
        labelVersion->setText(QApplication::translate("Dialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("Dialog", "Renderer", 0, QApplication::UnicodeUTF8));
        labelRenderer->setText(QApplication::translate("Dialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("Dialog", "Extensions", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // GLINFODIALOG_H
