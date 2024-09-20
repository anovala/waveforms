#pragma once
#include <QString>
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QJsonObject>
#include <QJsonArray>
#include <QPushButton>

enum WidgetType
{
    Label,
    CheckBox,
    RadioButton,
    ComboBox,
    SpinBox,
    DoubleSpinBox,
    LineEdit,
    PushButton,
    Null
};

struct WidgetInfo
{
    QString key;
    QString name;
    WidgetType type;
    QWidget* widget;
};

using WidgetHashType = QHash<QString, WidgetInfo>;

/// 移除"@@"符号函数
inline QString removeSymbols(QString input)
{
    static QRegularExpression regex("@@");
    QRegularExpressionMatch match = regex.match(input);
    if(match.hasMatch())
    {
        auto index = input.indexOf("@@");
        auto ret = input.replace(index, 2,"");
        return ret;
    }
    return input;
}


static WidgetType toWidgetType(const QString& type)
{
    static const QHash<QString, WidgetType> toEnumMap =
        {
            {"Label",			Label},
            {"CheckBox",		CheckBox},
            {"RadioButton",		RadioButton},
            {"ComboBox",		ComboBox},
            {"SpinBox",			SpinBox},
            {"DoubleSpinBox",	DoubleSpinBox},
            {"LineEdit",		LineEdit},
            {"PushButton",		PushButton}
        };
    auto&& pos = toEnumMap.find(type);
    if(pos != toEnumMap.end()) return pos.value();
    else return Null;
}

static WidgetInfo createrWidgetInfo(const QJsonObject object, QWidget *parent)
{
    auto name = object["name"].toString();
    auto type = toWidgetType(object["type"].toString());
    auto info = WidgetInfo{name,name, type, nullptr};
    switch (type)
    {
    case Label:
    {
        auto label = new QLabel(parent);
        label->setStyleSheet("background-color: #ffffff; padding: 4 4 4 4 px; border-radius: 5px;");
        info.widget = label;
        break;
    }
    case CheckBox: info.widget = new QCheckBox(name, parent); break;
    case RadioButton: info.widget = new QRadioButton(parent); break;
    case ComboBox:
    {
        auto box = new QComboBox(parent);
        auto ranges = object["ranges"].toArray();
        for(auto&& r : ranges) box->addItem(r.toString());
        info.widget = box;
        break;
    }
    case SpinBox:
    {
        auto spin = new QSpinBox(parent);
        auto ranges = object["ranges"].toArray();
        spin->setRange(ranges.at(0).toDouble(), ranges.at(1).toDouble());
        auto unit = object["unit"].toString(QString());
        spin->setSuffix(unit);
        spin->setSingleStep(1);
        if(!unit.isEmpty())
            name = QString("%1(%2)").arg(name,unit);
        info.widget = spin;
        break;
    }
    case DoubleSpinBox:
    {
        auto spin = new QDoubleSpinBox(parent);
        auto ranges = object["ranges"].toArray();
        spin->setRange(ranges.at(0).toDouble(), ranges.at(1).toDouble());
        auto unit = object["unit"].toString(QString());
        spin->setSuffix(unit);
        spin->setSingleStep(object["decimals"].toDouble(1));
        spin->setDecimals(object["decimals"].toInt(3));
        if(!unit.isEmpty())
            name = QString("%1(%2)").arg(name,unit);
        info.widget = spin;
        break;
    }
    case LineEdit: info.widget = new QLineEdit(parent); break;
    case PushButton: info.widget = new QPushButton(name, parent); break;
    default: break;
    }
    if(info.widget)
    {
        auto objName = removeSymbols(name);
        info.widget->setObjectName(objName);
    }
    return info;
}
