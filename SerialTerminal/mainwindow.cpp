#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial_()
{
    ui->setupUi(this);

    setWindowTitle("Serial Terminal");

    buttonGroup_ = new QButtonGroup(this);
    buttonGroup_->addButton(ui->rbtn_ascii, 0);
    buttonGroup_->addButton(ui->rbtn_hex, 1);

    ui->rbtn_ascii->setChecked(true);

    serialTimer_ = new QTimer(this);
    connect(serialTimer_, &QTimer::timeout, this, &MainWindow::readSerialData);
    connect(ui->le_sender, &QLineEdit::returnPressed, this, &MainWindow::on_btn_send_clicked);


    combobox_port_set();
    fillBaudrateComboBox();
    updateConnectionButton();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete serialTimer_;
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
            isConnected_ = true;
            updateConnectionButton();
            ui->comboBox_port->setEnabled(false);
            ui->comboBox_baudrate->setEnabled(false);
        }
    } else {
        serialTimer_->stop();
        serial_.disconnect();
        isConnected_ = false;
        updateConnectionButton();
        ui->comboBox_port->setEnabled(true);
        ui->comboBox_baudrate->setEnabled(true);
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
    ui->comboBox_baudrate->setCurrentText("2000000"); // по умолчанию
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

    QString commandWithCounter = QString("%1 >>> %2").arg(counter_send).arg(text);
    ui->textBrowser_command->append(commandWithCounter);
    counter_send++;

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

        QString formatted = "<<<\n";
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


