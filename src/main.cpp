

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "app/render_app.h"
#include "app/render_display.h"
#include "base/scene.h"
#include "cuda/cuda_backend.h"
#include "utils/logger.h"

#include "utils/args.h"

#include <getopt.h>

using namespace ntwr;

void parse_res(std::string str)
{
    using patched::args;
    size_t pos = str.find(" ");

    args.res = true;
    args.width = std::stoi(str.substr(0, pos));
    args.height = std::stoi(str.substr(pos + 1, std::string::npos));

}

void parse_nbvh_params(std::string str)
{
    using patched::args;

    size_t pos = str.find(" ");
    size_t prev = 0ul;

    //1 - 

    for(int i = 0; i < 3; ++i) {
        args.nbvh_param[i] = std::stoi(str.substr(prev, pos));
        prev = pos + 1;
        pos = str.find(" ", prev);
    }
    args.use_nbvh_params = true;
}


void parse_camera(std::string str) 
{
    using patched::args;

    for(int i = 0; i < str.size(); ++i) {
        if(str[i] == 'm') str[i] = '-';
    }

    size_t pos = str.find(" ");
    size_t prev = 0ul;

    for(int i = 0; i < 6; ++i) {
        args.camera_param[i] = std::stof(str.substr(prev, pos));
        prev = pos + 1;
        pos = str.find(" ", prev);
    }
    args.camera_fov = std::stof(str.substr(prev, pos));
    args.use_camera_params = true;
}

int main(int argc, char *argv[])
{
    using patched::args;

    if (argc < 1) {
        logger(LogLevel::Info,
               "Usage: %s [obj_scene_filename] [validation_mode (true/false)] {-n [res] [n_levels] [log2_hashmap_size]} {-c [camera] / -t [origin] [direction] [fov]} {-s [max_spp]} {-r [width] [height]} [config1] [config2] {-o [output_filename]}",
               argv[0]);
        return 1;
    }
    /*patch begin*/
    int c;
    while((c = getopt(argc, argv, "e:n:o:c:s:r:t:i")) != -1) {
        switch(c) {
        case 'o':
            args.output_file.emplace(optarg);
            break;
        case 'c':
            args.camera_file.emplace(optarg);
            break;
        case 's':
            args.spp = std::stoi(std::string(optarg));
            break;
        case 'r':
            parse_res(std::string(optarg));
            break;
        case 't':
            parse_camera(std::string(optarg));
            break;
        case 'n':
            parse_nbvh_params(std::string(optarg));
            break;
        case 'i':
            args.inference_mode = true;
            break;
        case 'e':
            args.envmap_file.emplace(optarg);
            break;
        case '?':
            std::cerr << "Unknown argument." << std::endl; 
            return false;
        }
    }  

    const int fargc = argc - optind;

    /*patch end*/

    glm::uvec2 window_size(args.width, args.height);

    std::cout << args.width << " " << args.height << std::endl;

    // Create display
    std::unique_ptr<RenderDisplay> display = std::make_unique<RenderDisplay>(window_size);

    std::filesystem::path scene_filename = "";
    bool validation_mode                 = false;

    if (fargc >= 1) {
        scene_filename = argv[optind];
        // Convert to posix-style path
        scene_filename = std::filesystem::path(scene_filename.generic_string());
    }

    if (fargc >= 2) {
        validation_mode = strcmp(argv[optind + 1], "true") == 0;
    }

    RenderAppConfigs render_app_configs = RenderAppConfigs(validation_mode, scene_filename);

    if (fargc >= 3) {
        for (size_t arg_idx = 2; arg_idx < fargc; arg_idx++) {
            render_app_configs.add_config_file(std::string(argv[optind + arg_idx]));
            if (!validation_mode && argc > 4) {
                logger(LogLevel::Warn,
                       "Only one config at can be loaded when not in validation mode! Skipping remaining.");
                break;
            }
        }
    }

    if (validation_mode) {
        logger(LogLevel::Warn, "Running  %d configs in validation mode!", render_app_configs.n_configs());
    }

    // Create main app and attach display to it
    std::unique_ptr<RenderApp> render_app =
        std::make_unique<RenderApp>(render_app_configs, scene_filename, display.get());

    try {
        render_app->run();
    } catch (std::runtime_error &e) {
        logger(LogLevel::Error, "%s", e.what());
        exit(1);
    }
    return 0;
}