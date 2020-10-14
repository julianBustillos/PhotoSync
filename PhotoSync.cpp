#include "PhotoSync.h"
#include <QtWidgets\QFileDialog>


PhotoSync::PhotoSync(QWidget *parent)
    : QMainWindow(parent)
{
    m_ui.setupUi(this);
    QObject::connect(m_ui.importToolButton, &QToolButton::clicked, this, &PhotoSync::askImportFolder);
    QObject::connect(m_ui.exportToolButton, &QToolButton::clicked, this, &PhotoSync::askExportFolder);
    QObject::connect(m_ui.negativePushButton, &QToolButton::clicked, this, &PhotoSync::close);
}

void PhotoSync::askImportFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Todo", m_ui.importEdit->text(), QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        m_ui.importEdit->setText(directory);
}

void PhotoSync::askExportFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, "Todo", m_ui.exportEdit->text(), QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty())
        m_ui.exportEdit->setText(directory);
}
