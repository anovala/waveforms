#include "WaveformsWidget.h"
#include "./ui_WaveformsWidget.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QSplineSeries>
#include <QValueAxis>
#include <QLegendMarker>

WaveformsWidget::WaveformsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WaveformsWidget)
    , m_xAxis(nullptr)
    , m_yAxis(nullptr)
{
    ui->setupUi(this);
    clear();
    QString sourceDir = QStringLiteral(SOURCE_DIR);
    initFromJson(sourceDir+ "/" + "/waveforms.json");
    connect(ui->waveformsBox,&QComboBox::currentIndexChanged,this,&WaveformsWidget::onWaveChanged);
    connect(ui->addBtn,&QPushButton::clicked,this,[this](){
        auto index = ui->waveformsBox->currentIndex();
        onAddWaveform(index);
    });

    m_xAxis = new QValueAxis();
    m_xAxis->setTitleText("Time (s)");
    m_xAxis->setTickCount(10);
    m_xAxis->setRange(-1,10);
    m_chart.addAxis(m_xAxis,Qt::AlignBottom);
    m_yAxis = new QValueAxis();
    m_yAxis->setTitleText("Voltage (v)")    ;
    m_yAxis->setTickCount(10);
    m_yAxis->setRange(-1,10);
    m_chart.addAxis(m_yAxis,Qt::AlignLeft);
    ui->chartView->setChart(&m_chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    m_stopPoint = {0,0};
    m_range = {0,0};

    search =  [](const WidgetListInfo &wlist, const QString &widetName) ->double{
        for(auto &&winfo : wlist)
        {
            if(winfo.name == widetName)
            {
                auto spinBox = static_cast<QSpinBox*>(winfo.widget);
                auto val = spinBox->value();
                return val;
            }
        }
        return 0;
    };

    this->resize(800,600);
}

WaveformsWidget::~WaveformsWidget()
{
    delete ui;
}

void WaveformsWidget::initFromJson(const QString &filePath)
{
    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "File open failed!";
        return ;
    }

    auto content = file.readAll();
    file.close();
    auto jsonDoc = QJsonDocument::fromJson(content);
    auto obj = jsonDoc.object();

    for(auto it = obj.begin(); it != obj.end(); it++)
    {
        addWaveItem(it.key(),it.value().toObject());
    }
}

void WaveformsWidget::addWaveItem(const QString &name, const QJsonObject &obj)
{
    auto waveName = obj["waveName"].toString();
    ui->waveformsBox->addItem(waveName);
    auto paramArr = obj["params"].toArray();

    auto widget = new QWidget(this);
    auto gridLayout = new QGridLayout();

    WidgetListInfo wList;
    for(int i = 0 ; i < paramArr.size(); ++i)
    {
        auto obj = paramArr.at(i).toObject();
        auto labelName = obj["name"].toString();
        auto label = new QLabel(labelName,this);
        gridLayout->addWidget(label,i,0);
        auto widgetInfo = createrWidgetInfo(obj,this);
        setDeafultVal(widgetInfo,obj);
        gridLayout->addWidget(widgetInfo.widget,i,1);
        wList.append(widgetInfo);
    }

    m_wavesInfo.insert({name,wList});
    widget->setLayout(gridLayout);
    ui->stackedWidget->addWidget(widget);
}

void WaveformsWidget::addLine(const QList<QPointF> &list)
{
    auto lines = new QLineSeries();
    lines->setColor(QColor(0xff,0xba,0x14));
    lines->append(list);
    m_chart.addSeries(lines);
    lines->attachAxis(m_xAxis);
    lines->attachAxis(m_yAxis);

    updateXyRange(list);
    hideAllMakers();
}

void WaveformsWidget::addSpLine(const QList<QPointF> &list)
{
    auto spLines = new QSplineSeries();
    spLines->setColor(QColor(0xff,0xba,0x14));
    spLines->append(list);
    m_chart.addSeries(spLines);
    spLines->attachAxis(m_xAxis);
    spLines->attachAxis(m_yAxis);

    updateXyRange(list);
    hideAllMakers();
}

QList<QPointF> WaveformsWidget::sinWaveform()
{
    yAxisCal func =
        [](double amp, double freq, double t) -> double{
            double ret = amp * std::sin(2 * M_PI *freq * t);
            return ret;
        };

    auto points = genWavePoints("sinwave",func);

    return points;
}

