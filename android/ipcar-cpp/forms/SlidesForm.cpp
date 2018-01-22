#include "SlidesForm.h"
#include "Utils.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QStackedLayout>
#include <QtGui/QImage>

//  SlidesForm::SlidesForm
//  -------------------------------------------------------------------------
SlidesForm::SlidesForm(QWidget *parent)
    : QWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/slides.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_back, "slides_back");    
    assign(this, m_next, "slides_next");
    assign(this, m_slide,"slides_slide");

    connect(m_back, SIGNAL(clicked()), this, SLOT(onBack()));   m_back->raise();
    connect(m_next, SIGNAL(clicked()), this, SLOT(onNext()));   m_next->raise();

    m_back->hide();
    m_next->show();

    m_current   = 0;

    updateUI();
}

//  SlidesForm::updateUI
//  -------------------------------------------------------------------------
void    SlidesForm::updateUI()
{
    QString filename;

    filename    = QString("qss/slidesForm/slide_%1.png")
                    .arg(m_current + 1, 2, 10, QChar('0'));
    filename    = getFilename(filename, false);

    QImage  image(filename);
    m_slide->setPixmap(QPixmap::fromImage(image));
    m_slide->adjustSize();
}

//  SlidesForm::onBack
//  -------------------------------------------------------------------------
void    SlidesForm::onBack()
{
    QString filename;

    updateUI();

    --m_current;

    //  check that we have more slides
    filename    = QString("qss/slidesForm/slide_%1.png")
                    .arg(m_current + 1, 2, 10, QChar('0'));
    filename    = getFilename(filename, false);

    m_back->setShown(filename != "");
    m_next->show();
}

//  SlidesForm::onNext
//  -------------------------------------------------------------------------
void    SlidesForm::onNext()
{
    QString filename;

    updateUI();

    ++m_current;

    //  check that we have more slides
    filename    = QString("qss/slidesForm/slide_%1.png")
                    .arg(m_current + 1, 2, 10, QChar('0'));
    filename    = getFilename(filename, false);

    m_next->setShown(filename != "");
    m_back->show();
}

