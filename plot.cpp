#include "plot.h"
#include "curvetracker.h"
#include <qwt_picker_machine.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_textlabel.h>
#include <qwt_plot_zoneitem.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>

#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

#include <math.h>
#include <numeric>


#include <qwt_point_data.h>

Plot::Plot( QWidget *parent ):
    QwtPlot( parent)
{
    setCanvasBackground(Qt::black);
    setPalette( Qt::black );

    // we want to have the axis scales like a frame around the
    // canvas
    plotLayout()->setAlignCanvasToScales( true );
    for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
        axisWidget( axis )->setMargin( 0 );

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setAutoFillBackground( false );
    canvas->setFrameStyle( QFrame::NoFrame );
    setCanvas( canvas );

    enableAxis( QwtPlot::xBottom, false );
    enableAxis( QwtPlot::xTop, false );
    enableAxis( QwtPlot::yLeft, false );
    enableAxis( QwtPlot::yRight, false );

    // axes
    setAxisTitle( QwtPlot::xBottom, "time" );

    CurveTracker* tracker = new CurveTracker( this->canvas() );

    // for the demo we want the tracker to be active without
    // having to click on the canvas
    tracker->setStateMachine( new QwtPickerTrackerMachine() );
    tracker->setRubberBandPen( QPen( "MediumOrchid" ) );

    panner = new QwtPlotPanner(this->canvas() );
    magnifier = new QwtPlotMagnifier(this->canvas() );
    magnifier->setAxisEnabled(QwtPlot::yLeft, false);
    magnifier->setAxisEnabled(QwtPlot::yRight, false);
    panner->setAxisEnabled(QwtPlot::yLeft, false);
    panner->setAxisEnabled(QwtPlot::yRight, false);

    magnifier->setZoomInKey(Qt::Key_Equal);

    allowed_colors.enqueue("DarkOrange");
    allowed_colors.enqueue("DodgerBlue");
    allowed_colors.enqueue("DarkSeaGreen");
    allowed_colors.enqueue("DeepPink");
    allowed_colors.enqueue("aquamarine");
    allowed_colors.enqueue("cornflowerblue");
    allowed_colors.enqueue("crimson");
    allowed_colors.enqueue("olivedrab");
    allowed_colors.enqueue("peru");
    allowed_colors.enqueue("royalblue");
    allowed_colors.enqueue("silver");
    allowed_colors.enqueue("thistle");
    allowed_colors.enqueue("limegreen");
}

void Plot::insertCurve( const QString &title, 
    const QColor &color, const QPolygonF &points)
{
    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setTitle( title );
    curve->setPen( color, 1.0),
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    curve->setYAxis( QwtPlot::yLeft );

    /*
    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
        QBrush( Qt::white ), QPen( color, 2 ), QSize( 8, 8 ) );
    curve->setSymbol( symbol );
    */

    curve->setSamples( points );

    curve->attach( this );
}


#include <fstream>
#include <sstream>
#include <string>


double getScaler(double value);


const double EPSILON = 1e-6;
std::map<std::string, RawData> ConvertStrings2Doubles(
        const std::map<std::string, std::vector<std::string> > values, const std::vector<double> dt) {
    std::map<std::string, RawData> ret;
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = values.begin(); it != values.end(); ++it) {
            auto arr = it->second;
            auto name = it->first;

            RawData d;

            d.minval = 1e37;
            d.maxval = -1e37;
            d.is_diff = true;

            double prevvalue = atof(arr[0].c_str());
            for(size_t i=0; i< arr.size(); ++i) {
                    double value = atof(arr[i].c_str());
                    if(value < d.minval) {
                            d.minval = value;
                    }
                    if(value > d.maxval) {
                            d.maxval = value;
                    }
                    if(d.is_diff && prevvalue > value) {
                            d.is_diff = false;
                    }
                    d.values.push_back(value);
                    prevvalue = value;
            }
            if( fabs(d.maxval - d.minval) < EPSILON) {
                    d.is_diff = false;
            }

            if(d.is_diff) {
                    d.maxval = -1e37;
                    double prevvalue = d.values[0];
                    for(size_t i=0; i< arr.size(); ++i) {
                            double value = d.values[i];
                            if(dt[i] > 0) {
                                    d.values[i] = (value-prevvalue)/dt[i];
                            } else {
                                    d.values[i] = 0;
                            }
                            if(d.values[i] > d.maxval) {
                                    d.maxval = d.values[i];
                            }
                            prevvalue = value;
                    }
            }
            d.scaler = getScaler(d.maxval);
            for(size_t i=0; i< arr.size(); ++i) {
                    d.values[i] /= d.scaler;
            }
            d.dt = dt;
            ret[name] = d;
    }
    return ret;
}

