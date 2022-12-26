greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += \
    src/debris.h \
    src/dummy.h \
    src/gamescene.h \
    src/missile.h \
    src/physics_object.h \
    src/pixmapmanager.h \
    src/resource_holder.h \
    src/team.h \
    src/utils.h \
    src/view.h \
    src/worm.h

SOURCES += \
    src/debris.cpp \
    src/dummy.cpp \
    src/gamescene.cpp \
    src/main.cpp \
    src/missile.cpp \
    src/physics_object.cpp \
    src/pixmapmanager.cpp \
    src/team.cpp \
    src/view.cpp \
    src/worm.cpp

RESOURCES += \
    resource.qrc
