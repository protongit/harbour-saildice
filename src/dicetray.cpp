#include "dicetray.h"

#include "diceatlas.h"
#include "dicegeometry.h"

#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QImage>
#include <QMatrix4x4>
#include <QQuickWindow>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGNode>
#include <QSGTexture>
#include <QVector4D>
#include <QtMath>
#include <algorithm>
#include <cstring>

namespace {

const char *diceVertexShader = R"(
attribute highp vec2 aVertex;
attribute highp vec2 aTexCoord;
attribute highp vec4 aColor;
attribute highp vec4 aInk;
uniform highp mat4 qt_Matrix;
varying highp vec2 vUV;
varying highp vec4 vColor;
varying highp vec4 vInk;
varying highp float vQ;
void main()
{
    gl_Position = qt_Matrix * vec4(aVertex, 0.0, 1.0);
    vQ = aInk.w;
    vUV = aTexCoord * aInk.w;
    vColor = aColor;
    vInk = aInk;
}
)";

const char *diceFragmentShader = R"(
precision mediump float;
uniform sampler2D uTexture;
uniform lowp float qt_Opacity;
varying highp vec2 vUV;
varying highp vec4 vColor;
varying highp vec4 vInk;
varying highp float vQ;
void main()
{
    highp vec2 uv = vUV / max(vQ, 0.0001);
    vec4 texel = texture2D(uTexture, uv);
    if (texel.a < 0.01)
        discard;
    vec3 base = vColor.rgb * (0.72 + 0.28 * texel.g);
    vec3 color = mix(base, vInk.rgb, texel.r);
    gl_FragColor = vec4(color, texel.a * vColor.a * qt_Opacity);
}
)";

struct SceneVertex
{
    float x, y;
    float u, v;
    float r, g, b, a;
    float ir, ig, ib, w;
};

const QSGGeometry::Attribute sceneAttributeList[] = {
    QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
    QSGGeometry::Attribute::create(1, 2, GL_FLOAT),
    QSGGeometry::Attribute::create(2, 4, GL_FLOAT),
    QSGGeometry::Attribute::create(3, 4, GL_FLOAT)
};

const QSGGeometry::AttributeSet sceneAttributes = {
    sizeof(sceneAttributeList) / sizeof(sceneAttributeList[0]),
    sizeof(SceneVertex),
    sceneAttributeList
};

class DiceMaterialShader;

class DiceMaterial : public QSGMaterial
{
public:
    explicit DiceMaterial(QSGTexture *texture = nullptr)
        : m_texture(texture)
    {
        setFlag(Blending);
    }

    QSGTexture *texture() const { return m_texture; }

    QSGMaterialType *type() const Q_DECL_OVERRIDE
    {
        static QSGMaterialType materialType;
        return &materialType;
    }

    int compare(const QSGMaterial *other) const Q_DECL_OVERRIDE
    {
        const DiceMaterial *material = static_cast<const DiceMaterial *>(other);
        const qintptr own = reinterpret_cast<qintptr>(m_texture);
        const qintptr theirs = reinterpret_cast<qintptr>(material->m_texture);
        return own == theirs ? 0 : (own < theirs ? -1 : 1);
    }

    QSGMaterialShader *createShader() const Q_DECL_OVERRIDE;

private:
    QSGTexture *m_texture;
};

class DiceMaterialShader : public QSGMaterialShader
{
public:
    DiceMaterialShader()
        : m_matrixLocation(-1), m_opacityLocation(-1), m_textureLocation(-1)
    {
    }

    const char *vertexShader() const Q_DECL_OVERRIDE { return diceVertexShader; }
    const char *fragmentShader() const Q_DECL_OVERRIDE { return diceFragmentShader; }

    char const *const *attributeNames() const Q_DECL_OVERRIDE
    {
        static const char *const names[] = { "aVertex", "aTexCoord", "aColor", "aInk", nullptr };
        return names;
    }

protected:
    void initialize() Q_DECL_OVERRIDE
    {
        m_matrixLocation = program()->uniformLocation("qt_Matrix");
        m_opacityLocation = program()->uniformLocation("qt_Opacity");
        m_textureLocation = program()->uniformLocation("uTexture");
    }

