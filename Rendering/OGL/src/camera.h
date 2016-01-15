#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/norm.hpp"

class Camera {
public:

private:
  float max_degrees_rotate;
  float zoom_factor;
  glm::vec3 eye_position;
  glm::vec3 eye_position_default;
  glm::vec3 look_at_point;
};
