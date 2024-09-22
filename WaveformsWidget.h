#pragma once

#include "CreateUIBase.h"
#include "qvalueaxis.h"
#include <QWidget>
#include <QChart>
#include <QLineSeries>
#include <QSplineSeries>
#include <QChartView>

#define AMP "幅度"
#define FREQ "频率"
#define CYCLE "周期"
#define VOL "电压"
#define PERSIST "持续时间"
#define PHASE "相位"

QT_BEGIN_NAMESPACE
namespace Ui {
class WaveformsWidget;
}
QT_END_NAMESPACE

using WidgetListInfo = QList<WidgetInfo>;
using SearchValByName = double(*)(const WidgetListInfo &wlist, const QString &widetName);
using yAxisCal = std::function<double(double,double,double)>;
using yAxisCalSine = std::function<double(double,double,double,int)>;

enum WaveEnum{
    DC = 0,
    Swatooth,
    Sine,
    Square,
    Traingle
};

struct Range{
    double xMax = 0.0;
    double xMin = 0.0;
    double yMax = 0.0;
    double yMin = 0.0;
};


inline void setDeafultVal(const WidgetInfo &winfo, const QJsonObject &obj)
{
    switch(winfo.type)
    {
    case SpinBox:
    {
        auto spin = static_cast<QSpinBox*>(winfo.widget);
        spin->setValue(obj["default"].toInt());
    }
    break;
    case DoubleSpinBox:
    {
        auto dSpin = static_cast<QDoubleSpinBox*>(winfo.widget);
        dSpin->setValue(obj["default"].toDouble());
    }
    break;
    default:
        break;
    }
}

class WaveformsWidget : public QWidget
{
    Q_OBJECT

public:
    WaveformsWidget(QWidget *parent = nullptr);
    ~WaveformsWidget();

private:
    void initFromJson(const QString &filePath);
    void addWaveItem(const QString &name,const QJsonObject &obj);

    void addLine(const QList<QPointF> &list);
    void addSpLine(const QList<QPointF> &list);

    QList<QPointF> sinWaveform();
    QList<QPointF> squareWaveform();
    QList<QPointF> sawtoothWaveform();
    QList<QPointF> traingleWaveform();
    QList<QPointF> straightLine();
    QList<QPointF> genWavePoints(const QString &waveName, yAxisCal);

    void updateXyRange(const QList<QPointF> &list);
    void hideAllMakers();

public slots:
    void onWaveChanged(int index);
    void onAddWaveform(int index);
    void clear();

private:
    Ui::WaveformsWidget *ui;
    QChart m_chart;
    SearchValByName search;
    QValueAxis *m_xAxis;
    QValueAxis *m_yAxis;

    QPointF m_stopPoint;
    std::map<QString,WidgetListInfo> m_wavesInfo;
    Range m_range;

    QList<QPointF> m_points;
};
