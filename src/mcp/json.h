// Just enough JSON for a JSON-RPC server.
//
// Written rather than pulled in because the MCP front end has no other
// dependency, and an executable that gets spawned once per session should not
// need a toolkit beside it on disk.
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace json {

class Value;
using Object = std::map<std::string, Value>;
using Array = std::vector<Value>;

class Value {
public:
    enum class Kind { Null, Bool, Number, String, Array, Object };

    Value() = default;
    Value(bool b) : kind_(Kind::Bool), bool_(b) {}
    Value(double n) : kind_(Kind::Number), number_(n) {}
    Value(int n) : kind_(Kind::Number), number_(n) {}
    Value(const char* s) : kind_(Kind::String), string_(s) {}
    Value(std::string s) : kind_(Kind::String), string_(std::move(s)) {}
    Value(Array a) : kind_(Kind::Array), array_(std::move(a)) {}
    Value(Object o) : kind_(Kind::Object), object_(std::move(o)) {}

    Kind kind() const { return kind_; }
    bool isNull() const { return kind_ == Kind::Null; }
    bool isObject() const { return kind_ == Kind::Object; }
    bool isArray() const { return kind_ == Kind::Array; }
    bool isString() const { return kind_ == Kind::String; }

    bool asBool(bool fallback = false) const;
    double asNumber(double fallback = 0) const;
    int asInt(int fallback = 0) const;
    const std::string& asString() const;
    const Array& asArray() const;
    const Object& asObject() const;

    // Object lookup that never throws: a missing key is a null value.
    const Value& operator[](const std::string& key) const;
    std::string str(const std::string& key, const std::string& fallback = std::string()) const;
    bool has(const std::string& key) const;

    std::string dump() const;

private:
    Kind kind_ = Kind::Null;
    bool bool_ = false;
    double number_ = 0;
    std::string string_;
    Array array_;
    Object object_;
};

// Returns false on malformed input; `error` says where.
bool parse(const std::string& text, Value& out, std::string& error);

} // namespace json