std::vector<double> Uptime2dt(const std::vector<std::string> uptime) {
    std::vector<double> d;
    double prevvalue = atof(uptime[0].c_str());
    for(size_t i=0; i< uptime.size(); ++i) {
        double value = atof(uptime[i].c_str());
        if(value > prevvalue) {
            d.push_back(value-prevvalue);
        } else { // time goes back, e.g. server restart
            d.push_back(1.0);
        }
        prevvalue = value;
    }
    return d;
}


std::map<std::string, std::vector<std::string> >
read_mysqladmin_file(const char* filename) {
        std::map<std::string, std::vector<std::string> > values;
    std::string line;
    std::ifstream myfile(filename);
    if (!myfile.is_open()) {
        return values;
    }

    std::string val_name;
    bool is_value = false;
    while ( getline (myfile,line) )
    {
        std::stringstream myStream(line);
        std::string str;

        is_value = false;
        while(myStream >> str) {
            if(str[0] != '|' && str[0] != '+' && str[0] != ' ' ) {
                if(is_value) {
                    values[val_name].push_back(str);
                } else {
                    val_name = str;
                    is_value = true;
                }

                if(str == "Variable_name") {
                    val_name = str;
                    is_value = true;
                }
            }
        }
    }

    myfile.close();
    return values;
}

std::map<std::string, std::vector<std::string> >
read_netstat_s_file(const char* filename) {
        std::map<std::string, std::vector<std::string> > values;
    std::string line;
    std::ifstream myfile(filename);
    if (!myfile.is_open()) {
        return values;
    }

    std::string val_name;
    std::string group_name;
    double start_ts = 0.0;


    while ( getline (myfile,line) )
    {
        std::stringstream myStream(line);
        std::string str;

        if (line.size() > 3 && line.find("TS") == 0) {
            myStream >> str;
            double ts = 0.0;
            myStream >> ts;
            if (start_ts < 0.1)
                start_ts = ts;
            values["Uptime"].push_back((QString("%1").arg(ts - start_ts)).toStdString());
            group_name = "";
            continue;
        }
        if(line[line.length()-1] == ':') {
            myStream >> str;
            group_name = str.substr(0, str.length()-1);
            continue;
        }
        auto pos = line.find_first_of("0123456789");
        if(pos != std::string::npos) {
            myStream.seekg(static_cast<long>(pos));
            uint64_t val = 0;
            myStream >> val;

            std::string name;
            for(const char& c: line) {
                if ( ('0' <= c && c <= '9') || c == ':')
                    continue;
                if (c == ' ') {
                    if (name.length() > 0 && name[name.length()-1] != '_')
                        name.push_back('_');
                } else {
                    name.push_back(c);
                }
            }
            values[std::string("netstats:") + group_name + std::string("_") + name].push_back(QString("%1").arg(val).toStdString());
        }
    }

    myfile.close();
    return values;
}


