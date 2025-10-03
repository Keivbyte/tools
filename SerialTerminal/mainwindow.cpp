#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial_()
{
    ui->setupUi(this);

    buttonGroup_ = new QButtonGroup(this);
    buttonGroup_->addButton(ui->rbtn_ascii, 0);
    buttonGroup_->addButton(ui->rbtn_hex, 1);

    ui->rbtn_ascii->setChecked(true);

    combobox_port_set();
    fillBaudrateComboBox();
    updateConnectionButton();
}

MainWindow::~MainWindow()
{
    delete ui;
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

