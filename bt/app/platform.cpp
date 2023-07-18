#include "platform.h"
#include "../ext/WinToast/include/wintoastlib.h"
#include "str.h"

namespace bt {

    // https://github.com/mohabouje/WinToast/blob/master/examples/console-example/main.cpp

    using namespace std;
    using namespace WinToastLib;

    class CustomHandler : public IWinToastHandler {
    public:
        void toastActivated() const {
            std::wcout << L"The user clicked in this toast" << std::endl;
            exit(0);
        }

        void toastActivated(int actionIndex) const {
            std::wcout << L"The user clicked on action #" << actionIndex << std::endl;
            exit(16 + actionIndex);
        }

        void toastDismissed(WinToastDismissalReason state) const {
            switch(state) {
                case UserCanceled:
                    std::wcout << L"The user dismissed this toast" << std::endl;
                    exit(1);
                    break;
                case TimedOut:
                    std::wcout << L"The toast has timed out" << std::endl;
                    exit(2);
                    break;
                case ApplicationHidden:
                    std::wcout << L"The application hid the toast using ToastNotifier.hide()" << std::endl;
                    exit(3);
                    break;
                default:
                    std::wcout << L"Toast not activated" << std::endl;
                    exit(4);
                    break;
            }
        }

        void toastFailed() const {
            std::wcout << L"Error showing current toast" << std::endl;
            exit(5);
        }
    };


    bool wintoast_initialised{false};

    void init_wintoast() {
        if(!WinToast::isCompatible()) {
            return;
        }

        WinToast::instance()->setAppName(str::to_wstr(APP_LONG_NAME));
        WinToast::instance()->setAppUserModelId(str::to_wstr(APP_LONG_NAME));

        if(!WinToast::instance()->initialize()) {
            return;
        }

        wintoast_initialised = true;
    }

    void platform::test() {
        if(!wintoast_initialised) {
            init_wintoast();
        }

        std::wstring imagePath = L"";
        WinToastTemplate::AudioOption audioOption = WinToastTemplate::AudioOption::Default;
        //std::wstring attribute = L"default";

        WinToastTemplate templ(!imagePath.empty() ? WinToastTemplate::ImageAndText02 : WinToastTemplate::Text02);
        templ.setTextField(L"Rule Hit!", WinToastTemplate::FirstLine);
        templ.setTextField(L"http://dsfdslfjdlj.com/dfadfadfadadf", WinToastTemplate::SecondLine);
        templ.setTextField(L"teams.microsoft.com", WinToastTemplate::ThirdLine);
        templ.setAudioOption(audioOption);
        //templ.setAttributionText(attribute);
        templ.setImagePath(imagePath);

        // optional
        // actions may conflict with ImGui
        templ.addAction(L"Copy URL");

        if(WinToast::instance()->showToast(templ, new CustomHandler()) < 0) {
            std::wcerr << L"Could not launch your toast notification!";
        }
    }
}