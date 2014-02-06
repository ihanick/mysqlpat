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

#include <math.h>

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
#include <vector>
#include <map>

std::map<std::string, std::vector<std::string> > values;

void read_mysqladmin_file(const char* filename) {
    std::string line;
    std::ifstream myfile(filename);
    if (!myfile.is_open()) {
        return;
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
}

void Plot::LoadFile(QString filename) {
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


    read_mysqladmin_file(filename.toLocal8Bit().constData());

    int line_num = 0;
    for (std::map<std::string, std::vector<std::string> >::iterator it = values.begin(); it != values.end(); ++it) {
        if(it->first == "Variable_name") {
            continue;
        }

      all_names.append(it->first.c_str());
      all_points.resize( all_names.size() );
      minvalues.resize( all_names.size() );
      maxvalues.resize( all_names.size() );

      bool is_diff = true;

      std::vector<std::string> vals = it->second;
      minvalues[line_num] = 1000000000;
      maxvalues[line_num] = -1000000000;

      qint64 prevval = QString(vals[0].c_str() ).toLongLong();

      for(int i=0; i<vals.size();++i) {
          qint64 val = QString(vals[i].c_str() ).toLongLong();
              if(minvalues[line_num] > val) {
                      minvalues[line_num] = val;
              }
              if(maxvalues[line_num] < val) {
                      maxvalues[line_num] = val;
              }
              if(i!=1 && val < prevval ) {
                      is_diff = false;
              }
              prevval = val;
      }


      if(minvalues[line_num] == maxvalues[line_num]) {
          is_diff = false;
      }

      if(is_diff) {
              prevval = QString(vals[0].c_str() ).toLongLong();
              qint64 prevts = QString(values["Uptime"][0].c_str() ).toLongLong()-1;
              maxvalues[line_num] =  -1000000000;
              for(int i=1; i<vals.size();++i) {
                      qint64 ts = QString(values["Uptime"][i].c_str() ).toLongLong();
                      qint64 val = QString(vals[i].c_str() ).toLongLong();
                      qint64 diff = val - prevval;
                      if(ts-prevts > 0) {
                              if(maxvalues[line_num] < diff/(ts-prevts))
                                      maxvalues[line_num] =  diff/(ts-prevts);
                      }
                      prevval = val;
                      prevts = ts;
              }
      }

      yscalers.append(1.0);

      if(maxvalues[line_num] < 10) {
              yscalers[line_num] = 1.0;
      } else if(maxvalues[line_num] < 100) {
              yscalers[line_num] = 10.0;
      } else if(maxvalues[line_num] < 1000) {
              yscalers[line_num] = 100.0;
      } else if(maxvalues[line_num] < 10000) {
              yscalers[line_num] = 1000.0;
      } else if(maxvalues[line_num] < 100000) {
              yscalers[line_num] = 10000.0;
      } else if(maxvalues[line_num] < 1000000) {
              yscalers[line_num] = 100000.0;
      } else if(maxvalues[line_num] < 10000000) {
              yscalers[line_num] = 1000000.0;
      } else if(maxvalues[line_num] < 100000000) {
              yscalers[line_num] = 10000000.0;
      } else if(maxvalues[line_num] < 1000000000) {
              yscalers[line_num] = 100000000.0;
      } else {
              yscalers[line_num] = maxvalues[line_num];
      }

      if(yscalers[line_num] == 0) {
          yscalers[line_num]  = 1.0;
      }

      if(is_diff) {
              prevval = QString(vals[0].c_str() ).toLongLong();
              qint64 prevts = QString(values["Uptime"][0].c_str() ).toLongLong()-1;
              for(int i=0; i<vals.size();++i) {
                      qint64 ts = QString(values["Uptime"][i].c_str() ).toLongLong();
                      qint64 val = QString(vals[i].c_str() ).toLongLong();
                      qint64 diff = val - prevval;
                      if(ts-prevts > 0) {
                              all_points[line_num] << QPointF(prevts+0.0001, diff/(ts-prevts)/yscalers[line_num]);
                              all_points[line_num] << QPointF(ts, diff/(ts-prevts)/yscalers[line_num]);
                      }
                      prevval = val;
                      prevts = ts;
              }
      } else {
          qint64 ts = QString(values["Uptime"][0].c_str() ).toLongLong();
          qint64 prevts = QString(values["Uptime"][0].c_str() ).toLongLong()-1;
              if(minvalues[line_num] == maxvalues[line_num]) {
                      all_points[line_num] << QPointF(ts, minvalues[line_num]/yscalers[line_num] - 0.001);
              }

              for(int i=0; i<vals.size();++i) {
                      ts = QString(values["Uptime"][i].c_str() ).toLongLong();
                      qint64 val = QString(vals[i].c_str() ).toLongLong();
                      all_points[line_num] << QPointF(prevts+0.0001, val/yscalers[line_num]);
                      all_points[line_num] << QPointF(ts, val/yscalers[line_num]);
                      prevts = ts;
              }
      }

      diff_status[QString(it->first.c_str())] = is_diff;

      line_num++;
    }

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

    insertCurve( all_names[2], allowed_colors.dequeue(), all_points[2]);

    this->replot();
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


double Plot::GetScaler(QString name) const {
    for ( int i = 0; i < all_names.size(); i++ ) {
        if(all_names[i] == name ) {
            return yscalers[i];
        }
    }
    return 1.0;
}

bool Plot::IsDiff(QString name) const {
    return diff_status[name];
}
