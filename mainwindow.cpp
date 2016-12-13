#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include "graphschoser.h"


MainWindow::MainWindow(QStringList arguments, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    chooser(new GraphsChooser)
{
    ui->setupUi(this);
    ui->mainToolBar->hide();
    ui->statusBar->hide();
    setCentralWidget(ui->plot);


    connect(chooser, &GraphsChooser::CurveDisabled, this, &MainWindow::on_CurveDisabled );
    connect(chooser, &GraphsChooser::CurveEnabled, this, &MainWindow::on_CurveEnabled );

    qDebug() << arguments;
    if (arguments.size() >= 2) {
        open_pat_file(arguments[1]);
    }
    for(int i = 2; i < arguments.size(); ++i) {
        RefreshGraphsMenu(ui->plot->AddFile(arguments[i]));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RefreshGraphsMenu(QStringList data) {
    QMenu* submenu = NULL;
    for(int i=0; i< data.size(); ++i) {
        //qDebug() << "adding menu:" << data[i];
            QStringList name_split = data[i].split('_');
                            QString menuname = name_split.at(0);
            if(menuname == "binlog") {
                menuname = "Binlog";
            }

            if(name_split.size() > 1) {
                    if(added_submenu.contains(menuname)) {
                            submenu = added_submenu[menuname];
                    } else {
                            submenu = ui->menuGraphs->addMenu(menuname);
                            added_submenu[menuname] = submenu;
                            QString AllName = QString("All: %0").arg(menuname);
                            submenu->addAction(AllName);
                            QAction* action = submenu->actions().at(submenu->actions().size()-1);
                            action->setCheckable(true);
                            connect(action, SIGNAL(triggered(bool)), this, SLOT(on_GraphGroup(bool)) );
                            added_menus[AllName] = action;
                    }
            } else {
                submenu = NULL;
            }

            if(submenu == NULL) {
                    ui->menuGraphs->addAction(data[i]);
                    QAction* action = ui->menuGraphs->actions().at(ui->menuGraphs->actions().size()-1);
                    action->setCheckable(true);
                    connect(action, SIGNAL(triggered(bool)), this, SLOT(on_GraphChecked(bool)) );
                    added_menus[data[i]] = action;
            } else {
                submenu->addAction(data[i]);
                QAction* action = submenu->actions().at(submenu->actions().size()-1);
                action->setCheckable(true);
                connect(action, SIGNAL(triggered(bool)), this, SLOT(on_GraphChecked(bool)) );
                added_menus[data[i]] = action;
                QString AllName = QString("All: %0").arg(menuname);
                added_menu_groups[AllName].append(data[i]);
            }
    }

}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open File"), "",
                tr("global status (*-mysqladmin);;vmstat (*-vmstat);;Netstat Extended (*-netstat_s);;IOstat (*-iostat)")
                );
    open_pat_file(fileName);
}

void MainWindow::toggle_curve_menu(QString curve_name, bool flag_state) {
    if (added_menus.contains(curve_name)) {
        added_menus[curve_name]->setChecked(flag_state);
        added_menus[curve_name]->triggered(flag_state);
    }
}


void MainWindow::open_pat_file(QString fileName) {
    if(fileName.isNull() || fileName.isEmpty() ) {
        return;
    }

    ui->plot->LoadFile(fileName);

    ui->menuGraphs->clear();

    QStringList newmenu_items;
    for(int i=1; i< ui->plot->all_names.size(); ++i) {
        newmenu_items.append(ui->plot->all_names[i]);
    }
    RefreshGraphsMenu(newmenu_items);

    // Enable any non-zero graph to avoid black screen
    toggle_curve_menu("Queries", true);
    toggle_curve_menu("vmstat_r", true);
    toggle_curve_menu("netstats:IpExt_OutOctets_", true);
}


void MainWindow::on_GraphGroup(bool is_checked) {
     QAction* action = qobject_cast<QAction*>(sender());
    if(action)
    {
        QString graph_name = action->text();
        qDebug() << "Got graph group action: " << graph_name << "state" << is_checked << added_menu_groups[graph_name];
        foreach(const QString& curve_name,  added_menu_groups[graph_name]) {
            toggle_curve_menu(curve_name, is_checked);
        }
    }
}

void MainWindow::on_GraphChecked(bool is_checked) {
    QAction* action = qobject_cast<QAction*>(sender());
    if(action)
    {
            //QString graph_name = action->data().toString();
            QString graph_name = action->text();
            qDebug() << "Got action: " << graph_name << "state" << is_checked;
            if(is_checked == false) {
                ui->plot->DetachCurve(graph_name);
            } else {
                ui->plot->AttachCurve(graph_name);
            }
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionAdd_File_triggered()
{

    QStringList fileNames = QFileDialog::getOpenFileNames(
                this, tr("Add file to existing graph"), "",
                tr("global status (*-mysqladmin);;vmstat (*-vmstat);;Netstat Extended (*-netstat_s);;IOstat (*-iostat)")
                );

    for(int i=0; i<fileNames.size(); ++i) {
            if(fileNames[i].isNull() || fileNames[i].isEmpty() ) {
                    continue;
            }

            RefreshGraphsMenu(ui->plot->AddFile(fileNames[i]));
    }
}

void MainWindow::on_actionChose_Curves_triggered()
{
    chooser->show();
}


void MainWindow::on_CurveEnabled(QString curve_name) {
    toggle_curve_menu(curve_name, true);
    //ui->plot->AttachCurve(curve_name);
}

void MainWindow::on_CurveDisabled(QString curve_name) {
    toggle_curve_menu(curve_name, false);
    //ui->plot->DetachCurve(curve_name);
}