std::map<std::string, std::vector<std::string> >
read_vmstat_file(const char* filename) {
        std::map<std::string, std::vector<std::string> > values;
        std::string line;
        std::ifstream myfile(filename);
        if (!myfile.is_open()) {
                return values;
        }

        std::vector<std::string> names;
        int time_from_start = 0;
        while ( getline (myfile,line) )
        {
                if(line[0]==' ' && line[1]=='r' && names.size() > 0) {
                        continue;
                }
                if(line[0] == 'p') {
                        continue;
                }
                std::stringstream myStream(line);
                std::string str;

                int item_id = 0;

                while(myStream >> str) {
                        if(line[0]==' ' && line[1]=='r') {
                                names.push_back( std::string("vmstat_") + str);
                        } else if(names.size() > 0) {
                                values[names[item_id]].push_back(str);
                                // first vmstat value is meanless
                                if(values[names[item_id]].size() == 2) {
                                    values[names[item_id]][0] = str;
                                }
                        }
                        item_id++;
                }

                values["Uptime"].push_back((QString("%1").arg(time_from_start)).toStdString());
                time_from_start++;
        }

        myfile.close();
        return values;
}

void Plot::DetachAllCurves() {
    all_names.clear();
    all_points.clear();
    minvalues.clear();
    maxvalues.clear();

    const QwtPlotItemList curves =
                    this->itemList( QwtPlotItem::Rtti_PlotCurve );
    for ( int i = 0; i < curves.size(); i++ ) {
            allowed_colors.enqueue( static_cast<const QwtPlotCurve *>(curves[i])->pen().color().name() );
            curves[i]->detach();
    }
}

double getScaler(double value) {
        if(value < 10) {
                return 1.0;
        } else if(value < 100) {
                return 10.0;
        } else if(value < 1000) {
                return 100.0;
        } else if(value < 10000) {
                return 1000.0;
        } else if(value < 100000) {
                return 10000.0;
        } else if(value < 1000000) {
                return 100000.0;
        } else if(value < 10000000) {
                return 1000000.0;
        } else if(value < 100000000) {
                return 10000000.0;
        } else if(value < 1000000000) {
                return 100000000.0;
        } else if(value == 0) {
                return 1.0;
        } else  {
                return value;
        }
}

void Plot::LoadFile(QString filename) {
        DetachAllCurves();
        curvals.clear();
        AddFile(filename);
}

