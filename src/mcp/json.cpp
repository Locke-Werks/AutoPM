#include "json.h"

#include <cmath>
#include <cstdio>
#include <sstream>

namespace json {
namespace {

const std::string kEmptyString;
const Array kEmptyArray;
const Object kEmptyObject;
const Value kNull;

void encodeUtf8(unsigned int codepoint, std::string& out) {
    if (codepoint < 0x80) {
        out += static_cast<char>(codepoint);
    } else if (codepoint < 0x800) {
        out += static_cast<char>(0xC0 | (codepoint >> 6));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint < 0x10000) {
        out += static_cast<char>(0xE0 | (codepoint >> 12));
        out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (codepoint >> 18));
        out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
}

struct Parser {
    const std::string& text;
    size_t pos = 0;
    std::string error;

    explicit Parser(const std::string& t) : text(t) {}

    void skip() {
        while (pos < text.size()) {
            const char c = text[pos];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++pos;
            else break;
        }
    }

    bool fail(const std::string& what) {
        if (error.empty()) error = what + " at byte " + std::to_string(pos);
        return false;
    }

    bool parseValue(Value& out) {
        skip();
        if (pos >= text.size()) return fail("unexpected end of input");
        switch (text[pos]) {
            case '{': return parseObject(out);
            case '[': return parseArray(out);
            case '"': {
                std::string s;
                if (!parseString(s)) return false;
                out = Value(std::move(s));
                return true;
            }
            case 't':
                if (text.compare(pos, 4, "true") != 0) return fail("bad literal");
                pos += 4;
                out = Value(true);
                return true;
            case 'f':
                if (text.compare(pos, 5, "false") != 0) return fail("bad literal");
                pos += 5;
                out = Value(false);
                return true;
            case 'n':
                if (text.compare(pos, 4, "null") != 0) return fail("bad literal");
                pos += 4;
                out = Value();
                return true;
            default: return parseNumber(out);
        }
    }

    bool parseObject(Value& out) {
        ++pos;   // {
        Object object;
        skip();
        if (pos < text.size() && text[pos] == '}') { ++pos; out = Value(std::move(object)); return true; }
        while (true) {
            skip();
            std::string key;
            if (!parseString(key)) return false;
            skip();
            if (pos >= text.size() || text[pos] != ':') return fail("expected :");
            ++pos;
            Value value;
            if (!parseValue(value)) return false;
            object.emplace(std::move(key), std::move(value));
            skip();
            if (pos < text.size() && text[pos] == ',') { ++pos; continue; }
            if (pos < text.size() && text[pos] == '}') { ++pos; break; }
            return fail("expected , or }");
        }
        out = Value(std::move(object));
        return true;
    }

    bool parseArray(Value& out) {
        ++pos;   // [
        Array array;
        skip();
        if (pos < text.size() && text[pos] == ']') { ++pos; out = Value(std::move(array)); return true; }
        while (true) {
            Value value;
            if (!parseValue(value)) return false;
            array.push_back(std::move(value));
            skip();
            if (pos < text.size() && text[pos] == ',') { ++pos; continue; }
            if (pos < text.size() && text[pos] == ']') { ++pos; break; }
            return fail("expected , or ]");
        }
        out = Value(std::move(array));
        return true;
    }

    bool parseString(std::string& out) {
        skip();
        if (pos >= text.size() || text[pos] != '"') return fail("expected a string");
        ++pos;
        out.clear();
        while (pos < text.size()) {
            const char c = text[pos++];
            if (c == '"') return true;
            if (c != '\\') { out += c; continue; }
            if (pos >= text.size()) return fail("unterminated escape");
            const char esc = text[pos++];
            switch (esc) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case '/':  out += '/';  break;
                case 'b':  out += '\b'; break;
                case 'f':  out += '\f'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                case 'u': {
                    if (pos + 4 > text.size()) return fail("short \\u escape");
                    unsigned int cp = std::stoul(text.substr(pos, 4), nullptr, 16);
                    pos += 4;
                    // A surrogate pair is two escapes and one character.
                    if (cp >= 0xD800 && cp <= 0xDBFF && pos + 6 <= text.size() &&
                        text[pos] == '\\' && text[pos + 1] == 'u') {
                        const unsigned int low = std::stoul(text.substr(pos + 2, 4), nullptr, 16);
                        if (low >= 0xDC00 && low <= 0xDFFF) {
                            cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                            pos += 6;
                        }
                    }
                    encodeUtf8(cp, out);
                    break;
                }
                default: return fail("unknown escape");
            }
        }
        return fail("unterminated string");
    }

