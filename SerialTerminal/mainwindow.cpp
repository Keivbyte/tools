#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial_()
{
    ui->setupUi(this);

    setWindowTitle("Serial Terminal");
    setWindowIcon(QIcon(":/ico/icon.png"));


    buttonGroup_ = new QButtonGroup(this);
    buttonGroup_->addButton(ui->rbtn_ascii, 0);
    buttonGroup_->addButton(ui->rbtn_hex, 1);

    ui->rbtn_ascii->setChecked(true);

    serialTimer_ = new QTimer(this);
    connect(serialTimer_, &QTimer::timeout, this, &MainWindow::readSerialData);
    connect(ui->le_sender, &QLineEdit::returnPressed, this, &MainWindow::on_btn_send_clicked);


    combobox_port_set();
    fillBaudrateComboBox();
    fillDataBitsComboBox();
    fillParityComboBox();
    fillStopBitsComboBox();
    fillFlowControlComboBox();
    updateConnectionButton();

    for (int i = 0; i < 20; i++) {
        QPushButton *btn = findChild<QPushButton*>(QString("macroButton%1").arg(i + 1));
        if (btn) {
            connect(btn, &QPushButton::clicked, this, &MainWindow::onMacroButtonClicked);
            macroButtons.append(btn);
        }
    }

    macrosDialog = new MacrosDialog(this);
    scriptDialog = new ScriptDialog(this);

    scriptDialog->setSerialPort(&serial_);

    connect(scriptDialog, &ScriptDialog::sendToMainTerminal,
            [this](const QString& text) {
                ui->textBrowser_command->append(text);
            });
}

MainWindow::~MainWindow()
{
    delete ui;
    delete serialTimer_;
    delete macrosDialog;
    delete scriptDialog;
}

void MainWindow::combobox_port_set(){
    std::vector<std::string> ports = serial_.get_port_list();
    ui->comboBox_port->clear();
    for(const auto& port : ports) {
        ui->comboBox_port->addItem(QString::fromStdString(port));
    }
}

void MainWindow::on_comboBox_port_currentIndexChanged(int index){
    serial_.select_port(index);
    ui->label_selectedPort->setText(ui->comboBox_port->currentText());
}

void MainWindow::on_pushButton_clicked() {
    if (!isConnected_) {
        if (serial_.connect(selectedBaudrate_)) {
            serial_.setDataBits(selectedDataBits_);
            serial_.setParity(selectedParity_.toStdString());
            serial_.setStopBits(selectedStopBits_);
            serial_.setFlowControl(selectedFlowControl_.toStdString());
            isConnected_ = true;
            updateConnectionButton();

            ui->comboBox_port->setEnabled(false);
            ui->comboBox_baudrate->setEnabled(false);
            ui->comboBox_databits->setEnabled(false);
            ui->comboBox_parity->setEnabled(false);
            ui->comboBox_stopbits->setEnabled(false);
            ui->comboBox_flowControl->setEnabled(false);
        }
    } else {
        serialTimer_->stop();
        serial_.disconnect();
        isConnected_ = false;
        updateConnectionButton();
        ui->comboBox_port->setEnabled(true);
        ui->comboBox_baudrate->setEnabled(true);
        ui->comboBox_databits->setEnabled(true);
        ui->comboBox_parity->setEnabled(true);
        ui->comboBox_stopbits->setEnabled(true);
        ui->comboBox_flowControl->setEnabled(true);
    }
}

void MainWindow::updateConnectionButton() {
    if (isConnected_) {
        ui->pushButton->setText("Disconnect");
    } else {
        ui->pushButton->setText("Connect");
    }
}

void MainWindow::on_comboBox_baudrate_currentIndexChanged(int index) {
    Q_UNUSED(index)
    selectedBaudrate_ = ui->comboBox_baudrate->currentData().toInt();
}

void MainWindow::fillBaudrateComboBox() {
    ui->comboBox_baudrate->clear();
    ui->comboBox_baudrate->addItem("9600", 9600);
    ui->comboBox_baudrate->addItem("19200", 19200);
    ui->comboBox_baudrate->addItem("38400", 38400);
    ui->comboBox_baudrate->addItem("115200", 115200);
    ui->comboBox_baudrate->addItem("2000000", 2000000);
    ui->comboBox_baudrate->setCurrentText("2000000");
}

void MainWindow::fillDataBitsComboBox() {
    ui->comboBox_databits->clear();
    ui->comboBox_databits->addItem("8", 8);
    ui->comboBox_databits->addItem("7", 7);
    ui->comboBox_databits->addItem("6", 6);
    ui->comboBox_databits->addItem("5", 5);
    ui->comboBox_databits->setCurrentText("8");
}

void MainWindow::fillParityComboBox() {
    ui->comboBox_parity->clear();
    ui->comboBox_parity->addItem("None", "None");
    ui->comboBox_parity->addItem("Even", "Even");
    ui->comboBox_parity->addItem("Odd", "Odd");
    ui->comboBox_parity->addItem("Space", "Space");
    ui->comboBox_parity->addItem("Mark", "Mark");
    ui->comboBox_parity->setCurrentText("None");
}

