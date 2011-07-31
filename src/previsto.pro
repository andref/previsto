QT      += core gui webkit
TARGET   = previsto
TEMPLATE = app
VERSION  = 0.1dev

SOURCES += \
    main.cpp\
    previewwindow.cpp \
    markdowndoc.cpp \
    previewwindowcontroller.cpp

HEADERS += \
    previewwindow.h \
    markdowndoc.h \
    previewwindowcontroller.h

FORMS   += \
    previewwindow.ui

RESOURCES += \
    resources/previsto.qrc

OTHER_FILES += \
    resources/Default.css

# Installation -------------------------------------------------------------------------------------

target.path = $$OUT_PWD/root/usr/bin
INSTALLS += target
