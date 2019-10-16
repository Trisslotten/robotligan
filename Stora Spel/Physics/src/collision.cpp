#include "collision.hpp"

#include <glm/glm.hpp>
#include <cmath>
#include <iostream>
#include <vector>

std::ostream& operator<<(std::ostream& o, glm::vec3 v) {
  return o << v.x << " " << v.y << " " << v.z;
}

struct Corners {
  glm::vec3 corners[8];
};

struct Triangle {
  glm::vec3 p0;
  glm::vec3 p1;
  glm::vec3 p2;
};

Corners GetCorners(const physics::OBB& obb) {
  glm::vec3 normals[3] = {};
  normals[0] = obb.normals[0] * obb.extents[0];
  normals[1] = obb.normals[1] * obb.extents[1];
  normals[2] = obb.normals[2] * obb.extents[2];
  
  Corners c = {};
  c.corners[0] = obb.center + (normals[0] + normals[1] + normals[2]);
  c.corners[1] = obb.center + (-normals[0] + normals[1] + normals[2]);
  c.corners[2] = obb.center + (normals[0] - normals[1] + normals[2]);
  c.corners[3] = obb.center + (normals[0] + normals[1] - normals[2]);
  c.corners[4] = obb.center + (-normals[0] - normals[1] + normals[2]);
  c.corners[5] = obb.center + (-normals[0] + normals[1] - normals[2]);
  c.corners[6] = obb.center + (normals[0] - normals[1] - normals[2]);
  c.corners[7] = obb.center + (-normals[0] - normals[1] - normals[2]);

  return c;
}

void SatTest(const glm::vec3& axis, const Corners& c, float* min, float* max) {
  *min = 10000000.f;   // infinity
  *max = -10000000.f;  // -infinity

  for (int i = 0; i < 8; ++i) {
    float dot_val = glm::dot(axis, c.corners[i]);
    if (dot_val < *min) *min = dot_val;
    if (dot_val > *max) *max = dot_val;
  }
}

bool IsBetween(float val, float lower, float upper) {
  return lower <= val && val <= upper;
}

bool Overlaps(float min1, float max1, float min2, float max2) {
  return IsBetween(min2, min1, max1) || IsBetween(min1, min2, max2);
}

physics::IntersectData physics::Intersect(const physics::Sphere& s1, const physics::Sphere& s2) {
  IntersectData data;
  data.move_vector = s1.center - s2.center;
  data.normal = data.move_vector;
  data.collision = glm::length(data.move_vector) <= s1.radius + s2.radius;

  return data;
}

physics::IntersectData physics::Intersect(const physics::Sphere& s,
                                          const physics::OBB& o) {
  IntersectData data;
  glm::vec3 retPt = o.center;
  glm::vec3 d = s.center - o.center;

  for (int i = 0; i < 3; ++i) {
    float dist = glm::dot(d, o.normals[i]);
    if (dist > o.extents[i]) dist = o.extents[i];
    if (dist < -o.extents[i]) dist = -o.extents[i];
    retPt += dist * o.normals[i];
  }
  glm::vec3 v = retPt - s.center;

  data.collision = glm::dot(v, v) <= s.radius * s.radius;

  if (data.collision) {
    glm::vec3 impact_vector = retPt - o.center;
    glm::vec3 impact_normal;
    float gap = 1000000000.f;

    for (int i = 0; i < 3; ++i) {
      float dot_val = glm::dot(o.normals[i], impact_vector);
      glm::vec3 temp_normal = o.normals[i];
      if (dot_val < 0) {
        dot_val = -dot_val;
        temp_normal *= -1;
      }
      float temp = o.extents[i] - dot_val;
      if (temp < gap) {
        gap = temp;
        impact_normal = temp_normal;
      } 
    }
    data.normal = impact_normal;
    data.move_vector = (s.radius - glm::length(retPt - s.center)) * data.normal;
  }

  return data;
}

