//
// Created by akicatt on 13.09.2026.
//

#ifndef MYDAK_BACKEND_MESSAGE_MODEL_H
#define MYDAK_BACKEND_MESSAGE_MODEL_H
#include <qtmetamacros.h>
#include <string_view>
#include <utility>
#include <qabstractitemmodel.h>
#include <qqmlintegration.h>

namespace mydak {
    struct message_obj {
        message_obj(QString message, const std::uint8_t type)
        : message(std::move(message)), type(type) {}

        static constexpr std::size_t properties_count = 2;
        QString message{};
        std::uint8_t type{};
    };

    class Message_model : public QAbstractListModel  {
        Q_OBJECT
        QML_NAMED_ELEMENT(Message_model)
    public:
        explicit Message_model(QObject* parent = nullptr) : QAbstractListModel(parent) {}
        [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
        [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

        Q_INVOKABLE void add_message(const QString &message, uint8_t type);
        //Q_INVOKABLE void append_messages(const std::vector<std::pair<std::string, message_type>> &message, uint8_t type);
        Q_INVOKABLE void clear();
    private:
        QList<message_obj> messages;
    };
}

#endif //MYDAK_BACKEND_MESSAGE_MODEL_H
