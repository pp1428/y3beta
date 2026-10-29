/********************************************************************************
** Form generated from reading UI file 'clinfo.ui'
**
** Created: Sat Nov 13 12:03:39 2010
**      by: Qt User Interface Compiler version 4.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLINFO_H
#define UI_CLINFO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QToolBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_clinfo
{
public:
    QAction *actionNew_Analysis;
    QAction *actionContents;
    QAction *actionCredits;
    QAction *actionDatabase;
    QAction *actionSettings;
    QAction *actionDatabase_2;
    QAction *actionUpdate_Database;
    QWidget *centralWidget;
    QComboBox *computeUnits;
    QToolBox *toolBox;
    QWidget *page_3;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *modelTextbox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QLineEdit *coresTextbox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QLineEdit *gflopsTextbox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_5;
    QLineEdit *openclTextbox;
    QLabel *image;
    QLabel *label;
    QMenuBar *menuBar;
    QMenu *menuSystem;
    QMenu *menuCredits;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *clinfo)
    {
        if (clinfo->objectName().isEmpty())
            clinfo->setObjectName(QString::fromUtf8("clinfo"));
        clinfo->resize(390, 280);
        clinfo->setMinimumSize(QSize(390, 280));
        clinfo->setMaximumSize(QSize(390, 280));
        actionNew_Analysis = new QAction(clinfo);
        actionNew_Analysis->setObjectName(QString::fromUtf8("actionNew_Analysis"));
        actionContents = new QAction(clinfo);
        actionContents->setObjectName(QString::fromUtf8("actionContents"));
        actionCredits = new QAction(clinfo);
        actionCredits->setObjectName(QString::fromUtf8("actionCredits"));
        actionDatabase = new QAction(clinfo);
        actionDatabase->setObjectName(QString::fromUtf8("actionDatabase"));
        actionSettings = new QAction(clinfo);
        actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
        actionDatabase_2 = new QAction(clinfo);
        actionDatabase_2->setObjectName(QString::fromUtf8("actionDatabase_2"));
        actionUpdate_Database = new QAction(clinfo);
        actionUpdate_Database->setObjectName(QString::fromUtf8("actionUpdate_Database"));
        actionUpdate_Database->setEnabled(false);
        centralWidget = new QWidget(clinfo);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        computeUnits = new QComboBox(centralWidget);
        computeUnits->setObjectName(QString::fromUtf8("computeUnits"));
        computeUnits->setGeometry(QRect(10, 10, 371, 21));
        toolBox = new QToolBox(centralWidget);
        toolBox->setObjectName(QString::fromUtf8("toolBox"));
        toolBox->setGeometry(QRect(10, 30, 371, 171));
        page_3 = new QWidget();
        page_3->setObjectName(QString::fromUtf8("page_3"));
        page_3->setGeometry(QRect(0, 0, 371, 140));
        widget = new QWidget(page_3);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(150, 0, 216, 136));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(60, 0));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_2);

        modelTextbox = new QLineEdit(widget);
        modelTextbox->setObjectName(QString::fromUtf8("modelTextbox"));
        modelTextbox->setReadOnly(true);

        horizontalLayout->addWidget(modelTextbox);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(widget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(60, 0));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(label_3);

        coresTextbox = new QLineEdit(widget);
        coresTextbox->setObjectName(QString::fromUtf8("coresTextbox"));
        coresTextbox->setReadOnly(true);

        horizontalLayout_2->addWidget(coresTextbox);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_4 = new QLabel(widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(60, 0));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_4);

        gflopsTextbox = new QLineEdit(widget);
        gflopsTextbox->setObjectName(QString::fromUtf8("gflopsTextbox"));
        gflopsTextbox->setReadOnly(true);

        horizontalLayout_3->addWidget(gflopsTextbox);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_5 = new QLabel(widget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setMinimumSize(QSize(60, 0));
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label_5);

        openclTextbox = new QLineEdit(widget);
        openclTextbox->setObjectName(QString::fromUtf8("openclTextbox"));
        openclTextbox->setReadOnly(true);

        horizontalLayout_4->addWidget(openclTextbox);


        verticalLayout->addLayout(horizontalLayout_4);

        image = new QLabel(page_3);
        image->setObjectName(QString::fromUtf8("image"));
        image->setGeometry(QRect(10, 20, 121, 101));
        image->setFrameShape(QFrame::Box);
        image->setPixmap(QPixmap(QString::fromUtf8("../Desktop/clinfoIMG/radeon.jpg")));
        image->setScaledContents(true);
        toolBox->addItem(page_3, QString::fromUtf8("Compute Unit"));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 200, 271, 17));
        clinfo->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(clinfo);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 390, 27));
        menuSystem = new QMenu(menuBar);
        menuSystem->setObjectName(QString::fromUtf8("menuSystem"));
        menuCredits = new QMenu(menuBar);
        menuCredits->setObjectName(QString::fromUtf8("menuCredits"));
        clinfo->setMenuBar(menuBar);
        mainToolBar = new QToolBar(clinfo);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        clinfo->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(clinfo);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        clinfo->setStatusBar(statusBar);

        menuBar->addAction(menuSystem->menuAction());
        menuBar->addAction(menuCredits->menuAction());
        menuSystem->addAction(actionNew_Analysis);
        menuSystem->addSeparator();
        menuSystem->addAction(actionUpdate_Database);
        menuCredits->addAction(actionCredits);

        retranslateUi(clinfo);

        toolBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(clinfo);
    } // setupUi

    void retranslateUi(QMainWindow *clinfo)
    {
        clinfo->setWindowTitle(QApplication::translate("clinfo", "clinfo", 0, QApplication::UnicodeUTF8));
        actionNew_Analysis->setText(QApplication::translate("clinfo", "New Analysis", 0, QApplication::UnicodeUTF8));
        actionContents->setText(QApplication::translate("clinfo", "Contents", 0, QApplication::UnicodeUTF8));
        actionCredits->setText(QApplication::translate("clinfo", "Credits", 0, QApplication::UnicodeUTF8));
        actionDatabase->setText(QApplication::translate("clinfo", "Database", 0, QApplication::UnicodeUTF8));
        actionSettings->setText(QApplication::translate("clinfo", "Settings", 0, QApplication::UnicodeUTF8));
        actionDatabase_2->setText(QApplication::translate("clinfo", "Database", 0, QApplication::UnicodeUTF8));
        actionUpdate_Database->setText(QApplication::translate("clinfo", "Update Database", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("clinfo", "Model", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("clinfo", "Cores*", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("clinfo", "GFlops", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("clinfo", "OpenCL", 0, QApplication::UnicodeUTF8));
        image->setText(QString());
        toolBox->setItemText(toolBox->indexOf(page_3), QApplication::translate("clinfo", "Compute Unit", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("clinfo", "*Cores/Core config [ shaders : tmu : rop ]", 0, QApplication::UnicodeUTF8));
        menuSystem->setTitle(QApplication::translate("clinfo", "System", 0, QApplication::UnicodeUTF8));
        menuCredits->setTitle(QApplication::translate("clinfo", "Help", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class clinfo: public Ui_clinfo {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLINFO_H
