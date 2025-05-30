#ifndef TOOMANYBLOCKS_UIDIALOG_H
#define TOOMANYBLOCKS_UIDIALOG_H

#include <string>

namespace UI::Dialog {
    bool Notification(const std::string& title, const std::string& message);
    bool Confirm(const std::string& title, const std::string& message);
    bool Error(const std::string& title, const std::string& message);
}  // namespace UI::Dialog

#endif