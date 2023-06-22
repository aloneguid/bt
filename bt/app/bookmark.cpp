#include "bookmark.h"

void bt::app::bookmark::read_firefox_bookmarks() {
    // Firefox stores bookmarks in profile folder, under places.sqlite, moz_bookmarks table
    //sample path: C:\Users\alone\AppData\Roaming\Mozilla\Firefox\Profiles\23ik9hpw.Videos\places.sqlite
    
}

void bt::app::bookmark::read_chrome_bookmarks() {
    // Chromium stores bookmarks in "Bookmarks" file, which is a JSON file under profile folder
}
