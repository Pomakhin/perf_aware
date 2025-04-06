#pragma once
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

enum class JSONValueType { None, Number, String, Object, Array };

struct JSONValue {
    JSONValue(JSONValueType type);
    JSONValue(const std::string& str);
    JSONValue(const double number);
    ~JSONValue();
    JSONValueType type = JSONValueType::None;
    union {
        double number_;
        std::string* str_;
        std::unordered_map<std::string, JSONValue*>* obj_;
        std::vector<JSONValue*>* array_;
    };
};

enum class JSONTokenType { None, ObjectKey, ObjectValue, Colon };
struct JSONToken {
    JSONValue* parent_value;
    JSONTokenType type = JSONTokenType::None;
};

class JSONParser {
public:
    void Parse(const char* input_str);
};
