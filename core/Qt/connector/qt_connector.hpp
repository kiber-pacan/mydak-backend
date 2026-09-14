//
// Created by akicatt on 14.09.2026.
//

#ifndef MYDAK_BACKEND_QT_CONNECTOR_H
#define MYDAK_BACKEND_QT_CONNECTOR_H
#include <qobject.h>

#include "client.hpp"
#include "logger.hpp"
#include "qt_connector.hpp"

namespace mydak {
    // Class for connecting mydak backend and Qt quick ui
    class qt_connector : public QObject {
        Q_OBJECT
    public:
        explicit qt_connector(client& c) : c(c) {}

        client& c;

        Q_INVOKABLE
        void send_message(const QString& message) const;
    };
}

#endif //MYDAK_BACKEND_QT_CONNECTOR_H
