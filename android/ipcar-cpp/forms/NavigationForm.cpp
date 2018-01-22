#include "NavigationForm.h"
#include "Utils.h"
#include "WidgetFactory.h"

#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>

QLabel*     createIcon(const char*  icon)
{
    QLabel*     label   = new QLabel;
    QPixmap*    pixmap  = new QPixmap(getFilename(icon));

    label->setPixmap(*pixmap);

    return  label;
}

NavigationForm::NavigationForm(QWidget *parent) :
    QWidget(parent)
{
    this->setObjectName("navigationForm");

    QGridLayout*    glayout = new QGridLayout;
    /*QHBoxLayout*    hlayout = new QHBoxLayout;
    QCheckBox*      checkbox;
    QPushButton*    pushbutton;

    glayout->addWidget(WidgetFactory::WhiteOnBlack(tr("Choose point of interest")),          0, 0, 1, 7);

    glayout->addWidget(createIcon("qss/navigationForm/poi_McDonalds.png"),                   1, 0, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        1, 1, Qt::AlignLeft);
    glayout->addWidget(createIcon("qss/navigationForm/poi_BurgerKing.png"),                  1, 2, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        1, 3, Qt::AlignLeft);
    glayout->addWidget(createIcon("qss/navigationForm/poi_ShellStation.png"),                1, 4, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        1, 5, Qt::AlignLeft);

    glayout->addWidget(WidgetFactory::Build<QWidget>("hseparator"),                     2, 0, 1, 7);

    glayout->addWidget(createIcon("qss/navigationForm/poi_HolidayInn.png"),                  3, 0, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        3, 1, Qt::AlignLeft);
    glayout->addWidget(createIcon("qss/navigationForm/poi_AppleStore.png"),                  3, 2, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        3, 3, Qt::AlignLeft);
    glayout->addWidget(createIcon("qss/navigationForm/poi_TMobile.png"),                     3, 4, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        3, 5, Qt::AlignLeft);

    glayout->addWidget(WidgetFactory::WhiteOnBlack(tr("Choose car status")),            4, 0, 1, 7);

    glayout->addWidget(createIcon("qss/navigationForm/css_CarSpeed.png"),                    5, 0, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        5, 1, Qt::AlignLeft);
    glayout->addWidget(createIcon("qss/navigationForm/css_Temperature.png"),                 5, 2, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        5, 3, Qt::AlignLeft);
    glayout->addWidget(createIcon("qss/navigationForm/css_Whell_PSI.png"),                   5, 4, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        5, 5, Qt::AlignLeft);

    glayout->addWidget(WidgetFactory::Build<QWidget>("hseparator"),                     6, 0, 1, 7);

    glayout->addWidget(createIcon("qss/navigationForm/css_RainActivity.png"),                7, 0, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        7, 1, Qt::AlignLeft);
    glayout->addWidget(createIcon("qss/navigationForm/css_LightSensor.png"),                 7, 2, Qt::AlignRight);
    glayout->addWidget(checkbox = new QCheckBox("    "),                                        7, 3, Qt::AlignLeft);

    QWidget*    bottom  = new QWidget;
    bottom->setObjectName("navigationForm--bottom");
    glayout->addWidget(bottom,                                                          8, 0, 1, 7);

    bottom->setLayout(hlayout);
    hlayout->addStretch();
    hlayout->addWidget(pushbutton = WidgetFactory::Build<QPushButton>("navigationForm_start"));
    hlayout->addWidget(pushbutton = WidgetFactory::Build<QPushButton>("navigationForm_cancel"));
    hlayout->addStretch();*/

	glayout->addWidget(createIcon("qss/navigationForm/background.png"),                   0, 0, Qt::AlignRight);
    this->setLayout(glayout);
    this->setContentsMargins(0, 0, 0, 0);
}
