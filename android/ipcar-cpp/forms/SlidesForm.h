#ifndef SLIDESFORM_H
#define SLIDESFORM_H

#include <QtGui/QWidget>
#include <QtGui/QStackedLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>

class SlidesForm
    : public QWidget
{
    Q_OBJECT

    public:
        explicit SlidesForm(QWidget *parent = 0);

    private slots:
        void    onBack();
        void    onNext();

    private:
        void    updateUI();

    private:
        QPushButton*    m_back;
        QPushButton*    m_next;
        QLabel*         m_slide;
        int             m_current;
};

#endif // SLIDESFORM_H