QList<QPointF> WaveformsWidget::squareWaveform()
{
    yAxisCal func =
        [](double amp, double freq, double t) -> double {
        double y =  amp * (std::sin(2 * M_PI * freq * t) >= 0 ? 1.0 : -1.0);
        return y;
    };
    auto points = genWavePoints("squarewave",func);

    return points;
}


QList<QPointF> WaveformsWidget::sawtoothWaveform()
{
    yAxisCal func =[](double amp, double freq, double t) -> double {
        double y = 2 * amp * (std::fmod(t * freq, 1.0)) - amp;
        return y;
    };

    auto points = genWavePoints("sawtoothwave",func);

    return points;
}

QList<QPointF> WaveformsWidget::traingleWaveform()
{
    yAxisCal func = [](double amp, double freq, double t) -> double {
        double y = 2 * amp * std::abs(2 * std::fmod(t * freq, 1.0) - 1.0) - amp;
        return y;
    };

    auto points = genWavePoints("trainglewave",func);
    return points;
}


QList<QPointF> WaveformsWidget::straightLine()
{
    auto wlist = m_wavesInfo.at("DC");
    auto vol = search(wlist,VOL);
    auto persit = search(wlist,PERSIST);

    int nums = 100;
    QList<QPointF> points;
    points.append(m_stopPoint);
    double elaspedTime = m_stopPoint.x();
    double dt = persit / nums;

    for(int i = 0; i < nums; ++i){
        double t = elaspedTime + i *dt;
        points.append({t,vol});
    }
    m_stopPoint = points.last();
    return points;
}

QList<QPointF> WaveformsWidget::genWavePoints(const QString &waveName, yAxisCal yCalculate)
{
    auto wlist = m_wavesInfo.at(waveName);
    auto freq = search(wlist,FREQ);
    auto amp = search(wlist,AMP);
    auto cycles = search(wlist,CYCLE);
    int nums = 100;

    QList<QPointF> points;
    points.append(m_stopPoint);
    double elaspedTime = m_stopPoint.x();

    double T = 1.0/freq;
    double totalTime = cycles * T;
    double dt = totalTime/nums;

    for(int i = 0; i < nums; ++i)
    {
        double t = elaspedTime + i * dt;
        double y = yCalculate(amp,t,freq);
        points.append({t,y});
    }

    m_stopPoint =  points.last();
    return points;
}

void WaveformsWidget::updateXyRange(const QList<QPointF> &list)
{
    for(auto &&point:list)
    {
        if(point.x() > m_range.xMax)
            m_range.xMax = point.x();
        else if(point.x() < m_range.xMin)
            m_range.xMin = point.x();

        if(point.y() > m_range.yMax)
            m_range.yMax = point.y();
        else if(point.y() < m_range.yMin)
            m_range.yMin = point.y();
    }

    if(m_range.yMin > 0)
        m_range.yMin = 0;
    double paddingX = (m_range.xMax - m_range.xMin) * 0.1;
    double paddingY = (m_range.yMax - m_range.yMin) * 0.1;

    auto xAxises  = m_chart.axes(Qt::Horizontal);
    for(auto axis : xAxises)
    {
        axis->setRange(m_range.xMin - paddingX, m_range.xMax + paddingX);
    }

    auto yAxises = m_chart.axes(Qt::Vertical);
    for(auto axis : yAxises)
    {
        axis->setRange(m_range.yMin - paddingY,m_range.yMax + paddingY);
    }
}

void WaveformsWidget::hideAllMakers()
{
    for (auto marker: m_chart.legend()->markers())
    {
        marker->setVisible(false);
    }
}

void WaveformsWidget::onWaveChanged(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void WaveformsWidget::onAddWaveform(int index)
{
    QList<QPointF> points;
    switch (index) {
    case DC:
        points = straightLine();
        break;
    case Swatooth:
        points = sawtoothWaveform();
        break;
    case Sine:
        points = sinWaveform();
        break;
    case Square:
        points = squareWaveform();
        break;
    case Traingle:
        points = traingleWaveform();
        break;
    default:
        break;
    }

    if(index == Sine)
        addSpLine(points);
    else
        addLine(points);
}

void WaveformsWidget::clear()
{
    while(ui->stackedWidget->count() > 0)
    {
        auto widget = ui->stackedWidget->widget(0);
        ui->stackedWidget->removeWidget(widget);
        widget->deleteLater();
    }

}
