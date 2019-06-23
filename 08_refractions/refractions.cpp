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

Vec3f refract(const Vec3f &I, const Vec3f &N, const float &refractive_index){
  float cosi = I*N;
  float etai = 1, etat = refractive_index;
  Vec3f n = N;
  if (cosi < 0) {
    // the ray is inside the object, swap the indices and
    // invert the normal to get the correct result
    cosi = -cosi;
    std::swap(etai, etat); n = -N;
  }
  float eta = etai/etat;
  float k = 1 - eta*eta*(1-cosi*cosi); // 1-sini*sini/n^2, k is cos_r*cos_r
  return k < 0 ? Vec3f(0, 0, 0) : I*-eta+n*(eta*cosi - sqrt(k));

}


bool scene_intersect(const Vec3f &orig,const Vec3f &dir,
  const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material )
{
  float spheres_dist = std::numeric_limits<float>::max();
  for(const auto &sphere: spheres){
    float dist_i;
    // sphere intersect with orig, ray
    if (sphere.ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
      spheres_dist = dist_i;
      hit = orig + dir*dist_i;
      N = (hit - sphere.center).normalize();
      material = sphere.material;
    }
  }
  return spheres_dist < 1000;
}

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir,const std::vector<Sphere> &spheres,
   const std::vector<Light> &lights, size_t depth = 0)
{
  Vec3f point, N;
  Material material;

  if (depth>4 ||!scene_intersect(orig, dir, spheres, point, N, material)) {
    return Vec3f(0.2, 0.7, 0.8);
  }

  // reflected ray
  // we use -dir because we want the I points to light
  Vec3f reflect_dir = reflect(-dir, N).normalize();
  Vec3f reflect_orig = reflect_dir*N < 0 ? point - N*1e-3 : point + N*1e-3;
  Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth +1);

  // refracted ray
  Vec3f refract_dir = refract(-dir, N, material.refractive_index).normalize();
  Vec3f refract_orig = refract_dir*N < 0 ? point-N*1e-3 : point + N*1e-3;
  Vec3f refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth +1);


  float diffuse_light_intensity = 0, specular_light_intensity = 0;
  for(const auto &light: lights){
    Vec3f light_dir = (light.position - point).normalize();
    // shadow check
    float light_distance = (light.position - point).norm();
    Vec3f shadow_orig = light_dir*N < 0? point - N*1e-3 : point + N*1e-3;
    // intersect, then we skip current light source
    Vec3f shadow_pt,shadow_N;
    Material tmpmaterial;
    if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N,
      tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance) continue;

    diffuse_light_intensity += light.intensity * std::max(0.f, light_dir * N);
    specular_light_intensity += powf(std::max(0.f, reflect(light_dir,N)*-dir),
     material.specular_exponent)*light.intensity; // -dir points to origin(camera)

  }
  return material.diffuse_color * diffuse_light_intensity* material.albedo[0]
  + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1]
  + reflect_color*material.albedo[2]
  + refract_color*material.albedo[3];
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
  Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0),Vec3f(0.4, 0.4, 0.3),   50.);
  Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0),Vec3f(0.3, 0.1, 0.1),   10.);
  Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0),Vec3f(1.0, 1.0, 1.0), 1425.);
  Material      glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8),Vec3f(0.6, 0.7, 0.8),  125.);

  std::vector<Sphere> spheres;
  spheres.push_back(Sphere(Vec3f(-3,     0,  -16), 2,      ivory));
  spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2,      glass));
  spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, red_rubber));
  spheres.push_back(Sphere(Vec3f(   7,    5, -18), 4,     mirror));

  std::vector<Light> lights;
  lights.push_back(Light(Vec3f(-20, 20, 20),1.5));
  lights.push_back(Light(Vec3f( 30, 50,-25),1.8));
  lights.push_back(Light(Vec3f( 30, 20, 30),1.7));

  render(spheres, lights);

  Vec3f in = Vec3f(-1, 1, 0);
  Vec3f n = Vec3f(0, 1, 0);
  return 0;
}
