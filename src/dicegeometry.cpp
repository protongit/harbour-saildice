#include "dicegeometry.h"

#include <QHash>
#include <QtMath>
#include <algorithm>
#include <limits>

namespace {

QHash<int, DiceMesh> meshes;

QVector3D normalized(const QVector3D &value)
{
    const float length = value.length();
    return length > 0.0f ? value / length : QVector3D(0.0f, 1.0f, 0.0f);
}

QVector<QVector3D> canonicalVertices(int type)
{
    QVector<QVector3D> vertices;
    const double phi = (1.0 + std::sqrt(5.0)) / 2.0;

    if (type == 4) {
        vertices << QVector3D(1, 1, 1) << QVector3D(1, -1, -1)
                 << QVector3D(-1, 1, -1) << QVector3D(-1, -1, 1);
    } else if (type == 6) {
        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2)
                    vertices << QVector3D(x, y, z);
    } else if (type == 8) {
        vertices << QVector3D(1, 0, 0) << QVector3D(-1, 0, 0)
                 << QVector3D(0, 1, 0) << QVector3D(0, -1, 0)
                 << QVector3D(0, 0, 1) << QVector3D(0, 0, -1);
    } else if (type == 10) {
        // Pentagonal trapezohedron. This height/ring ratio makes each of
        // the ten kites exactly planar.
        const double c = std::cos(M_PI / 5.0);
        const double h = 1.0;
        const double b = h * (1.0 - c) / (1.0 + c);
        const double r = 0.95;
        vertices << QVector3D(0, h, 0) << QVector3D(0, -h, 0);
        for (int i = 0; i < 5; ++i) {
            const double angle = 2.0 * M_PI * i / 5.0;
            vertices << QVector3D(r * std::cos(angle), b, r * std::sin(angle));
        }
        for (int i = 0; i < 5; ++i) {
            const double angle = 2.0 * M_PI * (i + 0.5) / 5.0;
            vertices << QVector3D(r * std::cos(angle), -b, r * std::sin(angle));
        }
    } else if (type == 12) {
        for (int x = -1; x <= 1; x += 2)
            for (int y = -1; y <= 1; y += 2)
                for (int z = -1; z <= 1; z += 2)
                    vertices << QVector3D(x, y, z);
        for (int a = -1; a <= 1; a += 2)
            for (int b = -1; b <= 1; b += 2) {
                vertices << QVector3D(0, a / phi, b * phi);
                vertices << QVector3D(a / phi, b * phi, 0);
                vertices << QVector3D(a * phi, 0, b / phi);
            }
    } else if (type == 20) {
        for (int a = -1; a <= 1; a += 2)
            for (int b = -1; b <= 1; b += 2) {
                vertices << QVector3D(0, a, b * phi);
                vertices << QVector3D(a, b * phi, 0);
                vertices << QVector3D(a * phi, 0, b);
            }
    }

    float radius = 0.0f;
    for (const QVector3D &vertex : vertices)
        radius = qMax(radius, vertex.length());
    if (radius > 0.0f) {
        for (QVector3D &vertex : vertices)
            vertex /= radius;
    }
    return vertices;
}

DiceFace makeFace(const QVector<QVector3D> &vertices, QVector<int> indices,
                  const QVector3D &outwardNormal)
{
    DiceFace face;
    face.indices = indices;
    face.normal = outwardNormal;
    face.center = QVector3D();
    for (int index : indices)
        face.center += vertices[index];
    face.center /= static_cast<float>(indices.size());

    QVector3D firstNormal;
    if (indices.size() >= 3) {
        firstNormal = QVector3D::crossProduct(vertices[indices[1]] - vertices[indices[0]],
                                              vertices[indices[2]] - vertices[indices[0]]);
    }
    if (QVector3D::dotProduct(firstNormal, face.normal) < 0.0f) {
        std::reverse(face.indices.begin(), face.indices.end());
    }

    const QVector3D reference = qAbs(face.normal.y()) < 0.9f
            ? QVector3D(0, 1, 0) : QVector3D(0, 0, -1);
    face.tangentV = normalized(reference - face.normal * QVector3D::dotProduct(reference, face.normal));
    face.tangentU = normalized(QVector3D::crossProduct(face.tangentV, face.normal));

    for (int index : face.indices) {
        const QVector3D delta = vertices[index] - face.center;
        face.projected << QPointF(QVector3D::dotProduct(delta, face.tangentU),
                                  QVector3D::dotProduct(delta, face.tangentV));
    }
    return face;
}

