//
// Created by akicatt on 26.09.2026.
//

#include "dialog_model.hpp"

#include "channel_holder.hpp"
#include "coh.hpp"


int mydak::Dialog_model::rowCount(const QModelIndex& parent) const {
    return static_cast<int>(std::size(dialogs));
}

QVariant mydak::Dialog_model::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= std::size(dialogs)) return {};

    // Message or type
    switch (role) {
        case Qt::UserRole + 0: return dialogs[index.row()].client;
        case Qt::UserRole + 1: return
            QString::fromUtf8(
                dialogs[index.row()].name.data(),
                static_cast<int>(std::size(dialogs[index.row()].name))
            );
        default: return {};
    }
}

QHash<int, QByteArray> mydak::Dialog_model::roleNames() const {
    QHash<int, QByteArray> hash;
    hash[Qt::UserRole + 0] = "client";
    hash[Qt::UserRole + 1] = "name";
    return hash;
}

Q_INVOKABLE void mydak::Dialog_model::add_dialog(const QByteArray& client) {
    beginInsertRows({}, static_cast<int>(std::size(dialogs)), static_cast<int>(std::size(dialogs)));
    dialogs.append(client);
    endInsertRows();
}

Q_INVOKABLE void mydak::Dialog_model::set_dialog(int client_index) {
    auto& io = coh::io();
    std::array<unsigned char, proto::E2E_KEYS_RAW_L> client; // NOLINT(*-pro-type-member-init)
    const auto& q_client = dialogs[client_index].client;
    memcpy(client.data(), q_client.data(), std::size(client));

    coh::detached([client] () -> asio::awaitable<void> {
        boost::system::error_code e;
        co_await channel_holder::set_dialog_channel->async_send(e, client);
        co_return;
    });
}


//Q_INVOKABLE void append_messages(const std::vector<std::pair<std::string, message_type>> &message, uint8_t type) {

//}


Q_INVOKABLE void mydak::Dialog_model::clear() {
    if (dialogs.isEmpty()) return;

    beginResetModel();
    dialogs.clear();
    endResetModel();
}