physics::IntersectData physics::Intersect(const physics::OBB& o1,
                                          const physics::OBB& o2) {
  IntersectData data;
  data.collision = true;
  Corners c1 = GetCorners(o1);
  Corners c2 = GetCorners(o2);

  std::vector<glm::vec3> test_normals;
  test_normals.reserve(15);

  for (int i = 0; i < 3; ++i) {
    test_normals.push_back(o1.normals[i]);
    for (int j = 0; j < 3; ++j) {
      if (i == 0) test_normals.push_back(o2.normals[j]);
      test_normals.push_back(glm::cross(o1.normals[i], o2.normals[j]));
    }
  }

  glm::vec3 collision_normal(0.f);
  float min_dist = 1000.f;
  for (int i = 0; i < test_normals.size(); ++i) {
    float min1, max1, min2, max2;
    SatTest(test_normals[i], c1, &min1, &max1);
    SatTest(test_normals[i], c2, &min2, &max2);

    if (!Overlaps(min1, max1, min2, max2)) {
      data.collision = false;
      return data;
    }

    if (glm::length(test_normals[i]) > 0.f) {
      if ((max2 - min1) / glm::length(test_normals[i]) < min_dist) {
        min_dist = (max2 - min1) / glm::length(test_normals[i]);
        collision_normal = test_normals[i];
      }
      if ((max1 - min2) / glm::length(test_normals[i]) < min_dist) {
        min_dist = (max1 - min2) / glm::length(test_normals[i]);
        collision_normal = -test_normals[i];
      }
    }
  }

  data.normal = collision_normal;
  data.move_vector = min_dist * data.normal;

  return data;
}

physics::IntersectData physics::Intersect(const physics::Arena& a,
                                          const physics::Sphere& s) {
  IntersectData data;
  data.collision = false;
  data.move_vector = glm::vec3(0.f);
  glm::vec3 n = glm::vec3(0.f);
  if (s.center.x + s.radius > a.xmax) {
    n += glm::vec3(-1.f, 0.f, 0.f);
    data.move_vector.x = a.xmax - (s.center.x + s.radius);
    data.collision = true;
  }
  if (s.center.x - s.radius < a.xmin) {
    n += glm::vec3(1.f, 0.f, 0.f);
    data.move_vector.x = a.xmin - (s.center.x - s.radius);
    data.collision = true;
  }
  if (s.center.y + s.radius > a.ymax) {
    n += glm::vec3(0.f, -1.f, 0.f);
    data.move_vector.y = a.ymax - (s.center.y + s.radius);
    data.collision = true;
  }
  if (s.center.y - s.radius < a.ymin) {
    n += glm::vec3(0.f, 1.f, 0.f);
    data.move_vector.y = a.ymin - (s.center.y - s.radius);
    data.collision = true;
  }
  if (s.center.z + s.radius > a.zmax) {
    n += glm::vec3(0.f, 0.f, -1.f);
    data.move_vector.z = a.zmax - (s.center.z + s.radius);
    data.collision = true;
  }
  if (s.center.z - s.radius < a.zmin) {
    n += glm::vec3(0.f, 0.f, 1.f);
    data.move_vector.z = a.zmin - (s.center.z - s.radius);
    data.collision = true;
  }

  if (data.collision) {
    data.normal = n; 
  }

  return data;
}

physics::IntersectData physics::Intersect(const physics::Arena& a,
                                          const physics::OBB& o) {
  IntersectData data;
  Corners c = GetCorners(o);
  glm::vec3 move = {};

  for (int i = 0; i < 8; ++i) {
    if (a.xmax - c.corners[i].x < move.x)
      move.x = a.xmax - c.corners[i].x;
    if (a.xmin - c.corners[i].x > move.x)
      move.x = a.xmin - c.corners[i].x;
    if (a.ymax - c.corners[i].y < move.y)
      move.y = a.ymax - c.corners[i].y;
    if (a.ymin - c.corners[i].y > move.y)
      move.y = a.ymin - c.corners[i].y;
    if (a.zmax - c.corners[i].z < move.z)
      move.z = a.zmax - c.corners[i].z;
    if (a.zmin - c.corners[i].z > move.z)
      move.z = a.zmin - c.corners[i].z;
  }

  data.move_vector = move;
  data.normal = glm::vec3(0.f);
  data.collision = move.x || move.y || move.z;
  return data;
 }

