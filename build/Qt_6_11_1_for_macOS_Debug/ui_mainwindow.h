/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>
#include "my_label.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    my_label *frame;
    QLabel *label_score;
    QLabel *label_lines;
    QLabel *label_level;
    QLabel *label_next_title;
    QLabel *label_next;
    QLabel *label_grid;
    QSpinBox *spinBox;
    QPushButton *btn_draw_grid;
    QPushButton *btn_pause;
    QPushButton *clear;
    QLabel *mouse_movement;
    QLabel *label_help;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(950, 640);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        frame = new my_label(centralWidget);
        frame->setObjectName("frame");
        frame->setGeometry(QRect(10, 10, 600, 600));
        frame->setFrameShape(QFrame::Box);
        label_score = new QLabel(centralWidget);
        label_score->setObjectName("label_score");
        label_score->setGeometry(QRect(630, 20, 300, 28));
        label_lines = new QLabel(centralWidget);
        label_lines->setObjectName("label_lines");
        label_lines->setGeometry(QRect(630, 50, 300, 28));
        label_level = new QLabel(centralWidget);
        label_level->setObjectName("label_level");
        label_level->setGeometry(QRect(630, 80, 300, 28));
        label_next_title = new QLabel(centralWidget);
        label_next_title->setObjectName("label_next_title");
        label_next_title->setGeometry(QRect(630, 115, 100, 25));
        label_next = new QLabel(centralWidget);
        label_next->setObjectName("label_next");
        label_next->setGeometry(QRect(630, 145, 130, 90));
        label_next->setFrameShape(QFrame::Box);
        label_grid = new QLabel(centralWidget);
        label_grid->setObjectName("label_grid");
        label_grid->setGeometry(QRect(630, 260, 71, 31));
        spinBox = new QSpinBox(centralWidget);
        spinBox->setObjectName("spinBox");
        spinBox->setGeometry(QRect(710, 260, 71, 31));
        spinBox->setMinimum(10);
        spinBox->setMaximum(30);
        spinBox->setValue(25);
        btn_draw_grid = new QPushButton(centralWidget);
        btn_draw_grid->setObjectName("btn_draw_grid");
        btn_draw_grid->setGeometry(QRect(800, 260, 131, 31));
        btn_pause = new QPushButton(centralWidget);
        btn_pause->setObjectName("btn_pause");
        btn_pause->setGeometry(QRect(630, 310, 300, 41));
        clear = new QPushButton(centralWidget);
        clear->setObjectName("clear");
        clear->setGeometry(QRect(630, 360, 300, 41));
        mouse_movement = new QLabel(centralWidget);
        mouse_movement->setObjectName("mouse_movement");
        mouse_movement->setGeometry(QRect(630, 415, 300, 25));
        label_help = new QLabel(centralWidget);
        label_help->setObjectName("label_help");
        label_help->setGeometry(QRect(630, 450, 300, 160));
        label_help->setWordWrap(true);
        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Computer Graphics Lab - Tetris", nullptr));
        frame->setText(QString());
        label_score->setText(QCoreApplication::translate("MainWindow", "Score: 0", nullptr));
        label_lines->setText(QCoreApplication::translate("MainWindow", "Lines: 0", nullptr));
        label_level->setText(QCoreApplication::translate("MainWindow", "Level: 1", nullptr));
        label_next_title->setText(QCoreApplication::translate("MainWindow", "Next piece:", nullptr));
        label_next->setText(QString());
        label_grid->setText(QCoreApplication::translate("MainWindow", "Grid Size:", nullptr));
        btn_draw_grid->setText(QCoreApplication::translate("MainWindow", "Draw Grid", nullptr));
        btn_pause->setText(QCoreApplication::translate("MainWindow", "Pause (P)", nullptr));
        clear->setText(QCoreApplication::translate("MainWindow", "New Game (N)", nullptr));
        mouse_movement->setText(QCoreApplication::translate("MainWindow", "Hover: ", nullptr));
        label_help->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
