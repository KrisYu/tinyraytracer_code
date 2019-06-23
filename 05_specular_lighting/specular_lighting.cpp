#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"
#include "sphere.h"

struct Light {
  Light(const Vec3f &p, const float &i): position(p), intensity(i){}
  Vec3f position;
  float intensity;
};


Vec3f reflect(const Vec3f &I, const Vec3f &N){
  return N*2.f*(I*N)-I;
}


bool scene_intersect(const Vec3f &orig,const Vec3f &dir,
  const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material )
{
  float spheres_dist = std::numeric_limits<float>::max();
  for(const auto &sphere: spheres){
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

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir,
  const std::vector<Sphere> &spheres, const std::vector<Light> &lights)
{
  Vec3f point, N;
  Material material;

  if (!scene_intersect(orig, dir, spheres, point, N, material)) {
    return Vec3f(0.2, 0.7, 0.8);
  }
  float diffuse_light_intensity = 0, specular_light_intensity = 0;
  for(const auto &light: lights){
    Vec3f light_dir = (light.position - point).normalize();
    diffuse_light_intensity += light.intensity * std::max(0.f, light_dir * N);
    specular_light_intensity += powf(std::max(0.f, reflect(light_dir,N)*-dir),
     material.specular_exponent)*light.intensity; // -dir points to origin(camera)
  }
  return material.diffuse_color * diffuse_light_intensity* material.albedo[0]
  + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1];
  // white as specular color
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights)
{
  const int width   = 1024;
  const int height  = 768;
  const float fov = M_PI/2.;
  std::vector<Vec3f> framebuffer(width*height);

  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      float x = (2*(i+0.5)/float(width)-1)*tan(fov/2.)*width/float(height);
      float y = -(2*(j+0.5)/float(height)-1)*tan(fov/2.);
      Vec3f dir = Vec3f(x, y, -1).normalize();
      framebuffer[i+j*width] = cast_ray(Vec3f(0,0,0), dir, spheres, lights);
    }
  }

  std::ofstream ofs;
  ofs.open("./out.ppm");
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (size_t i = 0; i < width*height; i++) {
    Vec3f &c = framebuffer[i];
    float max = std::max(c[0], std::max(c[1],c[2]));
    if (max > 1) c = c*(1./max); // make sure it won't acheive 100% white
    for (size_t j = 0; j < 3; j++) {
      ofs << (char)(255*std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }
  ofs.close();
}

int main(int argc, char const *argv[]) {
  Material      ivory(Vec2f(0.6, 0.3),Vec3f(0.4, 0.4, 0.3), 50.);
  Material red_rubber(Vec2f(0.9, 0.1),Vec3f(0.3, 0.1, 0.1), 10.);

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3,     0,  -16), 2,      ivory));
  spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber));
  spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, red_rubber));
  spheres.push_back(Sphere(Vec3f(   7,    5, -18), 4,      ivory));

  std::vector<Light> lights;
  lights.push_back(Light(Vec3f(-20, 20, 20),1.5));
  lights.push_back(Light(Vec3f( 30, 50,-25),1.8));
  lights.push_back(Light(Vec3f( 30, 20, 30),1.7));

  render(spheres, lights);
  return 0;
}
