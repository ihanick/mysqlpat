#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_GraphChecked(bool is_checked);

    void on_actionExit_triggered();

    void on_actionAdd_File_triggered();

private:
    Ui::MainWindow *ui;

    void RefreshGraphsMenu(QStringList data);
};

#endif // MAINWINDOW_H