void MainWindow::fillStopBitsComboBox() {
    ui->comboBox_stopbits->clear();
    ui->comboBox_stopbits->addItem("1", 1);
    ui->comboBox_stopbits->addItem("2", 2);
    ui->comboBox_stopbits->setCurrentText("1");
}

void MainWindow::fillFlowControlComboBox() {
    ui->comboBox_flowControl->clear();
    ui->comboBox_flowControl->addItem("None", "None");
    ui->comboBox_flowControl->addItem("Hardware", "Hardware");
    ui->comboBox_flowControl->addItem("Software", "Software");
    ui->comboBox_flowControl->setCurrentText("None");
}

std::vector<uint8_t> MainWindow::parseHexString(const QString& text) {
    std::vector<uint8_t> result;
    QString cleanText = text.simplified().remove(' ');

    for (int i = 0; i < cleanText.length(); i += 2) {
        bool ok;
        QString byteStr = cleanText.mid(i, 2);
        uint8_t byte = byteStr.toUInt(&ok, 16);
        if (ok) {
            result.push_back(byte);
        }
    }
    return result;
}

void MainWindow::on_btn_send_clicked(){
    if (!isConnected_) {
        return;
    }

    QString text = ui->le_sender->text();
    if (text.isEmpty()) {
        return;
    }

    QString commandWithCounter = QString("%1").arg(text);
    ui->textBrowser_command->append(commandWithCounter);

    ui->le_counter->setText(QString("Count -> %1").arg(++counter_send));

    if (ui->rbtn_ascii->isChecked()) {
        serial_.send_ascii(text.toStdString());
    } else if (ui->rbtn_hex->isChecked()) {
        std::vector<uint8_t> hex_data = parseHexString(text);
        serial_.send_hex(hex_data);
    }

    if (isConnected_) {
        serialTimer_->start(50);
        QTimer::singleShot(250, this, [this]() {
            if (isConnected_) {
                serialTimer_->stop();
            }
        });
    }
}

void MainWindow::readSerialData() {
    if (!isConnected_) {
        serialTimer_->stop();
        return;
    }

    std::string received = serial_.receive();
    if (!received.empty()) {
        QString data = QString::fromStdString(received);
        QStringList lines = data.split('\n', Qt::SkipEmptyParts);

        QString formatted = "\n";
        for (const QString& line : lines) {
            QString trimmed = line.trimmed();
            if (!trimmed.isEmpty()) {
                formatted += trimmed + "\n";
            }
        }
        if (formatted.endsWith('\n')) {
            formatted.chop(1);
        }

        ui->textBrowser_command->append(formatted);
    }
}

void MainWindow::on_comboBox_databits_currentIndexChanged(int index) {
    Q_UNUSED(index)
    selectedDataBits_ = ui->comboBox_databits->currentData().toInt();
}

void MainWindow::on_comboBox_parity_currentIndexChanged(int index) {
    Q_UNUSED(index)
    selectedParity_ = ui->comboBox_parity->currentText();
}

void MainWindow::on_comboBox_stopbits_currentIndexChanged(int index) {
    Q_UNUSED(index)
    selectedStopBits_ = ui->comboBox_stopbits->currentData().toInt();
}

void MainWindow::on_comboBox_flowControl_currentIndexChanged(int index) {
    Q_UNUSED(index)
    selectedFlowControl_ = ui->comboBox_flowControl->currentText();
}

void MainWindow::on_btn_clear_clicked(){
    ui->textBrowser_command->clear();
}

void MainWindow::on_btn_setmacros_clicked() {
    macrosDialog->show();
}

void MainWindow::onMacroButtonClicked() {
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());
    if (!clickedBtn) return;

    QString btnName = clickedBtn->objectName();
    if (!btnName.startsWith("macroButton")) return;

    int macroNum = btnName.mid(11).toInt() - 1; // "macroButton" + number
    if (macroNum < 0 || macroNum >= 20) return;

    // Loading a macro from the settings
    QSettings settings("macros.ini", QSettings::IniFormat);
    QString text = settings.value(QString("Macro%1/text").arg(macroNum), "").toString();
    int type = settings.value(QString("Macro%1/type").arg(macroNum), 0).toInt();

    if (text.isEmpty()) return;

    // send command
    QString commandWithCounter = QString("%1 >>> %2").arg(++counter_send).arg(text);
    ui->textBrowser_command->append(commandWithCounter);
    ui->le_counter->setText(QString("Count -> %1").arg(counter_send));

    if (type == 0) { // ASCII
        serial_.send_ascii(text.toStdString());
    } else { // HEX
        std::vector<uint8_t> hex_data = parseHexString(text);
        serial_.send_hex(hex_data);
    }

    if (isConnected_) {
        serialTimer_->start(50);
        QTimer::singleShot(250, this, [this]() {
            if (isConnected_) {
                serialTimer_->stop();
            }
        });
    }
}

void MainWindow::on_btn_script_clicked() {
    if (!isConnected_) {
        QMessageBox::warning(this, "Warning", "Not connected to device");
        return;
    }
    scriptDialog->show();
}
