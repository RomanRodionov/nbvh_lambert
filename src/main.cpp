

#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "app/render_app.h"
#include "app/render_display.h"
#include "base/scene.h"
#include "cuda/cuda_backend.h"
#include "utils/logger.h"

#include <optional>
#include <getopt.h>

using namespace ntwr;

std::optional<std::string> cli_save_path;

int main(int argc, char *argv[])
{
    if (argc < 1) {
        logger(LogLevel::Info,
               "Usage: %s [obj_scene_filename] [validation_mode (true/false)] {-c [camera]} [config1] [config2] {-o [output_filename]}",
               argv[0]);
        return 1;
    }
    /*patch begin*/
    int c;
    std::optional<std::string> output_file{};
    std::optional<std::string> camera_file{};
    while((c = getopt(argc, argv, "o:c:")) != -1) {
        switch(c) {
        case 'o':
            output_file.emplace(optarg);
            break;
        case 'c':
            camera_file.emplace(optarg);
            break;
        case '?':
            std::cerr << "[!] Unknown argument." << std::endl; 
            return false;
        }
    }  

    const int fargc = argc - optind;

    /*patch end*/

    glm::uvec2 window_size(1920, 1080);

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
        std::make_unique<RenderApp>(render_app_configs, scene_filename, display.get(), output_file, camera_file);

    try {
        render_app->run();
    } catch (std::runtime_error &e) {
        logger(LogLevel::Error, "%s", e.what());
        exit(1);
    }
    return 0;
}