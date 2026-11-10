#ifndef DICETRAY_H
#define DICETRAY_H

#include <QColor>
#include <QElapsedTimer>
#include <QQuaternion>
#include <QQuickItem>
#include <QScopedPointer>
#include <QTimer>
#include <QVariantList>
#include <QVector>
#include <QVector3D>
#include <random>

struct DiceVisual
{
    int type = 6;
    QColor color = QColor("#e8e0d0");
    int value = 1;
    QQuaternion orientation;
    QQuaternion startOrientation;
    QQuaternion targetOrientation;
    QVector3D spinAxis = QVector3D(0, 1, 0);
    QVector3D position;
    float radius = 0.6f;
    float hop = 0.0f;
    float scale = 1.0f;
    int delay = 0;
    int duration = 900;
    int turns = 2;
};

struct TrayScene;

class DiceTray : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantList dice READ dice WRITE setDice NOTIFY diceChanged)
    Q_PROPERTY(bool rolling READ rolling NOTIFY rollingChanged)

public:
    explicit DiceTray(QQuickItem *parent = nullptr);
    ~DiceTray() Q_DECL_OVERRIDE;

    QVariantList dice() const { return m_dice; }
    void setDice(const QVariantList &dice);
    bool rolling() const { return m_rolling; }

    Q_INVOKABLE void roll();

    const QVector<DiceVisual> &visuals() const { return m_visuals; }
    float cameraDistance() const { return m_cameraDistance; }

signals:
    void diceChanged();
    void rollingChanged();
    void rollFinished(const QVariantList &results);

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) Q_DECL_OVERRIDE;

private slots:
    void advanceRoll();

private:
    void rebuildVisuals();
    void layoutVisuals();
    void setRolling(bool rolling);
    int randomInt(int minimum, int maximum);

    QVariantList m_dice;
    QVector<DiceVisual> m_visuals;
    QVariantList m_pendingResults;
    QElapsedTimer m_rollClock;
    QTimer m_rollTimer;
    bool m_rolling = false;
    float m_cameraDistance = 5.6f;
    std::mt19937 m_random;
    QScopedPointer<TrayScene> m_scene;
};

#endif // DICETRAY_H
