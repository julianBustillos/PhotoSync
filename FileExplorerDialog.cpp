#include "FileExplorerDialog.h"
#include <QModelIndex>


FileExplorerDialog::FileExplorerDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    m_ui.fileTreeView->setModel(&m_model);
    // TODO !! set only directories for filesystemmodel
}

FileExplorerDialog::~FileExplorerDialog()
{
}

void FileExplorerDialog::setDirectory(QString directory)
{
    QModelIndex dirIndex = m_model.setRootPath(directory);
    m_ui.fileTreeView->collapseAll();
    m_ui.fileTreeView->scrollTo(dirIndex);
    m_ui.fileTreeView->selectionModel()->select(dirIndex, QItemSelectionModel::Select);
}

QString FileExplorerDialog::getDirectory()
{
    // Todo !! Only when choosed !!
    QModelIndex index = m_ui.fileTreeView->currentIndex();
    QString path;
    if (index.isValid())
        path = m_model.filePath(index);
    return path;
}
