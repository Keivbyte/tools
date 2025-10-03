#include "macrosdialog.h"
#include <QSettings>
#include <QMessageBox>

MacrosDialog::MacrosDialog(QWidget *parent) : QDialog(parent)
{
    mainLayout = new QVBoxLayout(this);

    for (int i = 0; i < 20; i++) {
        QHBoxLayout *rowLayout = new QHBoxLayout();

        QLabel *label = new QLabel(QString("Macro %1:").arg(i + 1));
        QLineEdit *lineEdit = new QLineEdit();
        QRadioButton *asciiButton = new QRadioButton("ASCII");
        QRadioButton *hexButton = new QRadioButton("HEX");

        asciiButton->setChecked(true); // ASCII

        buttonGroups[i] = new QButtonGroup(this);
        buttonGroups[i]->addButton(asciiButton, 0);
        buttonGroups[i]->addButton(hexButton, 1);

        rowLayout->addWidget(label);
        rowLayout->addWidget(lineEdit);
        rowLayout->addWidget(asciiButton);
        rowLayout->addWidget(hexButton);

        mainLayout->addLayout(rowLayout);

        lineEdits.append(lineEdit);
        asciiButtons.append(asciiButton);
        hexButtons.append(hexButton);
    }

    saveButton = new QPushButton("Save");
    connect(saveButton, &QPushButton::clicked, this, &MacrosDialog::onSaveClicked);

    mainLayout->addWidget(saveButton);

    loadMacros();
}

MacrosDialog::~MacrosDialog()
{
}

void MacrosDialog::loadMacros() {
    QSettings settings("macros.ini", QSettings::IniFormat);

    for (int i = 0; i < 20; i++) {
        QString text = settings.value(QString("Macro%1/text").arg(i), "").toString();
        int type = settings.value(QString("Macro%1/type").arg(i), 0).toInt();

        lineEdits[i]->setText(text);
        if (type == 0) {
            asciiButtons[i]->setChecked(true);
        } else {
            hexButtons[i]->setChecked(true);
        }
    }
}

void MacrosDialog::saveMacros() {
    QSettings settings("macros.ini", QSettings::IniFormat);

    for (int i = 0; i < 20; i++) {
        settings.setValue(QString("Macro%1/text").arg(i), lineEdits[i]->text());

        if (asciiButtons[i]->isChecked()) {
            settings.setValue(QString("Macro%1/type").arg(i), 0);
        } else {
            settings.setValue(QString("Macro%1/type").arg(i), 1);
        }
    }

    QMessageBox::information(this, "Saved", "Macros saved successfully!");
}

void MacrosDialog::onSaveClicked() {
    saveMacros();
}
