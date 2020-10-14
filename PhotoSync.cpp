#include "PhotoSync.h"
#include <QtWidgets\QFileDialog>
#include <QtWidgets\QMessageBox.h>


PhotoSync::PhotoSync(QWidget *parent)
    : QMainWindow(parent)
{
    m_ui.setupUi(this);
    QObject::connect(m_ui.importToolButton, &QToolButton::clicked, this, &PhotoSync::askImportFolder);
    QObject::connect(m_ui.exportToolButton, &QToolButton::clicked, this, &PhotoSync::askExportFolder);
    QObject::connect(m_ui.positivePushButton, &QToolButton::clicked, this, &PhotoSync::run);
    QObject::connect(m_ui.negativePushButton, &QToolButton::clicked, this, &PhotoSync::close);
}

void PhotoSync::askImportFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Import directory path", m_ui.importEdit->text(), QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        m_ui.importEdit->setText(directory);
}

void PhotoSync::askExportFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Export directory path", m_ui.exportEdit->text(), QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        m_ui.exportEdit->setText(directory);
}

void PhotoSync::run()
{
    if (m_ui.importEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Path error", "Import path is empty !");
        return;
    }
    if (m_ui.exportEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Path error", "Export path is empty !");
        return;
    }

    if (!QDir(m_ui.importEdit->text()).exists()) {
        QMessageBox::warning(this, "Path error", "Import path : \"" + m_ui.importEdit->text() + "\" not found !");
        return;
    }
    if (!QDir(m_ui.exportEdit->text()).exists()) {
        QMessageBox::warning(this, "Path error", "Export path : \"" + m_ui.exportEdit->text() + "\" not found !");
        return;
    }
    
    FileManager fileManager(m_ui);
    fileManager.run();

    //Todo check errors ??
}
