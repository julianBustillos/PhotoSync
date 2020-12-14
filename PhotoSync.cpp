#include "PhotoSync.h"
#include "MTPProxyModel.h"


PhotoSync::PhotoSync(QWidget *parent)
    : QMainWindow(parent), m_fileManager(this, m_ui)
{
    m_ui.setupUi(this);
    m_ui.progressBar->setValue(0);

    m_positiveDefaultText = m_ui.positivePushButton->text();

    QObject::connect(m_ui.importToolButton, &QToolButton::clicked, this, &PhotoSync::askImportFolder);
    QObject::connect(m_ui.exportToolButton, &QToolButton::clicked, this, &PhotoSync::askExportFolder);
    QObject::connect(m_ui.positivePushButton, &QToolButton::clicked, this, &PhotoSync::run);
    QObject::connect(m_ui.negativePushButton, &QToolButton::clicked, this, &PhotoSync::close);

PhotoSync::~PhotoSync()
{
}

void PhotoSync::askImportFolder()
{
    m_dialog.setWindowTitle("Import directory path");
    m_dialog.setDirectory(m_ui.importEdit->text());
    m_dialog.exec();
    QString directory = m_dialog.getDirectory();

    if (!directory.isEmpty())
        m_ui.importEdit->setText(directory);
}

void PhotoSync::askExportFolder()
{
    m_dialog.setWindowTitle("Export directory path");
    m_dialog.setDirectory(m_ui.exportEdit->text());
    m_dialog.exec();
    QString directory = m_dialog.getDirectory();

    if (!directory.isEmpty())
        m_ui.exportEdit->setText(directory);
}

void PhotoSync::run()
{
    QObject::disconnect(m_ui.positivePushButton, nullptr, nullptr, nullptr);
    QObject::connect(m_ui.positivePushButton, &QToolButton::clicked, this, &PhotoSync::cancel);
    m_ui.positivePushButton->setText("Cancel");

    m_fileManager.run();

    QObject::disconnect(m_ui.positivePushButton, nullptr, nullptr, nullptr);
    QObject::connect(m_ui.positivePushButton, &QToolButton::clicked, this, &PhotoSync::run);
    m_ui.positivePushButton->setText(m_positiveDefaultText);
}

void PhotoSync::cancel()
{
    m_fileManager.cancel();
}
