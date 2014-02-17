#include "graphschoser.h"
#include "ui_graphschoser.h"

#include <QTreeWidgetItem>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

GraphsChooser::GraphsChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GraphsChooser)
{
    ui->setupUi(this);

        ui->treeWidget->blockSignals(true);

    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setHeaderLabels(QStringList()<< "Name"<<"Description");

    QString str =
            "["
            "{"
            "    \"name\": \"Commands\","
            "    \"description\": \"Commands issued by client\","
            "    \"items\": ["
            "    {"
            "        \"name\": \"SELECT\","
            "        \"description\": \"Com_select: number of select queries\","
            "        \"curve\": \"Com_select\""
            "    },"
            "    {"
            "        \"name\": \"INSERT\","
            "        \"description\": \"Com_insert: number of insert queries\","
            "        \"curve\": \"Com_insert\""
            "    }"
            "    ]"
            "},"
            "{"
            "    \"name\": \"Storage Engines\","
            "    \"description\": \"Operations handled by storage engines\","
            "    \"items\": ["
            "    {"
            "        \"name\": \"COMMIT\","
            "        \"description\": \"Handler_commit: number of commits\","
            "        \"curve\": \"Handler_commit\""
            "    },"
            "    {"
            "        \"name\": \"PREPARE\","
            "        \"description\": \"Handler_prepare: prepares for two-phase commit\","
            "        \"curve\": \"Handler_prepare\""
            "    },"
            "    {"
            "        \"name\": \"DELETE\","
            "        \"description\": \"Handler_delete: number of deletes\","
            "        \"curve\": \"Handler_delete\""
            "    },"
            "    {"
            "        \"name\": \"READ FIRST\","
            "        \"description\": \"Handler_read_first: full index scan indicator\","
            "        \"curve\": \"Handler_read_first\""
            "    },"
            "    {"
            "        \"name\": \"READ LAST\","
            "        \"description\": \"Handler_read_last: usually ORDER BY DESC or full index scan\","
            "        \"curve\": \"Handler_read_last\""
            "    },"
            "    {"
            "        \"name\": \"READ NEXT\","
            "        \"description\": \"Handler_read_next: index scan\","
            "        \"curve\": \"Handler_read_next\""
            "    },"
            "    {"
            "        \"name\": \"READ PREV\","
            "        \"description\": \"Handler_read_prev: backward index scan\","
            "        \"curve\": \"Handler_read_prev\""
            "    },"
            "    {"
            "        \"name\": \"READ KEY\","
            "        \"description\": \"Handler_read_key: good indexing indicator\","
            "        \"curve\": \"Handler_read_key\""
            "    },"
            "    {"
            "        \"name\": \"DISCOVER\","
            "        \"description\": \"Handler_discover: NDB table discovery\","
            "        \"curve\": \"Handler_discover\""
            "    }"
            "    ]"
            "}"
            ""
            "]"
            ;

    QJsonDocument sd = QJsonDocument::fromJson(str.toUtf8());

    for(int g=0; g< sd.array().size();g++) {
        QJsonObject group_info = sd.array().at(g).toObject();
        QTreeWidgetItem *itm = AddRoot(
                    group_info.value( QString("name")).toString(),
                    group_info.value( QString("description")).toString()
                    );

        QJsonArray items_array = group_info.value(QString("items")).toArray();

        for(int i=0; i< items_array.size();i++) {
            QJsonObject curve_info = items_array.at(i).toObject();
            AddChild(itm,
                     curve_info.value( QString("name")).toString(),
                     curve_info.value( QString("description")).toString(),
                     curve_info.value( QString("curve")).toString()
                     );
        }
    }

    ui->treeWidget->resizeColumnToContents(0);

    ui->treeWidget->blockSignals(false);
}

GraphsChooser::~GraphsChooser()
{
    delete ui;
}

QTreeWidgetItem* GraphsChooser::AddRoot(QString name, QString description) {
     QTreeWidgetItem *itm =new QTreeWidgetItem(ui->treeWidget);
     itm->setText(0,name);
     itm->setText(1,description);
     itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
     itm->setCheckState(0,Qt::Unchecked);
     return itm;
}


QTreeWidgetItem* GraphsChooser::AddChild(QTreeWidgetItem *parent,QString name, QString Description, QString curve_name)
{
    QTreeWidgetItem *itm =new QTreeWidgetItem();
    itm->setText(0,name);
    itm->setText(1,Description);
    itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
    itm->setCheckState(0,Qt::Unchecked);
    itm->setData(0,Qt::UserRole,curve_name);
    parent->addChild(itm);
    return itm;
}

void GraphsChooser::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    for(int i=0;i < item->childCount();i++) {
        QTreeWidgetItem *itm = item->child(i);
        itm->setCheckState(0,item->checkState(column) );
    }

    if(item->childCount() == 0) {
        qDebug() << "changed " << item->text(column) << "is checked:" << (item->checkState(column) == Qt::Checked)
                    << item->data(0, Qt::UserRole).toString();
        if(item->checkState(column) == Qt::Checked) {
            emit CurveEnabled(item->data(0, Qt::UserRole).toString());
        } else {
            emit CurveDisabled(item->data(0, Qt::UserRole).toString());
        }

    }
}