    void updateState(const RenderState &state, QSGMaterial *newMaterial,
                     QSGMaterial *oldMaterial) Q_DECL_OVERRIDE
    {
        Q_UNUSED(oldMaterial);
        DiceMaterial *material = static_cast<DiceMaterial *>(newMaterial);
        if (!material || !material->texture())
            return;

        if (state.isMatrixDirty())
            program()->setUniformValue(m_matrixLocation, state.combinedMatrix());
        if (state.isOpacityDirty())
            program()->setUniformValue(m_opacityLocation, state.opacity());
        program()->setUniformValue(m_textureLocation, 0);
        material->texture()->bind();
    }

private:
    int m_matrixLocation;
    int m_opacityLocation;
    int m_textureLocation;
};

QSGMaterialShader *DiceMaterial::createShader() const
{
    return new DiceMaterialShader;
}

QImage makeShadowImage()
{
    QImage image(128, 128, QImage::Format_RGBA8888);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const float dx = (x + 0.5f) / 64.0f - 1.0f;
            const float dy = (y + 0.5f) / 64.0f - 1.0f;
            const float distance = std::sqrt(dx * dx + dy * dy);
            const float alpha = std::pow(qMax(0.0f, 1.0f - distance), 1.7f);
            image.setPixel(x, y, qRgba(0, 0, 0, qRound(alpha * 255.0f)));
        }
    }
    return image;
}

QSGGeometryNode *createGeometryNode(DiceMaterial *material)
{
    QSGGeometryNode *node = new QSGGeometryNode;
    node->setFlag(QSGNode::OwnedByParent, true);
    node->setFlag(QSGNode::OwnsGeometry, true);
    node->setFlag(QSGNode::OwnsMaterial, true);
    QSGGeometry *geometry = new QSGGeometry(sceneAttributes, 0);
    geometry->setDrawingMode(GL_TRIANGLES);
    geometry->setVertexDataPattern(QSGGeometry::DynamicPattern);
    node->setGeometry(geometry);
    node->setMaterial(material);
    return node;
}

QVector3D colorVector(const QColor &color)
{
    return QVector3D(color.redF(), color.greenF(), color.blueF());
}

float clampColor(float value)
{
    return qBound(0.0f, value, 1.0f);
}

void appendProjectedVertex(QVector<SceneVertex> &vertices, const QVector3D &world,
                           const QVector4D &clip, const QPointF &uv, float width, float height,
                           const QVector3D &base, const QVector3D &ink, float alpha)
{
    const QVector3D ndc = clip.toVector3D() / clip.w();
    SceneVertex vertex;
    vertex.x = (ndc.x() * 0.5f + 0.5f) * width;
    vertex.y = (0.5f - ndc.y() * 0.5f) * height;
    vertex.u = uv.x();
    vertex.v = uv.y();
    vertex.r = base.x();
    vertex.g = base.y();
    vertex.b = base.z();
    vertex.a = alpha;
    vertex.ir = ink.x();
    vertex.ig = ink.y();
    vertex.ib = ink.z();
    vertex.w = clip.w();
    Q_UNUSED(world);
    vertices << vertex;
}

struct FaceDraw
{
    float depth;
    QVector<SceneVertex> vertices;
};

