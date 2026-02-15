#pragma once

#include <glm/glm.hpp>

struct CameraData
{
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec4 camera_position;
    glm::vec4 hfovxy_focal;
};