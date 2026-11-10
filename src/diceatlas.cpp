#include "diceatlas.h"

#include "dicegeometry.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QtMath>

namespace {

QVector<QPointF> polygonForFace(const DiceMesh &mesh, int faceIndex)
{
    QVector<QPointF> polygon;
    const DiceFace &face = mesh.faces.at(faceIndex);
    for (const QPointF &point : face.projected)
        polygon << DiceGeometry::atlasPoint(mesh, faceIndex, point);
    return polygon;
}

double distanceToSegment(const QPointF &point, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const double lengthSquared = ab.x() * ab.x() + ab.y() * ab.y();
    if (lengthSquared < 0.0001)
        return std::hypot(point.x() - a.x(), point.y() - a.y());
    const double t = qBound(0.0, ((point.x() - a.x()) * ab.x() + (point.y() - a.y()) * ab.y()) / lengthSquared, 1.0);
    const QPointF closest = a + ab * t;
    return std::hypot(point.x() - closest.x(), point.y() - closest.y());
}

double availableRadius(const QVector<QPointF> &polygon, const QPointF &center)
{
    double radius = 10000.0;
    for (int i = 0; i < polygon.size(); ++i)
        radius = qMin(radius, distanceToSegment(center, polygon.at(i), polygon.at((i + 1) % polygon.size())));
    return radius * 0.82;
}

void drawPips(QPainter &painter, const QPointF &center, double radius, int value)
{
    QVector<QPointF> positions;
    const QPointF topLeft(-1, -1);
    const QPointF topRight(1, -1);
    const QPointF middleLeft(-1, 0);
    const QPointF middleRight(1, 0);
    const QPointF bottomLeft(-1, 1);
    const QPointF bottomRight(1, 1);
    const QPointF middle(0, 0);
    if (value == 1)
        positions << middle;
    else if (value == 2)
        positions << topLeft << bottomRight;
    else if (value == 3)
        positions << topLeft << middle << bottomRight;
    else if (value == 4)
        positions << topLeft << topRight << bottomLeft << bottomRight;
    else if (value == 5)
        positions << topLeft << topRight << middle << bottomLeft << bottomRight;
    else if (value == 6)
        positions << topLeft << topRight << middleLeft << middleRight << bottomLeft << bottomRight;

    const double spacing = radius * 0.74;
    const double pipRadius = qMax(3.0, radius * 0.22);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    for (const QPointF &position : positions) {
        const QPointF pip = center + position * spacing;
        painter.drawEllipse(pip, pipRadius, pipRadius);
    }
}

void drawNumber(QPainter &painter, const QPointF &center, double radius, int value)
{
    const QString text = QString::number(value);
    QFont font;
    font.setBold(true);
    int pixelSize = qMax(10, qRound(radius * 1.35));
    const double maxWidth = radius * 1.65;
    const double maxHeight = radius * 1.45;
    while (pixelSize > 10) {
        font.setPixelSize(pixelSize);
        const QFontMetricsF metrics(font);
        const QRectF bounds = metrics.boundingRect(text);
        if (bounds.width() <= maxWidth && bounds.height() <= maxHeight)
            break;
        pixelSize -= 2;
    }
    font.setPixelSize(pixelSize);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(center - QPointF(radius, radius), QSizeF(radius * 2, radius * 2)),
                     Qt::AlignCenter | Qt::TextSingleLine, text);
}

} // namespace

namespace DiceAtlas {

QImage generate(const DiceMesh &mesh)
{
    const QSize size = DiceGeometry::atlasSize(mesh);
    QImage shape(size, QImage::Format_ARGB32);
    QImage glyphs(size, QImage::Format_ARGB32);
    shape.fill(Qt::transparent);
    glyphs.fill(Qt::transparent);

    {
        QPainter painter(&shape);
        painter.setRenderHint(QPainter::Antialiasing, true);
        for (int faceIndex = 0; faceIndex < mesh.faces.size(); ++faceIndex) {
            const QVector<QPointF> polygon = polygonForFace(mesh, faceIndex);
            QPointF center;
            for (const QPointF &point : polygon)
                center += point;
            center /= static_cast<double>(polygon.size());
            QRadialGradient gradient(center, DiceGeometry::AtlasTileSize * 0.72);
            gradient.setColorAt(0.0, QColor(255, 255, 255));
            gradient.setColorAt(0.65, QColor(225, 225, 225));
            gradient.setColorAt(1.0, QColor(170, 170, 170));
            painter.setBrush(gradient);
            painter.setPen(QPen(QColor(115, 115, 115), 2.0));
            painter.drawPolygon(polygon.constData(), polygon.size());
        }
    }

    {
        QPainter painter(&glyphs);
        painter.setRenderHint(QPainter::Antialiasing, true);
        for (int faceIndex = 0; faceIndex < mesh.faces.size(); ++faceIndex) {
            const QVector<QPointF> polygon = polygonForFace(mesh, faceIndex);
            QPointF center;
            for (const QPointF &point : polygon)
                center += point;
            center /= static_cast<double>(polygon.size());
            const double radius = availableRadius(polygon, center);
            if (mesh.type == 6)
                drawPips(painter, center, radius, faceIndex + 1);
            else
                drawNumber(painter, center, radius, faceIndex + 1);
        }
    }

    QImage output(size, QImage::Format_ARGB32);
    for (int y = 0; y < size.height(); ++y) {
        for (int x = 0; x < size.width(); ++x) {
            const QRgb face = shape.pixel(x, y);
            const int alpha = qAlpha(face);
            const int glyph = qAlpha(glyphs.pixel(x, y));
            output.setPixel(x, y, qRgba(glyph, qRed(face), 255, alpha));
        }
    }
    return output.convertToFormat(QImage::Format_RGBA8888);
}

} // namespace DiceAtlas
