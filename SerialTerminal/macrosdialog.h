#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QLabel>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MacrosDialog;
}
QT_END_NAMESPACE

class MacrosDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MacrosDialog(QWidget *parent = nullptr);
    ~MacrosDialog();

    void loadMacros();
    void saveMacros();

private slots:
    void onSaveClicked();

private:
    QVBoxLayout *mainLayout;
    QVector<QLineEdit*> lineEdits;
    QVector<QRadioButton*> asciiButtons;
    QVector<QRadioButton*> hexButtons;
    QButtonGroup *buttonGroups[20];
    QPushButton *saveButton;
};
