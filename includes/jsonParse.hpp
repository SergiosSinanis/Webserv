#ifndef JSON_PARSE_HPP
#define JSON_PARSE_HPP

#include <string>
#include <map>

typedef std::map<std::string, std::string> JsonObject;

typedef std::map<int, JsonObject> PatchMap;

struct JsonParser
{
    std::string json;
    size_t pos;

    void skipWhitespace();
    char peek();
    char consume();
    void expect(char expected);
    std::string parseString();
    std::string parseValue();
    JsonObject parseObject();
    JsonObject parse(const std::string& jsonStr);
};

bool add_json_to_patch(PatchMap& patchMap, int index, const std::string& jsonString);
void print_patch_map(const PatchMap& patch);
std::string get_patch_value(const PatchMap& patchMap, int index, const std::string& key);

#endif