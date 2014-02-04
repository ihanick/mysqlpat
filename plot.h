#ifndef _PLOT_H_
#define _PLOT_H_

#include <qwt_plot.h>

#include <QQueue>
#include <QMap>

class QPolygonF;
class QwtPlotPanner;
class QwtPlotMagnifier;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget * = NULL );
    void LoadFile(QString filename);
    void DetachCurve(QString name);
    void AttachCurve(QString name);

    double GetScaler(QString name) const;
    bool IsDiff(QString name) const;

    QVector<QPolygonF> all_points;
    QVector<QString> all_names;
    QVector<qint64> maxvalues;
    QVector<qint64> minvalues;
    QVector<double> yscalers;

private:
    void insertCurve(const QString &title,
        const QColor &, const QPolygonF &);
    QwtPlotPanner* panner;
    QwtPlotMagnifier* magnifier;

    QQueue<QString> allowed_colors;
    QMap<QString, bool> diff_status;
};

#endif

