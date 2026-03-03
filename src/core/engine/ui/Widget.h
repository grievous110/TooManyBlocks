#ifndef TOOMANYBLOCKS_WIDGET_H
#define TOOMANYBLOCKS_WIDGET_H

#include <string>

namespace UI {
    class Widget {
    private:
        std::string m_errorString;

    public:
        virtual ~Widget() = default;

        virtual void render() = 0;

        inline void setError(const std::string& error) { m_errorString = error; }

        inline const char* getError() const { return m_errorString.c_str(); }

        inline bool hasError() const { return !m_errorString.empty(); }

        inline void clearError() { m_errorString.clear(); }
    };
}  // namespace UI

#endif
