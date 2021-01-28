#include "FileExplorerDialog.h"
#include "AggregateModel.h"
#include "MTPFileModel.h"
#include "FileSystemProxyModel.h"
#include <QModelIndex>


FileExplorerDialog::FileExplorerDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    AggregateModel *aggregateModel = new AggregateModel(this);
    aggregateModel->addModel(new FileSystemProxyModel(this));
    aggregateModel->addModel(new MTPFileModel(this));
    m_aggregableModel = aggregateModel;
    m_ui.fileTreeView->setModel(m_aggregableModel);
    m_ui.fileTreeView->setColumnWidth(0, 300);

    QObject::connect(m_aggregableModel, &AggregableItemModel::rootPathChanged, this, &FileExplorerDialog::rootPathChanged);
    QObject::connect(m_ui.chooseButton, &QPushButton::clicked, this, &FileExplorerDialog::chooseDirectory);
}

FileExplorerDialog::~FileExplorerDialog()
{
    if (m_aggregableModel)
        delete m_aggregableModel;
    m_aggregableModel = nullptr;
}

void FileExplorerDialog::setDirectory(QString directory)
{
    m_ui.fileTreeView->collapseAll();
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
    m_ui.fileTreeView->scrollTo(rootPathIndex);
    m_ui.fileTreeView->setCurrentIndex(rootPathIndex);
}

void FileExplorerDialog::chooseDirectory()
{
    QModelIndex index = m_ui.fileTreeView->currentIndex();
    m_directory = m_aggregableModel->filePath(index);
}
