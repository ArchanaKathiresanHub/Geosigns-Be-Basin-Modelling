/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created: Mon 17. Mar 14:12:04 2014
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Inventor/Qt/SoQtWrapper.h>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *action_Open;
    QAction *action_Quit;
    QAction *action_RenderAllSnapshots;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitter;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_4;
    QTreeWidget *treeWidget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_7;
    QSlider *sliderVerticalScale;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QRadioButton *radioButtonFormations;
    QRadioButton *radioButtonSurfaces;
    QRadioButton *radioButtonReservoirs;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QCheckBox *checkBoxDrawFaces;
    QCheckBox *checkBoxDrawEdges;
    QGroupBox *groupBoxROI;
    QGridLayout *gridLayout_2;
    QCheckBox *checkBoxROI;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label;
    QSlider *sliderMinI;
    QSlider *sliderMinK;
    QSlider *sliderMaxK;
    QLabel *label_2;
    QSlider *sliderMaxI;
    QLabel *label_3;
    QSlider *sliderMinJ;
    QLabel *label_4;
    QSlider *sliderMaxJ;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QRadioButton *radioButtonSkin;
    QRadioButton *radioButtonSlices;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QGridLayout *gridLayout;
    QLabel *labelSliceI;
    QSlider *sliderSliceI;
    QLabel *labelSliceJ;
    QSlider *sliderSliceJ;
    QRadioButton *radioButtonCrossSection;
    QLabel *label_8;
    QSlider *snapshotSlider;
    SoQtWrapper *renderWidget;
    QMenuBar *menubar;
    QMenu *menu_File;
    QMenu *menuTest;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(857, 833);
        action_Open = new QAction(MainWindow);
        action_Open->setObjectName(QString::fromUtf8("action_Open"));
        action_Quit = new QAction(MainWindow);
        action_Quit->setObjectName(QString::fromUtf8("action_Quit"));
        action_RenderAllSnapshots = new QAction(MainWindow);
        action_RenderAllSnapshots->setObjectName(QString::fromUtf8("action_RenderAllSnapshots"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout_3 = new QHBoxLayout(centralwidget);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        layoutWidget = new QWidget(splitter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        verticalLayout_4 = new QVBoxLayout(layoutWidget);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        treeWidget = new QTreeWidget(layoutWidget);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget->setHeaderItem(__qtreewidgetitem);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(treeWidget->sizePolicy().hasHeightForWidth());
        treeWidget->setSizePolicy(sizePolicy);

        verticalLayout_4->addWidget(treeWidget);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_2->addWidget(label_7);

        sliderVerticalScale = new QSlider(layoutWidget);
        sliderVerticalScale->setObjectName(QString::fromUtf8("sliderVerticalScale"));
        sliderVerticalScale->setMinimum(1);
        sliderVerticalScale->setMaximum(5);
        sliderVerticalScale->setPageStep(1);
        sliderVerticalScale->setOrientation(Qt::Horizontal);
        sliderVerticalScale->setTickPosition(QSlider::TicksBelow);
        sliderVerticalScale->setTickInterval(1);

        horizontalLayout_2->addWidget(sliderVerticalScale);


        verticalLayout_4->addLayout(horizontalLayout_2);

        groupBox_2 = new QGroupBox(layoutWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        radioButtonFormations = new QRadioButton(groupBox_2);
        radioButtonFormations->setObjectName(QString::fromUtf8("radioButtonFormations"));
        radioButtonFormations->setChecked(true);

        verticalLayout->addWidget(radioButtonFormations);

        radioButtonSurfaces = new QRadioButton(groupBox_2);
        radioButtonSurfaces->setObjectName(QString::fromUtf8("radioButtonSurfaces"));

        verticalLayout->addWidget(radioButtonSurfaces);

        radioButtonReservoirs = new QRadioButton(groupBox_2);
        radioButtonReservoirs->setObjectName(QString::fromUtf8("radioButtonReservoirs"));

        verticalLayout->addWidget(radioButtonReservoirs);


        verticalLayout_4->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(layoutWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        checkBoxDrawFaces = new QCheckBox(groupBox_3);
        checkBoxDrawFaces->setObjectName(QString::fromUtf8("checkBoxDrawFaces"));
        checkBoxDrawFaces->setChecked(true);

        verticalLayout_3->addWidget(checkBoxDrawFaces);

        checkBoxDrawEdges = new QCheckBox(groupBox_3);
        checkBoxDrawEdges->setObjectName(QString::fromUtf8("checkBoxDrawEdges"));
        checkBoxDrawEdges->setChecked(true);

        verticalLayout_3->addWidget(checkBoxDrawEdges);


        verticalLayout_4->addWidget(groupBox_3);

        groupBoxROI = new QGroupBox(layoutWidget);
        groupBoxROI->setObjectName(QString::fromUtf8("groupBoxROI"));
        gridLayout_2 = new QGridLayout(groupBoxROI);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        checkBoxROI = new QCheckBox(groupBoxROI);
        checkBoxROI->setObjectName(QString::fromUtf8("checkBoxROI"));

        gridLayout_2->addWidget(checkBoxROI, 0, 0, 1, 2);

        label_5 = new QLabel(groupBoxROI);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_2->addWidget(label_5, 0, 2, 1, 1);

        label_6 = new QLabel(groupBoxROI);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout_2->addWidget(label_6, 0, 3, 1, 1);

        label = new QLabel(groupBoxROI);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 1, 0, 1, 1);

        sliderMinI = new QSlider(groupBoxROI);
        sliderMinI->setObjectName(QString::fromUtf8("sliderMinI"));
        sliderMinI->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(sliderMinI, 1, 1, 1, 1);

        sliderMinK = new QSlider(groupBoxROI);
        sliderMinK->setObjectName(QString::fromUtf8("sliderMinK"));
        sliderMinK->setOrientation(Qt::Vertical);

        gridLayout_2->addWidget(sliderMinK, 1, 2, 4, 1);

        sliderMaxK = new QSlider(groupBoxROI);
        sliderMaxK->setObjectName(QString::fromUtf8("sliderMaxK"));
        sliderMaxK->setOrientation(Qt::Vertical);

        gridLayout_2->addWidget(sliderMaxK, 1, 3, 4, 1);

        label_2 = new QLabel(groupBoxROI);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 2, 0, 1, 1);

        sliderMaxI = new QSlider(groupBoxROI);
        sliderMaxI->setObjectName(QString::fromUtf8("sliderMaxI"));
        sliderMaxI->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(sliderMaxI, 2, 1, 1, 1);

        label_3 = new QLabel(groupBoxROI);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout_2->addWidget(label_3, 3, 0, 1, 1);

        sliderMinJ = new QSlider(groupBoxROI);
        sliderMinJ->setObjectName(QString::fromUtf8("sliderMinJ"));
        sliderMinJ->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(sliderMinJ, 3, 1, 1, 1);

        label_4 = new QLabel(groupBoxROI);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_2->addWidget(label_4, 4, 0, 1, 1);

        sliderMaxJ = new QSlider(groupBoxROI);
        sliderMaxJ->setObjectName(QString::fromUtf8("sliderMaxJ"));
        sliderMaxJ->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(sliderMaxJ, 4, 1, 1, 1);


        verticalLayout_4->addWidget(groupBoxROI);

        groupBox = new QGroupBox(layoutWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        radioButtonSkin = new QRadioButton(groupBox);
        radioButtonSkin->setObjectName(QString::fromUtf8("radioButtonSkin"));
        radioButtonSkin->setChecked(true);

        verticalLayout_2->addWidget(radioButtonSkin);

        radioButtonSlices = new QRadioButton(groupBox);
        radioButtonSlices->setObjectName(QString::fromUtf8("radioButtonSlices"));

        verticalLayout_2->addWidget(radioButtonSlices);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        labelSliceI = new QLabel(groupBox);
        labelSliceI->setObjectName(QString::fromUtf8("labelSliceI"));

        gridLayout->addWidget(labelSliceI, 0, 0, 1, 1);

        sliderSliceI = new QSlider(groupBox);
        sliderSliceI->setObjectName(QString::fromUtf8("sliderSliceI"));
        sliderSliceI->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(sliderSliceI, 0, 1, 1, 1);

        labelSliceJ = new QLabel(groupBox);
        labelSliceJ->setObjectName(QString::fromUtf8("labelSliceJ"));

        gridLayout->addWidget(labelSliceJ, 1, 0, 1, 1);

        sliderSliceJ = new QSlider(groupBox);
        sliderSliceJ->setObjectName(QString::fromUtf8("sliderSliceJ"));
        sliderSliceJ->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(sliderSliceJ, 1, 1, 1, 1);


        horizontalLayout->addLayout(gridLayout);


        verticalLayout_2->addLayout(horizontalLayout);

        radioButtonCrossSection = new QRadioButton(groupBox);
        radioButtonCrossSection->setObjectName(QString::fromUtf8("radioButtonCrossSection"));

        verticalLayout_2->addWidget(radioButtonCrossSection);


        verticalLayout_4->addWidget(groupBox);

        label_8 = new QLabel(layoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        verticalLayout_4->addWidget(label_8);

        snapshotSlider = new QSlider(layoutWidget);
        snapshotSlider->setObjectName(QString::fromUtf8("snapshotSlider"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(snapshotSlider->sizePolicy().hasHeightForWidth());
        snapshotSlider->setSizePolicy(sizePolicy1);
        snapshotSlider->setOrientation(Qt::Horizontal);
        snapshotSlider->setTickPosition(QSlider::TicksBelow);

        verticalLayout_4->addWidget(snapshotSlider);

        splitter->addWidget(layoutWidget);
        renderWidget = new SoQtWrapper(splitter);
        renderWidget->setObjectName(QString::fromUtf8("renderWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(renderWidget->sizePolicy().hasHeightForWidth());
        renderWidget->setSizePolicy(sizePolicy2);
        splitter->addWidget(renderWidget);

        horizontalLayout_3->addWidget(splitter);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 857, 21));
        menu_File = new QMenu(menubar);
        menu_File->setObjectName(QString::fromUtf8("menu_File"));
        menuTest = new QMenu(menubar);
        menuTest->setObjectName(QString::fromUtf8("menuTest"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu_File->menuAction());
        menubar->addAction(menuTest->menuAction());
        menu_File->addAction(action_Open);
        menu_File->addSeparator();
        menu_File->addAction(action_Quit);
        menuTest->addAction(action_RenderAllSnapshots);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        action_Open->setText(QApplication::translate("MainWindow", "&Open...", 0, QApplication::UnicodeUTF8));
        action_Quit->setText(QApplication::translate("MainWindow", "&Quit", 0, QApplication::UnicodeUTF8));
        action_RenderAllSnapshots->setText(QApplication::translate("MainWindow", "Render all snapshots", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("MainWindow", "Vertical scale", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "Mesh", 0, QApplication::UnicodeUTF8));
        radioButtonFormations->setText(QApplication::translate("MainWindow", "Formations", 0, QApplication::UnicodeUTF8));
        radioButtonSurfaces->setText(QApplication::translate("MainWindow", "Surfaces", 0, QApplication::UnicodeUTF8));
        radioButtonReservoirs->setText(QApplication::translate("MainWindow", "Reservoirs", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("MainWindow", "Render style", 0, QApplication::UnicodeUTF8));
        checkBoxDrawFaces->setText(QApplication::translate("MainWindow", "Faces", 0, QApplication::UnicodeUTF8));
        checkBoxDrawEdges->setText(QApplication::translate("MainWindow", "Edges", 0, QApplication::UnicodeUTF8));
        groupBoxROI->setTitle(QApplication::translate("MainWindow", "ROI", 0, QApplication::UnicodeUTF8));
        checkBoxROI->setText(QApplication::translate("MainWindow", "Enable", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("MainWindow", "minK", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MainWindow", "maxK", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "minI", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "maxI", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "minJ", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "maxJ", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("MainWindow", "Rendering", 0, QApplication::UnicodeUTF8));
        radioButtonSkin->setText(QApplication::translate("MainWindow", "Skin", 0, QApplication::UnicodeUTF8));
        radioButtonSlices->setText(QApplication::translate("MainWindow", "Slices", 0, QApplication::UnicodeUTF8));
        labelSliceI->setText(QApplication::translate("MainWindow", "I", 0, QApplication::UnicodeUTF8));
        labelSliceJ->setText(QApplication::translate("MainWindow", "J", 0, QApplication::UnicodeUTF8));
        radioButtonCrossSection->setText(QApplication::translate("MainWindow", "Cross section", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("MainWindow", "Time", 0, QApplication::UnicodeUTF8));
        menu_File->setTitle(QApplication::translate("MainWindow", "&File", 0, QApplication::UnicodeUTF8));
        menuTest->setTitle(QApplication::translate("MainWindow", "Test", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // MAINWINDOW_H
