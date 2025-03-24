#ifndef UIDIALOG_H
#define UIDIALOG_H

#include <string>

namespace UI::Dialog {
    bool Confirm(const std::string& title, const std::string& message);
    bool Error(const std::string& title, const std::string& message);
}

#endif