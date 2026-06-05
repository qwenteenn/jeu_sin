#include "Projectile.h"

void Projectile::Update(float dt) {
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    position.z += velocity.z * dt;
    lifetime -= dt;
    if (lifetime <= 0.0f) alive = false;
}

void Projectile::Draw() const {
    DrawSphere(position, radius, color);
    DrawSphereWires(position, radius * 1.25f, 8, 8, FadeColor(color, 0.55f));
}
