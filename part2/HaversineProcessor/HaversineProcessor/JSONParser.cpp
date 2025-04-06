#include "JSONParser.h"

#include <assert.h>
#include <stack>

JSONValue::JSONValue(JSONValueType type)
    : type(type) {
    switch (type) {
    case JSONValueType::None:
        break;
    case JSONValueType::Number:
        break;
    case JSONValueType::String:
        str_ = new std::string();
        break;
    case JSONValueType::Object:
        obj_ = new std::unordered_map<std::string, JSONValue*>();
        break;
    case JSONValueType::Array:
        array_ = new std::vector<JSONValue*>();
        break;
    default:;
    }
}
JSONValue::JSONValue(const std::string& str) {
    type = JSONValueType::String;
    str_ = new std::string(str);
}
JSONValue::JSONValue(const double number) {
    type = JSONValueType::Number;
    number_ = number;
}

void ProcessCharacterAsPartOfToken(const JSONToken& token, const char character) {
    if (token.parent_value) {
        if (token.parent_value->type == JSONValueType::String) {
            *token.parent_value->str_ += character;
        } else if (token.parent_value->type == JSONValueType::Number) {
        }
    }
}

void ProcessSeparator(std::stack<JSONToken>& stack, const JSONToken& token) {
    switch (token.parent_value->type) {
    case JSONValueType::None:
        break;
    case JSONValueType::Number:
        break;
    case JSONValueType::String:
        break;
    case JSONValueType::Object:
        break;
    case JSONValueType::Array:
        break;
    default:;
    }
}

bool IsStartOfNumber(const char ch) {
    return isdigit(ch) || ch == '-';
}
void ParseNumber(std::stack<JSONToken>& stack, const char*& current_ptr) {
    double number = 0.0;
    bool is_negative = *current_ptr == '-';
    if (is_negative) {
        current_ptr++;
    }
    double order = 1;
    while (isdigit(*current_ptr)) {
        const auto digit = static_cast<double>(*current_ptr++ - '0');
        number = number * 10.0 + digit;
    }
    if (*current_ptr == '.') {
        current_ptr++;
        double fraction = 0.0;
        order = 0.1;
        while (isdigit(*current_ptr)) {
            const auto digit = static_cast<double>(*current_ptr++ - '0');
            number += order * digit;
            order /= 10.0;
        }
    }
    if (is_negative) {
        number *= -1.0;
    }
    stack.push({new JSONValue(number), JSONTokenType::None});
}

void ParseSingleChar(std::stack<JSONToken>& stack, const char*& current_ptr) {
    const JSONToken current_token = stack.top();
    switch (*current_ptr) {
    case '{':
        stack.push({new JSONValue(JSONValueType::Object), JSONTokenType::None});
        break;
    case '[':
        stack.push({new JSONValue(JSONValueType::Array), JSONTokenType::None});
        break;
    case ',':
        ProcessSeparator(stack, current_token);
        break;
    case '"':
        if (current_token.parent_value->type == JSONValueType::String) {
            auto token_type = current_token.type;
            stack.pop();
            switch (token_type) {
            case JSONTokenType::None:
                // TODO: array
                break;
            case JSONTokenType::ObjectKey:
                assert(stack.top().parent_value->type == JSONValueType::Object);
                stack.top().parent_value->obj_->insert({*current_token.parent_value->str_, new JSONValue(JSONValueType::None)});
                break;
            case JSONTokenType::ObjectValue: {
                JSONToken obj_key_token = stack.top();
                assert(obj_key_token.type == JSONTokenType::ObjectKey);
                stack.pop();
                JSONToken obj_token = stack.top();
                assert(obj_token.parent_value->type == JSONValueType::Object);
                obj_token.parent_value->obj_->at(*obj_key_token.parent_value->str_) = new JSONValue(*current_token.parent_value->str_);
            } break;
            default:;
            }
        } else if (current_token.parent_value->type == JSONValueType::Object) {
            if (current_token.type == JSONTokenType::None) {
                stack.push({new JSONValue(JSONValueType::String), JSONTokenType::ObjectKey});
            } else if (current_token.type == JSONTokenType::Colon) {
                stack.pop();
                stack.push({new JSONValue(JSONValueType::String), JSONTokenType::ObjectValue});
            }
        }
        break;
    case ':': {
        if (current_token.parent_value && current_token.parent_value->type == JSONValueType::Object) {
            stack.push({current_token.parent_value, JSONTokenType::Colon});
        } else {
            ProcessCharacterAsPartOfToken(current_token, ':');
        }
    } break;
    default: {
        ProcessCharacterAsPartOfToken(current_token, *current_ptr);
    }
    }
    current_ptr++;
}

JSONValue::~JSONValue() {
}
void JSONParser::Parse(const char* input_str) {
    std::stack<JSONToken> token_stack;
    token_stack.push({nullptr, JSONTokenType::None});
    const char* current_ptr = input_str;
    while (*current_ptr != '\0') {
        const JSONToken current_token = token_stack.top();
        if (current_token.type == JSONTokenType::Colon && IsStartOfNumber(*current_ptr)) {
            ParseNumber(token_stack, current_ptr);
        } else {
            ParseSingleChar(token_stack, current_ptr);
        }
    }
}