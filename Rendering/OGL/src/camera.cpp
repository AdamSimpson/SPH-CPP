#pragma once

#include "glm/glm.hpp"

class Camera {
public:
  Camera(): max_degrees_rotate{40.0f},
            zoom_factor{1.0f},
            eye_position{0.0f, 0.0f, 0.8f},
            eye_position_default{0.0f, 0.0f, 0.8f},
            look_at_point{0.0f, -0.5f, -0.7f},
            {};

private:
  float max_degrees_rotate;
  float zoom_factor;
  glm::vec3 eye_positions;
  glm::vec3 eye_positions_default;
  glm::vec3 look_at_point;
};
