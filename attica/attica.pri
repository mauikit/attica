

#LIBS += -lc++_shared

QT += core network

ANDROID_EXTRA_LIBS += $$PWD/libKF5Attica.so
LIBS += -L$$PWD/./ -lKF5Attica

INCLUDEPATH += $$PWD/Attica
DEPENDPATH += $$PWD/Attica
