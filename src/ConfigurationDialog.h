#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include "ui_ConfigurationDialog.h"

#include <QDialog>
#include <QSettings>

namespace Ui {
class ConfigurationDialog;
}

class ConfigurationDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigurationDialog(QWidget *parent = 0);
    ~ConfigurationDialog();

    void setThreshold();
    int threshold {100};

public slots:
    void accept();
    void reset();
    void newTreshold(int);

private:
    Ui::ConfigurationDialog *ui;
    QSettings settings {"Rock et Chansons", "Boites Electriques"};
};

#endif // CONFIGURATIONDIALOG_H
