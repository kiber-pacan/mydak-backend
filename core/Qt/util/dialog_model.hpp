//
// Created by akicatt on 26.09.2026.
//

#ifndef MYDAK_BACKEND_DIALOG_MODEL_H
#define MYDAK_BACKEND_DIALOG_MODEL_H

#include <qtmetamacros.h>
#include <string_view>
#include <utility>
#include <qabstractitemmodel.h>
#include <qqmlintegration.h>

#include "namer.hpp"
#include "proto.hpp"

namespace mydak {
    struct dialog_model {
        dialog_model() = default;
        dialog_model(
            const QByteArray& q_client
        )
            : client(q_client)
        {
            uint16_t value;
            memcpy(&value, q_client.data(), sizeof(value));
            name = namer::get_name(value);
        }

        QByteArray client{};
        std::string_view name;
    };

    class Dialog_model : public QAbstractListModel  {
        Q_OBJECT
        QML_NAMED_ELEMENT(Dialog_model)
    public:
        explicit Dialog_model(QObject* parent = nullptr) : QAbstractListModel(parent) {}
        [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
        [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

        Q_INVOKABLE void add_dialog(const QByteArray& client);
        Q_INVOKABLE void set_dialog(int index);

        //Q_INVOKABLE void append_messages(const std::vector<std::pair<std::string, message_type>> &message, uint8_t type);
        Q_INVOKABLE void clear();
    private:
        QList<dialog_model> dialogs;
    };
}

#endif //MYDAK_BACKEND_DIALOG_MODEL_H
