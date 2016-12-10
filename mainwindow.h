#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class GraphsChooser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QStringList arguments, QWidget *parent = 0);
    ~MainWindow();

    void open_pat_file(QString filename);
    void toggle_curve_menu(QString curve_name, bool flag_state);

private slots:
    void on_actionOpen_triggered();

    void on_GraphChecked(bool is_checked);

    void on_actionExit_triggered();

    void on_actionAdd_File_triggered();

    void on_actionChose_Curves_triggered();


    void on_CurveEnabled(QString curve_name);
    void on_CurveDisabled(QString curve_name);

private:
    Ui::MainWindow *ui;
    GraphsChooser *chooser;
    QHash<QString,QMenu* > added_submenu;
    QHash<QString,QAction* > added_menus;

    void RefreshGraphsMenu(QStringList data);
};

#endif // MAINWINDOW_H
