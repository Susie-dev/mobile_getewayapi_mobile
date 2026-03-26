#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <QAbstractListModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QList>

struct Order {
    int id;
    QString orderNo;
    QString goodsName;
    QString origin;
    QString destination;
    double targetTemp;
    QString status;
};

class OrderModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum OrderRoles {
        IdRole = Qt::UserRole + 1,
        OrderNoRole,
        GoodsNameRole,
        OriginRole,
        DestinationRole,
        TargetTempRole,
        StatusRole
    };

    explicit OrderModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadOrders();
    Q_INVOKABLE void updateOrderStatus(const QString &orderNo, const QString &newStatus);

private:
    QList<Order> m_orders;
};

#endif // ORDERMODEL_H