// https://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
float ComputeSquaredDistance(const glm::vec3& center, const Triangle& tri,
  glm::vec3* closest_point) {
   glm::vec3 edges[3] = {tri.p1 - tri.p0, tri.p2 - tri.p0, tri.p2 - tri.p1};

   glm::vec3 D = tri.p0 - center;
   float a = glm::dot(edges[0], edges[0]);
   float b = glm::dot(edges[0], edges[1]);
   float c = glm::dot(edges[1], edges[1]);
   float d = glm::dot(edges[0], D);
   float e = glm::dot(edges[1], D);
   float f = glm::dot(D, D);
   //float sigma = a * c - b * b;
   //if (sigma < 0) sigma = -sigma;

   float s = b * e - c * d;
   float t = b * d - a * e;
   float det = a * c - b * b;
   if (det < 0) det = -det;

   if (s + t <= det) {
     if (s < 0) {
       if (t < 0) {
         // region 4
         return 1000.f;
         if (d < 0) {
           t = 0;
           if (-d >= a)
             s = 1;
           else
             s = -d / a;
         } else {
           s = 0;
           if (e >= 0)
             t = 0;
           else if (-e >= c)
             t = 1;
           else
             t = -e / c;
         }
       } else {
         // region 3
         return 1000.f;
         s = 0;
         if (e >= 0)
           t = 0;
         else if (-e >= c)
           t = 1;
         else
           t = -e / c;
       }
     } else if (t < 0) {
       // region 5
       return 1000.f;
       t = 0;
       if (d >= 0)
         s = 0;
       else if (-d >= a)
         s = 1;
       else
         s = -d / a;
     } else {
       // region 0
       s /= det;
       t /= det;
     }
   } else {
     if (s < 0) {
       //region 2
       return 1000.f;
       float temp0 = b + d;
       float temp1 = c + e;
       if (temp1 > temp0) {
         float numer = temp1 - temp0;
         float denom = a - 2 * b + c;
         if (numer >= denom)
           s = 1;
         else
           s = numer / denom;

         t = 1 - s;
       } else {
         s = 0;
         if (temp1 <= 0)
           t = 1;
         else if (e >= 0)
           t = 0;
         else
           t = -e / c;
       }
     } else if (t < 0) {
       // region 6
       return 1000.f;
       float temp0 = b + e;
       float temp1 = a + d;
       if (temp1 > temp0) {
         float numer = temp1 - temp0;
         float denom = a - 2 * b + c;
         if (numer >= denom)
           t = 1;
         else
           t = numer / denom;

         s = 1 - t;
       } else {
         t = 0;
         if (temp1 <= 0)
           s = 1;
         else if (d >= 0)
           s = 0;
         else
           s = -d / a;
       }
     } else {
       // region 1
       return 1000.f;
       float numer = (c + e) - (b + d);
       if (numer <= 0.f) {
         s = 0;
       } else {
         float denom = a - 2 * b + c;
         if (numer >= denom) {
           s = 1;
         } else {
           s = numer / denom;
         }
       }

       t = 1 - s;
     }
   }

   *closest_point = tri.p0 + s * edges[0] + t * edges[1];
   return a * s * s + 2 * b * s * t + c * t * t + 2 * d * s + 2 * e * t + f;
 }

physics::IntersectData physics::Intersect(const MeshHitbox& m,
                                           const Sphere& s) {
  IntersectData data;
  data.normal = glm::vec3(0.f);
  data.collision = false;
  float rsqrt = sqrtf(s.radius);
  glm::vec3 closest_point;
  for (size_t i = 0; i < m.indices.size(); i += 3) {
    Triangle tri{m.pos[m.indices[i]], m.pos[m.indices[i + 1]],
                 m.pos[m.indices[i + 2]]};
    float dist = ComputeSquaredDistance(s.center, tri, &closest_point);

    glm::vec3 temp = glm::normalize(glm::cross(tri.p1 - tri.p0, tri.p2 - tri.p0));
    if (dist <= rsqrt) {
      if (fabs(temp.x) < 0.05f && fabs(temp.x) > 0.0) {
        temp.x = 0.f;
      }
      if (fabs(temp.y) < 0.05f && fabs(temp.y) > 0.0) {
        temp.y = 0.f;
      }
      if (fabs(temp.z) < 0.05f && fabs(temp.z) > 0.0) {
        temp.z = 0.f;
      }
      data.normal += temp;
      data.collision = true;
      data.move_vector = (rsqrt - dist) * data.normal;

      return data;
    } else if (dist != 1000.f && glm::dot(temp, tri.p0 - s.center) > 0.f) {
      data.collision = true;
      data.normal = temp;
      data.move_vector = data.normal * glm::dot(temp, tri.p0 - s.center);

      return data;
    }
  }

  data.normal = glm::normalize(data.normal);

  return data;
}