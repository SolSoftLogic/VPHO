#ifndef FILECHOOSER_H
#define FILECHOOSER_H

#include <QtGui/QListView>
#include <QtGui/QFileSystemModel>

class FileChooser
    : public QListView
{
    Q_OBJECT

    public:
        explicit FileChooser(QWidget *parent = 0);

    signals:
        void    selected(   const QString&  file);

    public slots:
        void    onClicked(QModelIndex   index);

    private:
        QFileSystemModel    m_model;
        QModelIndex         m_root;
};

#endif // FILECHOOSER_H
