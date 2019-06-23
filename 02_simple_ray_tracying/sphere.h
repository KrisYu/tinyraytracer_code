#include "geometry.h"

struct Sphere {
  Vec3f center;
  float radius;

  Sphere(const Vec3f &c, const float &r): center(c), radius(r) {}

  // ray : orig + dir
  // t0 : the intersect point near origin
  bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const
  {
    Vec3f L = center - orig;
    float tca = L*dir;
    float d2 = L*L - tca*tca;
    if (d2 > radius*radius) return false;
    float thc = sqrt(radius*radius - d2);
    t0       = tca - thc;
    float t1 = tca + thc;
    if (t0 < 0) t0 = t1;
    if (t0 < 0) return false;
    return true;
  }

};