    bool parseNumber(Value& out) {
        const size_t start = pos;
        if (pos < text.size() && (text[pos] == '-' || text[pos] == '+')) ++pos;
        while (pos < text.size() &&
               (std::isdigit(static_cast<unsigned char>(text[pos])) || text[pos] == '.' ||
                text[pos] == 'e' || text[pos] == 'E' || text[pos] == '-' || text[pos] == '+'))
            ++pos;
        if (pos == start) return fail("expected a value");
        try {
            out = Value(std::stod(text.substr(start, pos - start)));
        } catch (...) {
            return fail("bad number");
        }
        return true;
    }
};

void dumpString(const std::string& s, std::ostringstream& out) {
    out << '"';
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b";  break;
            case '\f': out << "\\f";  break;
            case '\n': out << "\\n";  break;
            case '\r': out << "\\r";  break;
            case '\t': out << "\\t";  break;
            default:
                if (c < 0x20) {
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                    out << buffer;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    out << '"';
}

void dumpValue(const Value& value, std::ostringstream& out) {
    switch (value.kind()) {
        case Value::Kind::Null: out << "null"; return;
        case Value::Kind::Bool: out << (value.asBool() ? "true" : "false"); return;
        case Value::Kind::Number: {
            const double n = value.asNumber();
            if (n == static_cast<long long>(n)) out << static_cast<long long>(n);
            else out << n;
            return;
        }
        case Value::Kind::String: dumpString(value.asString(), out); return;
        case Value::Kind::Array: {
            out << '[';
            bool first = true;
            for (const Value& item : value.asArray()) {
                if (!first) out << ',';
                first = false;
                dumpValue(item, out);
            }
            out << ']';
            return;
        }
        case Value::Kind::Object: {
            out << '{';
            bool first = true;
            for (const auto& pair : value.asObject()) {
                if (!first) out << ',';
                first = false;
                dumpString(pair.first, out);
                out << ':';
                dumpValue(pair.second, out);
            }
            out << '}';
            return;
        }
    }
}

} // namespace

bool Value::asBool(bool fallback) const { return kind_ == Kind::Bool ? bool_ : fallback; }
double Value::asNumber(double fallback) const { return kind_ == Kind::Number ? number_ : fallback; }
int Value::asInt(int fallback) const {
    return kind_ == Kind::Number ? static_cast<int>(number_) : fallback;
}
const std::string& Value::asString() const { return kind_ == Kind::String ? string_ : kEmptyString; }
const Array& Value::asArray() const { return kind_ == Kind::Array ? array_ : kEmptyArray; }
const Object& Value::asObject() const { return kind_ == Kind::Object ? object_ : kEmptyObject; }

const Value& Value::operator[](const std::string& key) const {
    if (kind_ != Kind::Object) return kNull;
    auto it = object_.find(key);
    return it == object_.end() ? kNull : it->second;
}

std::string Value::str(const std::string& key, const std::string& fallback) const {
    const Value& found = (*this)[key];
    return found.isString() ? found.asString() : fallback;
}

bool Value::has(const std::string& key) const {
    return kind_ == Kind::Object && object_.find(key) != object_.end();
}

std::string Value::dump() const {
    std::ostringstream out;
    dumpValue(*this, out);
    return out.str();
}

bool parse(const std::string& text, Value& out, std::string& error) {
    Parser parser(text);
    if (!parser.parseValue(out)) {
        error = parser.error;
        return false;
    }
    return true;
}

} // namespace json
