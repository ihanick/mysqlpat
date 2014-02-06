#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->hide();
    ui->statusBar->hide();
    setCentralWidget(ui->plot);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "*-mysqladmin", tr("Files (*-mysqladmin)"));

    ui->plot->LoadFile(fileName);

    ui->menuGraphs->clear();

    QHash<QString,QMenu* > added_submenu;
    QMenu* submenu = NULL;
    for(int i=1; i< ui->plot->all_names.size(); ++i) {
            QStringList name_split = ui->plot->all_names[i].split('_');
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
                    }
            } else {
                submenu = NULL;
            }

            if(submenu == NULL) {
                    ui->menuGraphs->addAction(ui->plot->all_names[i]);
                    QAction* action = ui->menuGraphs->actions().at(ui->menuGraphs->actions().size()-1);
                    action->setCheckable(true);
                    if(i==2 || i==3 || i==4 || i==63) {
                        action->setChecked(true);
                    }
                    connect(action, SIGNAL(triggered(bool)), this, SLOT(on_GraphChecked(bool)) );
            } else {
                submenu->addAction(ui->plot->all_names[i]);
                QAction* action = submenu->actions().at(submenu->actions().size()-1);
                action->setCheckable(true);
                if(i==2 ) {
                        action->setChecked(true);
                }
                connect(action, SIGNAL(triggered(bool)), this, SLOT(on_GraphChecked(bool)) );
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
