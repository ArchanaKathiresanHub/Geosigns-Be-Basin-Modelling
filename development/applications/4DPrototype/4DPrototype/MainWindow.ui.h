/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created: Mon Oct 19 12:20:26 2015
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
#include <QtGui/QSlider>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
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
    QAction *action_OpenGLInfo;
    QAction *action_RenderAllSlices;
    QAction *action_SwitchProperties;
    QAction *action_ViewAll;
    QAction *action_ViewTop;
    QAction *action_ViewLeft;
    QAction *action_ViewFront;
    QAction *action_ViewBottom;
    QAction *action_ViewRight;
    QAction *action_ViewBack;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitter;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_3;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout;
    QTreeWidget *treeWidget;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_6;
    QTreeWidget *treeWidgetProperties;
    QGroupBox *groupBoxFeatures;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *checkBoxTraps;
    QCheckBox *checkBoxFlowLines;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_7;
    QSlider *sliderVerticalScale;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_4;
    QCheckBox *checkBoxDrawGrid;
    QCheckBox *checkBoxDrawFaces;
    QCheckBox *checkBoxDrawEdges;
    QCheckBox *checkBoxPerspective;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QGridLayout *gridLayout;
    QCheckBox *checkBoxSliceI;
    QSlider *sliderSliceI;
    QCheckBox *checkBoxSliceJ;
    QSlider *sliderSliceJ;
    QLabel *label_8;
    QSlider *snapshotSlider;
    SoQtWrapper *renderWidget;
    QMenuBar *menubar;
    QMenu *menu_File;
    QMenu *menuTest;
    QMenu *menuHelp;
    QMenu *menu_View;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1114, 900);
        action_Open = new QAction(MainWindow);
        action_Open->setObjectName(QString::fromUtf8("action_Open"));
        action_Quit = new QAction(MainWindow);
        action_Quit->setObjectName(QString::fromUtf8("action_Quit"));
        action_RenderAllSnapshots = new QAction(MainWindow);
        action_RenderAllSnapshots->setObjectName(QString::fromUtf8("action_RenderAllSnapshots"));
        action_OpenGLInfo = new QAction(MainWindow);
        action_OpenGLInfo->setObjectName(QString::fromUtf8("action_OpenGLInfo"));
        action_RenderAllSlices = new QAction(MainWindow);
        action_RenderAllSlices->setObjectName(QString::fromUtf8("action_RenderAllSlices"));
        action_SwitchProperties = new QAction(MainWindow);
        action_SwitchProperties->setObjectName(QString::fromUtf8("action_SwitchProperties"));
        action_ViewAll = new QAction(MainWindow);
        action_ViewAll->setObjectName(QString::fromUtf8("action_ViewAll"));
        action_ViewTop = new QAction(MainWindow);
        action_ViewTop->setObjectName(QString::fromUtf8("action_ViewTop"));
        action_ViewLeft = new QAction(MainWindow);
        action_ViewLeft->setObjectName(QString::fromUtf8("action_ViewLeft"));
        action_ViewFront = new QAction(MainWindow);
        action_ViewFront->setObjectName(QString::fromUtf8("action_ViewFront"));
        action_ViewBottom = new QAction(MainWindow);
        action_ViewBottom->setObjectName(QString::fromUtf8("action_ViewBottom"));
        action_ViewRight = new QAction(MainWindow);
        action_ViewRight->setObjectName(QString::fromUtf8("action_ViewRight"));
        action_ViewBack = new QAction(MainWindow);
        action_ViewBack->setObjectName(QString::fromUtf8("action_ViewBack"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout_3 = new QHBoxLayout(centralwidget);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        layoutWidget = new QWidget(splitter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        verticalLayout_3 = new QVBoxLayout(layoutWidget);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(layoutWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setTabPosition(QTabWidget::South);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout = new QVBoxLayout(tab);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        treeWidget = new QTreeWidget(tab);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        treeWidget->setHeaderItem(__qtreewidgetitem);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(treeWidget->sizePolicy().hasHeightForWidth());
        treeWidget->setSizePolicy(sizePolicy);
        treeWidget->header()->setVisible(false);

        verticalLayout->addWidget(treeWidget);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout_6 = new QVBoxLayout(tab_2);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        treeWidgetProperties = new QTreeWidget(tab_2);
        QTreeWidgetItem *__qtreewidgetitem1 = new QTreeWidgetItem();
        __qtreewidgetitem1->setText(3, QString::fromUtf8("4"));
        __qtreewidgetitem1->setText(2, QString::fromUtf8("3"));
        __qtreewidgetitem1->setText(1, QString::fromUtf8("2"));
        __qtreewidgetitem1->setText(0, QString::fromUtf8("1"));
        treeWidgetProperties->setHeaderItem(__qtreewidgetitem1);
        treeWidgetProperties->setObjectName(QString::fromUtf8("treeWidgetProperties"));
        treeWidgetProperties->setColumnCount(4);

        verticalLayout_6->addWidget(treeWidgetProperties);

        tabWidget->addTab(tab_2, QString());

        verticalLayout_3->addWidget(tabWidget);

        groupBoxFeatures = new QGroupBox(layoutWidget);
        groupBoxFeatures->setObjectName(QString::fromUtf8("groupBoxFeatures"));
        verticalLayout_2 = new QVBoxLayout(groupBoxFeatures);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        checkBoxTraps = new QCheckBox(groupBoxFeatures);
        checkBoxTraps->setObjectName(QString::fromUtf8("checkBoxTraps"));

        verticalLayout_2->addWidget(checkBoxTraps);

        checkBoxFlowLines = new QCheckBox(groupBoxFeatures);
        checkBoxFlowLines->setObjectName(QString::fromUtf8("checkBoxFlowLines"));

        verticalLayout_2->addWidget(checkBoxFlowLines);


        verticalLayout_3->addWidget(groupBoxFeatures);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_7 = new QLabel(layoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_2->addWidget(label_7);

        sliderVerticalScale = new QSlider(layoutWidget);
        sliderVerticalScale->setObjectName(QString::fromUtf8("sliderVerticalScale"));
        sliderVerticalScale->setMinimum(0);
        sliderVerticalScale->setMaximum(10);
        sliderVerticalScale->setPageStep(1);
        sliderVerticalScale->setOrientation(Qt::Horizontal);
        sliderVerticalScale->setTickPosition(QSlider::TicksBelow);
        sliderVerticalScale->setTickInterval(1);

        horizontalLayout_2->addWidget(sliderVerticalScale);


        verticalLayout_3->addLayout(horizontalLayout_2);

        groupBox_3 = new QGroupBox(layoutWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_4 = new QVBoxLayout(groupBox_3);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        checkBoxDrawGrid = new QCheckBox(groupBox_3);
        checkBoxDrawGrid->setObjectName(QString::fromUtf8("checkBoxDrawGrid"));

        verticalLayout_4->addWidget(checkBoxDrawGrid);

        checkBoxDrawFaces = new QCheckBox(groupBox_3);
        checkBoxDrawFaces->setObjectName(QString::fromUtf8("checkBoxDrawFaces"));
        checkBoxDrawFaces->setChecked(true);

        verticalLayout_4->addWidget(checkBoxDrawFaces);

        checkBoxDrawEdges = new QCheckBox(groupBox_3);
        checkBoxDrawEdges->setObjectName(QString::fromUtf8("checkBoxDrawEdges"));
        checkBoxDrawEdges->setChecked(true);

        verticalLayout_4->addWidget(checkBoxDrawEdges);

        checkBoxPerspective = new QCheckBox(groupBox_3);
        checkBoxPerspective->setObjectName(QString::fromUtf8("checkBoxPerspective"));
        checkBoxPerspective->setEnabled(true);
        checkBoxPerspective->setChecked(true);

        verticalLayout_4->addWidget(checkBoxPerspective);


        verticalLayout_3->addWidget(groupBox_3);

        groupBox = new QGroupBox(layoutWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        checkBoxSliceI = new QCheckBox(groupBox);
        checkBoxSliceI->setObjectName(QString::fromUtf8("checkBoxSliceI"));

        gridLayout->addWidget(checkBoxSliceI, 0, 0, 1, 1);

        sliderSliceI = new QSlider(groupBox);
        sliderSliceI->setObjectName(QString::fromUtf8("sliderSliceI"));
        sliderSliceI->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(sliderSliceI, 0, 1, 1, 1);

        checkBoxSliceJ = new QCheckBox(groupBox);
        checkBoxSliceJ->setObjectName(QString::fromUtf8("checkBoxSliceJ"));

        gridLayout->addWidget(checkBoxSliceJ, 1, 0, 1, 1);

        sliderSliceJ = new QSlider(groupBox);
        sliderSliceJ->setObjectName(QString::fromUtf8("sliderSliceJ"));
        sliderSliceJ->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(sliderSliceJ, 1, 1, 1, 1);


        horizontalLayout->addLayout(gridLayout);


        verticalLayout_3->addWidget(groupBox);

        label_8 = new QLabel(layoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        verticalLayout_3->addWidget(label_8);

        snapshotSlider = new QSlider(layoutWidget);
        snapshotSlider->setObjectName(QString::fromUtf8("snapshotSlider"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(snapshotSlider->sizePolicy().hasHeightForWidth());
        snapshotSlider->setSizePolicy(sizePolicy1);
        snapshotSlider->setOrientation(Qt::Horizontal);
        snapshotSlider->setTickPosition(QSlider::TicksBelow);

        verticalLayout_3->addWidget(snapshotSlider);

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
        menubar->setGeometry(QRect(0, 0, 1114, 21));
        menu_File = new QMenu(menubar);
        menu_File->setObjectName(QString::fromUtf8("menu_File"));
        menuTest = new QMenu(menubar);
        menuTest->setObjectName(QString::fromUtf8("menuTest"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        menu_View = new QMenu(menubar);
        menu_View->setObjectName(QString::fromUtf8("menu_View"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu_File->menuAction());
        menubar->addAction(menu_View->menuAction());
        menubar->addAction(menuTest->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menu_File->addAction(action_Open);
        menu_File->addSeparator();
        menu_File->addAction(action_Quit);
        menuTest->addAction(action_RenderAllSnapshots);
        menuTest->addAction(action_RenderAllSlices);
        menuTest->addAction(action_SwitchProperties);
        menuHelp->addAction(action_OpenGLInfo);
        menu_View->addAction(action_ViewAll);
        menu_View->addSeparator();
        menu_View->addAction(action_ViewTop);
        menu_View->addAction(action_ViewLeft);
        menu_View->addAction(action_ViewFront);
        menu_View->addAction(action_ViewBottom);
        menu_View->addAction(action_ViewRight);
        menu_View->addAction(action_ViewBack);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        action_Open->setText(QApplication::translate("MainWindow", "&Open...", 0, QApplication::UnicodeUTF8));
        action_Quit->setText(QApplication::translate("MainWindow", "&Quit", 0, QApplication::UnicodeUTF8));
        action_RenderAllSnapshots->setText(QApplication::translate("MainWindow", "Render all snapshots", 0, QApplication::UnicodeUTF8));
        action_OpenGLInfo->setText(QApplication::translate("MainWindow", "OpenGL Info", 0, QApplication::UnicodeUTF8));
        action_RenderAllSlices->setText(QApplication::translate("MainWindow", "Render all slices", 0, QApplication::UnicodeUTF8));
        action_SwitchProperties->setText(QApplication::translate("MainWindow", "Switch properties", 0, QApplication::UnicodeUTF8));
        action_ViewAll->setText(QApplication::translate("MainWindow", "View &all", 0, QApplication::UnicodeUTF8));
        action_ViewTop->setText(QApplication::translate("MainWindow", "&Top", 0, QApplication::UnicodeUTF8));
        action_ViewLeft->setText(QApplication::translate("MainWindow", "&Left", 0, QApplication::UnicodeUTF8));
        action_ViewFront->setText(QApplication::translate("MainWindow", "&Front", 0, QApplication::UnicodeUTF8));
        action_ViewBottom->setText(QApplication::translate("MainWindow", "&Bottom", 0, QApplication::UnicodeUTF8));
        action_ViewRight->setText(QApplication::translate("MainWindow", "&Right", 0, QApplication::UnicodeUTF8));
        action_ViewBack->setText(QApplication::translate("MainWindow", "Bac&k", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "Structure", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "Properties", 0, QApplication::UnicodeUTF8));
        groupBoxFeatures->setTitle(QApplication::translate("MainWindow", "Features", 0, QApplication::UnicodeUTF8));
        checkBoxTraps->setText(QApplication::translate("MainWindow", "Traps", 0, QApplication::UnicodeUTF8));
        checkBoxFlowLines->setText(QApplication::translate("MainWindow", "Flow lines", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("MainWindow", "Vertical scale", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("MainWindow", "Render style", 0, QApplication::UnicodeUTF8));
        checkBoxDrawGrid->setText(QApplication::translate("MainWindow", "Coordinate grid", 0, QApplication::UnicodeUTF8));
        checkBoxDrawFaces->setText(QApplication::translate("MainWindow", "Faces", 0, QApplication::UnicodeUTF8));
        checkBoxDrawEdges->setText(QApplication::translate("MainWindow", "Edges", 0, QApplication::UnicodeUTF8));
        checkBoxPerspective->setText(QApplication::translate("MainWindow", "Perspective", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("MainWindow", "Slices", 0, QApplication::UnicodeUTF8));
        checkBoxSliceI->setText(QApplication::translate("MainWindow", "I", 0, QApplication::UnicodeUTF8));
        checkBoxSliceJ->setText(QApplication::translate("MainWindow", "J", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("MainWindow", "Time", 0, QApplication::UnicodeUTF8));
        menu_File->setTitle(QApplication::translate("MainWindow", "&File", 0, QApplication::UnicodeUTF8));
        menuTest->setTitle(QApplication::translate("MainWindow", "Test", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
        menu_View->setTitle(QApplication::translate("MainWindow", "&View", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // MAINWINDOW_H
