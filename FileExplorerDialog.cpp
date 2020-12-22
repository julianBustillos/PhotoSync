#include "FileExplorerDialog.h"
#include <QModelIndex>


FileExplorerDialog::FileExplorerDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    //m_ui.fileTreeView->setModel(&m_fileSystemModel);
    m_ui.fileTreeView->setModel(&m_MTPFileModel);

    // TODO: set only directories for filesystemmodel
    /*
    m_fileDialog.setFileMode(QFileDialog::Directory);
    m_fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    m_fileDialog.setOption(QFileDialog::ShowDirsOnly, false);
    */

    QObject::connect(&m_MTPFileModel, &MTPFileModel::rootPathChanged, this, &FileExplorerDialog::rootPathChanged);
    QObject::connect(m_ui.chooseButton, &QPushButton::clicked, this, &FileExplorerDialog::chooseDirectory);
}

FileExplorerDialog::~FileExplorerDialog()
{
}

void FileExplorerDialog::setDirectory(QString directory)
{
    //QModelIndex dirIndex = m_fileSystemModel.setRootPath(directory);
    m_MTPFileModel.setRootPath(directory);

}

QString FileExplorerDialog::getDirectory()
{
    return m_directory;
}

void FileExplorerDialog::rootPathChanged(const QModelIndex &rootPathIndex)
{

    //m_directory = rootPathIndex.isValid() ? directory : ""; //TODO get directory from index
    m_ui.fileTreeView->collapseAll();
    m_ui.fileTreeView->scrollTo(rootPathIndex, QAbstractItemView::PositionAtCenter);
    m_ui.fileTreeView->setCurrentIndex(rootPathIndex);
}

void FileExplorerDialog::chooseDirectory()
{
    QModelIndex index = m_ui.fileTreeView->currentIndex();
    QString directory;
    if (index.isValid())
        directory = m_MTPFileModel.filePath(index);
        //directory = m_fileSystemModel.filePath(index);
    m_directory = directory;
}
