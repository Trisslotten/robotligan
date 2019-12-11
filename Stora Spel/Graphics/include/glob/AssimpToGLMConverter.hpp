#include <assimp/postprocess.h>
#include <sstream>
namespace glob {
namespace AssToGLM {
glm::mat3 ConvertToGLM3x3(aiMatrix3x3 aiMat) {
  return {aiMat.a1, aiMat.b1, aiMat.c1, aiMat.a2, aiMat.b2,
          aiMat.c2, aiMat.a3, aiMat.b3, aiMat.c3};
}

glm::mat4 ConvertToGLM4x4(aiMatrix4x4 aiMat) {
  return {aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1, aiMat.a2, aiMat.b2,
          aiMat.c2, aiMat.d2, aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
          aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4};
}

glm::vec3 ConvertToGLMVec3(aiVector3D aiVec) {
  return glm::vec3(aiVec.x, aiVec.y, aiVec.z);
}

glm::quat ConvertToGLMQuat(aiQuaternion aiQuat) {
  return glm::quat(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);
}

std::string PrintVec3(glm::vec3 vec) {
  std::stringstream ret;
  ret << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
  return ret.str();
}
std::string PrintQuat(glm::quat quat) {
  std::stringstream ret;
  ret << "(" << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w
      << ")";
  return ret.str();
}
std::string PrintMat4(glm::mat4 mat) {
  std::stringstream ret;
  ret << mat[0][0] << ", " << mat[0][1] << ", " << mat[0][2] << ", "
      << mat[0][3] << "\n"
      << mat[1][0] << ", " << mat[1][1] << ", " << mat[1][2] << ", "
      << mat[1][3] << "\n"
      << mat[2][0] << ", " << mat[2][1] << ", " << mat[2][2] << ", "
      << mat[2][3] << "\n"
      << mat[3][0] << ", " << mat[3][1] << ", " << mat[3][2] << ", "
      << mat[3][3];

  return ret.str();
}
}  // namespace AssToGLM
}  // namespace glob