QVector<SceneVertex> buildDieGeometry(const DiceVisual &visual, const QMatrix4x4 &viewProjection,
                                      const QMatrix4x4 &view, const QVector3D &camera,
                                      float width, float height, float &centerDepth)
{
    const DiceMesh &mesh = DiceGeometry::mesh(visual.type);
    const QSize atlas = DiceGeometry::atlasSize(mesh);
    const float scale = visual.radius * visual.scale;
    float minimumY = 1000.0f;
    for (const QVector3D &vertex : mesh.vertices)
        minimumY = qMin(minimumY, visual.orientation.rotatedVector(vertex).y() * scale);

    QMatrix4x4 model;
    model.translate(visual.position.x(), -minimumY + visual.hop, visual.position.z());
    model.rotate(visual.orientation);
    model.scale(scale);

    const QVector3D centerWorld = model * QVector3D();
    centerDepth = (view * QVector4D(centerWorld, 1.0f)).z();

    const QVector3D lightDirection = QVector3D(-0.45f, 0.85f, 0.3f).normalized();
    const QColor dieColor = visual.color;
    const QVector3D baseColor = colorVector(dieColor);
    const float luminance = 0.2126f * dieColor.redF() + 0.7152f * dieColor.greenF()
            + 0.0722f * dieColor.blueF();
    const QVector3D inkColor = luminance > 0.52f
            ? QVector3D(0.075f, 0.07f, 0.065f)
            : QVector3D(0.96f, 0.95f, 0.90f);

    QVector<FaceDraw> faces;
    for (int faceIndex = 0; faceIndex < mesh.faces.size(); ++faceIndex) {
        const DiceFace &face = mesh.faces.at(faceIndex);
        const QVector3D normal = model.mapVector(face.normal).normalized();
        const QVector3D faceCenter = model * face.center;
        if (QVector3D::dotProduct(normal, camera - faceCenter) <= 0.0f)
            continue;

        const QVector3D viewDirection = (camera - faceCenter).normalized();
        const float diffuse = qMax(0.0f, QVector3D::dotProduct(normal, lightDirection));
        const QVector3D halfway = (lightDirection + viewDirection).normalized();
        const float specular = std::pow(qMax(0.0f, QVector3D::dotProduct(normal, halfway)), 32.0f) * 0.18f;
        const QVector3D base = QVector3D(clampColor(baseColor.x() * (0.52f + 0.48f * diffuse) + specular),
                                         clampColor(baseColor.y() * (0.52f + 0.48f * diffuse) + specular),
                                         clampColor(baseColor.z() * (0.52f + 0.48f * diffuse) + specular));
        const QVector3D ink = QVector3D(clampColor(inkColor.x() * (0.68f + 0.32f * diffuse)),
                                        clampColor(inkColor.y() * (0.68f + 0.32f * diffuse)),
                                        clampColor(inkColor.z() * (0.68f + 0.32f * diffuse)));

        FaceDraw draw;
        draw.depth = (view * QVector4D(faceCenter, 1.0f)).z();
        for (int triangle = 1; triangle + 1 < face.indices.size(); ++triangle) {
            const int corners[3] = { 0, triangle, triangle + 1 };
            QVector<QVector4D> clips;
            QVector<QPointF> uvs;
            for (int corner : corners) {
                const int vertexIndex = face.indices.at(corner);
                const QVector3D world = model * mesh.vertices.at(vertexIndex);
                clips << (viewProjection * QVector4D(world, 1.0f));
                const QPointF atlasPoint = DiceGeometry::atlasPoint(mesh, faceIndex,
                                                                   face.projected.at(corner));
                uvs << QPointF(atlasPoint.x() / atlas.width(), atlasPoint.y() / atlas.height());
            }
            if (clips.at(0).w() <= 0.0f || clips.at(1).w() <= 0.0f || clips.at(2).w() <= 0.0f)
                continue;
            for (int corner = 0; corner < 3; ++corner) {
                const QVector3D world = model * mesh.vertices.at(face.indices.at(corners[corner]));
                appendProjectedVertex(draw.vertices, world, clips.at(corner), uvs.at(corner),
                                      width, height, base, ink, 1.0f);
            }
        }
        if (!draw.vertices.isEmpty())
            faces << draw;
    }

    std::sort(faces.begin(), faces.end(), [](const FaceDraw &a, const FaceDraw &b) {
        return a.depth < b.depth;
    });

    QVector<SceneVertex> vertices;
    for (const FaceDraw &face : faces)
        vertices += face.vertices;
    return vertices;
}

