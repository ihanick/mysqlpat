#ifndef GRAPHSCHOSER_H
#define GRAPHSCHOSER_H

#include <QDialog>

class QTreeWidgetItem;

namespace Ui {
class GraphsChooser;
}

class GraphsChooser : public QDialog
{
    Q_OBJECT

public:
    explicit GraphsChooser(QWidget *parent = 0);
    ~GraphsChooser();

signals:
    void CurveEnabled(QString);
    void CurveDisabled(QString);

private:
    Ui::GraphsChooser *ui;

protected:
    QTreeWidgetItem *AddChild(QTreeWidgetItem *parent, QString name, QString Description, QString curve_name);
    QTreeWidgetItem *AddRoot(QString name, QString description);
private slots:
    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);
};

#endif // GRAPHSCHOSER_H
