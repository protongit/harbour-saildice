#ifndef DICEATLAS_H
#define DICEATLAS_H

#include <QImage>

struct DiceMesh;

namespace DiceAtlas {

// RGBA texture: R = printed glyph, G = face shading, A = face coverage.
// The atlas is colour-neutral so one texture serves every die colour.
QImage generate(const DiceMesh &mesh);

} // namespace DiceAtlas

#endif // DICEATLAS_H
