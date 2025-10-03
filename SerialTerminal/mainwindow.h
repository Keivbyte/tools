#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QTimer>
#include "serial.h"
#include "macrosdialog.h"
#include "scriptdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_comboBox_port_currentIndexChanged(int index);
    void on_comboBox_baudrate_currentIndexChanged(int index);
    void on_comboBox_databits_currentIndexChanged(int index);
    void on_comboBox_parity_currentIndexChanged(int index);
    void on_comboBox_stopbits_currentIndexChanged(int index);
    void on_comboBox_flowControl_currentIndexChanged(int index);
    void on_pushButton_clicked();

    void on_btn_send_clicked();
    void on_btn_clear_clicked();

    void on_btn_setmacros_clicked();
    void onMacroButtonClicked();
    void on_btn_script_clicked();

private:
    Ui::MainWindow *ui;
    SerialPort serial_;
    bool isConnected_ = false;
    int selectedBaudrate_= 2000000;
    int selectedDataBits_ = 8;
    int selectedStopBits_ = 1;
    QString selectedFlowControl_ = "None";
    QString selectedParity_ = "None";
    QButtonGroup *buttonGroup_;
    QTimer *serialTimer_;

    MacrosDialog *macrosDialog;
    QVector<QPushButton*> macroButtons;
    ScriptDialog *scriptDialog;

    void combobox_port_set();
    void updateConnectionButton();
    void fillBaudrateComboBox();
    void fillDataBitsComboBox();
    void fillParityComboBox();
    void fillStopBitsComboBox();
    void fillFlowControlComboBox();
    void readSerialData();
    std::vector<uint8_t> parseHexString(const QString& text);

    int counter_send {0};

};
#endif // MAINWINDOW_H
