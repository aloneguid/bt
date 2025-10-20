#pragma once
#include <string>
#include <vector>
#include <functional>

namespace bt
{
    class system_check {
    public:
        system_check(const std::string& id, const std::string& name,
            const std::string& description,
            std::function<bool(std::string& error)> perform_check,
            std::function<bool()> mitigate)
            : id{id}, name{name}, description{description},
            perform_check{perform_check},
            mitigate{mitigate} {

        }

        const std::string id;
        const std::string name;
        const std::string description;

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

        /**
         * @brief Registers as browser, and registers browser protocol
        */
        static void register_as_browser_and_custom_protocol();

        static void unregister_all();

        static std::string get_custom_proto_reg_path();

        static std::string get_browser_registration_reg_path();

        static void register_protocol();

        static void register_browser();

    private:

        static bool is_installed_as_browser(const std::string& name, std::string& error_message);

        static void uninstall_as_browser(
            const std::string& proto_name,
            const std::string& name);
    };
}