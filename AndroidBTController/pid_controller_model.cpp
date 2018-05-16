#include "pid_controller_model.h"

PIDControllerRate::PIDControllerRate(QString rateName) :
    m_rateName(rateName), m_rateValue(0)
{

}


QString PIDControllerRate::rateName() const
{
    return m_rateName;
}

void PIDControllerRate::setRateName(QString &value)
{
    m_rateName = value;
}

float PIDControllerRate::rateValue() const
{
    return m_rateValue;
}

void PIDControllerRate::setRateValue(float value)
{
    m_rateValue = value;
}

/*** Model ***/

PIDControllerModel::PIDControllerModel(QObject *parent) :
    QAbstractListModel(parent)
{
    addRate(PIDControllerRate( "P-rate" ));
    addRate(PIDControllerRate( "I-rate" ));
    addRate(PIDControllerRate( "D-rate" ));
}

void PIDControllerModel::addRate(const PIDControllerRate &rate)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_rates << rate;
    endInsertRows();
}

QVariant PIDControllerModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= m_rates.count())
        return QVariant();

    const PIDControllerRate &rate = m_rates[index.row()];
    if (role == NameRole)
        return rate.rateName();
    else if (role == ValueRole)
        return rate.rateValue();
    return QVariant();
}

int PIDControllerModel::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_rates.count();
}

QHash<int, QByteArray> PIDControllerModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "rateName";
    roles[ValueRole] = "rateValue";
    return roles;
}

// Make it editable

bool PIDControllerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    switch (role) {
        case NameRole:
            return false;           // Read-only
        case ValueRole:
            {
//                qDebug() << "> Rate before: " << m_rates[index.row()].rateValue();
                double floatValue = value.toFloat();
                m_rates[index.row()].setRateValue(floatValue);
//                qDebug() << "> Rate after:  " << m_rates[index.row()].rateValue();
            }
            break;
        default:
            return false;
    }

//    emit dataChanged(index, index, QVector<int>() << role);

    return true;
}

Qt::ItemFlags PIDControllerModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

void PIDControllerModel::handleNewRates(float Prate, float Irate, float Drate)
{
//    qDebug() << "Rates before: " << m_rates[0].rateValue() << " / " << m_rates[1].rateValue() << " / " << m_rates[2].rateValue();

    m_rates[0].setRateValue(Prate);
    m_rates[1].setRateValue(Irate);
    m_rates[2].setRateValue(Drate);

//    qDebug() << "Rates update: " << m_rates[0].rateValue() << " / " << m_rates[1].rateValue() << " / " << m_rates[2].rateValue();

    for ( int i = 0; i < m_rates.length(); i++ )
        emit dataChanged(index(i), index(i), QVector<int>() << ValueRole);
}

void PIDControllerModel::applyRates()
{
    QVector<float> rates;

    for ( int i = 0; i < m_rates.length(); i++ )
        rates << m_rates[i].rateValue();

    emit sendRates(rates);
}
