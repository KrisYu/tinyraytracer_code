#include "geometry.h"

struct Material {
  Material(const Vec2f &a,const Vec3f &color, const float &spec):
  albedo(a), diffuse_color(color), specular_exponent(spec) {}
  Material(): albedo(1,0), diffuse_color(), specular_exponent() {}
  Vec2f albedo; // albedo[0] - diffuse albedo, albedo[1] - specular albedo
  Vec3f diffuse_color;
  float specular_exponent;
};

struct Sphere {
  Vec3f center;
  float radius;
  Material material;

  Sphere(const Vec3f &c, const float &r, const Material &m): center(c), radius(r), material(m) {}

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
