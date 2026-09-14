//
// Created by akicatt on 13.09.2026.
//

#include "message_model.hpp"
#include <iostream>

int mydak::Message_model::rowCount(const QModelIndex& parent) const {
    return static_cast<int>(std::size(messages));
}

QVariant mydak::Message_model::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= std::size(messages)) return {};

    // Message or type
    switch (role) {
        case Qt::UserRole + 0: return messages[index.row()].message;
        case Qt::UserRole + 1: return messages[index.row()].type;
        default: return {};
    }
}

QHash<int, QByteArray> mydak::Message_model::roleNames() const {
    QHash<int, QByteArray> hash;
    hash[Qt::UserRole + 0] = "message";
    hash[Qt::UserRole + 1] = "type";
    return hash;
}

Q_INVOKABLE void mydak::Message_model::add_message(const QString& message, std::uint8_t type) {
    beginInsertRows({}, std::size(messages), std::size(messages));
    messages.append({message, type});
    endInsertRows();
}