QVector<SceneVertex> buildShadowGeometry(const QVector<DiceVisual> &visuals,
                                         const QMatrix4x4 &viewProjection,
                                         float width, float height)
{
    QVector<SceneVertex> vertices;
    for (const DiceVisual &visual : visuals) {
        const float scale = visual.radius * (1.35f + visual.hop * 0.35f);
        const float alpha = 0.32f / (1.0f + visual.hop * 1.8f);
        const QVector3D corners[4] = {
            QVector3D(visual.position.x() - scale, 0.006f, visual.position.z() - scale),
            QVector3D(visual.position.x() + scale, 0.006f, visual.position.z() - scale),
            QVector3D(visual.position.x() - scale, 0.006f, visual.position.z() + scale),
            QVector3D(visual.position.x() + scale, 0.006f, visual.position.z() + scale)
        };
        const QPointF uvs[4] = { QPointF(0, 0), QPointF(1, 0), QPointF(0, 1), QPointF(1, 1) };
        const int sequence[6] = { 0, 1, 2, 1, 3, 2 };
        const QVector3D black(0.0f, 0.0f, 0.0f);
        for (int index : sequence) {
            const QVector4D clip = viewProjection * QVector4D(corners[index], 1.0f);
            if (clip.w() <= 0.0f)
                continue;
            appendProjectedVertex(vertices, corners[index], clip, uvs[index],
                                  width, height, black, black, alpha);
        }
    }
    return vertices;
}

} // namespace

struct TrayScene
{
    QSGNode *root = nullptr;
    QSGGeometryNode *shadowNode = nullptr;
    QVector<QSGGeometryNode *> dieNodes;
    QVector<int> dieTypes;
    QHash<int, QSGTexture *> textures;
    QSGTexture *shadowTexture = nullptr;

    ~TrayScene()
    {
        qDeleteAll(textures);
        delete shadowTexture;
    }
};

DiceTray::DiceTray(QQuickItem *parent)
    : QQuickItem(parent)
    , m_scene(new TrayScene)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::NoButton);
    std::random_device device;
    m_random.seed(device() ^ static_cast<unsigned int>(QDateTime::currentMSecsSinceEpoch()));
    m_rollTimer.setInterval(16);
    connect(&m_rollTimer, &QTimer::timeout, this, &DiceTray::advanceRoll);
}

DiceTray::~DiceTray()
{
}

