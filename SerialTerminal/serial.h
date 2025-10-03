#pragma once

#include <string>
#include <vector>
#include <stdint.h>

class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    const std::vector<std::string>& get_port_list() const;
    bool select_port(int index);
    bool select_port(const std::string& name);
    std::string get_selected_port() const;

    bool connect(int baudrate = 2000000);
    void disconnect();

    bool send_ascii(const std::string& message);
    bool send_hex(const std::vector<uint8_t>& hex_data);
    std::string receive();

private:
    std::vector<std::string> port_list_;
    std::string selected_port_;
    int fd_;

    void search_ports();
    bool setup_port(int baudrate);
};
