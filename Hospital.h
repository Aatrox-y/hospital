#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Hospital.h"

class Hospital : public QMainWindow
{
    Q_OBJECT

public:
    Hospital(QWidget *parent = nullptr);
    ~Hospital();

private:
    Ui::HospitalClass ui;
};

