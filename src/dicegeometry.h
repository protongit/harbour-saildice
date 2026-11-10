#ifndef DICEGEOMETRY_H
#define DICEGEOMETRY_H

#include <QPointF>
#include <QQuaternion>
#include <QRectF>
#include <QSize>
#include <QVector>
#include <QVector3D>

struct DiceFace
{
    QVector<int> indices;
    QVector3D normal;
    QVector3D center;
    QVector3D tangentU;
    QVector3D tangentV;
    QVector<QPointF> projected;
};

struct DiceMesh
{
    int type = 0;
    QVector<QVector3D> vertices;
    QVector<DiceFace> faces;
};

namespace DiceGeometry {

const int AtlasTileSize = 192;
const int AtlasColumns = 5;

QVector<int> supportedTypes();
void prebuild();
const DiceMesh &mesh(int type);

// Rotation which places the selected face upwards and its printed value
// upright for the elevated tray camera.
QQuaternion landingQuaternion(const DiceMesh &mesh, int faceIndex);

QSize atlasSize(const DiceMesh &mesh);
QRectF atlasTileRect(int faceIndex);
QPointF atlasPoint(const DiceMesh &mesh, int faceIndex, const QPointF &point);

} // namespace DiceGeometry

#endif // DICEGEOMETRY_H
