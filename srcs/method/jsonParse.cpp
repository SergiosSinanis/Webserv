#include <map>
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <stdexcept>

typedef std::map<std::string, std::string> JsonObject;
typedef std::map<int, JsonObject> PatchMap;

struct JsonParser
{
	std::string json;
	size_t pos;

	void skipWhitespace()
	{
		while (pos < json.length() && std::isspace(json[pos]))
		{
			pos++;
		}
	}

	char peek() //verifier le prochain char sans bouger
	{
		skipWhitespace();
		return pos < json.length() ? json[pos] : '\0';
	}

	char consume() //i++
	{
		skipWhitespace();
		return pos < json.length() ? json[pos++] : '\0';
	}

	void expect(char expected) //verifie le prochain char est un char specifique
	{
		char c = consume();
		if (c != expected)
		{
			throw std::runtime_error
			("Expected '" + std::string(1, expected) + "' but got '" + std::string(1, c) + "'");
		}
	}

	std::string parseString()
	{
		expect('"');
		std::string result;

		while (pos < json.length() && json[pos] != '"')
		{
			if (json[pos] == '\\') {
				pos++; // skip escape character
				if (pos >= json.length())
				{
					throw std::runtime_error("Unterminated string escape");
				}

				switch (json[pos])
				{
					case '"': result += '"';
						break;
					case '\\': result += '\\';
						break;
					case '/': result += '/';
						break;
					case 'b': result += '\b';
						break;
					case 'f': result += '\f';
						break;
					case 'n': result += '\n';
						break;
					case 'r': result += '\r';
						break;
					case 't': result += '\t';
						break;
					default:
						throw std::runtime_error("Invalid escape sequence");
				}
			}
			else
			{
				result += json[pos];
			}
			pos++;
		}

		if (pos >= json.length())
		{
			throw std::runtime_error("Unterminated string");
		}

		expect('"');
		return result;
	}

	std::string parseValue()
	{
		char c = peek();

		if (c == '"')
		{
			return parseString();
		}
		else if (c == '{' || c == '[')
		{
			// nested objects/arrays, store as substring
			size_t start = pos;
			int depth = 0;

			while (pos < json.length())
			{
				if (json[pos] == '{' || json[pos] == '[')
				{
					depth++;
				}
				else if (json[pos] == '}' || json[pos] == ']')
				{
					depth--;
					if (depth == 0)
					{
						pos++;
						break;
					}
				}
				else if (json[pos] == '"')
				{
					pos++;
					while (pos < json.length() && json[pos] != '"')
					{
						if (json[pos] == '\\') pos++;
						pos++;
					}
					if (pos < json.length()) pos++;
				}
				pos++;
			}

			return json.substr(start, pos - start);
		}
		else if (c == 't' || c == 'f' || c == 'n' || std::isdigit(c) || c == '-')
		{
			// parse les literals (true, false, null, numbers)
			size_t start = pos;

			if (c == 't' && json.substr(pos, 4) == "true")
			{
				pos += 4;
				return "true";
			}
			else if (c == 'f' && json.substr(pos, 5) == "false")
			{
				pos += 5;
				return "false";
			}
			else if (c == 'n' && json.substr(pos, 4) == "null")
			{
				pos += 4;
				return "null";
			}
			else
			{
				// parse number
				if (c == '-') pos++;
				while (pos < json.length() &&
					(std::isdigit(json[pos]) || json[pos] == '.' || json[pos] == 'e' || json[pos] == 'E' ||
						json[pos] == '+' || json[pos] == '-'))
				{
					pos++;
				}
				return json.substr(start, pos - start);
			}
		}

		throw std::runtime_error("Invalid JSON value");
	}

	JsonObject parseObject()
	{
		JsonObject obj;
		expect('{');

		if (peek() == '}')
		{
			consume();
			return obj;
		}

		while (true)
		{
			std::string key = parseString();
			expect(':');
			std::string value = parseValue();

			obj[key] = value;

			char next = peek();
			if (next == '}')
			{
				consume();
				break;
			}
			else if (next == ',')
			{
				consume();
			}
			else
			{
				throw std::runtime_error("Expected ',' or '}' in object");
			}
		}

		return obj;
	}

	JsonObject parse(const std::string& jsonStr)
	{
		json = jsonStr;
		pos = 0;

		try
		{
			JsonObject result = parseObject();
			skipWhitespace();
			if (pos < json.length())
			{
				throw std::runtime_error("Extra characters after JSON");
			}
			return result;
		}
		catch (const std::exception& e)
		{
			std::stringstream ss;
			ss << "JSON Parse Error at position " << pos << ": " << e.what();
			throw std::runtime_error(ss.str());
		}
	}
};

// parse JSON string and add to patch map with given index
bool add_json_to_patch(PatchMap& patchMap, int index, const std::string& jsonString)
{
    JsonParser parser;

    try
	{
        JsonObject obj = parser.parse(jsonString);
        patchMap[index] = obj;
        std::cout << "Successfully parsed JSON for index " << index << std::endl;
        return true;
    }
	catch (const std::exception& e)
	{
        std::cerr << "JSON validation failed for index " << index << ": " << e.what() << std::endl;
        return false;
    }
}

// testing
void print_patch_map(const PatchMap& patch)
{
    std::cout << "\nPatch Map Contents:\n";
    for (PatchMap::const_iterator it = patch.begin(); it != patch.end(); ++it)
	{
        std::cout << "Index " << it->first << ":\n";
        const JsonObject& obj = it->second;
        for (JsonObject::const_iterator objIt = obj.begin(); objIt != obj.end(); ++objIt)
		{
            std::cout << "  \"" << objIt->first << "\": \"" << objIt->second << "\"\n";
        }
    }
}

std::string get_patch_value(const PatchMap& patchMap, int index, const std::string& key)
{
    PatchMap::const_iterator patchIt = patchMap.find(index);
    if (patchIt != patchMap.end())
	{
        JsonObject::const_iterator keyIt = patchIt->second.find(key);
        if (keyIt != patchIt->second.end())
		{
            return keyIt->second;
        }
    }
    return "";
}