void orderOppositePairs(DiceMesh &mesh)
{
    if (mesh.type == 4 || mesh.faces.isEmpty())
        return;

    const int count = mesh.faces.size();
    QVector<DiceFace> ordered = mesh.faces;
    QVector<bool> used(count, false);
    for (int label = 1; label <= count / 2; ++label) {
        int first = -1;
        for (int i = 0; i < count; ++i) {
            if (!used[i]) {
                first = i;
                break;
            }
        }
        if (first < 0)
            break;
        int opposite = -1;
        float best = std::numeric_limits<float>::max();
        for (int i = 0; i < count; ++i) {
            if (used[i] || i == first)
                continue;
            const float distance = (mesh.faces[i].normal + mesh.faces[first].normal).length();
            if (distance < best) {
                best = distance;
                opposite = i;
            }
        }
        if (opposite < 0)
            break;
        used[first] = true;
        used[opposite] = true;
        ordered[label - 1] = mesh.faces[first];
        ordered[count - label] = mesh.faces[opposite];
    }
    mesh.faces = ordered;
}

DiceMesh buildMesh(int type)
{
    DiceMesh mesh;
    mesh.type = type;
    mesh.vertices = canonicalVertices(type);
    const int count = mesh.vertices.size();
    const float epsilon = 0.00008f;
    QList<QVector<int>> seen;

    // Brute-force convex hull: with at most 20 vertices this is quick and,
    // more importantly, gives exact planar polygons for every solid.
    for (int i = 0; i < count - 2; ++i) {
        for (int j = i + 1; j < count - 1; ++j) {
            for (int k = j + 1; k < count; ++k) {
                QVector3D normal = QVector3D::crossProduct(mesh.vertices[j] - mesh.vertices[i],
                                                           mesh.vertices[k] - mesh.vertices[i]);
                if (normal.lengthSquared() < 1e-10f)
                    continue;
                normal.normalize();
                bool positive = false;
                bool negative = false;
                for (int p = 0; p < count; ++p) {
                    if (p == i || p == j || p == k)
                        continue;
                    const float side = QVector3D::dotProduct(normal, mesh.vertices[p] - mesh.vertices[i]);
                    positive = positive || side > epsilon;
                    negative = negative || side < -epsilon;
                }
                if (positive && negative)
                    continue;
                if (positive)
                    normal = -normal;

                QVector<int> polygon;
                for (int p = 0; p < count; ++p) {
                    if (qAbs(QVector3D::dotProduct(normal, mesh.vertices[p] - mesh.vertices[i])) < epsilon * 2.0f)
                        polygon << p;
                }
                if (polygon.size() < 3)
                    continue;
                QVector<int> key = polygon;
                std::sort(key.begin(), key.end());
                if (seen.contains(key))
                    continue;
                seen << key;

                QVector3D center;
                for (int index : polygon)
                    center += mesh.vertices[index];
                center /= static_cast<float>(polygon.size());
                const QVector3D reference = qAbs(normal.y()) < 0.9f
                        ? QVector3D(0, 1, 0) : QVector3D(0, 0, -1);
                const QVector3D axisU = normalized(QVector3D::crossProduct(reference, normal));
                const QVector3D axisV = normalized(QVector3D::crossProduct(normal, axisU));
                std::sort(polygon.begin(), polygon.end(), [&](int a, int b) {
                    const QVector3D da = mesh.vertices[a] - center;
                    const QVector3D db = mesh.vertices[b] - center;
                    const float aa = std::atan2(QVector3D::dotProduct(da, axisV),
                                                QVector3D::dotProduct(da, axisU));
                    const float ab = std::atan2(QVector3D::dotProduct(db, axisV),
                                                QVector3D::dotProduct(db, axisU));
                    return aa < ab;
                });
                mesh.faces << makeFace(mesh.vertices, polygon, normal);
            }
        }
    }
    orderOppositePairs(mesh);
    return mesh;
}

