include(../../../qbs_version.pri)

isEmpty(QBSLIBDIR) {
    QBSLIBDIR = $${OUT_PWD}/../../../lib
}

LIBNAME=qbsqtprofilesetup

unix {
    LIBS += -L$${QBSLIBDIR} -l$${LIBNAME}
}

!disable_rpath {
    linux-*:QMAKE_LFLAGS += -Wl,-z,origin \'-Wl,-rpath,\$\$ORIGIN/../lib\'
    macx:QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../lib
}

!CONFIG(static, static|shared) {
    QBSQTPROFILELIBSUFFIX = $${QBS_VERSION_MAJ}
}

win32 {
    CONFIG(debug, debug|release) {
        QBSQTPROFILELIB = $${LIBNAME}d$${QBSQTPROFILELIBSUFFIX}
    }
    CONFIG(release, debug|release) {
        QBSQTPROFILELIB = $${LIBNAME}$${QBSQTPROFILELIBSUFFIX}
    }
    win32-msvc* {
        LIBS += /LIBPATH:$$QBSLIBDIR
        QBSQTPROFILELIB = $${QBSQTPROFILELIB}.lib
        LIBS += Shell32.lib
    } else {
        LIBS += -L$${QBSLIBDIR}
        QBSQTPROFILELIB = lib$${QBSQTPROFILELIB}
    }
    LIBS += $${QBSQTPROFILELIB}
}

INCLUDEPATH += \
    $$PWD

CONFIG += depend_includepath

CONFIG(static, static|shared) {
    DEFINES += QBS_STATIC_LIB
}