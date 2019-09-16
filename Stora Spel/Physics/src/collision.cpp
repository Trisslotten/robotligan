#include "collision.hpp"

#include <glm/glm.hpp>
#include <vector>

struct Corners {
  glm::vec3 corners[8];
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

bool physics::Intersect(const physics::Sphere& s1, const physics::Sphere& s2) {
  float d = glm::distance(s1.center, s2.center);

  return d <= (s1.radius + s2.radius);
}

bool physics::Intersect(const physics::Sphere& s, const physics::OBB& o,
               glm::vec3* normal) {
  glm::vec3 retPt = o.center;
  glm::vec3 d = s.center - o.center;

  for (int i = 0; i < 3; ++i) {
    float dist = glm::dot(d, o.normals[i]);
    if (dist > o.extents[i]) dist = o.extents[i];
    if (dist < -o.extents[i]) dist = -o.extents[i];
    retPt += dist * o.normals[i];
  }
  glm::vec3 v = retPt - s.center;

  bool intersect = glm::dot(v, v) <= s.radius * s.radius;

  if (intersect) {
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
      float temp = o.extents[0] - dot_val;
      if (temp < gap) {
        gap = temp;
        impact_normal = temp_normal;
      } 
    }

    *normal = impact_normal;
  }

  return intersect;
}

bool physics::Intersect(const physics::OBB& o1, const physics::OBB& o2) {
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

  for (int i = 0; i < test_normals.size(); ++i) {
    float min1, max1, min2, max2;
    SatTest(test_normals[i], c1, &min1, &max1);
    SatTest(test_normals[i], c2, &min2, &max2);

    if (!Overlaps(min1, max1, min2, max2)) {
      return false;
    }
  }

  return true;
}

bool physics::Intersect(const physics::Arena& a, const physics::Sphere& s,
               glm::vec3* normal) {
  if (s.center.x + s.radius >= a.xmax) {
    *normal = glm::vec3(-1.f, 0.f, 0.f);
    return true;
  }
  if (s.center.x - s.radius <= a.xmin) {
    *normal = glm::vec3(1.f, 0.f, 0.f);
    return true;
  }
  if (s.center.y + s.radius >= a.ymax) {
    *normal = glm::vec3(0.f, -1.f, 0.f);
    return true;
  }
  if (s.center.y - s.radius <= a.ymin) {
    *normal = glm::vec3(0.f, 1.f, 0.f);
    return true;
  }
  if (s.center.z + s.radius >= a.zmax) {
    *normal = glm::vec3(0.f, 0.f, -1.f);
    return true;
  }
  if (s.center.z - s.radius <= a.zmin) {
    *normal = glm::vec3(0.f, 0.f, 1.f);
    return true;
  }

  return false;
}

bool physics::Intersect(const physics::Arena& a, const physics::OBB& o,
                        glm::vec3* move_vector) {
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

  *move_vector = move;

  return move.x || move.y || move.z;
 }