#include "GL/glew.h"
#include "world.h"
#include "SFML/OpenGL.hpp"

World::World() :matrices_binding_index_{0},
           light_binding_index_{1}
{
    camera_.max_degrees_rotate   = 40.0f;
    camera_.zoom_factor          = 1.0f;
    camera_.eye_position         = {0.0f, 0.0f, 0.8f};
    camera_.look_at_point        = {0.0f, -0.5f, -0.7f};

    light_.position = {0.3, 0.5, -0.4, 1.0};
    light_.intensity = {0.8, 0.8, 0.8, 1.0};
    light_.intensity_ambient = {0.1, 0.1, 0.1, 1.0};
    light_.attenuation = 1.0;

    /*
      init view matrix uniform
    */
    glGenBuffers(1, &matricies_UBO_);

    // Create and Allocate buffer storage
    glBindBuffer(GL_UNIFORM_BUFFER, matricies_UBO_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_STREAM_DRAW);

    // Attach to binding index
    glBindBufferRange(GL_UNIFORM_BUFFER, matrices_binding_index_, matricies_UBO_,
                      0, sizeof(glm::mat4) * 2);

    /*
      init light uniform
    */
    glGenBuffers(1, &light_UBO_);

    // Create and Allocate buffer storage
    glBindBuffer(GL_UNIFORM_BUFFER, light_UBO_);
    glBufferData(GL_UNIFORM_BUFFER, 4*sizeof(glm::vec4) + sizeof(float),
                 NULL, GL_STREAM_DRAW);

    // Attach to binding index
    glBindBufferRange(GL_UNIFORM_BUFFER, light_binding_index_, light_UBO_,
                      0, 4*sizeof(glm::vec4) + sizeof(float));

    // Set uniform values
//    update_view(state);

    // Unbind
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
};
