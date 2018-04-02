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
 
	//QStandardItem *devicetreenode = new QStandardItem(QString("HIKVISION"));
	//devicetreenode->setEditable(0);
    
	
    //add by pyd
    QStandardItem *pDeviceRootItem = new QStandardItem(QString("device tree"));
    //devicetreenode->appendRow(pDeviceRootItem);
    parentItem->appendRow(pDeviceRootItem);
    pDeviceRootItem->setIcon(QIcon(":/image/tree.bmp"));
	pDeviceRootItem->setEditable(0);
 
	QList<TreeItem*> parents1;
	QString deviceBegin("<device>");
	QString deviceEnd("</device>");
	QString channelBegin("<channel>");
	QString channelEnd("</channel>");

	QStandardItem *pDeviceItem = NULL;
	QStandardItem *pChannelItem = NULL;
    for ( QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it )
    {   
        if ((*it)==deviceBegin)
        {
			++it;
			pDeviceItem = new QStandardItem(*it);
			pDeviceRootItem->appendRow(pDeviceItem);
            pDeviceItem->setIcon(QIcon(":/image/logout.bmp"));
			pDeviceItem->setEditable(0);
        }
 
        QString chn =channelBegin;
        if ((*it)==chn)
        {
			++it;
			pChannelItem = new QStandardItem(*it);
			pDeviceItem->appendRow(pChannelItem);
            pChannelItem->setIcon(QIcon(":/image/camera.bmp"));
			pChannelItem->setEditable(0);
        }
    }
}

