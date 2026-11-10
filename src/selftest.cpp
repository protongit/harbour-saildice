#include "selftest.h"

#include "dicegeometry.h"

#include <QTextStream>
#include <random>

namespace {

bool nearlyEqual(const QVector3D &a, const QVector3D &b, float tolerance = 0.0003f)
{
    return (a - b).length() < tolerance;
}

} // namespace

int runSelfTest()
{
    QTextStream out(stdout);
    int failures = 0;
    auto check = [&](bool condition, const QString &message) {
        if (!condition) {
            ++failures;
            out << "FAIL: " << message << "\n";
        }
    };

    DiceGeometry::prebuild();
    for (int type : DiceGeometry::supportedTypes()) {
        const DiceMesh &mesh = DiceGeometry::mesh(type);
        check(mesh.faces.size() == type, QString("D%1: expected %2 faces, got %3")
              .arg(type).arg(type).arg(mesh.faces.size()));
        check(!mesh.vertices.isEmpty(), QString("D%1: no vertices").arg(type));
        for (int faceIndex = 0; faceIndex < mesh.faces.size(); ++faceIndex) {
            const DiceFace &face = mesh.faces.at(faceIndex);
            check(face.indices.size() >= 3, QString("D%1 face %2: degenerate").arg(type).arg(faceIndex + 1));
            check(face.indices.size() == face.projected.size(),
                  QString("D%1 face %2: projection mismatch").arg(type).arg(faceIndex + 1));
            check(qAbs(face.normal.length() - 1.0f) < 0.0002f,
                  QString("D%1 face %2: normal not unit").arg(type).arg(faceIndex + 1));
            check(QVector3D::dotProduct(face.center, face.normal) > 0.0f,
                  QString("D%1 face %2: normal points inwards").arg(type).arg(faceIndex + 1));
            for (int corner = 0; corner < face.indices.size(); ++corner) {
                const int vertexIndex = face.indices.at(corner);
                check(vertexIndex >= 0 && vertexIndex < mesh.vertices.size(),
                      QString("D%1 face %2: invalid vertex").arg(type).arg(faceIndex + 1));
                const float planeDistance = QVector3D::dotProduct(mesh.vertices.at(vertexIndex) - face.center, face.normal);
                check(qAbs(planeDistance) < 0.0003f,
                      QString("D%1 face %2: non-planar vertex (%3)")
                      .arg(type).arg(faceIndex + 1).arg(planeDistance));
                const QPointF atlas = DiceGeometry::atlasPoint(mesh, faceIndex, face.projected.at(corner));
                const QRectF tile = DiceGeometry::atlasTileRect(faceIndex);
                check(tile.adjusted(-1, -1, 1, 1).contains(atlas),
                      QString("D%1 face %2: UV outside atlas tile").arg(type).arg(faceIndex + 1));
            }

            const QQuaternion landing = DiceGeometry::landingQuaternion(mesh, faceIndex);
            check(nearlyEqual(landing.rotatedVector(face.normal), QVector3D(0, 1, 0)),
                  QString("D%1 face %2: result face does not land upwards").arg(type).arg(faceIndex + 1));
            check(nearlyEqual(landing.rotatedVector(face.tangentV), QVector3D(0, 0, -1)),
                  QString("D%1 face %2: printed value is not upright").arg(type).arg(faceIndex + 1));
        }
        if (type != 4 && mesh.faces.size() >= 2) {
            for (int i = 0; i < mesh.faces.size() / 2; ++i) {
                const QVector3D sum = mesh.faces.at(i).normal + mesh.faces.at(mesh.faces.size() - 1 - i).normal;
                check(sum.length() < 0.001f,
                      QString("D%1: opposite face pairing is inconsistent").arg(type));
            }
        }
        out << "PASS: D" << type << " geometry, " << mesh.faces.size() << " landing orientations\n";
    }

    std::mt19937 random(1234567);
    for (int type : DiceGeometry::supportedTypes()) {
        QVector<int> counts(type + 1, 0);
        std::uniform_int_distribution<int> distribution(1, type);
        const int rolls = type * 10000;
        for (int i = 0; i < rolls; ++i) {
            const int value = distribution(random);
            check(value >= 1 && value <= type, QString("D%1: RNG out of range").arg(type));
            ++counts[value];
        }
        const int expected = rolls / type;
        for (int value = 1; value <= type; ++value) {
            check(qAbs(counts[value] - expected) < expected * 0.12,
                  QString("D%1 value %2: non-uniform RNG (%3 vs %4)")
                  .arg(type).arg(value).arg(counts[value]).arg(expected));
        }
        out << "PASS: D" << type << " RNG, " << rolls << " rolls\n";
    }

    if (failures) {
        out << failures << " check(s) failed\n";
        return 1;
    }
    out << "All checks passed\n";
    return 0;
}
