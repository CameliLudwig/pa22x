/********************************************************************************
** Form generated from reading UI file 'pa22x.ui'
**
** Created by: Qt User Interface Compiler version 5.12.9
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PA22X_H
#define UI_PA22X_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <qcustomplot.h>
#include "peimageview.h"
#include "ruler.h"

QT_BEGIN_NAMESPACE

class Ui_pa22x
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout_6;
    QSpacerItem *verticalSpacer_10;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_15;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_6;
    QLabel *PEChanneSelectionLabel;
    QSpacerItem *horizontalSpacer;
    QComboBox *PEChannelSelection;
    QSpacerItem *verticalSpacer_6;
    QHBoxLayout *horizontalLayout_10;
    QLabel *PEDualModeLabel;
    QSpacerItem *horizontalSpacer_2;
    QComboBox *PEDualModeSelection;
    QSpacerItem *verticalSpacer_8;
    QHBoxLayout *horizontalLayout_7;
    QLabel *PEGainLabel;
    QSpacerItem *horizontalSpacer_3;
    QDoubleSpinBox *PEGainInput;
    QSpacerItem *horizontalSpacer_6;
    QComboBox *PEGainSetp;
    QSpacerItem *verticalSpacer_9;
    QHBoxLayout *horizontalLayout_8;
    QLabel *PERangeLabel;
    QSpacerItem *horizontalSpacer_4;
    QDoubleSpinBox *PERangeInput;
    QSpacerItem *horizontalSpacer_7;
    QComboBox *PERangeStep;
    QSpacerItem *verticalSpacer_7;
    QHBoxLayout *horizontalLayout_9;
    QLabel *PEDelayLabel;
    QSpacerItem *horizontalSpacer_5;
    QDoubleSpinBox *PEDelayInput;
    QSpacerItem *horizontalSpacer_8;
    QComboBox *PEDelayStep;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_3;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_11;
    QPushButton *WaSaveWave;
    QSpacerItem *horizontalSpacer_9;
    QPushButton *pushButton_mode;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *PESaveParameters;
    QSpacerItem *horizontalSpacer_10;
    QPushButton *STSaveWave;
    QSpacerItem *verticalSpacer_3;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *HisData;
    QSpacerItem *horizontalSpacer_11;
    QPushButton *PELoadParameters;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_21;
    QLabel *label_aveD90;
    QLabel *label_22;
    QLabel *label_avepass;
    QVBoxLayout *verticalLayout_2;
    QWidget *PEAWaveBackgroundView;
    QGridLayout *gridLayout_14;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout_4;
    QHBoxLayout *horizontalLayout_5;
    QGridLayout *gridLayout;
    Ruler *PERulerVerView;
    PEImageView *PEAWaveView;
    Ruler *PERulerHorView;
    QCustomPlot *customPlot4;
    QHBoxLayout *horizontalLayout_16;
    QComboBox *deviceSelection;
    QSpacerItem *horizontalSpacer_14;
    QLabel *label_19;
    QLineEdit *lineEdit_Desk;
    QSpacerItem *horizontalSpacer_15;
    QLabel *PEChannelCountLabel;
    QComboBox *PEChannelCountSelection;
    QGroupBox *groupBox_6;
    QGridLayout *gridLayout_10;
    QVBoxLayout *verticalLayout_6;
    QHBoxLayout *horizontalLayout_18;
    QLabel *label_20;
    QDoubleSpinBox *spb_specfmin;
    QLabel *label_23;
    QDoubleSpinBox *spb_specfmax;
    QLabel *label_24;
    QDoubleSpinBox *spb_histime;
    QLabel *label_25;
    QSpinBox *sb_avenum;
    QHBoxLayout *horizontalLayout_19;
    QCustomPlot *customPlot3;
    QWidget *widget_28;
    QGridLayout *gridLayout_2;
    QLabel *label_26;
    QLabel *label_27;
    QSpinBox *spb_time;
    QDoubleSpinBox *dspb_width;
    QWidget *widget_30;
    QGridLayout *gridLayout_11;
    QLabel *label_d90;
    QLabel *label_Ad90;
    QLabel *label_pass2;
    QLabel *label_d50;
    QLabel *label_Ad50;
    QLabel *label_Pass;
    QHBoxLayout *horizontalLayout;
    QLabel *IPAddressLabel;
    QLineEdit *IPAddressInput;
    QSpinBox *PEPRFInput;
    QPushButton *connectButton;
    QPushButton *simulateDetectButton;
    QPushButton *PESaveWave;
    QPushButton *SpecAnaly;
    QPushButton *CloseWaveSave;
    QPushButton *Inserve;
    QWidget *widget_21;
    QGridLayout *gridLayout_5;
    QLabel *label_headerIcon;
    QLabel *label_7;
    QSpacerItem *verticalSpacer_11;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *pa22x)
    {
        if (pa22x->objectName().isEmpty())
            pa22x->setObjectName(QString::fromUtf8("pa22x"));
        pa22x->setEnabled(true);
        pa22x->resize(2018, 1002);
        pa22x->setMouseTracking(false);
        centralwidget = new QWidget(pa22x);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout_6 = new QGridLayout(centralwidget);
        gridLayout_6->setSpacing(6);
        gridLayout_6->setContentsMargins(11, 11, 11, 11);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        verticalSpacer_10 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_6->addItem(verticalSpacer_10, 1, 0, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_15 = new QGridLayout(groupBox);
        gridLayout_15->setSpacing(6);
        gridLayout_15->setContentsMargins(11, 11, 11, 11);
        gridLayout_15->setObjectName(QString::fromUtf8("gridLayout_15"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        PEChanneSelectionLabel = new QLabel(groupBox);
        PEChanneSelectionLabel->setObjectName(QString::fromUtf8("PEChanneSelectionLabel"));
        PEChanneSelectionLabel->setMinimumSize(QSize(150, 40));
        PEChanneSelectionLabel->setMaximumSize(QSize(150, 40));
        QFont font;
        font.setPointSize(20);
        PEChanneSelectionLabel->setFont(font);
        PEChanneSelectionLabel->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_6->addWidget(PEChanneSelectionLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer);

        PEChannelSelection = new QComboBox(groupBox);
        PEChannelSelection->setObjectName(QString::fromUtf8("PEChannelSelection"));
        PEChannelSelection->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PEChannelSelection->sizePolicy().hasHeightForWidth());
        PEChannelSelection->setSizePolicy(sizePolicy);
        PEChannelSelection->setMinimumSize(QSize(180, 40));
        PEChannelSelection->setMaximumSize(QSize(180, 40));
        PEChannelSelection->setFont(font);
        PEChannelSelection->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	color: black;\n"
"    border: 1px solid black\n"
"    background: rgb(255,255,255);\n"
"	selection-color: black;\n"
"    selection-background-color: black;\n"
"	height: 20px;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"	subcontrol-position: top right;\n"
"	border-left: 0px solid lightgray;\n"
"	width: 22px;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"    image: url(:/images/down-arrow.png);\n"
"    width: 16px;\n"
"    height: 16px;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView {\n"
"    border: 1px solid lightgray;\n"
"	color: black;\n"
"    background: rgb(255,255,255);\n"
"	padding: 0px 0px 0px 0px;\n"
"	outline: none;\n"
"	selection-color: black;\n"
"    selection-background-color: lightgray;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView::item \n"
"{\n"
"	margin-top: 2px;\n"
"}\n"
""));

        horizontalLayout_6->addWidget(PEChannelSelection);


        verticalLayout->addLayout(horizontalLayout_6);

        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_6);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setSpacing(6);
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        PEDualModeLabel = new QLabel(groupBox);
        PEDualModeLabel->setObjectName(QString::fromUtf8("PEDualModeLabel"));
        PEDualModeLabel->setMinimumSize(QSize(190, 40));
        PEDualModeLabel->setMaximumSize(QSize(190, 40));
        PEDualModeLabel->setFont(font);
        PEDualModeLabel->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_10->addWidget(PEDualModeLabel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_10->addItem(horizontalSpacer_2);

        PEDualModeSelection = new QComboBox(groupBox);
        PEDualModeSelection->setObjectName(QString::fromUtf8("PEDualModeSelection"));
        PEDualModeSelection->setMinimumSize(QSize(150, 40));
        PEDualModeSelection->setMaximumSize(QSize(150, 40));
        PEDualModeSelection->setFont(font);
        PEDualModeSelection->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	color: black;\n"
"    border: 1px solid black\n"
"    background: rgb(255,255,255);\n"
"	selection-color: black;\n"
"    selection-background-color: black;\n"
"	height: 20px;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"	subcontrol-position: top right;\n"
"	border-left: 0px solid lightgray;\n"
"	width: 22px;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"    image: url(:/images/down-arrow.png);\n"
"    width: 16px;\n"
"    height: 16px;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView {\n"
"    border: 1px solid lightgray;\n"
"	color: black;\n"
"    background: rgb(255,255,255);\n"
"	padding: 0px 0px 0px 0px;\n"
"	outline: none;\n"
"	selection-color: black;\n"
"    selection-background-color: lightgray;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView::item \n"
"{\n"
"	margin-top: 2px;\n"
"}\n"
""));

        horizontalLayout_10->addWidget(PEDualModeSelection);


        verticalLayout->addLayout(horizontalLayout_10);

        verticalSpacer_8 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_8);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        PEGainLabel = new QLabel(groupBox);
        PEGainLabel->setObjectName(QString::fromUtf8("PEGainLabel"));
        PEGainLabel->setMinimumSize(QSize(190, 40));
        PEGainLabel->setMaximumSize(QSize(190, 40));
        PEGainLabel->setFont(font);
        PEGainLabel->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_7->addWidget(PEGainLabel);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_3);

        PEGainInput = new QDoubleSpinBox(groupBox);
        PEGainInput->setObjectName(QString::fromUtf8("PEGainInput"));
        PEGainInput->setEnabled(true);
        PEGainInput->setMinimumSize(QSize(120, 40));
        PEGainInput->setMaximumSize(QSize(120, 40));
        PEGainInput->setFont(font);
        PEGainInput->setStyleSheet(QString::fromUtf8("QDoubleSpinBox {\n"
"    border: 1px solid darkgray;\n"
"	border-color: darkgray darkgray darkgray darkgray; \n"
"	color: black;\n"
"	selection-color: black;\n"
"    background: white;\n"
"    selection-background-color: white;\n"
"	height: 18px;\n"
"}\n"
""));
        PEGainInput->setReadOnly(false);
        PEGainInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
        PEGainInput->setDecimals(1);

        horizontalLayout_7->addWidget(PEGainInput);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_6);

        PEGainSetp = new QComboBox(groupBox);
        PEGainSetp->setObjectName(QString::fromUtf8("PEGainSetp"));
        PEGainSetp->setMinimumSize(QSize(100, 40));
        PEGainSetp->setMaximumSize(QSize(100, 40));
        PEGainSetp->setFont(font);
        PEGainSetp->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	color: black;\n"
"    border: 1px solid black\n"
"    background: rgb(255,255,255);\n"
"	selection-color: black;\n"
"    selection-background-color: black;\n"
"	height: 20px;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"	subcontrol-position: top right;\n"
"	border-left: 0px solid lightgray;\n"
"	width: 22px;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"    image: url(:/images/down-arrow.png);\n"
"    width: 16px;\n"
"    height: 16px;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView {\n"
"    border: 1px solid lightgray;\n"
"	color: black;\n"
"    background: rgb(255,255,255);\n"
"	padding: 0px 0px 0px 0px;\n"
"	outline: none;\n"
"	selection-color: black;\n"
"    selection-background-color: lightgray;\n"
"}\n"
""));

        horizontalLayout_7->addWidget(PEGainSetp);


        verticalLayout->addLayout(horizontalLayout_7);

        verticalSpacer_9 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_9);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        PERangeLabel = new QLabel(groupBox);
        PERangeLabel->setObjectName(QString::fromUtf8("PERangeLabel"));
        PERangeLabel->setMinimumSize(QSize(190, 40));
        PERangeLabel->setMaximumSize(QSize(190, 40));
        PERangeLabel->setFont(font);
        PERangeLabel->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_8->addWidget(PERangeLabel);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_4);

        PERangeInput = new QDoubleSpinBox(groupBox);
        PERangeInput->setObjectName(QString::fromUtf8("PERangeInput"));
        PERangeInput->setEnabled(true);
        PERangeInput->setMinimumSize(QSize(120, 40));
        PERangeInput->setMaximumSize(QSize(120, 40));
        PERangeInput->setFont(font);
        PERangeInput->setStyleSheet(QString::fromUtf8("QDoubleSpinBox {\n"
"    border: 1px solid darkgray;\n"
"	border-color: darkgray darkgray darkgray darkgray; \n"
"	color: black;\n"
"	selection-color: black;\n"
"    background: rgb(255,255,255);\n"
"    selection-background-color: white;\n"
"	height: 18px;\n"
"}\n"
""));
        PERangeInput->setReadOnly(false);
        PERangeInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
        PERangeInput->setDecimals(1);
        PERangeInput->setMinimum(15.000000000000000);
        PERangeInput->setMaximum(10000.000000000000000);

        horizontalLayout_8->addWidget(PERangeInput);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_7);

        PERangeStep = new QComboBox(groupBox);
        PERangeStep->setObjectName(QString::fromUtf8("PERangeStep"));
        PERangeStep->setMinimumSize(QSize(100, 40));
        PERangeStep->setMaximumSize(QSize(100, 40));
        PERangeStep->setFont(font);
        PERangeStep->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	color: black;\n"
"    border: 1px solid black\n"
"    background: rgb(255,255,255);\n"
"	selection-color: black;\n"
"    selection-background-color: black;\n"
"	height: 20px;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"	subcontrol-position: top right;\n"
"	border-left: 0px solid lightgray;\n"
"	width: 22px;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"    image: url(:/images/down-arrow.png);\n"
"    width: 16px;\n"
"    height: 16px;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView {\n"
"    border: 1px solid lightgray;\n"
"	color: black;\n"
"    background: rgb(255,255,255);\n"
"	padding: 0px 0px 0px 0px;\n"
"	outline: none;\n"
"	selection-color: black;\n"
"    selection-background-color: lightgray;\n"
"}\n"
""));

        horizontalLayout_8->addWidget(PERangeStep);


        verticalLayout->addLayout(horizontalLayout_8);

        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_7);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        PEDelayLabel = new QLabel(groupBox);
        PEDelayLabel->setObjectName(QString::fromUtf8("PEDelayLabel"));
        PEDelayLabel->setMinimumSize(QSize(190, 40));
        PEDelayLabel->setMaximumSize(QSize(190, 40));
        PEDelayLabel->setFont(font);
        PEDelayLabel->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_9->addWidget(PEDelayLabel);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_5);

        PEDelayInput = new QDoubleSpinBox(groupBox);
        PEDelayInput->setObjectName(QString::fromUtf8("PEDelayInput"));
        PEDelayInput->setEnabled(true);
        PEDelayInput->setMinimumSize(QSize(120, 40));
        PEDelayInput->setMaximumSize(QSize(120, 40));
        PEDelayInput->setFont(font);
        PEDelayInput->setStyleSheet(QString::fromUtf8("QDoubleSpinBox {\n"
"    border: 1px solid darkgray;\n"
"	border-color: darkgray darkgray darkgray darkgray; \n"
"	color: black;\n"
"	selection-color: black;\n"
"    background: rgb(255,255,255);\n"
"    selection-background-color: white;\n"
"	height: 18px;\n"
"}\n"
""));
        PEDelayInput->setReadOnly(false);
        PEDelayInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
        PEDelayInput->setDecimals(1);

        horizontalLayout_9->addWidget(PEDelayInput);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_8);

        PEDelayStep = new QComboBox(groupBox);
        PEDelayStep->setObjectName(QString::fromUtf8("PEDelayStep"));
        PEDelayStep->setMinimumSize(QSize(100, 40));
        PEDelayStep->setMaximumSize(QSize(100, 40));
        PEDelayStep->setFont(font);
        PEDelayStep->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	color: black;\n"
"    border: 1px solid black\n"
"    background: rgb(255,255,255);\n"
"	selection-color: black;\n"
"    selection-background-color: black;\n"
"	height: 20px;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"	subcontrol-position: top right;\n"
"	border-left: 0px solid lightgray;\n"
"	width: 22px;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"    image: url(:/images/down-arrow.png);\n"
"    width: 16px;\n"
"    height: 16px;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView {\n"
"    border: 1px solid lightgray;\n"
"	color: black;\n"
"    background: rgb(255,255,255);\n"
"	padding: 0px 0px 0px 0px;\n"
"	outline: none;\n"
"	selection-color: black;\n"
"    selection-background-color: lightgray;\n"
"}\n"
""));

        horizontalLayout_9->addWidget(PEDelayStep);


        verticalLayout->addLayout(horizontalLayout_9);


        gridLayout_15->addLayout(verticalLayout, 0, 0, 1, 1);


        verticalLayout_3->addWidget(groupBox);

        groupBox_4 = new QGroupBox(centralwidget);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        gridLayout_3 = new QGridLayout(groupBox_4);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setSpacing(6);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        WaSaveWave = new QPushButton(groupBox_4);
        WaSaveWave->setObjectName(QString::fromUtf8("WaSaveWave"));
        WaSaveWave->setMinimumSize(QSize(180, 50));
        WaSaveWave->setMaximumSize(QSize(180, 50));
        WaSaveWave->setFont(font);

        horizontalLayout_11->addWidget(WaSaveWave);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_9);

        pushButton_mode = new QPushButton(groupBox_4);
        pushButton_mode->setObjectName(QString::fromUtf8("pushButton_mode"));
        pushButton_mode->setMinimumSize(QSize(180, 50));
        pushButton_mode->setMaximumSize(QSize(180, 50));
        pushButton_mode->setFont(font);

        horizontalLayout_11->addWidget(pushButton_mode);


        verticalLayout_4->addLayout(horizontalLayout_11);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        PESaveParameters = new QPushButton(groupBox_4);
        PESaveParameters->setObjectName(QString::fromUtf8("PESaveParameters"));
        PESaveParameters->setMinimumSize(QSize(180, 50));
        PESaveParameters->setMaximumSize(QSize(180, 50));
        PESaveParameters->setFont(font);

        horizontalLayout_2->addWidget(PESaveParameters);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_10);

        STSaveWave = new QPushButton(groupBox_4);
        STSaveWave->setObjectName(QString::fromUtf8("STSaveWave"));
        STSaveWave->setMinimumSize(QSize(180, 50));
        STSaveWave->setMaximumSize(QSize(180, 50));
        STSaveWave->setFont(font);

        horizontalLayout_2->addWidget(STSaveWave);


        verticalLayout_4->addLayout(horizontalLayout_2);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer_3);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        HisData = new QPushButton(groupBox_4);
        HisData->setObjectName(QString::fromUtf8("HisData"));
        HisData->setMinimumSize(QSize(180, 50));
        HisData->setMaximumSize(QSize(180, 50));
        HisData->setFont(font);

        horizontalLayout_3->addWidget(HisData);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_11);

        PELoadParameters = new QPushButton(groupBox_4);
        PELoadParameters->setObjectName(QString::fromUtf8("PELoadParameters"));
        PELoadParameters->setMinimumSize(QSize(180, 50));
        PELoadParameters->setMaximumSize(QSize(180, 50));
        PELoadParameters->setFont(font);

        horizontalLayout_3->addWidget(PELoadParameters);


        verticalLayout_4->addLayout(horizontalLayout_3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_21 = new QLabel(groupBox_4);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_21->sizePolicy().hasHeightForWidth());
        label_21->setSizePolicy(sizePolicy1);
        label_21->setMinimumSize(QSize(65, 30));
        label_21->setMaximumSize(QSize(65, 30));
        label_21->setFont(font);

        horizontalLayout_4->addWidget(label_21);

        label_aveD90 = new QLabel(groupBox_4);
        label_aveD90->setObjectName(QString::fromUtf8("label_aveD90"));
        label_aveD90->setMinimumSize(QSize(110, 50));
        label_aveD90->setMaximumSize(QSize(110, 50));
        label_aveD90->setFont(font);
        label_aveD90->setStyleSheet(QString::fromUtf8("QLabel {\n"
"   border: 1px solid black;\n"
"}"));

        horizontalLayout_4->addWidget(label_aveD90);

        label_22 = new QLabel(groupBox_4);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        sizePolicy1.setHeightForWidth(label_22->sizePolicy().hasHeightForWidth());
        label_22->setSizePolicy(sizePolicy1);
        label_22->setMinimumSize(QSize(150, 30));
        label_22->setMaximumSize(QSize(150, 30));
        label_22->setFont(font);

        horizontalLayout_4->addWidget(label_22);

        label_avepass = new QLabel(groupBox_4);
        label_avepass->setObjectName(QString::fromUtf8("label_avepass"));
        sizePolicy1.setHeightForWidth(label_avepass->sizePolicy().hasHeightForWidth());
        label_avepass->setSizePolicy(sizePolicy1);
        label_avepass->setMinimumSize(QSize(110, 50));
        label_avepass->setMaximumSize(QSize(110, 50));
        label_avepass->setFont(font);
        label_avepass->setStyleSheet(QString::fromUtf8("QLabel {\n"
"   border: 1px solid black;\n"
"}"));
        label_avepass->setFrameShape(QFrame::NoFrame);

        horizontalLayout_4->addWidget(label_avepass);


        verticalLayout_4->addLayout(horizontalLayout_4);


        gridLayout_3->addLayout(verticalLayout_4, 0, 0, 1, 1);


        verticalLayout_3->addWidget(groupBox_4);


        gridLayout_6->addLayout(verticalLayout_3, 2, 1, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        PEAWaveBackgroundView = new QWidget(centralwidget);
        PEAWaveBackgroundView->setObjectName(QString::fromUtf8("PEAWaveBackgroundView"));
        PEAWaveBackgroundView->setEnabled(true);
        PEAWaveBackgroundView->setMinimumSize(QSize(1250, 730));
        PEAWaveBackgroundView->setMaximumSize(QSize(1250, 730));
        PEAWaveBackgroundView->setStyleSheet(QString::fromUtf8(""));
        gridLayout_14 = new QGridLayout(PEAWaveBackgroundView);
        gridLayout_14->setSpacing(6);
        gridLayout_14->setContentsMargins(11, 11, 11, 11);
        gridLayout_14->setObjectName(QString::fromUtf8("gridLayout_14"));
        groupBox_5 = new QGroupBox(PEAWaveBackgroundView);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        gridLayout_4 = new QGridLayout(groupBox_5);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        PERulerVerView = new Ruler(groupBox_5);
        PERulerVerView->setObjectName(QString::fromUtf8("PERulerVerView"));
        sizePolicy1.setHeightForWidth(PERulerVerView->sizePolicy().hasHeightForWidth());
        PERulerVerView->setSizePolicy(sizePolicy1);
        PERulerVerView->setMinimumSize(QSize(25, 256));
        PERulerVerView->setMaximumSize(QSize(25, 256));
        PERulerVerView->setAutoFillBackground(false);
        PERulerVerView->setStyleSheet(QString::fromUtf8("QWidget {\n"
"	color: black;\n"
"	background:white;\n"
"	border: 1px solid darkgray;\n"
"}"));

        gridLayout->addWidget(PERulerVerView, 0, 0, 1, 1);

        PEAWaveView = new PEImageView(groupBox_5);
        PEAWaveView->setObjectName(QString::fromUtf8("PEAWaveView"));
        sizePolicy1.setHeightForWidth(PEAWaveView->sizePolicy().hasHeightForWidth());
        PEAWaveView->setSizePolicy(sizePolicy1);
        PEAWaveView->setMinimumSize(QSize(448, 256));
        PEAWaveView->setMaximumSize(QSize(448, 256));
        PEAWaveView->setAutoFillBackground(false);
        PEAWaveView->setStyleSheet(QString::fromUtf8("QWidget {\n"
"    background:white;\n"
"    border: 1px solid darkgray;\n"
"}"));

        gridLayout->addWidget(PEAWaveView, 0, 1, 1, 1);

        PERulerHorView = new Ruler(groupBox_5);
        PERulerHorView->setObjectName(QString::fromUtf8("PERulerHorView"));
        sizePolicy1.setHeightForWidth(PERulerHorView->sizePolicy().hasHeightForWidth());
        PERulerHorView->setSizePolicy(sizePolicy1);
        PERulerHorView->setMinimumSize(QSize(448, 25));
        PERulerHorView->setMaximumSize(QSize(448, 25));
        PERulerHorView->setAutoFillBackground(false);
        PERulerHorView->setStyleSheet(QString::fromUtf8("QWidget {\n"
"	color: black;\n"
"	background:white;\n"
"	border: 1px solid darkgray;\n"
"}"));

        gridLayout->addWidget(PERulerHorView, 1, 1, 1, 1);


        horizontalLayout_5->addLayout(gridLayout);

        customPlot4 = new QCustomPlot(groupBox_5);
        customPlot4->setObjectName(QString::fromUtf8("customPlot4"));
        customPlot4->setMinimumSize(QSize(640, 280));
        customPlot4->setMaximumSize(QSize(640, 280));

        horizontalLayout_5->addWidget(customPlot4);


        gridLayout_4->addLayout(horizontalLayout_5, 0, 0, 1, 1);


        gridLayout_14->addWidget(groupBox_5, 1, 0, 1, 1);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setSpacing(6);
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        deviceSelection = new QComboBox(PEAWaveBackgroundView);
        deviceSelection->setObjectName(QString::fromUtf8("deviceSelection"));
        sizePolicy1.setHeightForWidth(deviceSelection->sizePolicy().hasHeightForWidth());
        deviceSelection->setSizePolicy(sizePolicy1);
        deviceSelection->setMinimumSize(QSize(180, 30));
        deviceSelection->setMaximumSize(QSize(180, 30));
        QFont font1;
        font1.setPointSize(15);
        deviceSelection->setFont(font1);
        deviceSelection->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	color: black;\n"
"    border: 1px solid darkgray;\n"
"    background: rgb(255,255,255);\n"
"	selection-color: black;\n"
"    selection-background-color: black;\n"
"	height: 22px;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"	subcontrol-position: top right;\n"
"	border-left: 0px solid lightgray;\n"
"	width: 22px;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"    image: url(:/images/down-arrow.png);\n"
"    width: 16px;\n"
"    height: 16px;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView {\n"
"    border: 1px solid lightgray;\n"
"	color: black;\n"
"    background: rgb(255,255,255);\n"
"	padding: 0px 0px 0px 0px;\n"
"	outline: none;\n"
"	selection-color: black;\n"
"    selection-background-color: lightgray;\n"
"}\n"
""));

        horizontalLayout_16->addWidget(deviceSelection);

        horizontalSpacer_14 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_16->addItem(horizontalSpacer_14);

        label_19 = new QLabel(PEAWaveBackgroundView);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setMinimumSize(QSize(0, 30));
        label_19->setMaximumSize(QSize(16777215, 30));
        label_19->setFont(font1);
        label_19->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    border: 0px solid darkgray;\n"
"}"));

        horizontalLayout_16->addWidget(label_19);

        lineEdit_Desk = new QLineEdit(PEAWaveBackgroundView);
        lineEdit_Desk->setObjectName(QString::fromUtf8("lineEdit_Desk"));
        sizePolicy1.setHeightForWidth(lineEdit_Desk->sizePolicy().hasHeightForWidth());
        lineEdit_Desk->setSizePolicy(sizePolicy1);
        lineEdit_Desk->setMinimumSize(QSize(120, 30));
        lineEdit_Desk->setMaximumSize(QSize(120, 30));
        lineEdit_Desk->setFont(font1);
        lineEdit_Desk->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    border: 1px solid darkgray;\n"
"}"));

        horizontalLayout_16->addWidget(lineEdit_Desk);

        horizontalSpacer_15 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_16->addItem(horizontalSpacer_15);

        PEChannelCountLabel = new QLabel(PEAWaveBackgroundView);
        PEChannelCountLabel->setObjectName(QString::fromUtf8("PEChannelCountLabel"));
        PEChannelCountLabel->setMinimumSize(QSize(0, 30));
        PEChannelCountLabel->setMaximumSize(QSize(16777215, 30));
        PEChannelCountLabel->setFont(font1);
        PEChannelCountLabel->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_16->addWidget(PEChannelCountLabel);

        PEChannelCountSelection = new QComboBox(PEAWaveBackgroundView);
        PEChannelCountSelection->setObjectName(QString::fromUtf8("PEChannelCountSelection"));
        sizePolicy1.setHeightForWidth(PEChannelCountSelection->sizePolicy().hasHeightForWidth());
        PEChannelCountSelection->setSizePolicy(sizePolicy1);
        PEChannelCountSelection->setMinimumSize(QSize(100, 30));
        PEChannelCountSelection->setMaximumSize(QSize(100, 30));
        PEChannelCountSelection->setFont(font1);
        PEChannelCountSelection->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	color: black;\n"
"    border: 1px solid black\n"
"    background: rgb(255,255,255);\n"
"	selection-color: black;\n"
"    selection-background-color: black;\n"
"	height: 22px;\n"
"}\n"
"\n"
"QComboBox::drop-down {\n"
"	subcontrol-position: top right;\n"
"	border-left: 0px solid lightgray;\n"
"	width: 22px;\n"
"}\n"
"\n"
"QComboBox::down-arrow {\n"
"    image: url(:/images/down-arrow.png);\n"
"    width: 16px;\n"
"    height: 16px;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView {\n"
"    border: 1px solid lightgray;\n"
"	color: black;\n"
"    background: rgb(255,255,255);\n"
"	padding: 0px 0px 0px 0px;\n"
"	outline: none;\n"
"	selection-color: black;\n"
"    selection-background-color: lightgray;\n"
"}\n"
""));

        horizontalLayout_16->addWidget(PEChannelCountSelection);


        gridLayout_14->addLayout(horizontalLayout_16, 0, 0, 1, 1);

        groupBox_6 = new QGroupBox(PEAWaveBackgroundView);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        gridLayout_10 = new QGridLayout(groupBox_6);
        gridLayout_10->setSpacing(6);
        gridLayout_10->setContentsMargins(11, 11, 11, 11);
        gridLayout_10->setObjectName(QString::fromUtf8("gridLayout_10"));
        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setSpacing(6);
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        label_20 = new QLabel(groupBox_6);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setMinimumSize(QSize(160, 30));
        label_20->setMaximumSize(QSize(160, 30));
        label_20->setFont(font1);

        horizontalLayout_18->addWidget(label_20);

        spb_specfmin = new QDoubleSpinBox(groupBox_6);
        spb_specfmin->setObjectName(QString::fromUtf8("spb_specfmin"));
        spb_specfmin->setMinimumSize(QSize(90, 30));
        spb_specfmin->setMaximumSize(QSize(90, 30));
        spb_specfmin->setFont(font1);
        spb_specfmin->setStyleSheet(QString::fromUtf8("QDoubleSpinBox {\n"
"    border: 1px solid darkgray;\n"
"}"));
        spb_specfmin->setSingleStep(0.100000000000000);
        spb_specfmin->setValue(2.000000000000000);

        horizontalLayout_18->addWidget(spb_specfmin);

        label_23 = new QLabel(groupBox_6);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        label_23->setMinimumSize(QSize(160, 30));
        label_23->setMaximumSize(QSize(160, 30));
        label_23->setFont(font1);
        label_23->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    border: 0px solid darkgray;\n"
"}"));
        label_23->setTextFormat(Qt::AutoText);

        horizontalLayout_18->addWidget(label_23);

        spb_specfmax = new QDoubleSpinBox(groupBox_6);
        spb_specfmax->setObjectName(QString::fromUtf8("spb_specfmax"));
        spb_specfmax->setMinimumSize(QSize(90, 30));
        spb_specfmax->setMaximumSize(QSize(90, 30));
        spb_specfmax->setFont(font1);
        spb_specfmax->setStyleSheet(QString::fromUtf8("QDoubleSpinBox {\n"
"    border: 1px solid darkgray;\n"
"}"));
        spb_specfmax->setFrame(true);
        spb_specfmax->setSingleStep(0.100000000000000);
        spb_specfmax->setValue(8.000000000000000);

        horizontalLayout_18->addWidget(spb_specfmax);

        label_24 = new QLabel(groupBox_6);
        label_24->setObjectName(QString::fromUtf8("label_24"));
        label_24->setMinimumSize(QSize(160, 30));
        label_24->setMaximumSize(QSize(160, 30));
        label_24->setFont(font1);
        label_24->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    border: 0px solid darkgray;\n"
"}"));

        horizontalLayout_18->addWidget(label_24);

        spb_histime = new QDoubleSpinBox(groupBox_6);
        spb_histime->setObjectName(QString::fromUtf8("spb_histime"));
        spb_histime->setMinimumSize(QSize(90, 30));
        spb_histime->setMaximumSize(QSize(90, 30));
        spb_histime->setFont(font1);
        spb_histime->setStyleSheet(QString::fromUtf8("QDoubleSpinBox {\n"
"    border: 1px solid darkgray;\n"
"}"));
        spb_histime->setMaximum(360.000000000000000);
        spb_histime->setSingleStep(1.000000000000000);
        spb_histime->setValue(2.000000000000000);

        horizontalLayout_18->addWidget(spb_histime);

        label_25 = new QLabel(groupBox_6);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setMinimumSize(QSize(100, 30));
        label_25->setMaximumSize(QSize(100, 30));
        label_25->setFont(font1);

        horizontalLayout_18->addWidget(label_25);

        sb_avenum = new QSpinBox(groupBox_6);
        sb_avenum->setObjectName(QString::fromUtf8("sb_avenum"));
        sizePolicy.setHeightForWidth(sb_avenum->sizePolicy().hasHeightForWidth());
        sb_avenum->setSizePolicy(sizePolicy);
        sb_avenum->setMinimumSize(QSize(60, 30));
        sb_avenum->setMaximumSize(QSize(60, 30));
        sb_avenum->setFont(font1);
        sb_avenum->setStyleSheet(QString::fromUtf8("QSpinBox {\n"
"    border: 1px solid darkgray;\n"
"}\n"
""));
        sb_avenum->setMinimum(1);
        sb_avenum->setMaximum(100);
        sb_avenum->setValue(6);

        horizontalLayout_18->addWidget(sb_avenum);


        verticalLayout_6->addLayout(horizontalLayout_18);

        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setSpacing(6);
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        customPlot3 = new QCustomPlot(groupBox_6);
        customPlot3->setObjectName(QString::fromUtf8("customPlot3"));
        customPlot3->setMinimumSize(QSize(470, 250));
        customPlot3->setMaximumSize(QSize(470, 250));
        customPlot3->setStyleSheet(QString::fromUtf8("QWidget {\n"
"    border: 1px solid darkgray;\n"
"}"));

        horizontalLayout_19->addWidget(customPlot3);

        widget_28 = new QWidget(groupBox_6);
        widget_28->setObjectName(QString::fromUtf8("widget_28"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(widget_28->sizePolicy().hasHeightForWidth());
        widget_28->setSizePolicy(sizePolicy2);
        widget_28->setMinimumSize(QSize(260, 250));
        widget_28->setMaximumSize(QSize(260, 250));
        widget_28->setStyleSheet(QString::fromUtf8(""));
        gridLayout_2 = new QGridLayout(widget_28);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(10, -1, -1, -1);
        label_26 = new QLabel(widget_28);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        sizePolicy1.setHeightForWidth(label_26->sizePolicy().hasHeightForWidth());
        label_26->setSizePolicy(sizePolicy1);
        label_26->setMinimumSize(QSize(210, 40));
        label_26->setMaximumSize(QSize(210, 40));
        label_26->setFont(font);
        label_26->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    border: 0px solid darkgray;\n"
"}"));

        gridLayout_2->addWidget(label_26, 0, 0, 1, 1);

        label_27 = new QLabel(widget_28);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        sizePolicy1.setHeightForWidth(label_27->sizePolicy().hasHeightForWidth());
        label_27->setSizePolicy(sizePolicy1);
        label_27->setMinimumSize(QSize(190, 40));
        label_27->setMaximumSize(QSize(180, 40));
        label_27->setFont(font);
        label_27->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    border: 0px solid darkgray;\n"
"}"));

        gridLayout_2->addWidget(label_27, 2, 0, 1, 1);

        spb_time = new QSpinBox(widget_28);
        spb_time->setObjectName(QString::fromUtf8("spb_time"));
        sizePolicy1.setHeightForWidth(spb_time->sizePolicy().hasHeightForWidth());
        spb_time->setSizePolicy(sizePolicy1);
        spb_time->setMinimumSize(QSize(150, 40));
        spb_time->setMaximumSize(QSize(150, 40));
        spb_time->setFont(font);
        spb_time->setMaximum(100000);
        spb_time->setValue(5);

        gridLayout_2->addWidget(spb_time, 3, 0, 1, 1);

        dspb_width = new QDoubleSpinBox(widget_28);
        dspb_width->setObjectName(QString::fromUtf8("dspb_width"));
        sizePolicy1.setHeightForWidth(dspb_width->sizePolicy().hasHeightForWidth());
        dspb_width->setSizePolicy(sizePolicy1);
        dspb_width->setMinimumSize(QSize(150, 40));
        dspb_width->setMaximumSize(QSize(150, 40));
        dspb_width->setFont(font);
        dspb_width->setDecimals(1);
        dspb_width->setMaximum(100.000000000000000);
        dspb_width->setSingleStep(0.100000000000000);
        dspb_width->setValue(16.000000000000000);

        gridLayout_2->addWidget(dspb_width, 1, 0, 1, 1);


        horizontalLayout_19->addWidget(widget_28);

        widget_30 = new QWidget(groupBox_6);
        widget_30->setObjectName(QString::fromUtf8("widget_30"));
        widget_30->setMinimumSize(QSize(350, 250));
        widget_30->setMaximumSize(QSize(350, 250));
        widget_30->setStyleSheet(QString::fromUtf8(""));
        gridLayout_11 = new QGridLayout(widget_30);
        gridLayout_11->setSpacing(6);
        gridLayout_11->setContentsMargins(11, 11, 11, 11);
        gridLayout_11->setObjectName(QString::fromUtf8("gridLayout_11"));
        label_d90 = new QLabel(widget_30);
        label_d90->setObjectName(QString::fromUtf8("label_d90"));
        sizePolicy1.setHeightForWidth(label_d90->sizePolicy().hasHeightForWidth());
        label_d90->setSizePolicy(sizePolicy1);
        label_d90->setMinimumSize(QSize(120, 40));
        label_d90->setMaximumSize(QSize(120, 40));
        label_d90->setFont(font);
        label_d90->setStyleSheet(QString::fromUtf8("QLabel {\n"
"   border: 1px solid black;\n"
"}"));
        label_d90->setFrameShape(QFrame::NoFrame);

        gridLayout_11->addWidget(label_d90, 1, 1, 1, 1);

        label_Ad90 = new QLabel(widget_30);
        label_Ad90->setObjectName(QString::fromUtf8("label_Ad90"));
        sizePolicy1.setHeightForWidth(label_Ad90->sizePolicy().hasHeightForWidth());
        label_Ad90->setSizePolicy(sizePolicy1);
        label_Ad90->setMinimumSize(QSize(140, 40));
        label_Ad90->setMaximumSize(QSize(140, 40));
        label_Ad90->setFont(font);
        label_Ad90->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    border: 0px solid darkgray;\n"
"}"));

        gridLayout_11->addWidget(label_Ad90, 1, 0, 1, 1);

        label_pass2 = new QLabel(widget_30);
        label_pass2->setObjectName(QString::fromUtf8("label_pass2"));
        sizePolicy1.setHeightForWidth(label_pass2->sizePolicy().hasHeightForWidth());
        label_pass2->setSizePolicy(sizePolicy1);
        label_pass2->setMinimumSize(QSize(120, 40));
        label_pass2->setMaximumSize(QSize(120, 40));
        label_pass2->setFont(font);
        label_pass2->setStyleSheet(QString::fromUtf8("QLabel {\n"
"   border: 1px solid black;\n"
"}"));
        label_pass2->setFrameShape(QFrame::NoFrame);

        gridLayout_11->addWidget(label_pass2, 2, 1, 1, 1);

        label_d50 = new QLabel(widget_30);
        label_d50->setObjectName(QString::fromUtf8("label_d50"));
        sizePolicy1.setHeightForWidth(label_d50->sizePolicy().hasHeightForWidth());
        label_d50->setSizePolicy(sizePolicy1);
        label_d50->setMinimumSize(QSize(120, 40));
        label_d50->setMaximumSize(QSize(120, 40));
        label_d50->setFont(font);
        label_d50->setStyleSheet(QString::fromUtf8("QLabel {\n"
"   border: 1px solid black;\n"
"}"));
        label_d50->setFrameShape(QFrame::NoFrame);

        gridLayout_11->addWidget(label_d50, 0, 1, 1, 1);

        label_Ad50 = new QLabel(widget_30);
        label_Ad50->setObjectName(QString::fromUtf8("label_Ad50"));
        sizePolicy1.setHeightForWidth(label_Ad50->sizePolicy().hasHeightForWidth());
        label_Ad50->setSizePolicy(sizePolicy1);
        label_Ad50->setMinimumSize(QSize(140, 40));
        label_Ad50->setMaximumSize(QSize(140, 40));
        label_Ad50->setFont(font);
        label_Ad50->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    border: 0px solid darkgray;\n"
"}"));

        gridLayout_11->addWidget(label_Ad50, 0, 0, 1, 1);

        label_Pass = new QLabel(widget_30);
        label_Pass->setObjectName(QString::fromUtf8("label_Pass"));
        sizePolicy1.setHeightForWidth(label_Pass->sizePolicy().hasHeightForWidth());
        label_Pass->setSizePolicy(sizePolicy1);
        label_Pass->setMinimumSize(QSize(150, 40));
        label_Pass->setMaximumSize(QSize(150, 40));
        label_Pass->setFont(font);
        label_Pass->setStyleSheet(QString::fromUtf8("QLabel {\n"
"    border: 0px solid darkgray;\n"
"}"));

        gridLayout_11->addWidget(label_Pass, 2, 0, 1, 1);


        horizontalLayout_19->addWidget(widget_30);


        verticalLayout_6->addLayout(horizontalLayout_19);


        gridLayout_10->addLayout(verticalLayout_6, 0, 0, 1, 1);


        gridLayout_14->addWidget(groupBox_6, 2, 0, 1, 1);


        verticalLayout_2->addWidget(PEAWaveBackgroundView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        IPAddressLabel = new QLabel(centralwidget);
        IPAddressLabel->setObjectName(QString::fromUtf8("IPAddressLabel"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(IPAddressLabel->sizePolicy().hasHeightForWidth());
        IPAddressLabel->setSizePolicy(sizePolicy3);
        IPAddressLabel->setMinimumSize(QSize(120, 40));
        IPAddressLabel->setMaximumSize(QSize(100, 40));
        QFont font2;
        font2.setPointSize(16);
        font2.setBold(false);
        font2.setItalic(false);
        font2.setWeight(50);
        IPAddressLabel->setFont(font2);
        IPAddressLabel->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout->addWidget(IPAddressLabel);

        IPAddressInput = new QLineEdit(centralwidget);
        IPAddressInput->setObjectName(QString::fromUtf8("IPAddressInput"));
        QSizePolicy sizePolicy4(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(250);
        sizePolicy4.setVerticalStretch(40);
        sizePolicy4.setHeightForWidth(IPAddressInput->sizePolicy().hasHeightForWidth());
        IPAddressInput->setSizePolicy(sizePolicy4);
        IPAddressInput->setMinimumSize(QSize(150, 40));
        IPAddressInput->setMaximumSize(QSize(150, 40));
        IPAddressInput->setFont(font2);
        IPAddressInput->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout->addWidget(IPAddressInput);

        PEPRFInput = new QSpinBox(centralwidget);
        PEPRFInput->setObjectName(QString::fromUtf8("PEPRFInput"));
        PEPRFInput->setMinimumSize(QSize(80, 19));
        PEPRFInput->setMaximumSize(QSize(80, 16777215));
        PEPRFInput->setStyleSheet(QString::fromUtf8("QSpinBox {\n"
"    border: 1px solid darkgray;\n"
"	border-color: darkgray darkgray darkgray darkgray; \n"
"	color: black;\n"
"	selection-color: black;\n"
"    background: rgb(255,255,255);\n"
"    selection-background-color: white;\n"
"	height: 18px;\n"
"}"));
        PEPRFInput->setReadOnly(false);
        PEPRFInput->setButtonSymbols(QAbstractSpinBox::NoButtons);
        PEPRFInput->setMinimum(50);
        PEPRFInput->setMaximum(10000);

        horizontalLayout->addWidget(PEPRFInput);

        connectButton = new QPushButton(centralwidget);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));
        QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(90);
        sizePolicy5.setVerticalStretch(30);
        sizePolicy5.setHeightForWidth(connectButton->sizePolicy().hasHeightForWidth());
        connectButton->setSizePolicy(sizePolicy5);
        connectButton->setMinimumSize(QSize(150, 40));
        connectButton->setMaximumSize(QSize(200, 60));
        QFont font3;
        font3.setPointSize(20);
        font3.setBold(false);
        font3.setItalic(false);
        font3.setWeight(50);
        connectButton->setFont(font3);
        connectButton->setStyleSheet(QString::fromUtf8(""));
        connectButton->setIconSize(QSize(16, 16));

        horizontalLayout->addWidget(connectButton);

        simulateDetectButton = new QPushButton(centralwidget);
        simulateDetectButton->setObjectName(QString::fromUtf8("simulateDetectButton"));
        sizePolicy5.setHeightForWidth(simulateDetectButton->sizePolicy().hasHeightForWidth());
        simulateDetectButton->setSizePolicy(sizePolicy5);
        simulateDetectButton->setMinimumSize(QSize(150, 40));
        simulateDetectButton->setMaximumSize(QSize(200, 60));
        simulateDetectButton->setFont(font);

        horizontalLayout->addWidget(simulateDetectButton);

        PESaveWave = new QPushButton(centralwidget);
        PESaveWave->setObjectName(QString::fromUtf8("PESaveWave"));
        sizePolicy5.setHeightForWidth(PESaveWave->sizePolicy().hasHeightForWidth());
        PESaveWave->setSizePolicy(sizePolicy5);
        PESaveWave->setMinimumSize(QSize(150, 40));
        PESaveWave->setMaximumSize(QSize(200, 60));
        PESaveWave->setFont(font);
        PESaveWave->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout->addWidget(PESaveWave);

        SpecAnaly = new QPushButton(centralwidget);
        SpecAnaly->setObjectName(QString::fromUtf8("SpecAnaly"));
        sizePolicy5.setHeightForWidth(SpecAnaly->sizePolicy().hasHeightForWidth());
        SpecAnaly->setSizePolicy(sizePolicy5);
        SpecAnaly->setMinimumSize(QSize(150, 40));
        SpecAnaly->setMaximumSize(QSize(200, 60));
        SpecAnaly->setFont(font);

        horizontalLayout->addWidget(SpecAnaly);

        CloseWaveSave = new QPushButton(centralwidget);
        CloseWaveSave->setObjectName(QString::fromUtf8("CloseWaveSave"));
        sizePolicy5.setHeightForWidth(CloseWaveSave->sizePolicy().hasHeightForWidth());
        CloseWaveSave->setSizePolicy(sizePolicy5);
        CloseWaveSave->setMinimumSize(QSize(150, 40));
        CloseWaveSave->setMaximumSize(QSize(200, 60));
        CloseWaveSave->setFont(font);

        horizontalLayout->addWidget(CloseWaveSave);

        Inserve = new QPushButton(centralwidget);
        Inserve->setObjectName(QString::fromUtf8("Inserve"));
        sizePolicy5.setHeightForWidth(Inserve->sizePolicy().hasHeightForWidth());
        Inserve->setSizePolicy(sizePolicy5);
        Inserve->setMinimumSize(QSize(150, 40));
        Inserve->setMaximumSize(QSize(200, 60));
        Inserve->setFont(font);

        horizontalLayout->addWidget(Inserve);


        verticalLayout_2->addLayout(horizontalLayout);


        gridLayout_6->addLayout(verticalLayout_2, 2, 0, 1, 1);

        widget_21 = new QWidget(centralwidget);
        widget_21->setObjectName(QString::fromUtf8("widget_21"));
        QSizePolicy sizePolicy6(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(widget_21->sizePolicy().hasHeightForWidth());
        widget_21->setSizePolicy(sizePolicy6);
        widget_21->setMinimumSize(QSize(0, 100));
        widget_21->setMaximumSize(QSize(16777215, 100));
        widget_21->setStyleSheet(QString::fromUtf8(""));
        gridLayout_5 = new QGridLayout(widget_21);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        gridLayout_5->setHorizontalSpacing(12);
        gridLayout_5->setVerticalSpacing(4);
        gridLayout_5->setContentsMargins(2, 2, 2, 7);
        label_headerIcon = new QLabel(widget_21);
        label_headerIcon->setObjectName(QString::fromUtf8("label_headerIcon"));
        label_headerIcon->setMinimumSize(QSize(64, 64));
        label_headerIcon->setMaximumSize(QSize(90, 90));
        label_headerIcon->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_headerIcon, 1, 0, 1, 1);

        label_7 = new QLabel(widget_21);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        sizePolicy2.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy2);
        label_7->setMinimumSize(QSize(0, 58));
        label_7->setMaximumSize(QSize(16777215, 58));
        QFont font4;
        font4.setPointSize(40);
        label_7->setFont(font4);
        label_7->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_5->addWidget(label_7, 1, 1, 1, 1);


        gridLayout_6->addWidget(widget_21, 0, 0, 1, 2);

        verticalSpacer_11 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_6->addItem(verticalSpacer_11, 1, 1, 1, 1);

        pa22x->setCentralWidget(centralwidget);
        menubar = new QMenuBar(pa22x);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 2018, 21));
        pa22x->setMenuBar(menubar);
        statusbar = new QStatusBar(pa22x);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        pa22x->setStatusBar(statusbar);

        retranslateUi(pa22x);

        QMetaObject::connectSlotsByName(pa22x);
    } // setupUi

    void retranslateUi(QMainWindow *pa22x)
    {
        pa22x->setWindowTitle(QApplication::translate("pa22x", "pa22x", nullptr));
        groupBox->setTitle(QApplication::translate("pa22x", "Argument", nullptr));
        PEChanneSelectionLabel->setText(QApplication::translate("pa22x", "\351\200\232\351\201\223\351\200\211\346\213\251", nullptr));
        PEDualModeLabel->setText(QApplication::translate("pa22x", "\345\215\225\345\217\214\346\250\241\345\274\217", nullptr));
        PEGainLabel->setText(QApplication::translate("pa22x", "\345\242\236\347\233\212(dB)", nullptr));
        PERangeLabel->setText(QApplication::translate("pa22x", "\350\214\203\345\233\264(\346\257\253\347\261\263)", nullptr));
        PEDelayLabel->setText(QApplication::translate("pa22x", "\347\247\273\344\275\215(\345\276\256\347\247\222)", nullptr));
        groupBox_4->setTitle(QString());
        WaSaveWave->setText(QApplication::translate("pa22x", "\350\203\214\346\231\257\344\277\241\345\217\267", nullptr));
        pushButton_mode->setText(QApplication::translate("pa22x", "\344\272\256\350\211\262\346\250\241\345\274\217", nullptr));
        PESaveParameters->setText(QApplication::translate("pa22x", "\344\277\235\345\255\230\345\217\202\346\225\260", nullptr));
        STSaveWave->setText(QApplication::translate("pa22x", "\344\277\241\345\217\267\344\277\235\345\255\230", nullptr));
        HisData->setText(QApplication::translate("pa22x", "\345\216\206\345\217\262\346\225\260\346\215\256", nullptr));
        PELoadParameters->setText(QApplication::translate("pa22x", "\345\212\240\350\275\275\345\217\202\346\225\260", nullptr));
        label_21->setText(QApplication::translate("pa22x", "\347\273\206\345\272\246", nullptr));
        label_aveD90->setText(QString());
        label_22->setText(QApplication::translate("pa22x", "\351\200\232\350\277\207\347\216\207(%)", nullptr));
        label_avepass->setText(QString());
        groupBox_5->setTitle(QString());
        label_19->setText(QApplication::translate("pa22x", "\347\250\213\345\272\217\350\267\257\345\276\204", nullptr));
        lineEdit_Desk->setText(QApplication::translate("pa22x", "w1215", nullptr));
        PEChannelCountLabel->setText(QApplication::translate("pa22x", "\351\200\232\351\201\223\346\225\260", nullptr));
        groupBox_6->setTitle(QString());
        label_20->setText(QApplication::translate("pa22x", "\346\234\200\345\260\217\351\242\221\347\216\207(MHz)\357\274\232", nullptr));
        label_23->setText(QApplication::translate("pa22x", "\346\234\200\345\244\247\351\242\221\347\216\207(MHz)\357\274\232", nullptr));
        label_24->setText(QApplication::translate("pa22x", "\346\225\260\346\215\256\346\227\266\351\227\264(min)\357\274\232", nullptr));
        label_25->setText(QApplication::translate("pa22x", "\346\225\260\346\215\256\345\271\263\345\235\207", nullptr));
        label_26->setText(QApplication::translate("pa22x", "\346\265\213\351\207\217\345\256\275\345\272\246(mm)", nullptr));
        label_27->setText(QApplication::translate("pa22x", "\346\243\200\346\265\213\351\227\264\351\232\224(s)", nullptr));
        label_d90->setText(QString());
        label_Ad90->setText(QApplication::translate("pa22x", "D90(\316\274m)", nullptr));
        label_pass2->setText(QString());
        label_d50->setText(QString());
        label_Ad50->setText(QApplication::translate("pa22x", "D50(\316\274m)", nullptr));
        label_Pass->setText(QApplication::translate("pa22x", "\351\200\232\350\277\207\347\216\207(%)", nullptr));
        IPAddressLabel->setText(QApplication::translate("pa22x", "\346\234\215\345\212\241\345\231\250\345\234\260\345\235\200:", nullptr));
        IPAddressInput->setText(QApplication::translate("pa22x", "192.168.22.22", nullptr));
        connectButton->setText(QApplication::translate("pa22x", "\350\277\236\346\216\245", nullptr));
        simulateDetectButton->setText(QApplication::translate("pa22x", "\346\250\241\346\213\237\346\243\200\346\265\213", nullptr));
        PESaveWave->setText(QApplication::translate("pa22x", "\345\220\257\345\212\250", nullptr));
        SpecAnaly->setText(QApplication::translate("pa22x", "\346\243\200\346\265\213", nullptr));
        CloseWaveSave->setText(QApplication::translate("pa22x", "\345\201\234\346\255\242", nullptr));
        Inserve->setText(QApplication::translate("pa22x", "\351\200\200\345\207\272", nullptr));
        label_headerIcon->setText(QString());
        label_7->setText(QApplication::translate("pa22x", "<html><head/><body><p><span style=\" font-weight:600;\">      \347\237\263\347\201\260\347\237\263\346\265\206\346\266\262\347\273\206\345\272\246\345\234\250\347\272\277\346\243\200\346\265\213\347\263\273\347\273\237</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class pa22x: public Ui_pa22x {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PA22X_H
