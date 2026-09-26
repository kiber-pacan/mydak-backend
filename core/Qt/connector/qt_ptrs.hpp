//
// Created by akicatt on 13.09.2026.
//

#ifndef MYDAK_BACKEND_QT_PTRS_H
#define MYDAK_BACKEND_QT_PTRS_H
#include <future>
#include <QGuiApplication>
#include <qobject.h>
#include <QQmlApplicationEngine>

namespace mydak {
    // Struct for holding pointers to qt objects
    struct qt_ptrs{
        qt_ptrs() = default;
        qt_ptrs(const qt_ptrs& connector) = default;

        QObject* dialog_rectangle{};
        QObject* recipient_bar{};
        QObject* user_bar{};
        QObject* dialog_selector{};

        QGuiApplication* app{};
        QQmlApplicationEngine* app_engine{};


        void test();
    };
}


#endif //MYDAK_BACKEND_QT_PTRS_H