QStringList Plot::AddFile(QString filename) {
    QStringList res;

    QFileInfo f(filename);
    QStringList filename_parts = f.fileName().split('-');

    QString file_date_part;
    if(filename_parts.size() > 1) {
            file_date_part = filename_parts.at(0);
    } else { // we haven't found file type
            qDebug() << "No file name parts";
            return res;
    }


    QDateTime timestamp =  QDateTime::fromString(file_date_part, "yyyy_MM_dd_hh_mm_ss");

    std::map<std::string, std::vector<std::string> > str_values;


    if( filename_parts.at(filename_parts.size()-1) == "mysqladmin" ) {
            str_values = read_mysqladmin_file(filename.toLocal8Bit().constData());
    } else if( filename_parts.at(filename_parts.size()-1) == "netstat_s" ) {
            str_values = read_netstat_s_file(filename.toLocal8Bit().constData());
    } else if(filename_parts.at(filename_parts.size()-1) == "vmstat" ) {
            str_values = read_vmstat_file(filename.toLocal8Bit().constData());
    } else {
            qDebug() << "Unknown file extension";
            return res;
    }


    auto localdt = Uptime2dt(str_values["Uptime"]);
    auto localvals = ConvertStrings2Doubles(str_values, localdt);

    double start_timestamp = localvals["Uptime"].minval;
    if(timestamp.isValid()) {
            start_timestamp = timestamp.toMSecsSinceEpoch()/1000.0;
    }

    for (auto it = localvals.begin(); it != localvals.end(); ++it) {
            if(it->first == "Uptime") { // don't override uptime
                    continue;
            }

            int new_sample_position = 0;

            if(curvals.find(it->first) != curvals.end() ) {
                double end_timestamp = std::accumulate(localdt.begin(),localdt.end(),start_timestamp);
                if(start_timestamp <= curvals[it->first].start_time) {
                    new_sample_position = -1;
                } else if(end_timestamp >= curvals[it->first].end_time) {
                    new_sample_position = 1;
                }

                if(start_timestamp >= curvals[it->first].start_time
                        &&
                        end_timestamp <= curvals[it->first].end_time
                        ) {
                    new_sample_position = 2;
                }

            }

            localvals[it->first].start_time = start_timestamp;
            localvals[it->first].end_time = std::accumulate(
                                    localvals[it->first].dt.begin(),
                            localvals[it->first].dt.end(),
                            localvals[it->first].start_time);

            int line_num;
            if(new_sample_position == 0) {
                    res.append(it->first.c_str());
                    all_names.append(it->first.c_str());
                    all_points.resize( all_names.size() );
                    all_names.size();
            } else {
                for ( int i = 0; i < all_names.size(); i++ ) {
                        if(all_names[i] == it->first.c_str() ) {
                            line_num = i;
                            break;
                        }
                }
            }

            if(new_sample_position) {
                double newscaler = curvals[it->first].scaler;
                if(localvals[it->first].scaler > newscaler ) {
                    newscaler = localvals[it->first].scaler;
                }

                for(size_t i=0; i< localvals[it->first].values.size(); ++i ) {
                    localvals[it->first].values[i] *= localvals[it->first].scaler/newscaler;
                }
                for(size_t i=0; i< curvals[it->first].values.size(); ++i ) {
                    curvals[it->first].values[i] *= curvals[it->first].scaler/newscaler;
                }

                curvals[it->first].scaler = newscaler;
            }


            if(new_sample_position == 0) {
                curvals[it->first] = localvals[it->first];
            } else if(new_sample_position == 1) {
                curvals[it->first].values.insert(curvals[it->first].values.end(),
                        localvals[it->first].values.begin(),
                        localvals[it->first].values.end()
                        );
                size_t merge_pos = curvals[it->first].dt.size();
                curvals[it->first].dt.insert(curvals[it->first].dt.end(),
                                localvals[it->first].dt.begin(),
                                localvals[it->first].dt.end()
                                );
                curvals[it->first].dt[merge_pos] = localvals[it->first].start_time - curvals[it->first].end_time;

                curvals[it->first].end_time = localvals[it->first].end_time;
            } else if(new_sample_position == -1) {
                curvals[it->first].values.swap(localvals[it->first].values);
                curvals[it->first].dt.swap(localvals[it->first].dt);

                curvals[it->first].values.insert(curvals[it->first].values.end(),
                                localvals[it->first].values.begin(),
                                localvals[it->first].values.end()
                                );
                size_t merge_pos = curvals[it->first].dt.size();
                curvals[it->first].dt.insert(curvals[it->first].dt.end(),
                                localvals[it->first].dt.begin(),
                                localvals[it->first].dt.end()
                                );

                curvals[it->first].dt[merge_pos] = curvals[it->first].start_time - localvals[it->first].end_time;
                curvals[it->first].start_time = start_timestamp;
                //curvals[it->first].end_time = curvals[it->first].end_time;

            } else if(new_sample_position == 2) {
                    double ts = curvals[it->first].start_time;

                    std::vector<double> val;
                    std::vector<double> dt;

                    double prevts =  ts;
                    for(size_t i=0; i<curvals[it->first].dt.size(); i++ ) {

                            if( ts < localvals[it->first].start_time) {
                                    ts += curvals[it->first].dt[i];
                                    val.push_back(curvals[it->first].values[i]);
                                    dt.push_back(ts-prevts);
                            } else {
                                    break;
                            }
                            prevts = ts;
                    }

                    val.pop_back();
                    dt.pop_back();


                    prevts = std::accumulate(dt.begin(), dt.end(), curvals[it->first].start_time);
                    size_t idx = dt.size();
                    val.insert(val.end(), localvals[it->first].values.begin(), localvals[it->first].values.end());
                    dt.insert(dt.end(), localvals[it->first].dt.begin(), localvals[it->first].dt.end());

                    dt[idx]=localvals[it->first].start_time - prevts;

                    prevts = std::accumulate(dt.begin(), dt.end(), curvals[it->first].start_time);

                    ts = curvals[it->first].start_time;
                    for(size_t i=0; i<curvals[it->first].dt.size(); i++ ) {
                            if(ts < prevts) {
                                    ts += curvals[it->first].dt[i];
                                    continue;
                            }
                            ts += curvals[it->first].dt[i];
                            val.push_back(curvals[it->first].values[i]);
                            dt.push_back(ts-prevts);

                            prevts = ts;
                    }

                    curvals[it->first].values.swap(val);
                    curvals[it->first].dt.swap(dt);
            }

            if(new_sample_position == 0) {
                curvals[it->first] = it->second;
                curvals[it->first].start_time = start_timestamp;
                curvals[it->first].end_time = localvals[it->first].end_time;
            }

            RefreshCurvePoints(it->first.c_str());


            line_num++;
    }

    this->replot();
    return res;
}

