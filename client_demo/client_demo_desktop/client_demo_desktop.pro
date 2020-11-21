QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/client_demo/stream_render \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/ability \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/connection \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/detail_message_format \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/media_chain \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/net_message_listener \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/peer \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/server \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/loadbalance \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/resource \
    /home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/resource/stream_pusher

LIBS += -lzlog -lboost_thread -lnet_message_listener -lmessage -lserver -lability -ljsoncpp -lstream_render -lSDL2 \
    -L/home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/client_demo/stream_render/ \
    -L/home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/net_message_listener/ \
    -L/home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/detail_message_format/ \
    -L/home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/server/ \
    -L/home/codea/Documents/CDUT/Subjects/软件工程/MediaCore/common/ability/ \

SOURCES += \
    client.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    client.h \
    widget.h

FORMS += \
    widget.ui

TRANSLATIONS += \
    client_demo_desktop_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
