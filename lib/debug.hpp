#pragma once

#include "io.hpp"

#define __DEBUGGER_OUTPUT cerr

namespace Debugger {
    class TableManager {
        struct Table {
            std::vector<std::string> columns;
            std::vector<std::vector<std::string>> rows;
        };
        std::map<int, Table> tables;
        static TableManager& getInstance() {
            static TableManager instance;
            return instance;
        }

    public:
        static void addRow(int line, const std::vector<std::string>& header,
                           const std::vector<std::string>& row) {
            auto& table = getInstance().tables[line];
            if (table.columns.empty()) {
                table.columns = header;
            }
            table.rows.push_back(row);
        }
        static void flush() {
            auto& instance = getInstance();
            for (const auto& [line, table] : instance.tables) {
                vector<size_t> colWidths(table.columns.size(), 0);
                size_t width = 0;
                for (size_t i = 0; i < table.columns.size(); ++i)
                    colWidths[i] = table.columns[i].size();
                for (const auto& row : table.rows)
                    for (size_t i = 0; i < row.size(); ++i)
                        colWidths[i] = std::max(colWidths[i], row[i].size());
                for (size_t i = 0; i < colWidths.size(); ++i)
                    width += colWidths[i] + 2;
                __DEBUGGER_OUTPUT << std::left << "[DEBUG TABLE] L" << line << "\n";
                for (size_t i = 0; i < table.columns.size(); ++i) {
                    __DEBUGGER_OUTPUT << '+' << std::string(colWidths[i] + 2, '-')
                                      << (i + 1 < table.columns.size() ? "" : "+\n");
                }
                for (size_t i = 0; i < table.columns.size(); ++i) {
                    __DEBUGGER_OUTPUT << "| " << std::setw(colWidths[i]) << table.columns[i]
                                      << (i + 1 < table.columns.size() ? " " : " |\n");
                }
                for (size_t i = 0; i < table.columns.size(); ++i) {
                    __DEBUGGER_OUTPUT << '+' << std::string(colWidths[i] + 2, '-')
                                      << (i + 1 < table.columns.size() ? "" : "+\n");
                }
                for (const auto& row : table.rows) {
                    __DEBUGGER_OUTPUT << "| ";
                    for (size_t i = 0; i < row.size(); ++i)
                        __DEBUGGER_OUTPUT << std::setw(colWidths[i]) << row[i]
                                          << (i + 1 < row.size() ? " | " : " |\n");
                }
                for (size_t i = 0; i < table.columns.size(); ++i) {
                    __DEBUGGER_OUTPUT << '+' << std::string(colWidths[i] + 2, '-')
                                      << (i + 1 < table.columns.size() ? "" : "+\n");
                }
            }
            __DEBUGGER_OUTPUT.flush();
            instance.tables.clear();
        }
        ~TableManager() {
            flush();
        }
    };

    template <class T> std::string toStr(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
    inline std::string strip(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }
    inline std::vector<std::string> split(const std::string& str) {
        std::vector<std::string> res;
        size_t depth = 0, start = 0;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == ',' && depth == 0) {
                res.push_back(strip(str.substr(start, i - start)));
                start = i + 1;
            } else if (str[i] == '(' || str[i] == '[' || str[i] == '{' || str[i] == '<')
                depth++;
            else if (str[i] == ')' || str[i] == ']' || str[i] == '}' || str[i] == '>')
                depth--;
        }
        res.push_back(strip(str.substr(start)));
        return res;
    }
    template <class... Args> void tdebug_helper(int line, const char* names, const Args&... args) {
        std::vector<std::string> nameList = split(names);
        std::vector<std::string> valueList = {toStr(args)...};
        TableManager::addRow(line, nameList, valueList);
    }
    template <class... Args> void debug_impl(int line, const char* names, const Args&... args) {
        __DEBUGGER_OUTPUT << "[DEBUG] L" << line << ": ";
        auto nameList = split(names);
        size_t i = 0;
        ((__DEBUGGER_OUTPUT << (i++ ? ", " : "") << nameList[i - 1] << " = " << args), ...);
        __DEBUGGER_OUTPUT << endl;
    }
} // namespace Debugger

#define debug(...) Debugger::debug_impl(__LINE__, #__VA_ARGS__, __VA_ARGS__)
#define tdebug(...) Debugger::tdebug_helper(__LINE__, #__VA_ARGS__, __VA_ARGS__)
#define tflush() Debugger::TableManager::flush();
