/**
 * @brief
 * Handle CLI initialisation
**/

#include "finalize.hpp"

#include "initialize.hpp"

// Basic C++ headers
#include <iomanip>

#include "console.hpp"
#include "basic.hpp"
#include "optimization.hpp"

namespace Console {
    namespace Internal {
        std::string getDuration() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            // Hours
            auto h = std::chrono::duration_cast<std::chrono::hours>(duration);
            duration -= h;

            // Minutes
            auto m = std::chrono::duration_cast<std::chrono::minutes>(duration);
            duration -= m;

            // Seconds
            auto s = std::chrono::duration_cast<std::chrono::seconds>(duration);
            duration -= s;

            // Milliseconds
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

            // 5. Format the output with leading zeros (e.g., 05:02:09.045)
            std::stringstream ss;
            ss << std::setfill('0') 
               << std::setw(2) << h.count() << ":"
               << std::setw(2) << m.count() << ":"
               << std::setw(2) << s.count() << "."
               << std::setw(3) << ms.count();

            return ss.str();
        }

        // Handle CLI finalisation
        void finalize() {
            if (minimalProtocolFinalization) { //TMP
                std::cout << std::endl;
                return;
            }

            // Keep track of the program's status
            std::string status;
            if (Console::ProcessReport::programStatus == 0) {
                status = "Success";
            } else {
                status = "Failure";
            }

            // Attach other status strings
            int warnings = Statistics::warningReports;
            int errors = Statistics::criticalReports + Statistics::fatalReports;
            int reports = warnings + errors + Statistics::actionReports;

            // Simple message
            if (errors > 0 && warnings > 0) {
                status.append(", with warnings & errors!");
            } else if (errors > 0) {
                status.append(", with errors!");
            } else if (warnings > 0) {
                status.append(", with warnings!");
            } else {
                status.append("!");
            }

            // Report statistics
            std::stringstream actionsString,
                warningsString,
                criticalsString,
                fatalsString,
                timeString;
            if (Statistics::actionReports > 0) {
                actionsString << color(std::to_string(Statistics::actionReports), Color::sea_green)
                    << color(" action(s)", Color::light_sea_green);
            }
            if (Statistics::warningReports > 0) {
                if (Statistics::actionReports > 0) {
                    warningsString << color(", ", Color::light_sea_green);
                }
                warningsString << color(std::to_string(Statistics::warningReports), Color::golden_rod)
                    << color(" warning(s)", Color::light_sea_green);
            }
            if (Statistics::criticalReports > 0) {
                if (Statistics::actionReports > 0 || Statistics::warningReports > 0) {
                    criticalsString << color(", ", Color::light_sea_green);
                }
                criticalsString << color(std::to_string(Statistics::criticalReports), Color::crimson)
                    << color(" error(s)", Color::light_sea_green);
            }
            if (Statistics::fatalReports > 0) {
                if (Statistics::actionReports > 0 || Statistics::warningReports > 0
                    || Statistics::criticalReports > 0) {
                    fatalsString << color(", ", Color::light_sea_green);
                }
                fatalsString << color(std::to_string(Statistics::fatalReports), Color::crimson)
                    << color(" fatal error(s)", Color::light_sea_green);
            }

            timeString << color("Duration: ", Color::light_sea_green) << color(getDuration(), Color::light_sea_green);


            /**
             * Failure!
             * Success, with warnings!
             * Success, with errors!
             * Success, with warnings & errors!
             * Success!
             * Success, with warnings!
             * Success, with errors!
             * 
             *                 |\__/,|   (`\
             *               _.|o o  |_   ) )
             * -------------(((---(((---------------------------------
             * 
             * 0 action(s), 0 warning(s), 0 error(s), 0 fatal error(s)
            **/
            // Print summary
            std::cout << '\n' << '\n' << color(status, Color::light_sea_green) << std::endl;
            if (reports > 0) {
                std::cout << '\n';
                std::cout << color("                |\\__/,|   (`\\", Color::golden_rod) << '\n';
                std::cout << color("              _.|o o  |_   ) )", Color::golden_rod) << '\n';
                std::cout << color("-------------", Color::light_sea_green) << color("(((", Color::golden_rod)
                    << color("---", Color::light_sea_green) << color("(((", Color::golden_rod)
                    << color("---------------------------------", Color::light_sea_green) << '\n' << '\n';
                std::cout << actionsString.str() << warningsString.str() << criticalsString.str() << fatalsString.str()
                    << std::endl;
            }
            std::cout << '\n' << timeString.str() << std::endl;

            // Revert
            Optimization::reverse();
        }
    }
}
