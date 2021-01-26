#include "PhotoSync.h"
#include <QMessageBox>


WPDManager *PhotoSync::WPDInstance = nullptr;

PhotoSync::PhotoSync(QWidget *parent)
    : QMainWindow(parent)
{
    m_ui.setupUi(this);
    m_ui.progressBar->setValue(0);

    m_positiveDefaultText = m_ui.positivePushButton->text();

    m_fileManager = new FileManager(this);
    m_dialog = new FileExplorerDialog(this);

    QObject::connect(m_ui.importToolButton, &QToolButton::clicked, this, &PhotoSync::askImportFolder);
    QObject::connect(m_ui.exportToolButton, &QToolButton::clicked, this, &PhotoSync::askExportFolder);
    QObject::connect(m_ui.positivePushButton, &QToolButton::clicked, this, &PhotoSync::run);
    QObject::connect(m_ui.negativePushButton, &QToolButton::clicked, this, &PhotoSync::close);
    QObject::connect(m_fileManager, &FileManager::warning, this, &PhotoSync::createWarning);
    QObject::connect(m_fileManager, &FileManager::progressBarValue, this, &PhotoSync::setProgressBarValue);
    QObject::connect(m_fileManager, &FileManager::progressBarMaximum, this, &PhotoSync::setProgressBarMaximum);
    QObject::connect(m_fileManager, &FileManager::output, this, &PhotoSync::appendOutput);
    QObject::connect(m_fileManager, &FileManager::finished, this, &PhotoSync::finish);

    //DEBUG
    m_ui.importEdit->setText("Juju S8/Phone/IMPORT_PHOTOSYNC");
    //m_ui.importEdit->setText("C:/Users/Julian Bustillos/Downloads/IMPORT_PHOTOSYNC");
    //m_ui.exportEdit->setText("Juju S8/Phone/EXPORT_PHOTOSYNC");
    m_ui.exportEdit->setText("C:/Users/Julian Bustillos/Downloads/EXPORT_PHOTOSYNC");

    //DEBUG
}

PhotoSync::~PhotoSync()
{
    if (m_fileManager)
        delete m_fileManager;
    m_fileManager = nullptr;

    if (m_dialog)
        delete m_dialog;
    m_dialog = nullptr;

    if (WPDInstance)
        delete WPDInstance;
    WPDInstance = nullptr;
}

WPDManager & PhotoSync::getWPDInstance()
{
    if (!WPDInstance)
        WPDInstance = new WPDManager();
    return *WPDInstance;
}

void PhotoSync::askImportFolder()
{
    if (m_dialog) {
        m_dialog->setWindowTitle("Import directory path");
        m_dialog->setDirectory(m_ui.importEdit->text());
        m_dialog->exec();
        QString directory = m_dialog->getDirectory();

        if (!directory.isEmpty())
            m_ui.importEdit->setText(directory);
    }
}

void PhotoSync::askExportFolder()
{
    if (m_dialog) {
        m_dialog->setWindowTitle("Export directory path");
        m_dialog->setDirectory(m_ui.exportEdit->text());
        m_dialog->exec();
        QString directory = m_dialog->getDirectory();

        if (!directory.isEmpty())
            m_ui.exportEdit->setText(directory);
    }
}

void PhotoSync::run()
{
    QObject::disconnect(m_ui.positivePushButton, nullptr, nullptr, nullptr);
    QObject::connect(m_ui.positivePushButton, &QToolButton::clicked, m_fileManager, &FileManager::cancel);
    m_ui.positivePushButton->setText("Cancel");

    if (m_fileManager) {
        m_fileManager->setPaths(m_ui.importEdit->text(), m_ui.exportEdit->text());
        m_fileManager->start(QThread::NormalPriority);
    }
}

void PhotoSync::createWarning(QString title, QString message)
{
    QMessageBox::warning(this, title, message);
}

void PhotoSync::setProgressBarValue(int value)
{
    m_ui.progressBar->setValue(value);
}

void PhotoSync::setProgressBarMaximum(int maximum)
{
    m_ui.progressBar->setMaximum(maximum);
}

void PhotoSync::appendOutput(QString output)
{
    m_ui.textEditOutput->append(output);
}

void PhotoSync::finish()
{
    QObject::disconnect(m_ui.positivePushButton, nullptr, nullptr, nullptr);
    QObject::connect(m_ui.positivePushButton, &QToolButton::clicked, this, &PhotoSync::run);
    m_ui.positivePushButton->setText(m_positiveDefaultText);
}
