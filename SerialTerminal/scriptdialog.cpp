#include "scriptdialog.h"
#include <QThread>
#include <QTimer>
#include <QRegularExpression>
#include <QEventLoop>
#include <QDebug>

ScriptDialog::ScriptDialog(QWidget *parent) : QDialog(parent), serialPort(nullptr)
{
    setWindowTitle("Script Editor");
    resize(600, 400);

    scriptViewer = new QTextBrowser();
    browseButton = new QPushButton("Browse");
    runButton = new QPushButton("Run");
    clearButton = new QPushButton("Clear");

    connect(browseButton, &QPushButton::clicked, this, &ScriptDialog::browseScript);
    connect(runButton, &QPushButton::clicked, this, &ScriptDialog::runScript);
    connect(clearButton, &QPushButton::clicked, this, &ScriptDialog::clearScript);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(browseButton);
    buttonLayout->addWidget(runButton);
    buttonLayout->addWidget(clearButton);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(scriptViewer);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void ScriptDialog::setSerialPort(SerialPort* serial) {
    serialPort = serial;
}

void ScriptDialog::browseScript() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Script",
        "",
        "Script Files (*.tsc)"
        );

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            scriptViewer->setPlainText(file.readAll());
        } else {
            QMessageBox::warning(this, "Error", "Could not open file");
        }
    }
}

void ScriptDialog::clearScript() {
    scriptViewer->clear();
}

void ScriptDialog::runScript() {
    QString scriptContent = scriptViewer->toPlainText();
    if (scriptContent.isEmpty()) {
        QMessageBox::information(this, "Info", "No script to run");
        return;
    }

    executeScript(scriptContent);
}

void ScriptDialog::executeScript(const QString& script) {
    m_variables.clear();

    QStringList lines = script.split('\n');
    bool inProgramSection = false;

    for (QString line : lines) {
        int commentIndex = line.indexOf("//");
        if (commentIndex != -1) {
            line = line.left(commentIndex);
        }

        line = line.trimmed();

        if (line.isEmpty()) {
            continue;
        }

        if (line.startsWith("program") || line.startsWith("var ") || line == "begin") {
            if (line == "begin") {
                inProgramSection = true;
            }
            continue;
        }

        if (line == "end.") {
            break;
        }

        if (!inProgramSection) {
            continue;
        }

        processScriptLine(line);
    }
}

void ScriptDialog::processScriptLine(const QString& line) {
    QString cleanLine = line.trimmed();

    if (cleanLine.endsWith(';')) {
        cleanLine.chop(1);
    }

    if (cleanLine.contains(":=")) {
        handleVariableAssignment(cleanLine);
        return;
    }

    if (cleanLine.startsWith("comsendstr")) {
        handleComSendStr(cleanLine);
        return;
    }

    if (cleanLine.startsWith("Delay")) {
        handleDelay(cleanLine);
        return;
    }
}

void ScriptDialog::handleVariableAssignment(const QString& line) {
    QStringList parts = line.split(":=");
    if (parts.size() != 2) {
        qDebug() << "Invalid variable assignment:" << line;
        return;
    }

    QString varName = parts[0].trimmed();
    QString value = parts[1].trimmed();

    QString processedValue = processStringValue(value);

    m_variables[varName] = processedValue;
}

void ScriptDialog::handleComSendStr(const QString& line) {
    int start = line.indexOf('(');
    int end = line.indexOf(')');

    if (start == -1 || end == -1) {
        qDebug() << "Invalid comsendstr syntax:" << line;
        return;
    }

    QString param = line.mid(start + 1, end - start - 1).trimmed();

    QString processedParam = processStringValue(param);

    if (processedParam.startsWith('\'') && processedParam.endsWith('\'')) {
        processedParam = processedParam.mid(1, processedParam.length() - 2);
    }

    // Отправляем команду в основное окно
    emit sendToMainTerminal(">>> " + processedParam);

    // Отправляем команду через serialPort
    if (serialPort) {
        serialPort->send_ascii(processedParam.toStdString());
    }
}

void ScriptDialog::handleDelay(const QString& line) {
    int start = line.indexOf('(');
    int end = line.indexOf(')');

    if (start == -1 || end == -1) {
        qDebug() << "Invalid Delay syntax:" << line;
        return;
    }

    QString param = line.mid(start + 1, end - start - 1).trimmed();

    bool ok;
    int delayMs = param.toInt(&ok);

    if (ok) {
        // Используем QTimer вместо QThread::msleep
        QEventLoop loop;
        QTimer::singleShot(delayMs, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

QString ScriptDialog::processStringValue(const QString& value) {
    QString result = value;

    for (auto it = m_variables.constBegin(); it != m_variables.constEnd(); ++it) {
        QString varName = it.key();
        QString varValue = it.value();

        QRegularExpression regex(QString("\\b%1\\b").arg(QRegularExpression::escape(varName)));
        result.replace(regex, varValue);
    }

    QRegularExpression escapeRegex("#([0-9]+)");
    QRegularExpressionMatchIterator it = escapeRegex.globalMatch(result);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString escapeSeq = match.captured(0);
        QString numberStr = match.captured(1);

        bool ok;
        int charCode = numberStr.toInt(&ok);
        if (ok) {
            QChar replacementChar(charCode);
            result.replace(escapeSeq, replacementChar);
        }
    }

    if (result.contains('+')) {
        QStringList concatParts = result.split('+');
        QString concatenated;

        for (QString part : concatParts) {
            QString trimmedPart = part.trimmed();

            if (trimmedPart.startsWith('\'') && trimmedPart.endsWith('\'')) {
                trimmedPart = trimmedPart.mid(1, trimmedPart.length() - 2);
            }

            concatenated += trimmedPart;
        }

        result = concatenated;
    }

    return result;
}
