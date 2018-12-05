#include <QIcon>
#include <QtGui>
#include "treeitem.h"
#include "treemodel.h"
 

 TreeModel::TreeModel(const QString  &data, QObject *parent)
     : QStandardItemModel(parent)
 {
     QList<QVariant> rootData;
     rootData << "DeviceTree";
     rootItem = new TreeItem(rootData);
     setupModelData(data.split(QString("\n")), rootItem);
 }
 

 TreeModel::~TreeModel()
 {
     delete rootItem;
 }


void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent)
 {
    QList<TreeItem*> parents;
    parents << parent;
    QStandardItem *parentItem = invisibleRootItem();
    QStandardItem *pDeviceRootItem = new QStandardItem(QString("NERCMS_NVR"));
    parentItem->appendRow(pDeviceRootItem);
    pDeviceRootItem->setIcon(QIcon(":/images/login.bmp"));
    pDeviceRootItem->setEditable(0);

	QString channelBegin("<channel>");

	QStandardItem *pChannelItem = NULL;

    for ( QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it )
    {
        QString chn =channelBegin;
        if ((*it)==chn)
        {
			++it;
			pChannelItem = new QStandardItem(*it);
            pDeviceRootItem->appendRow(pChannelItem);
            pChannelItem->setIcon(QIcon(":/images/camera.bmp"));
			pChannelItem->setEditable(0);
        }
    }
}