void Plot::RefreshCurvePoints(QString name) {
    int line_num = all_names.size();
    for ( int i = 0; i < all_names.size(); i++ ) {
            if(all_names[i] == name ) {
                    line_num = i;
                    break;
            }
    }
    if(line_num == all_names.size() ) {
            return;
    }

    double prevts = curvals[name.toStdString()].start_time;
    auto vals = curvals[name.toStdString()].values;

    all_points[line_num].clear();
    //first fake point to show values for constant curves
    all_points[line_num] << QPointF(prevts-0.0001, vals[0]-0.0001);
    for(size_t i=0; i<vals.size();++i) {
            double ts = prevts+curvals[name.toStdString()].dt[i];
            all_points[line_num] << QPointF(prevts+0.0001, vals[i]);
            all_points[line_num] << QPointF(ts, vals[i]);
            prevts = ts;
    }

    ReAttachCurve(name);
}

void Plot::DetachCurve(QString name) {
    const QwtPlotItemList curves =
        this->itemList( QwtPlotItem::Rtti_PlotCurve );

    for ( int i = 0; i < curves.size(); i++ ) {
        if(curves[i]->title().text() == name ) {
            allowed_colors.enqueue( static_cast<const QwtPlotCurve *>(curves[i])->pen().color().name() );
            curves[i]->detach();
            qDebug() << "Removed plot:" << name;
        }
    }

    this->replot();
}

void Plot::AttachCurve(QString name) {
    for ( int i = 0; i < all_names.size(); i++ ) {
        if(all_names[i] == name ) {
                QString colorname;
                if(allowed_colors.isEmpty()) {
                        colorname = "DeepPink";
                } else {
                        colorname = allowed_colors.dequeue();
                }
                insertCurve( all_names[i], colorname, all_points[i]);
                qDebug() << "Added plot:" << name;
                break;
        }
    }
    this->replot();
}


void Plot::ReAttachCurve(QString name) {
    const QwtPlotItemList curves =
                    this->itemList( QwtPlotItem::Rtti_PlotCurve );

    for ( int i = 0; i < all_names.size(); i++ ) {
            if(all_names[i] != name ) {
                    continue;
            }
            for ( int j = 0; j < curves.size(); j++ ) {
                    if(curves[j]->title().text() != name ) {
                        continue;
                    }
                    curves[j]->detach();
                    QString colorname = static_cast<const QwtPlotCurve *>(curves[j])->pen().color().name();
                    insertCurve( all_names[i], colorname, all_points[i]);
                    qDebug() << "ReAdded plot:" << name;
                    return;
            }
            return;
    }
}


double Plot::GetScaler(QString name) const {
    const std::string name_str(name.toLocal8Bit().constData());
    const RawData d = curvals.find(name_str)->second;
    return d.scaler;
}

bool Plot::IsDiff(QString name) const {
    const std::string name_str(name.toLocal8Bit().constData());
    const RawData d = curvals.find(name_str)->second;
    return d.is_diff;
}

