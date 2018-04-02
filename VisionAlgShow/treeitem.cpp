#include <QStringList>
#include <QMessageBox>
#include "treeitem.h"


TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parent)
{
    parentItem = parent;
    itemData = data;
}


TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}


void TreeItem::appendChild(TreeItem *item)
{
    childItems.append(item);
}


TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}


int TreeItem::childCount() const
{
    return childItems.count();
}


int TreeItem::columnCount() const
{
    return itemData.count();
}


QVariant TreeItem::data(int column) const
{
	QString data =itemData.value(column).toString();
	//QMessageBox::information(this,tr("TreeItem::data"),tr("data=%1").arg(data.toLatin1().data()));
	//printf("%s\n",data.toLatin1().data());
    return itemData.value(column);
}

bool TreeItem::setData(const QVariant &value)
{
	//printf("1232434534555=%s\n",value.toString().toLatin1().data());
  	//itemData.clear();
	//itemData.append(value);
	//printf("1232434534555=%s\n",value.toString().toLatin1().data());
		
	 	return true;
     
    // return false;


}


TreeItem *TreeItem::parent()
{
    return parentItem;
}


int TreeItem::row() const
{
    if (parentItem)
    {
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));
    }
    return 0;
}