QQuaternion quaternionFromBasis(const QVector3D &localX, const QVector3D &localY,
                                const QVector3D &localZ, const QVector3D &worldX,
                                const QVector3D &worldY, const QVector3D &worldZ)
{
    float m[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    const QVector3D local[3] = {localX, localY, localZ};
    const QVector3D world[3] = {worldX, worldY, worldZ};
    for (int axis = 0; axis < 3; ++axis) {
        for (int row = 0; row < 3; ++row) {
            for (int column = 0; column < 3; ++column)
                m[row][column] += world[axis][row] * local[axis][column];
        }
    }

    const float trace = m[0][0] + m[1][1] + m[2][2];
    float w, x, y, z;
    if (trace > 0.0f) {
        const float s = std::sqrt(trace + 1.0f) * 2.0f;
        w = 0.25f * s;
        x = (m[2][1] - m[1][2]) / s;
        y = (m[0][2] - m[2][0]) / s;
        z = (m[1][0] - m[0][1]) / s;
    } else if (m[0][0] > m[1][1] && m[0][0] > m[2][2]) {
        const float s = std::sqrt(1.0f + m[0][0] - m[1][1] - m[2][2]) * 2.0f;
        w = (m[2][1] - m[1][2]) / s;
        x = 0.25f * s;
        y = (m[0][1] + m[1][0]) / s;
        z = (m[0][2] + m[2][0]) / s;
    } else if (m[1][1] > m[2][2]) {
        const float s = std::sqrt(1.0f + m[1][1] - m[0][0] - m[2][2]) * 2.0f;
        w = (m[0][2] - m[2][0]) / s;
        x = (m[0][1] + m[1][0]) / s;
        y = 0.25f * s;
        z = (m[1][2] + m[2][1]) / s;
    } else {
        const float s = std::sqrt(1.0f + m[2][2] - m[0][0] - m[1][1]) * 2.0f;
        w = (m[1][0] - m[0][1]) / s;
        x = (m[0][2] + m[2][0]) / s;
        y = (m[1][2] + m[2][1]) / s;
        z = 0.25f * s;
    }
    return QQuaternion(w, x, y, z).normalized();
}

} // namespace

namespace DiceGeometry {

QVector<int> supportedTypes()
{
    return QVector<int>() << 4 << 6 << 8 << 10 << 12 << 20;
}

void prebuild()
{
    for (int type : supportedTypes()) {
        if (!meshes.contains(type))
            meshes.insert(type, buildMesh(type));
    }
}

const DiceMesh &mesh(int type)
{
    if (!meshes.contains(type))
        meshes.insert(type, buildMesh(type));
    return meshes[type];
}

QQuaternion landingQuaternion(const DiceMesh &mesh, int faceIndex)
{
    if (mesh.faces.isEmpty())
        return QQuaternion();
    const DiceFace &face = mesh.faces.at((faceIndex % mesh.faces.size() + mesh.faces.size()) % mesh.faces.size());
    return quaternionFromBasis(face.tangentU, face.tangentV, face.normal,
                               QVector3D(1, 0, 0), QVector3D(0, 0, -1), QVector3D(0, 1, 0));
}

QSize atlasSize(const DiceMesh &mesh)
{
    const int rows = (mesh.faces.size() + AtlasColumns - 1) / AtlasColumns;
    return QSize(AtlasColumns * AtlasTileSize, rows * AtlasTileSize);
}

QRectF atlasTileRect(int faceIndex)
{
    return QRectF((faceIndex % AtlasColumns) * AtlasTileSize,
                  (faceIndex / AtlasColumns) * AtlasTileSize,
                  AtlasTileSize, AtlasTileSize);
}

QPointF atlasPoint(const DiceMesh &mesh, int faceIndex, const QPointF &point)
{
    const DiceFace &face = mesh.faces.at(faceIndex);
    QRectF bounds;
    for (int i = 0; i < face.projected.size(); ++i) {
        const QPointF p = face.projected.at(i);
        if (i == 0)
            bounds = QRectF(p, QSizeF(0, 0));
        else {
            bounds.setLeft(qMin(bounds.left(), p.x()));
            bounds.setRight(qMax(bounds.right(), p.x()));
            bounds.setTop(qMin(bounds.top(), p.y()));
            bounds.setBottom(qMax(bounds.bottom(), p.y()));
        }
    }
    const double scale = (AtlasTileSize - 28.0) / qMax(0.0001, static_cast<double>(qMax(bounds.width(), bounds.height())));
    const QPointF projectedCenter = bounds.center();
    const QPointF tileCenter = atlasTileRect(faceIndex).center();
    return tileCenter + QPointF((point.x() - projectedCenter.x()) * scale,
                                -(point.y() - projectedCenter.y()) * scale);
}

} // namespace DiceGeometry
