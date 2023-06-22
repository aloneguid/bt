#include <string>

namespace bt::app {
    /**
     * @brief Describes a browser independent bookmark.
    */
    class bookmark {
    public:
        void read_firefox_bookmarks();
        void read_chrome_bookmarks();

    private:
    };
}