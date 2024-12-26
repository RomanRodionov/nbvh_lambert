#ifndef UTILS_ARGS_H_
#define UTILS_ARGS_H_
#include <optional>
#include <string>

namespace patched {
    struct AArgs
    {
        std::optional<std::string> camera_file;
        std::optional<std::string> output_file;
        std::optional<std::string> envmap_file

        int spp = -1;

        bool res = false;
        int width = 1920;
        int height = 1080;

        bool use_camera_params = false;
        float camera_param[6]; //from*3 to*3
        float camera_fov;

        bool use_nbvh_params = false;
        int nbvh_param[3]; //res n_levels log2_hashmap_size

        int max_path = -1;
        bool inference_mode = false;

    };

    extern AArgs args;

}
#endif