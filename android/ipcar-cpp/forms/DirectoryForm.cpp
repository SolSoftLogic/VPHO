#include "DirectoryForm.h"
#include "Utils.h"
#include "Commands.h"

#include <QtGui/QVBoxLayout>

DirectoryForm::DirectoryForm(QWidget *parent) :
    QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/directory.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    m_fileChooser   = new FileChooser;

    assign(this, m_placeholder, "directory_placeholder");
    assign(this, m_close,       "directory_close");

    layout          = new QVBoxLayout;
    m_placeholder->setLayout(layout);
    layout->addWidget(m_fileChooser);

    connect(m_fileChooser,  SIGNAL(selected(QString)),   this,  SLOT(onSelected(QString)));
    connect(m_close,        SIGNAL(clicked()),          this,   SLOT(onCloseClicked()));
}

//  DirectoryForm::onCloseClicked
//  -------------------------------------------------------------------------
void    DirectoryForm::onCloseClicked()
{
    hideMe(this);
}

//  DirectoryForm::onSelected
//  -------------------------------------------------------------------------
void    DirectoryForm::onSelected(const QString& file)
{
    hideMe(this);
    emit    selected(file);
}

