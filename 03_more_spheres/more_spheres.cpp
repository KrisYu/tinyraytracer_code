#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"
#include "sphere.h"

bool scene_intersect(const Vec3f &orig,const Vec3f &dir,
  const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material )
{
  float spheres_dist = std::numeric_limits<float>::max();
  for(const auto sphere: spheres){
    float dist_i;
    if (sphere.ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
      spheres_dist = dist_i;
      hit = orig + dir*dist_i;
      N = (hit - sphere.center).normalize();
      material = sphere.material;
    }
  }
  return spheres_dist < 1000;
}

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres){
  Vec3f point, N;
  Material material;

  if (!scene_intersect(orig, dir, spheres, point, N, material)) {
    return Vec3f(0.2, 0.7, 0.8);
  }
  return material.diffuse_color;
}

void render(const std::vector<Sphere> &spheres) {
  const int width   = 1024;
  const int height  = 768;
  const float fov = M_PI/2.;
  std::vector<Vec3f> framebuffer(width*height);

  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      float x = (2*(i+0.5)/float(width)-1)*tan(fov/2.)*width/float(height);
      float y = -(2*(j+0.5)/float(height)-1)*tan(fov/2.);
      Vec3f dir = Vec3f(x, y, -1).normalize();
      framebuffer[i+j*width] = cast_ray(Vec3f(0,0,0), dir, spheres);
    }
  }

  std::ofstream ofs;
  ofs.open("./out.ppm");
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (size_t i = 0; i < width*height; i++) {
    for (size_t j = 0; j < 3; j++) {
      ofs << (char)(255*std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }
  ofs.close();
}

int main(int argc, char const *argv[]) {
  Material      ivory(Vec3f(0.4, 0.4, 0.3));
  Material red_rubber(Vec3f(0.3, 0.1, 0.1));

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3,     0,  -16), 2,      ivory));
  spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber));
  spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, red_rubber));
  spheres.push_back(Sphere(Vec3f(   7,    5, -18), 4,      ivory));

  render(spheres);
  return 0;
}