QSGNode *DiceTray::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    if (!oldNode) {
        m_scene->root = nullptr;
        m_scene->shadowNode = nullptr;
        m_scene->dieNodes.clear();
        m_scene->dieTypes.clear();
        qDeleteAll(m_scene->textures);
        m_scene->textures.clear();
        delete m_scene->shadowTexture;
        m_scene->shadowTexture = nullptr;
    }

    QSGNode *root = oldNode ? oldNode : new QSGNode;
    m_scene->root = root;

    if (!m_scene->shadowNode) {
        if (!m_scene->shadowTexture && window()) {
            m_scene->shadowTexture = window()->createTextureFromImage(makeShadowImage(),
                                                                     QQuickWindow::TextureHasAlphaChannel);
            if (m_scene->shadowTexture)
                m_scene->shadowTexture->setFiltering(QSGTexture::Linear);
        }
        m_scene->shadowNode = createGeometryNode(new DiceMaterial(m_scene->shadowTexture));
        root->appendChildNode(m_scene->shadowNode);
    }

    bool rebuild = m_scene->dieNodes.size() != m_visuals.size();
    for (int i = 0; !rebuild && i < m_visuals.size(); ++i)
        rebuild = m_scene->dieTypes.at(i) != m_visuals.at(i).type;

    if (rebuild) {
        for (QSGGeometryNode *node : m_scene->dieNodes) {
            root->removeChildNode(node);
            delete node;
        }
        m_scene->dieNodes.clear();
        m_scene->dieTypes.clear();
        for (const DiceVisual &visual : m_visuals) {
            if (!m_scene->textures.contains(visual.type) && window()) {
                QSGTexture *texture = window()->createTextureFromImage(
                            DiceAtlas::generate(DiceGeometry::mesh(visual.type)),
                            QQuickWindow::TextureHasAlphaChannel);
                if (texture)
                    texture->setFiltering(QSGTexture::Linear);
                m_scene->textures.insert(visual.type, texture);
            }
            QSGGeometryNode *node = createGeometryNode(
                        new DiceMaterial(m_scene->textures.value(visual.type)));
            root->appendChildNode(node);
            m_scene->dieNodes << node;
            m_scene->dieTypes << visual.type;
        }
    }

    const float itemWidth = static_cast<float>(width());
    const float itemHeight = static_cast<float>(height());
    if (itemWidth <= 0.0f || itemHeight <= 0.0f)
        return root;

    const float aspect = itemWidth / itemHeight;
    QMatrix4x4 projection;
    projection.perspective(35.0f, aspect, 0.1f, 60.0f);
    const QVector3D camera(0.0f, m_cameraDistance * 0.72f, m_cameraDistance * 0.69f);
    QMatrix4x4 view;
    view.lookAt(camera, QVector3D(0, 0.15f, 0), QVector3D(0, 1, 0));
    const QMatrix4x4 viewProjection = projection * view;

    QVector<float> depths;
    depths.reserve(m_visuals.size());
    for (int i = 0; i < m_visuals.size(); ++i) {
        float depth = 0.0f;
        const QVector<SceneVertex> vertices = buildDieGeometry(m_visuals.at(i), viewProjection, view,
                                                               camera, itemWidth, itemHeight, depth);
        QSGGeometry *geometry = m_scene->dieNodes.at(i)->geometry();
        geometry->allocate(vertices.size());
        if (!vertices.isEmpty())
            std::memcpy(geometry->vertexData(), vertices.constData(),
                        vertices.size() * sizeof(SceneVertex));
        m_scene->dieNodes.at(i)->markDirty(QSGNode::DirtyGeometry);
        depths << depth;
    }

    const QVector<SceneVertex> shadowVertices = buildShadowGeometry(m_visuals, viewProjection,
                                                                    itemWidth, itemHeight);
    QSGGeometry *shadowGeometry = m_scene->shadowNode->geometry();
    shadowGeometry->allocate(shadowVertices.size());
    if (!shadowVertices.isEmpty())
        std::memcpy(shadowGeometry->vertexData(), shadowVertices.constData(),
                    shadowVertices.size() * sizeof(SceneVertex));
    m_scene->shadowNode->markDirty(QSGNode::DirtyGeometry);

    QVector<int> order;
    order.reserve(m_visuals.size());
    for (int i = 0; i < m_visuals.size(); ++i)
        order << i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return depths.at(a) < depths.at(b);
    });

    for (QSGGeometryNode *node : m_scene->dieNodes)
        root->removeChildNode(node);
    for (int index : order)
        root->appendChildNode(m_scene->dieNodes.at(index));

    return root;
}

void DiceTray::setDice(const QVariantList &dice)
{
    if (m_rolling)
        return;
    QVariantList normalized;
    const QVector<int> supported = DiceGeometry::supportedTypes();
    for (const QVariant &value : dice) {
        const QVariantMap map = value.toMap();
        const int type = map.value(QStringLiteral("type"), 6).toInt();
        if (!supported.contains(type))
            continue;
        QColor color(map.value(QStringLiteral("color"), QStringLiteral("#e8e0d0")).toString());
        if (!color.isValid())
            color = QColor("#e8e0d0");
        QVariantMap die;
        die.insert(QStringLiteral("type"), type);
        die.insert(QStringLiteral("color"), color.name());
        normalized << die;
    }
    m_dice = normalized;
    rebuildVisuals();
    emit diceChanged();
    update();
}

void DiceTray::rebuildVisuals()
{
    m_visuals.clear();
    for (const QVariant &value : m_dice) {
        const QVariantMap map = value.toMap();
        DiceVisual visual;
        visual.type = map.value(QStringLiteral("type"), 6).toInt();
        visual.color = QColor(map.value(QStringLiteral("color")).toString());
        visual.value = 1;
        visual.orientation = DiceGeometry::landingQuaternion(DiceGeometry::mesh(visual.type), 0);
        m_visuals << visual;
    }
    layoutVisuals();
    update();
}

