#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QFileDialog>
#include <QMessageBox>
#include <QMap>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

#include "serial.h"

class ScriptDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptDialog(QWidget *parent = nullptr);
    void setSerialPort(SerialPort* serial);

private slots:
    void browseScript();
    void runScript();
    void clearScript();

signals:
    void sendToMainTerminal(const QString& text);

private:
    QTextBrowser *scriptViewer;
    QPushButton *browseButton;
    QPushButton *runButton;
    QPushButton *clearButton;
    SerialPort *serialPort;

    QMap<QString, QString> m_variables;

    void executeScript(const QString& script);

    void processScriptLine(const QString& line);
    void handleVariableAssignment(const QString& line);
    void handleComSendStr(const QString& line);
    void handleDelay(const QString& line);
    QString processStringValue(const QString& value);

};
