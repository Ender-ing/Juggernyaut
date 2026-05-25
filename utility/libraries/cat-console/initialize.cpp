/**
 * @brief
 * Handle CLI initialisation
**/

#include "include/console/initialize.hpp"

#include "include/console/basic.hpp"
#include "include/console/optimization.hpp"

namespace Console {
    namespace Internal {
        // Track status
        static bool isInitialized = false;
        bool isTrackingTime = false;
        std::chrono::high_resolution_clock::time_point start;

        void startClock() {
            start = std::chrono::high_resolution_clock::now();
            isTrackingTime = true;
        }

        // Handle CLI initialisation
        void initialize(const InitializationData &data, const std::string &extra) {
            if (isInitialized) { // TMP
                return;
            }

            // Optimise console output
            Optimization::optimize();

            /**
             *   ,-.       _,---._ __  / \
             *  /  )    .-'       `./ /   \   <name> v??? - <version>
             * (  (   ,'            `/    /|  Build: <build_type> - <arch>
             *  \  `-"             \'\   / |  Platform: <os_name> <os_version>
             *   `.              ,  \ \ /  |
             *    /`.          ,'-`----Y   |  <extra>
             *   (            ;        |   '
             *   |  ,-.    ,-'         |  /
             *   |  | (   |      jug   | /    (Cat-chy Art by Hayley Jane Wakenshaw)
             *   )  |  \  `.___________|/
             *   `--'   `--'
            **/
            // Print initial console prompt
            std::cout << std::endl << '\n' << color("  ,-.       _,---._ __ ", Color::golden_rod)
                << color(" / \\    ", Color::light_sea_green) << color(data.name, Color::light_sea_green)
                << color(" v", Color::blue_violet)
                << color(data.version, Color::blue_violet) << '\n';
            std::cout << color(" /  )    .-'       `./", Color::golden_rod)
                << color(" /   \\   Build: ", Color::light_sea_green)
                << color(data.build, Color::light_sea_green)
                << color(" - ", Color::light_sea_green) << color(data.arch, Color::light_sea_green) << '\n';
            std::cout << color("(  (   ,'            `", Color::golden_rod)
                << color("/    /|  Platform: ", Color::light_sea_green)
                << color(data.osName, Color::light_sea_green) << color(" ", Color::light_sea_green)
                << color(data.osVersion, Color::light_sea_green) << '\n';
            std::cout << color(" \\  `-\"             \\'", Color::golden_rod)
                << color("\\   / |", Color::light_sea_green) << '\n';
            std::cout << color("  `.              ,  \\", Color::golden_rod)
                << color(" \\ /  |  ", Color::light_sea_green) << color(extra, Color::light_sea_green)
                << '\n';
            std::cout << color("   /`.          ,'", Color::golden_rod) << color("-", Color::light_sea_green)
                << color("`", Color::golden_rod) << color("----Y   |", Color::light_sea_green) << '\n';
            std::cout << color("  (            ;", Color::golden_rod)
                << color("        |   '", Color::light_sea_green) << '\n';
            std::cout << color("  |  ,-.    ,-'", Color::golden_rod) << color("         |  /", Color::light_sea_green)
                << '\n';
            std::cout << color("  |  | (   |", Color::golden_rod)
                << color("      jug   | /    (Cat-chy Art by Hayley Jane Wakenshaw)", Color::light_sea_green)
                << '\n';
            std::cout << color("  )  |  \\  `.", Color::golden_rod) << color("___________|/", Color::light_sea_green)
                << '\n';
            std::cout << color("  `--'   `--'", Color::golden_rod) << '\n' << std::endl;

            // Update status to prevent duplicate calls
            isInitialized = true;
        }
    }
}
