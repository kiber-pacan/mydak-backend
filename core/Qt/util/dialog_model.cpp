//
// Created by akicatt on 26.09.2026.
//

#include "dialog_model.hpp"



int mydak::Dialog_model::rowCount(const QModelIndex& parent) const {
    return static_cast<int>(std::size(dialogs));
}

QVariant mydak::Dialog_model::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= std::size(dialogs)) return {};

    // Message or type
    switch (role) {
        case Qt::UserRole + 0: return dialogs[index.row()].name;
        default: return {};
    }
}

QHash<int, QByteArray> mydak::Dialog_model::roleNames() const {
    QHash<int, QByteArray> hash;
    hash[Qt::UserRole + 0] = "name";
    return hash;
}

Q_INVOKABLE void mydak::Dialog_model::add_dialog(const QString& name) {
    beginInsertRows({}, static_cast<int>(std::size(dialogs)), static_cast<int>(std::size(dialogs)));
    dialogs.append(name);
    endInsertRows();
}

//Q_INVOKABLE void append_messages(const std::vector<std::pair<std::string, message_type>> &message, uint8_t type) {

//}


Q_INVOKABLE void mydak::Dialog_model::clear() {
    if (dialogs.isEmpty()) return;

    beginResetModel();
    dialogs.clear();
    endResetModel();
}