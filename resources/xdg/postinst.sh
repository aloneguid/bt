#!/bin/sh
set -e

# To find default web browser: xdg-settings get default-web-browser

if [ "$1" = "configure" ]; then
    update-alternatives --install /usr/bin/x-www-browser x-www-browser /usr/bin/bt 100
    update-alternatives --install /usr/bin/gnome-www-browser gnome-www-browser /usr/bin/bt 100


    # Add the installing user to "input" group so hotkey detection works.
    # $SUDO_USER is only set when installed via `sudo dpkg`/`sudo apt`; it's
    # empty for gdebi, software-center, or root-console installs, so fall
    # back to logname (the user of the controlling terminal) if unset.
    TARGET_USER="${SUDO_USER:-$(logname 2>/dev/null || true)}"

    if [ -n "$TARGET_USER" ] && [ "$TARGET_USER" != "root" ]; then
        if getent group input >/dev/null 2>&1; then
            if id -nG "$TARGET_USER" 2>/dev/null | grep -qw input; then
                : # already a member, nothing to do
            else
                usermod -aG input "$TARGET_USER" || \
                    echo "could not add $TARGET_USER to 'input' group; hotkey detection may need manual setup." >&2
                echo "added $TARGET_USER to 'input' group. Log out and back in for hotkey detection to work."
            fi
        else
            echo "'input' group not found; skipping group setup. Hotkey detection may not work without it." >&2
        fi
    else
        echo "could not determine the installing user; run 'sudo usermod -aG input \$USER' manually for hotkey detection." >&2
    fi
fi

if [ -x /usr/bin/update-desktop-database ]; then
    update-desktop-database -q
fi


