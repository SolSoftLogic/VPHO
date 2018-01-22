#include "Utils.h"
#include "FileChooser.h"

#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QtCore/QStringList>

FileChooser::FileChooser(QWidget *parent) :
    QListView(parent)
{
#ifdef HOME_PATH_SDCARD
    QDir    home    = QDir("/sdcard");
#else
    QDir    home    = QDir::home();
#endif
    home.cd("./" PROJECTNAME);

    m_model.setFilter(QDir::AllEntries);
    m_model.setResolveSymlinks(true);
    m_root  = m_model.setRootPath(home.absolutePath());

    QStringList nameFilter;
    nameFilter.append(".");

    setModel(&m_model);

    setViewMode(QListView::IconMode);
    setFlow(QListView::TopToBottom);
    setMovement(QListView::Static);
    setUniformItemSizes(true);
//    setGridSize(QSize(200, 50));
    setWordWrap(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setRootIndex(m_root);

    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
}

//  FileChooser::onClicked
//  -------------------------------------------------------------------------
void    FileChooser::onClicked(QModelIndex  index)
{
    if(m_model.isDir(index))
    {
        QString path    = m_model.data(index,  QFileSystemModel::FilePathRole).toString();
        QString root    = m_model.data(m_root, QFileSystemModel::FilePathRole).toString();

        QDir    dir(path);

        if(dir.absolutePath().left(root.length()) == root)
        {
            setRootIndex(m_model.index(dir.absolutePath()));
        }
    }
    else
    {
        emit selected(m_model.data(index, QFileSystemModel::FilePathRole).toString());
    }
}
