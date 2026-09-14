//
// Created by akicatt on 14.09.2026.
//

#include "qt_connector.hpp"



void mydak::qt_connector::send_message(const QString& message) const {
    c.send_message(message.toStdString());
}