void DiceTray::layoutVisuals()
{
    const int count = m_visuals.size();
    if (!count)
        return;
    const int columns = count == 1 ? 1 : (count <= 4 ? 2 : (count <= 9 ? 3 : 4));
    const int rows = (count + columns - 1) / columns;
    const float spacingX = columns == 2 ? 1.55f : (columns == 3 ? 1.25f : 1.05f);
    const float spacingZ = rows == 2 ? 1.35f : 1.10f;
    const float radius = count == 1 ? 0.82f : (count <= 4 ? 0.64f : (count <= 9 ? 0.50f : 0.42f));
    for (int i = 0; i < count; ++i) {
        const int row = i / columns;
        const int column = i % columns;
        const int inRow = qMin(columns, count - row * columns);
        m_visuals[i].position = QVector3D((column - (inRow - 1) * 0.5f) * spacingX, 0,
                                          (row - (rows - 1) * 0.5f) * spacingZ);
        m_visuals[i].radius = radius;
    }
    m_cameraDistance = count == 1 ? 4.3f : (count <= 4 ? 5.6f : (count <= 9 ? 6.6f : 7.2f));
}

int DiceTray::randomInt(int minimum, int maximum)
{
    std::uniform_int_distribution<int> distribution(minimum, maximum);
    return distribution(m_random);
}

void DiceTray::setRolling(bool rolling)
{
    if (m_rolling == rolling)
        return;
    m_rolling = rolling;
    emit rollingChanged();
}

void DiceTray::roll()
{
    if (m_rolling || m_visuals.isEmpty())
        return;
    m_pendingResults.clear();
    m_rollClock.start();
    for (int i = 0; i < m_visuals.size(); ++i) {
        DiceVisual &visual = m_visuals[i];
        visual.startOrientation = visual.orientation;
        visual.value = randomInt(1, visual.type);
        visual.targetOrientation = DiceGeometry::landingQuaternion(DiceGeometry::mesh(visual.type), visual.value - 1);
        visual.spinAxis = QVector3D(randomInt(-100, 100) / 100.0f,
                                    randomInt(-100, 100) / 100.0f,
                                    randomInt(-100, 100) / 100.0f);
        if (visual.spinAxis.lengthSquared() < 0.01f)
            visual.spinAxis = QVector3D(0, 1, 0);
        visual.spinAxis.normalize();
        visual.turns = randomInt(2, 3);
        visual.delay = i * 65;
        visual.duration = randomInt(820, 1060);
        QVariantMap result;
        result.insert(QStringLiteral("type"), visual.type);
        result.insert(QStringLiteral("color"), visual.color.name());
        result.insert(QStringLiteral("value"), visual.value);
        m_pendingResults << result;
    }
    setRolling(true);
    m_rollTimer.start();
    advanceRoll();
}

void DiceTray::advanceRoll()
{
    const qint64 elapsed = m_rollClock.elapsed();
    bool allDone = true;
    for (DiceVisual &visual : m_visuals) {
        const float t = qBound(0.0f, static_cast<float>(elapsed - visual.delay) / visual.duration, 1.0f);
        if (t < 1.0f)
            allDone = false;
        const float eased = 1.0f - std::pow(1.0f - t, 3.0f);
        const QQuaternion base = QQuaternion::slerp(visual.startOrientation, visual.targetOrientation, eased);
        const QQuaternion spin = QQuaternion::fromAxisAndAngle(visual.spinAxis,
                                                               (1.0f - eased) * 360.0f * visual.turns);
        visual.orientation = (base * spin).normalized();
        visual.hop = std::sin(static_cast<float>(M_PI) * t) * 0.58f;
        visual.scale = 1.0f + std::sin(static_cast<float>(M_PI) * t) * 0.04f;
    }
    if (allDone) {
        m_rollTimer.stop();
        for (DiceVisual &visual : m_visuals) {
            visual.orientation = visual.targetOrientation;
            visual.hop = 0.0f;
            visual.scale = 1.0f;
        }
        setRolling(false);
        update();
        emit rollFinished(m_pendingResults);
        return;
    }
    update();
}
