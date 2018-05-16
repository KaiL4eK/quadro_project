#ifndef CONTROLLER_MODEL_H
#define CONTROLLER_MODEL_H

#include <QObject>
#include <QString>
#include <QAbstractListModel>
#include <QList>
#include <QVariant>

#include <QDebug>

class PIDControllerRate
{
public:
    explicit PIDControllerRate(QString rateName);

    QString rateName() const;
    void setRateName(QString &value);

    float rateValue() const;
    void setRateValue(float value);


public slots:

private:
    QString     m_rateName;
    float       m_rateValue;
};

/*** Model ***/

class PIDControllerModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PIDControllerModel(QObject *parent = nullptr);

    enum RatesRoles {
        NameRole = Qt::UserRole + 1,
        ValueRole
    };

    void addRate(const PIDControllerRate &rate);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void handleNewRates( float Prate, float Irate, float Drate );

    Q_INVOKABLE void applyRates();

Q_SIGNALS:
    void sendRates(QVector<float> &rates);

public slots:


protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<PIDControllerRate> m_rates;
};

#endif // CONTROLLER_MODEL_H
