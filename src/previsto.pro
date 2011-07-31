QT      += core gui webkit
TARGET   = previsto
TEMPLATE = app
VERSION  = 0.1dev

SOURCES += \
    main.cpp\
    previewwindow.cpp \
    markdowndoc.cpp \
    appcontroller.cpp

HEADERS += \
    previewwindow.h \
    markdowndoc.h \
    appcontroller.h

FORMS   += \
    previewwindow.ui

RESOURCES += \
    resources/previsto.qrc

OTHER_FILES += \
    resources/Default.css \
    notes.markdown

# Installation -------------------------------------------------------------------------------------

target.path = $$OUT_PWD/root/usr/bin
INSTALLS += target
