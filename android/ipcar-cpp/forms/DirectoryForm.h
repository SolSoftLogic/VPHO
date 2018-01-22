#ifndef DIRECTORYFORM_H
#define DIRECTORYFORM_H

#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include "FileChooser.h"

class DirectoryForm
    : public QWidget
{
    Q_OBJECT
    public:
        explicit DirectoryForm(QWidget *parent = 0);

    signals:
        void            selected(const QString& file);

    public slots:
        void            onCloseClicked();
        void            onSelected(const QString& file);

    public:
        FileChooser*    m_fileChooser;
        QWidget*        m_placeholder;
        QPushButton*    m_close;
};

#endif // DIRECTORYFORM_H
