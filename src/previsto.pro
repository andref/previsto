QT      += core gui webkit
TARGET   = previsto
TEMPLATE = app
VERSION  = 0.3

SOURCES += \
    main.cpp\
    previewwindow.cpp \
    markdowndoc.cpp \
    appcontroller.cpp \
    aboutwindow.cpp

HEADERS += \
    previewwindow.h \
    markdowndoc.h \
    appcontroller.h \
    aboutwindow.h

FORMS   += \
    previewwindow.ui \
    aboutwindow.ui

RESOURCES += \
    resources/previsto.qrc

OTHER_FILES += \
    resources/Default.css \
    notes.markdown

# Installation -------------------------------------------------------------------------------------

target.path = $$OUT_PWD/root/usr/bin
INSTALLS += target
