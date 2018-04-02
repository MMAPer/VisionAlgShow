#ifndef TREEMODEL_H_
#define TREEMODEL_H_

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QObject>
#include <QStringList>
#include <QStandardItemModel>



class TreeItem;

class TreeModel : public QStandardItemModel
{
    Q_OBJECT
		
public:
	
	void setupModelData(const QStringList &lines, TreeItem *parent);
    TreeModel(const QString &data, QObject *parent = 0);
    ~TreeModel();

private:
  
    TreeItem *rootItem;
};

#endif /* TREEMODEL_H_ */

