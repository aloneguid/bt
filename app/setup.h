#pragma once
#include <string>
#include <vector>
#include <functional>
#include "platform.h"

namespace bt {
    class system_check {
    public:
        system_check(const std::string& id, const std::string& name,
            const std::string& description,
            const std::string& fix_description,
            std::function<bool(std::string& error)> perform_check,
            std::function<bool()> mitigate)
            : id{id}, name{name}, description{description},
            fix_description{fix_description},
            perform_check{perform_check},
            mitigate{mitigate} {
        }

        const std::string id;
        const std::string name;
        const std::string description;
        const std::string fix_description;  // The rest of the message "Press this button to..."

        /**
         * @brief Whether the check is currently passing
         */
        bool is_ok{false};

        /**
         * @brief Error message if the check is not passing (if available)
         */
        std::string error_message;

        void recheck() { is_ok = perform_check(error_message); }
        void fix() { mitigate(); }

    private:
        std::function<bool(std::string& error_message)> perform_check;
        std::function<bool()> mitigate;
    };

    class setup {
    public:

        static std::vector<system_check> get_checks();

    private:
#if PLATFORM_WINDOWS
        static std::string get_shell_url_association_prog_id(const std::string& protocol_name);
		static std::string get_prog_id_application_name(const std::string& prog_id);
#endif
    };
}