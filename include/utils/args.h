#ifndef UTILS_ARGS_H_
#define UTILS_ARGS_H_
#include <optional>
#include <string>

namespace patched {
    struct AArgs
    {
        std::optional<std::string> camera_file;
        std::optional<std::string> output_file;
        int spp = -1;
        int width = 1920;
        int height = 1080;
        bool use_camera_params = false;
        float camera_param[6];
        float camera_fov;
        int max_path = -1;
    };

    extern AArgs args;

}
#endif