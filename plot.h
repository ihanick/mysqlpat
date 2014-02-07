#ifndef _PLOT_H_
#define _PLOT_H_

#include <qwt_plot.h>

#include <QQueue>
#include <QMap>

class QPolygonF;
class QwtPlotPanner;
class QwtPlotMagnifier;


#include <vector>
#include <map>

struct RawData {
    std::vector<double> values; // if is_diff=1, contains val[i]-minval
    std::vector<double> dt;
    double minval;
    double maxval; // if is_diff=1, abs_max_val - minval
    double scaler; // getScaler(maxval);
    bool is_diff;
    double start_time;
    double end_time;
};



class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget * = NULL );
    void LoadFile(QString filename);
    QStringList AddFile(QString filename);
    void DetachCurve(QString name);
    void AttachCurve(QString name);
    void RefreshCurvePoints(QString name);

    void DetachAllCurves();

    void ReAttachCurve(QString name);

    double GetScaler(QString name) const;
    bool IsDiff(QString name) const;

    QVector<QPolygonF> all_points;
    QVector<QString> all_names;
    QVector<qint64> maxvalues;
    QVector<qint64> minvalues;

private:
    void insertCurve(const QString &title,
        const QColor &, const QPolygonF &);
    QwtPlotPanner* panner;
    QwtPlotMagnifier* magnifier;

    QQueue<QString> allowed_colors;

    std::map<std::string, RawData> curvals;
};

#endif

