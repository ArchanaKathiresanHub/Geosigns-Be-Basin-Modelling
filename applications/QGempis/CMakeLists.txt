CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

PROJECT(QGempis)

FIND_PACKAGE(Qt4 REQUIRED)

include(${QT_USE_FILE})

INCLUDE_DIRECTORIES( . )
INCLUDE_DIRECTORIES( SettingsUI )

# Add Chilkat stuff
INCLUDE_DIRECTORIES("C:\\ChilkatVC8\\include")
LINK_DIRECTORIES("C:\\ChilkatVC8\\libs")

QT4_WRAP_UI(QGEMPIS_UIS_H MainWindow.ui SettingsUI/PasswordDialog.ui WaitWidget.ui )
QT4_WRAP_CPP(QGEMPIS_MOCS MainWindow.h SettingsUI/PasswordDialog.h BackgroundSSHSession.h SSHSession.h WaitWidget.h)

ADD_EXECUTABLE(qgempis main.cpp ${QGEMPIS_UIS_H} ${QGEMPIS_MOCS} MainWindow.cpp SettingsUI/PasswordDialog.cpp GlobalSettings.cpp SSHSession.cpp BackgroundSSHSession.cpp WaitWidget.cpp)
# See http://www.chilkatsoft.com/faq/LinkErrors.html
TARGET_LINK_LIBRARIES(qgempis ${QT_LIBRARIES} ChilkatRel wininet rpcrt4 crypt32 ws2_32)
