#include "FileExplorerDialog.h"
#include "MTPFileModel.h"
#include "FileSystemProxyModel.h"
#include <QModelIndex>

//DEBUG
#include "QIdentityProxyModelCOPY.h"
//DEBUG

FileExplorerDialog::FileExplorerDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    //m_aggregableModel = new MTPFileModel(this);
    m_aggregableModel = new FileSystemProxyModel(this);
    //m_aggregableModel = new QIdentityProxyModelCOPY(this);
    m_ui.fileTreeView->setModel(m_aggregableModel);
    m_ui.fileTreeView->setColumnWidth(0, 300);

    // TODO: set only directories for filesystemmodel
    /*
    m_fileDialog.setFileMode(QFileDialog::Directory);
    m_fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    m_fileDialog.setOption(QFileDialog::ShowDirsOnly, false);
    */

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
    if (m_aggregableModel)
        m_aggregableModel->setRootPath(directory);
}

QString FileExplorerDialog::getDirectory()
{
    return m_directory;
}

void FileExplorerDialog::rootPathChanged(const QModelIndex &rootPathIndex)
{
    m_directory = m_aggregableModel->filePath(rootPathIndex);
    m_ui.fileTreeView->collapseAll();
    m_ui.fileTreeView->scrollTo(rootPathIndex);
    m_ui.fileTreeView->setCurrentIndex(rootPathIndex);
}

void FileExplorerDialog::chooseDirectory()
{
    QModelIndex index = m_ui.fileTreeView->currentIndex();
    m_directory = m_aggregableModel->filePath(index);
}
