#include "serial.h"

#include <filesystem>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <cstring>
#include <iostream>
#include <vector>
#include <string>

SerialPort::SerialPort() : fd_(-1) {
    search_ports();
}

SerialPort::~SerialPort() {
    disconnect();
}

void SerialPort::search_ports() {
    port_list_.clear();
    const std::string dev_path = "/dev/";

    for (const auto& entry : std::filesystem::directory_iterator(dev_path)) {
        const std::string name = entry.path().filename();
        if ( name.find("ttyUSB") == 0 || name.find("ttyACM") == 0) {
            port_list_.push_back(entry.path());
        }
    }
}
const std::vector<std::string>& SerialPort::get_port_list() const {
    return port_list_;
}

bool SerialPort::select_port(int index) {
    if (index < 0 || index >= static_cast<int>(port_list_.size())) return false;
    selected_port_ = port_list_[index];
    return true;
}

bool SerialPort::select_port(const std::string& name) {
    for (const auto& port : port_list_) {
        if (port == name) {
            selected_port_ = port;
            return true;
        }
    }
    return false;
}

std::string SerialPort::get_selected_port() const {
    return selected_port_;
}

bool SerialPort::connect(int baudrate) {
    if (fd_ >= 0) {
        std::cerr << "Port already connected.\n";
        return false;
    }
    if (selected_port_.empty()) {
        std::cerr << "No port selected!\n";
        return false;
    }
    fd_ = ::open(selected_port_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd_ < 0) {
        std::cerr << "Failed to open port: " << selected_port_ << "\n";
        return false;
    }
    return setup_port(baudrate);
}

void SerialPort::disconnect() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool SerialPort::setup_port(int baudrate) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd_, &tty) != 0) return false;

    speed_t speed;
    switch (baudrate) {
    case 9600: speed = B9600; break;
    case 19200: speed = B19200; break;
    case 38400: speed = B38400; break;
    case 115200: speed = B115200; break;
    default: speed = B2000000;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag &= ~CSIZE;
    switch (dataBits_) {
    case 5: tty.c_cflag |= CS5; break;
    case 6: tty.c_cflag |= CS6; break;
    case 7: tty.c_cflag |= CS7; break;
    case 8: tty.c_cflag |= CS8; break;
    default: tty.c_cflag |= CS8; break;
    }

    if (parity_ == "None") {
        tty.c_cflag &= ~(PARENB | PARODD);
    } else {
        if (parity_ == "Even") {
            tty.c_cflag &= ~PARODD;
            tty.c_cflag |= PARENB;
        } else if (parity_ == "Odd") {
            tty.c_cflag |= (PARENB | PARODD);
        } else if (parity_ == "Space") {
            tty.c_cflag &= ~PARODD;
            tty.c_cflag |= (PARENB | CMSPAR);
        } else if (parity_ == "Mark") {
            tty.c_cflag |= (PARENB | PARODD | CMSPAR);
        }
    }

    if (stopBits_ == 2) {
        tty.c_cflag |= CSTOPB;
    } else {
        tty.c_cflag &= ~CSTOPB;
    }

    if (flowControl_ == "Hardware") {
        tty.c_cflag |= CRTSCTS;
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    } else if (flowControl_ == "Software") {
        tty.c_cflag &= ~CRTSCTS;
        tty.c_iflag |= (IXON | IXOFF | IXANY);
    } else {
        tty.c_cflag &= ~CRTSCTS;
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    }

    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;
    tty.c_cflag |= (CLOCAL | CREAD);

    return tcsetattr(fd_, TCSANOW, &tty) == 0;
}

bool SerialPort::send_ascii(const std::string& message) {
    if (fd_ < 0) return false;
    std::string message_plus = message + "\r\n";
    ssize_t len = ::write(fd_, message_plus.c_str(), message_plus.size());
    return len == (ssize_t)message_plus.size();
}

bool SerialPort::send_hex(const std::vector<uint8_t>& hex_data) {
    if (fd_ < 0) return false;
    ssize_t len = ::write(fd_, hex_data.data(), hex_data.size());
    return len == (ssize_t)hex_data.size();
}

std::string SerialPort::receive() {
    if (fd_ < 0) return "";

    std::string result;
    std::vector<char> buf(2048);

    while (true) {
        ssize_t n = ::read(fd_, buf.data(), buf.size());
        if (n <= 0) break;
        result += std::string(buf.begin(), buf.begin() + n);
    }

    return result;
}
