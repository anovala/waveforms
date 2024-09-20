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

QT_BEGIN_NAMESPACE
namespace Ui {
class WaveformsWidget;
}
QT_END_NAMESPACE

using WidgetListInfo = QList<WidgetInfo>;
using SearchValByName = double(*)(const WidgetListInfo &wlist, const QString &widetName);

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

    void updateXyRange(const QList<QPointF> &list);

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
    QPointF m_startPoint;
    std::map<QString,WidgetListInfo> m_wavesInfo;
    Range m_range;
};
