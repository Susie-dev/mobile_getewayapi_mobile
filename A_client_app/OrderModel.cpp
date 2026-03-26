#include "OrderModel.h"
#include <QSqlError>
#include <QDebug>

OrderModel::OrderModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int OrderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_orders.count();
}

QVariant OrderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_orders.count())
        return QVariant();

    const Order &order = m_orders[index.row()];

    switch (role) {
    case IdRole: return order.id;
    case OrderNoRole: return order.orderNo;
    case GoodsNameRole: return order.goodsName;
    case OriginRole: return order.origin;
    case DestinationRole: return order.destination;
    case TargetTempRole: return order.targetTemp;
    case StatusRole: return order.status;
    default: return QVariant();
    }
}

QHash<int, QByteArray> OrderModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[OrderNoRole] = "orderNo";
    roles[GoodsNameRole] = "goodsName";
    roles[OriginRole] = "origin";
    roles[DestinationRole] = "destination";
    roles[TargetTempRole] = "targetTemp";
    roles[StatusRole] = "status";
    return roles;
}

void OrderModel::loadOrders()
{
    beginResetModel();
    m_orders.clear();

    QSqlQuery query("SELECT id, order_no, goods_name, origin, destination, target_temp, status FROM orders ORDER BY id DESC");
    while (query.next()) {
        Order order;
        order.id = query.value(0).toInt();
        order.orderNo = query.value(1).toString();
        order.goodsName = query.value(2).toString();
        order.origin = query.value(3).toString();
        order.destination = query.value(4).toString();
        order.targetTemp = query.value(5).toDouble();
        order.status = query.value(6).toString();
        m_orders.append(order);
    }
    endResetModel();
}

void OrderModel::updateOrderStatus(const QString &orderNo, const QString &newStatus)
{
    QSqlQuery query;
    query.prepare("UPDATE orders SET status = ? WHERE order_no = ?");
    query.addBindValue(newStatus);
    query.addBindValue(orderNo);
    
    if (query.exec()) {
        // 重新加载数据以刷新界面
        loadOrders();
    } else {
        qDebug() << "Failed to update order status:" << query.lastError();
    }
}
