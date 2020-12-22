#include "FileExplorerDialog.h"
#include "MTPFileModel.h"
#include <QModelIndex>


FileExplorerDialog::FileExplorerDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    m_aggregableModel = new MTPFileModel(this);
    m_ui.fileTreeView->setModel(m_aggregableModel);
    //m_ui.fileTreeView->setModel(&m_fileSystemModel);
    //m_ui.fileTreeView->setModel(&m_MTPFileModel);
    m_ui.fileTreeView->setColumnWidth(0, 300);

    // TODO: set only directories for filesystemmodel
    /*
    m_fileDialog.setFileMode(QFileDialog::Directory);
    m_fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    m_fileDialog.setOption(QFileDialog::ShowDirsOnly, false);
    */

    //QObject::connect(&m_MTPFileModel, &MTPFileModel::rootPathChanged, this, &FileExplorerDialog::rootPathChanged);
    QObject::connect(m_aggregableModel, &AggregableItemModel::rootPathChanged, this, &FileExplorerDialog::rootPathChanged);
    QObject::connect(m_ui.chooseButton, &QPushButton::clicked, this, &FileExplorerDialog::chooseDirectory);
}

FileExplorerDialog::~FileExplorerDialog()
{
    if (m_aggregableModel)
        delete m_aggregableModel;
}

void FileExplorerDialog::setDirectory(QString directory)
{
    //QModelIndex dirIndex = m_fileSystemModel.setRootPath(directory);
    //m_MTPFileModel.setRootPath(directory);
    if (m_aggregableModel)
        m_aggregableModel->setRootPath(directory);
}

QString FileExplorerDialog::getDirectory()
{
    return m_directory;
}

void FileExplorerDialog::rootPathChanged(const QModelIndex &rootPathIndex)
{

    //m_directory = m_MTPFileModel.filePath(rootPathIndex);
    m_directory = m_aggregableModel->filePath(rootPathIndex);
    m_ui.fileTreeView->collapseAll();
    m_ui.fileTreeView->scrollTo(rootPathIndex);
    m_ui.fileTreeView->setCurrentIndex(rootPathIndex);
}

void FileExplorerDialog::chooseDirectory()
{
    QModelIndex index = m_ui.fileTreeView->currentIndex();
    QString directory;
    if (index.isValid())
        directory = m_aggregableModel->filePath(index);
        //directory = m_MTPFileModel.filePath(index);
        //directory = m_fileSystemModel.filePath(index);
    m_directory = directory;
}
