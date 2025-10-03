#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include "serial.h"

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
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    SerialPort serial_;
    bool isConnected_ = false;
    int selectedBaudrate_= 2000000;
    QButtonGroup *buttonGroup_;

    void combobox_port_set();
    void updateConnectionButton();
    void fillBaudrateComboBox();

};
#endif // MAINWINDOW_H
