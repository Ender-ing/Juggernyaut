/**
 * @brief
 * Handle CLI initialisation
**/

#include "initialize.hpp"

// Comms headers
#include "basic.hpp"

namespace Comms {
    namespace CLI {
        // Track status
        static bool isInitialized = false;

        // Handle CLI initialisation
        void initialize() {
            if (isInitialized) { // TMP
                return;
            }

            /**
             *   ,-.       _,---._ __  / \
             *  /  )    .-'       `./ /   \
             * (  (   ,'            `/    /|  Juggernyaut v???
             *  \  `-"             \'\   / |
             *   `.              ,  \ \ /  |  Copyright (C) 2025 Ender-ing GitHub Organisation
             *    /`.          ,'-`----Y   |
             *   (            ;        |   '
             *   |  ,-.    ,-'         |  /
             *   |  | (   |      jug   | /    (Cat-chy Art by Hayley Jane Wakenshaw)
             *   )  |  \  `.___________|/
             *   `--'   `--'
            **/
            // Print initial console prompt
            std::cout << std::endl << std::endl << color("  ,-.       _,---._ __ ", Color::golden_rod)
                << color(" / \\", Color::light_sea_green) << std::endl;
            std::cout << color(" /  )    .-'       `./", Color::golden_rod)
                << color(" /   \\", Color::light_sea_green) << std::endl;
            std::cout << color("(  (   ,'            `", Color::golden_rod)
                << color("/    /|  Juggernyaut ", Color::light_sea_green) << color("v", Color::blue_violet)
                << color(MAIN_TARGET_BINARY_VERSION, Color::blue_violet) << std::endl;
            std::cout << color(" \\  `-\"             \\'", Color::golden_rod)
                << color("\\   / |", Color::light_sea_green) << std::endl;
            std::cout << color("  `.              ,  \\", Color::golden_rod)
                << color(" \\ /  |  Copyright (C) 2025 Ender-ing GitHub Organisation", Color::light_sea_green)
                << std::endl;
            std::cout << color("   /`.          ,'", Color::golden_rod) << color("-", Color::light_sea_green)
                << color("`", Color::golden_rod) << color("----Y   |", Color::light_sea_green) << std::endl;
            std::cout << color("  (            ;", Color::golden_rod)
                << color("        |   '", Color::light_sea_green) << std::endl;
            std::cout << color("  |  ,-.    ,-'", Color::golden_rod) << color("         |  /", Color::light_sea_green)
                << std::endl;
            std::cout << color("  |  | (   |", Color::golden_rod)
                << color("      jug   | /    (Cat-chy Art by Hayley Jane Wakenshaw)", Color::light_sea_green)
                << std::endl;
            std::cout << color("  )  |  \\  `.", Color::golden_rod) << color("___________|/", Color::light_sea_green)
                << std::endl;
            std::cout << color("  `--'   `--'", Color::golden_rod) << std::endl << std::endl;

            // Update status to prevent duplicate calls
            isInitialized = true;
        }
    }
}
