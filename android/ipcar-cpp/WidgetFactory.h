#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#ifndef  ANDROID_VERSION

QString     getQPushButtonStylesheet(
                        const QString&  objname);

class  WidgetFactory
{
    public:
        template<typename T> static
        T*      Build(const QString&    text,
                      const QString&    objectname)
        {
            T*  widget  = new T();
            widget->setObjectName(objectname);
            widget->setText(text);
            return  widget;
        }

        template<typename T> static
        T*              Build(const QString&    objectname)
        {
            T*  widget  = new T();
            widget->setObjectName(objectname);
            return  widget;
        }

        template<> static
        QPushButton*    Build<QPushButton>(const QString&    objectname)
        {
            QPushButton*    widget  = new QPushButton;

            widget->setObjectName(objectname);
            widget->setStyleSheet(getQPushButtonStylesheet(objectname));

            return  widget;
        }

        static
        QLabel* BlackOnWhite(const QString& text)
        {
            QLabel* label   = new QLabel(text);

            label->setProperty("type", QVariant("blackOnWhite"));
//            label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

            return  label;
        }

        static
        QLabel* WhiteOnBlack(const QString& text)
        {
            QLabel* label   = new QLabel(text);

            label->setProperty("type", QVariant("whiteOnBlack"));
//            label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

            return  label;
        }
};

#endif
#endif // WIDGETFACTORY_H
