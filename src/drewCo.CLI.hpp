// ******************************************************************************************
// Copyright 2026 Andrew A. Ritz
// ******************************************************************************************

#ifndef DREWCO_CLI_HPP
#define DREWCO_CLI_HPP

// drewCo.CLI - a small, header-only C++17 command line parser.
// The only non-standard dependency is toml++: https://github.com/marzer/tomlplusplus

#include <toml.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace drewCo::CLI {

    inline std::string Trim(std::string_view value)
    {
        std::size_t first = 0;
        while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) { ++first; }
        std::size_t last = value.size();
        while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) { --last; }
        return std::string(value.substr(first, last - first));
    }

    inline std::string Lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }

    inline bool IEquals(std::string_view left, std::string_view right)
    {
        if (left.size() != right.size()) { return false; }
        for (std::size_t i = 0; i < left.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(left[i])) !=
                std::tolower(static_cast<unsigned char>(right[i]))) {
                return false;
            }
        }
        return true;
    }

    inline bool EndsWithI(std::string_view value, std::string_view suffix)
    {
        return value.size() >= suffix.size() &&
            IEquals(value.substr(value.size() - suffix.size()), suffix);
    }

    inline std::vector<std::string> Split(std::string_view input, char delimiter)
    {
        std::vector<std::string> result;
        std::size_t begin = 0;
        for (;;) {
            const std::size_t end = input.find(delimiter, begin);
            result.push_back(Trim(input.substr(begin, end == std::string_view::npos ? end : end - begin)));
            if (end == std::string_view::npos) { return result; }
            begin = end + 1;
        }
    }

    inline std::string Join(const std::vector<std::string>& values, std::string_view separator)
    {
        std::string result;
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i != 0) { result += separator; }
            result += values[i];
        }
        return result;
    }

    inline std::string NormalizeWhitespace(std::string_view text)
    {
        std::string result;
        bool whitespace = true;
        for (unsigned char c : text) {
            if (std::isspace(c)) {
                if (!whitespace) { result.push_back(' '); }
                whitespace = true;
            }
            else {
                result.push_back(static_cast<char>(c));
                whitespace = false;
            }
        }
        if (!result.empty() && result.back() == ' ') { result.pop_back(); }
        return result;
    }

    inline std::string NodeToString(const toml::node& node)
    {
        if (const auto value = node.value<std::string>()) { return *value; }
        if (const auto value = node.value<std::int64_t>()) { return std::to_string(*value); }
        if (const auto value = node.value<double>()) {
            std::ostringstream out;
            out << std::setprecision(std::numeric_limits<double>::max_digits10) << *value;
            return out.str();
        }
        if (const auto value = node.value<bool>()) { return *value ? "true" : "false"; }
        std::ostringstream out;
        out << toml::default_formatter{ node };
        return out.str();
    }

    enum class DataType
    {
        String,
        Boolean,
        Int8,
        UInt8,
        Int16,
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float32,
        Float64,
        Date,
        Time,
        DateTime,
        StringArray,
        BooleanArray,
        Int8Array,
        UInt8Array,
        Int16Array,
        UInt16Array,
        Int32Array,
        UInt32Array,
        Int64Array,
        UInt64Array,
        Float32Array,
        Float64Array,
        DateArray,
        TimeArray,
        DateTimeArray
    };

    inline bool IsArray(DataType type)
    {
        return type >= DataType::StringArray;
    }

    inline DataType ArrayType(DataType element)
    {
        switch (element) {
        case DataType::String: return DataType::StringArray;
        case DataType::Boolean: return DataType::BooleanArray;
        case DataType::Int8: return DataType::Int8Array;
        case DataType::UInt8: return DataType::UInt8Array;
        case DataType::Int16: return DataType::Int16Array;
        case DataType::UInt16: return DataType::UInt16Array;
        case DataType::Int32: return DataType::Int32Array;
        case DataType::UInt32: return DataType::UInt32Array;
        case DataType::Int64: return DataType::Int64Array;
        case DataType::UInt64: return DataType::UInt64Array;
        case DataType::Float32: return DataType::Float32Array;
        case DataType::Float64: return DataType::Float64Array;
        case DataType::Date: return DataType::DateArray;
        case DataType::Time: return DataType::TimeArray;
        case DataType::DateTime: return DataType::DateTimeArray;
        default: throw std::invalid_argument("Expected a scalar data type");
        }
    }

    inline DataType ElementType(DataType array)
    {
        switch (array) {
        case DataType::StringArray: return DataType::String;
        case DataType::BooleanArray: return DataType::Boolean;
        case DataType::Int8Array: return DataType::Int8;
        case DataType::UInt8Array: return DataType::UInt8;
        case DataType::Int16Array: return DataType::Int16;
        case DataType::UInt16Array: return DataType::UInt16;
        case DataType::Int32Array: return DataType::Int32;
        case DataType::UInt32Array: return DataType::UInt32;
        case DataType::Int64Array: return DataType::Int64;
        case DataType::UInt64Array: return DataType::UInt64;
        case DataType::Float32Array: return DataType::Float32;
        case DataType::Float64Array: return DataType::Float64;
        case DataType::DateArray: return DataType::Date;
        case DataType::TimeArray: return DataType::Time;
        case DataType::DateTimeArray: return DataType::DateTime;
        default: throw std::invalid_argument("Expected an array data type");
        }
    }

    struct CommandOptions
    {
        bool IsRequired = false;
        std::vector<std::string> Options;
        std::vector<std::string> Aliases;
        std::optional<std::string> TypeHint;
    };

    struct TypeHint
    {
        bool IsArray = false;
        bool IsSigned = false;
        bool IsFloat = false;
        int DataSize = 0;

        explicit TypeHint(std::string_view hint)
        {
            std::string value = Trim(hint);
            if (value.size() >= 2 && value.compare(value.size() - 2, 2, "[]") == 0) {
                IsArray = true;
                value.resize(value.size() - 2);
            }
            if (value.size() < 2 || (value[0] != 'i' && value[0] != 'u' && value[0] != 'f')) {
                throw std::invalid_argument("Invalid type hint: " + std::string(hint));
            }
            IsFloat = value[0] == 'f';
            IsSigned = IsFloat || value[0] == 'i';
            try {
                std::size_t used = 0;
                DataSize = std::stoi(value.substr(1), &used);
                if (used != value.size() - 1) { throw std::invalid_argument("bad suffix"); }
            }
            catch (...) {
                throw std::invalid_argument("Invalid type hint: " + std::string(hint));
            }
            if (!IsPowerOfTwo(DataSize) || DataSize < 8) {
                throw std::invalid_argument("Data size must be a power of two and at least 8 bits");
            }
            if (IsFloat && DataSize != 32 && DataSize != 64) {
                throw std::invalid_argument("Portable floating-point hints must be f32 or f64");
            }
            if (!IsFloat && DataSize > 64) {
                throw std::invalid_argument("Portable integer hints must be between 8 and 64 bits");
            }
        }

        DataType AsType() const
        {
            DataType result;
            if (IsFloat) {
                result = DataSize == 32 ? DataType::Float32 : DataType::Float64;
            }
            else if (IsSigned) {
                switch (DataSize) {
                case 8: result = DataType::Int8; break;
                case 16: result = DataType::Int16; break;
                case 32: result = DataType::Int32; break;
                case 64: result = DataType::Int64; break;
                default: throw std::invalid_argument("Unsupported signed integer size");
                }
            }
            else {
                switch (DataSize) {
                case 8: result = DataType::UInt8; break;
                case 16: result = DataType::UInt16; break;
                case 32: result = DataType::UInt32; break;
                case 64: result = DataType::UInt64; break;
                default: throw std::invalid_argument("Unsupported unsigned integer size");
                }
            }
            return IsArray ? ArrayType(result) : result;
        }

        static bool IsPowerOfTwo(int value)
        {
            return value > 0 && (value & (value - 1)) == 0;
        }
    };

    struct CCommandOption
    {
        std::string Name;
        DataType DataTypeValue = DataType::String;
        std::optional<std::string> DefaultValue;
        std::string HelpText;
        std::optional<std::string> TypeHintValue;
        bool IsValid = true;
        bool IsRequired = false;
        std::vector<std::string> Options;
        std::vector<std::string> Aliases;

        bool HasAlias(std::string_view value) const
        {
            return std::find(Aliases.begin(), Aliases.end(), value) != Aliases.end();
        }

        std::string GetCLIName() const
        {
            return Aliases.empty() ? "--" + Name : Join(Aliases, ", ");
        }
    };

    struct CCommandDef
    {
        std::string Name;
        std::string HelpText;
        std::optional<std::string> Alias;
        std::vector<CCommandOption> Options;

        std::string GetCLIName() const
        {
            return Alias ? *Alias : Lower(Name);
        }

        CCommandOption* GetOptionByName(std::string_view value)
        {
            for (auto& option : Options) {
                if (option.Name == value || ("--" + option.Name) == value || option.HasAlias(value))
                {
                    return &option;
                }
            }
            return nullptr;
        }

        const CCommandOption* GetOptionByName(std::string_view value) const
        {
            for (const auto& option : Options) {
                if (option.Name == value || ("--" + option.Name) == value || option.HasAlias(value))
                {
                    return &option;
                }
            }
            return nullptr;
        }
    };

    struct ValidationError
    {
        std::string Message;
        explicit ValidationError(std::string message) : Message(std::move(message)) {}
    };

    struct CommandValidationResult
    {
        std::vector<ValidationError> Errors;
        void AddError(std::string message) { Errors.emplace_back(std::move(message)); }
    };

    class ICommand
    {
    public:
        virtual ~ICommand() = default;
        virtual CommandValidationResult Validate() const = 0;
        virtual CCommandDef Configure() const = 0;
    };

    class TextWrapper
    {
    public:
        static std::vector<std::string> Wrap(std::string_view message, std::size_t columnWidth,
            std::size_t indentWidth)
        {
            if (columnWidth == 0) { throw std::invalid_argument("Column width must be positive"); }
            if (indentWidth >= columnWidth) { throw std::invalid_argument("Indent must be less than column width"); }

            const std::size_t contentWidth = columnWidth - indentWidth;
            const std::string indent(indentWidth, ' ');
            const std::string normalized = NormalizeWhitespace(message);
            std::vector<std::string> lines;
            std::string current;
            std::istringstream words(normalized);
            std::string word;
            while (words >> word) {
                if (!current.empty() && current.size() + 1 + word.size() <= contentWidth) {
                    current += " " + word;
                    continue;
                }
                if (current.empty() && word.size() <= contentWidth) {
                    current = std::move(word);
                    continue;
                }
                if (!current.empty()) { lines.push_back(indent + current); current.clear(); }
                std::size_t position = 0;
                while (word.size() - position > contentWidth) {
                    lines.push_back(indent + word.substr(position, contentWidth));
                    position += contentWidth;
                }
                current = word.substr(position);
            }
            if (!current.empty()) { lines.push_back(indent + current); }
            return lines;
        }
    };

    class HelpWriter
    {
    public:
        explicit HelpWriter(std::ostream& output = std::cout, std::size_t width = 80)
            : Output_(&output), MaxWidth_(std::max<std::size_t>(width, 1))
        {
            Init();
        }

        void Init()
        {
            Col1Width = std::min<std::size_t>(20, MaxWidth_ / 2);
            Col1Width = std::max<std::size_t>(Col1Width, 1);
            Col2Width = MaxWidth_ - Col1Width;
            WriteColumns_ = Col2Width > 0;
        }

        void SetWidth(std::size_t width) { MaxWidth_ = std::max<std::size_t>(width, 1); Init(); }
        void SetCol1Size(std::size_t width)
        {
            Col1Width = (std::min)(width, MaxWidth_);
            Col2Width = MaxWidth_ - Col1Width;
            WriteColumns_ = Col2Width > Indent;
        }
        void SetIndent(std::size_t value) { Indent = value; }
        void SetProgramInfo(std::string name, std::string version = {})
        {
            ProgramName_ = std::move(name);
            Version_ = std::move(version);
        }

        void WriteMessage() { *Output_ << '\n'; }
        void WriteMessage(std::string_view message) { *Output_ << std::string(Indent, ' ') << message << '\n'; }
        void WriteMessage(std::string_view column1, std::string_view column2)
        {
            if (!WriteColumns_) {
                for (const auto& line : TextWrapper::Wrap(column1, MaxWidth_, Indent)) { *Output_ << line << '\n'; }
                for (const auto& line : TextWrapper::Wrap(column2, MaxWidth_, Indent)) { *Output_ << line << '\n'; }
                return;
            }
            auto left = TextWrapper::Wrap(column1, Col1Width, Indent);
            auto right = TextWrapper::Wrap(column2, Col2Width, (std::min)(Indent * 2, Col2Width - 1));
            left.resize((std::max)(left.size(), right.size()));
            right.resize(left.size());
            for (std::size_t i = 0; i < left.size(); ++i) {
                *Output_ << std::left << std::setw(static_cast<int>(Col1Width)) << left[i] << right[i] << '\n';
            }
        }

        void WriteNameAndVersion()
        {
            WriteMessage(Version_.empty() ? ProgramName_ : ProgramName_ + " " + Version_);
        }
        void WriteVersion() { WriteMessage(Version_); }

        std::size_t Col1Width = 1;
        std::size_t Col2Width = 0;
        std::size_t Indent = 0;

    private:
        std::ostream* Output_;
        std::size_t MaxWidth_;
        bool WriteColumns_ = true;
        std::string ProgramName_ = "program";
        std::string Version_;
    };

    class Helpers
    {
    public:
        static toml::table FromFile(const std::string& path, std::string& commandName,
            const std::optional<std::string>& expectedName = std::nullopt)
        {
            toml::table root = toml::parse_file(path);
            if (root.size() != 1) { throw std::invalid_argument("Invalid number of keys in command file"); }
            auto iterator = root.begin();
            commandName = std::string(iterator->first.str());
            if (expectedName && commandName != *expectedName) {
                throw std::invalid_argument("Expected command name '" + *expectedName + "' but got '" + commandName + "'");
            }
            toml::table* table = iterator->second.as_table();
            if (!table) { throw std::invalid_argument("The command root must be a TOML table"); }
            return std::move(*table);
        }

        static std::pair<std::string, CommandOptions> ParseTOMLComment(std::string_view input,
            bool isCommandDef)
        {
            CommandOptions options;
            std::vector<std::string> text;
            std::istringstream lines{ std::string(input) };
            std::string line;
            while (std::getline(lines, line)) {
                const std::string value = Trim(line);
                if (value.rfind("@REQUIRED", 0) == 0) {
                    options.IsRequired = true;
                }
                else if (value.rfind("@TYPE", 0) == 0) {
                    options.TypeHint = Trim(value.substr(5));
                }
                else if (value.rfind("@OPTIONS", 0) == 0) {
                    options.Options = Split(value.substr(8), ',');
                }
                else if (value.rfind("@ALIAS", 0) == 0) {
                    options.Aliases = Split(value.substr(6), ',');
                    ValidateAliases(options.Aliases, isCommandDef);
                }
                else {
                    text.push_back(value);
                }
            }
            return { Join(text, "\n"), std::move(options) };
        }

        static DataType GetDataType(const toml::node& node, const CommandOptions& options = {})
        {
            if (options.TypeHint) { return TypeHint(*options.TypeHint).AsType(); }
            if (node.is_string()) { return DataType::String; }
            if (node.is_integer()) { return DataType::Int64; }
            if (node.is_floating_point()) { return DataType::Float64; }
            if (node.is_boolean()) { return DataType::Boolean; }
            if (node.is_date_time()) { return DataType::DateTime; }
            if (node.is_date()) { return DataType::Date; }
            if (node.is_time()) { return DataType::Time; }
            if (const toml::array* array = node.as_array()) {
                if (array->empty()) { throw std::invalid_argument("Cannot infer the type of an empty TOML array; use @TYPE"); }
                DataType element = GetDataType(*array->get(0));
                if (IsArray(element)) { throw std::invalid_argument("Nested arrays are not supported"); }
                for (std::size_t i = 1; i < array->size(); ++i) {
                    if (GetDataType(*array->get(i)) != element) {
                        throw std::invalid_argument("Mixed-type arrays are not supported");
                    }
                }
                return ArrayType(element);
            }
            if (node.is_table()) { throw std::invalid_argument("Nested tables are not supported"); }
            throw std::invalid_argument("Unsupported TOML value type");
        }

    private:
        static void ValidateAliases(const std::vector<std::string>& aliases, bool command)
        {
            int shortCount = 0, longCount = 0, otherCount = 0;
            for (const auto& alias : aliases) {
                if (alias.rfind("--", 0) == 0) { ++longCount; }
                else if (alias.rfind('-', 0) == 0) { ++shortCount; }
                else { ++otherCount; }
            }
            const bool invalid = command
                ? (shortCount != 0 || longCount != 0 || otherCount != 1)
                : (shortCount > 1 || longCount > 1 || otherCount != 0);
            if (invalid) { throw std::invalid_argument("Invalid alias specification: " + Join(aliases, ", ")); }
        }
    };

    class DefGenerator
    {
        struct DefinitionComments
        {
            std::string Command;
            std::vector<std::pair<std::string, std::string>> Options;
        };

    public:
        std::vector<CCommandDef> ParseCommandDefsFromTOML(const std::string& path) const
        {
            std::ifstream input(path);
            if (!input) { throw std::runtime_error("The file at path '" + path + "' does not exist"); }
            const auto comments = ScanComments(input);
            toml::table root = toml::parse_file(path);
            std::vector<CCommandDef> result;
            for (auto&& [key, node] : root) {
                const toml::table* table = node.as_table();
                if (!table) { throw std::invalid_argument("Every top-level TOML value must be a command table"); }
                const std::string name(key.str());
                const auto commentIt = comments.find(name);
                const std::string commandComment = commentIt == comments.end() ? std::string{} : commentIt->second.Command;
                auto parsedCommand = Helpers::ParseTOMLComment(commandComment, true);
                CCommandDef definition;
                definition.Name = name;
                definition.HelpText = std::move(parsedCommand.first);
                if (!parsedCommand.second.Aliases.empty()) { definition.Alias = parsedCommand.second.Aliases.front(); }

                for (auto&& [optionKey, optionNode] : *table) {
                    const std::string optionName(optionKey.str());
                    std::string optionComment;
                    if (commentIt != comments.end()) {
                        const auto found = std::find_if(commentIt->second.Options.begin(), commentIt->second.Options.end(),
                            [&](const auto& pair) { return pair.first == optionName; });
                        if (found != commentIt->second.Options.end()) { optionComment = found->second; }
                    }
                    auto parsedOption = Helpers::ParseTOMLComment(optionComment, false);
                    CCommandOption option;
                    option.Name = optionName;
                    option.HelpText = std::move(parsedOption.first);
                    option.IsRequired = parsedOption.second.IsRequired;
                    option.Options = std::move(parsedOption.second.Options);
                    option.Aliases = std::move(parsedOption.second.Aliases);
                    option.TypeHintValue = parsedOption.second.TypeHint;
                    option.DataTypeValue = Helpers::GetDataType(optionNode, parsedOption.second);
                    option.DefaultValue = NodeToString(optionNode);
                    definition.Options.push_back(std::move(option));
                }
                result.push_back(std::move(definition));
            }
            return result;
        }

    private:
        static std::vector<std::pair<std::string, DefinitionComments>> ScanCommentsVector(std::istream& input)
        {
            std::vector<std::pair<std::string, DefinitionComments>> result;
            std::vector<std::string> pending;
            std::string currentCommand;
            std::string line;
            while (std::getline(input, line)) {
                const std::string trimmed = Trim(line);
                if (trimmed.empty()) { continue; }
                if (trimmed[0] == '#') {
                    std::string comment = Trim(trimmed.substr(1));
                    pending.push_back(std::move(comment));
                    continue;
                }
                if (trimmed.front() == '[') {
                    const std::size_t close = trimmed.find(']');
                    if (close == std::string::npos) { pending.clear(); continue; }
                    currentCommand = Trim(std::string_view(trimmed).substr(1, close - 1));
                    result.push_back({ currentCommand, {Join(pending, "\n"), {}} });
                    pending.clear();
                    continue;
                }
                const std::size_t equals = trimmed.find('=');
                if (equals != std::string::npos && !currentCommand.empty()) {
                    std::string key = Trim(std::string_view(trimmed).substr(0, equals));
                    if (key.size() >= 2 && ((key.front() == '"' && key.back() == '"') ||
                        (key.front() == '\'' && key.back() == '\''))) {
                        key = key.substr(1, key.size() - 2);
                    }
                    result.back().second.Options.push_back({ std::move(key), Join(pending, "\n") });
                }
                pending.clear();
            }
            return result;
        }

        class CommentMap
        {
        public:
            using value_type = std::pair<std::string, DefinitionComments>;
            using const_iterator = std::vector<value_type>::const_iterator;
            explicit CommentMap(std::vector<value_type> values) : Values_(std::move(values)) {}
            const_iterator find(const std::string& key) const
            {
                return std::find_if(Values_.begin(), Values_.end(), [&](const auto& value) { return value.first == key; });
            }
            const_iterator end() const { return Values_.end(); }
        private:
            std::vector<value_type> Values_;
        };

        static CommentMap ScanComments(std::istream& input) { return CommentMap(ScanCommentsVector(input)); }
    };

    class CParser
    {
    public:
        static constexpr int DEFAULT_ERROR_CODE = -1;
        using HydrateFunction = ICommand * (*)(const toml::table&);
        using CommandFunction = int (*)(ICommand*);

        explicit CParser(int errorCode = DEFAULT_ERROR_CODE, std::ostream& output = std::cout)
            : ErrorCode(errorCode), HelpWriter_(output) {
        }

        void SetProgramInfo(std::string name, std::string version = {})
        {
            HelpWriter_.SetProgramInfo(std::move(name), std::move(version));
        }

        void Register(const ICommand& command, HydrateFunction hydrate, CommandFunction onCommand)
        {
            if (!hydrate || !onCommand) { throw std::invalid_argument("Command callbacks cannot be null"); }
            CCommandDef definition = command.Configure();
            if (ResolveEntry(definition.Name)) { throw std::invalid_argument("A command with this name or alias is already registered"); }
            Commands_.push_back({ std::move(definition), hydrate, onCommand });
        }

        int ExecuteCommandLine(int argc, const char* const argv[])
        {
            std::vector<std::string> args;

            // NOTE: We drop the first arg on purpose!  This will be the name of the exe that we are running.
            for (int i = 1; i < argc; ++i) { args.emplace_back(argv[i]); }
            return ExecuteCommandLine(args);
        }

        int ExecuteCommandLine(const std::vector<std::string>& args)
        {
            Messages_.clear();
            if (args.empty() || args.front() == HelpCommand) {
                PrintHelpAndMessages(nullptr, nullptr);
                return ErrorCode;
            }
            if (std::find(args.begin(), args.end(), VersionCommand) != args.end()) {
                HelpWriter_.WriteVersion();
                return ErrorCode;
            }

            std::string commandName;
            toml::table table;
            std::size_t firstOption = 1;
            if (EndsWithI(args.front(), ".toml")) {
                table = Helpers::FromFile(args.front(), commandName);
            }
            else {
                commandName = args.front();
            }
            DefEntry* entry = ResolveEntry(commandName);
            if (!entry) {
                Messages_.push_back("Unknown command: " + args.front() + "!");
                PrintHelpAndMessages(nullptr, nullptr);
                return ErrorCode;
            }

            OptionsParseResult parsed = ParseOptionValues(args, firstOption, table, *entry);
            if (!parsed.Errors.empty()) {
                for (const auto& error : parsed.Errors) { Messages_.push_back(error); }
                PrintHelpAndMessages(entry, &parsed);
                return ErrorCode;
            }
            ICommand* command = entry->Hydrate(table);
            if (!command) { throw std::runtime_error("The hydrate callback returned null"); }
            const CommandValidationResult validation = command->Validate();
            if (!validation.Errors.empty()) {
                Messages_.push_back("There are validation errors!");
                for (const auto& error : validation.Errors) { Messages_.push_back(error.Message); }
                PrintHelpAndMessages(entry, nullptr);
                return ErrorCode;
            }
            return entry->OnCommand(command);
        }

        int ErrorCode;

    private:
        static constexpr const char* HelpCommand = "--help";
        static constexpr const char* VersionCommand = "--version";

        struct DefEntry
        {
            CCommandDef Def;
            HydrateFunction Hydrate;
            CommandFunction OnCommand;
        };
        struct OptionAndValue { const CCommandOption* Option; std::string Value; bool Valid; };
        struct OptionsParseResult
        {
            std::vector<std::string> Errors;
            std::vector<OptionAndValue> Values;
            bool HasValidOption(const CCommandOption& option) const
            {
                return std::any_of(Values.begin(), Values.end(),
                    [&](const auto& item) { return item.Valid && item.Option == &option; });
            }
        };

        DefEntry* ResolveEntry(std::string_view name)
        {
            for (auto& entry : Commands_) {
                if (IEquals(entry.Def.Name, name) || (entry.Def.Alias && IEquals(*entry.Def.Alias, name))) {
                    return &entry;
                }
            }
            return nullptr;
        }

        void PrintHelpAndMessages(const DefEntry* entry, const OptionsParseResult* parsed)
        {
            HelpWriter_.Init();
            HelpWriter_.WriteNameAndVersion();
            HelpWriter_.WriteMessage();
            for (const auto& message : Messages_) { HelpWriter_.WriteMessage(message); }
            if (!Messages_.empty()) { HelpWriter_.WriteMessage(); }

            std::vector<std::pair<std::string, std::string>> lines;
            if (entry) {
                for (const auto& option : entry->Def.Options) {
                    if (parsed && parsed->HasValidOption(option)) { continue; }
                    lines.push_back({ option.GetCLIName(), option.HelpText });
                }
                lines.push_back({ HelpCommand, "Display this help message." });
            }
            else {
                for (const auto& command : Commands_) { lines.push_back({ command.Def.GetCLIName(), command.Def.HelpText }); }
                lines.push_back({ HelpCommand, "Display help information for a specific command." });
                lines.push_back({ VersionCommand, "Display version information." });
            }
            constexpr std::size_t indent = 2;
            std::size_t width = HelpWriter_.Col1Width;
            for (const auto& line : lines) { width = (std::max)(width, line.first.size() + indent); }
            HelpWriter_.SetIndent(indent);
            HelpWriter_.SetCol1Size(width);
            for (const auto& line : lines) { HelpWriter_.WriteMessage(line.first, line.second); HelpWriter_.WriteMessage(); }
        }

        static void SetScalar(toml::table& table, const CCommandOption& option, const std::string& raw)
        {
            const auto signedValue = [&]() -> std::int64_t {
                std::size_t used = 0; const long long value = std::stoll(raw, &used, 0);
                if (used != raw.size()) { throw std::invalid_argument("invalid integer"); }
                return static_cast<std::int64_t>(value);
                };
            const auto unsignedValue = [&]() -> std::uint64_t {
                std::size_t used = 0; const unsigned long long value = std::stoull(raw, &used, 0);
                if (used != raw.size()) { throw std::invalid_argument("invalid integer"); }
                return static_cast<std::uint64_t>(value);
                };
            switch (option.DataTypeValue) {
            case DataType::String: table.insert_or_assign(option.Name, raw); return;
            case DataType::Boolean:
                if (IEquals(raw, "true")) { table.insert_or_assign(option.Name, true); return; }
                if (IEquals(raw, "false")) { table.insert_or_assign(option.Name, false); return; }
                throw std::invalid_argument("expected true or false");
            case DataType::Int8: case DataType::Int16: case DataType::Int32: case DataType::Int64:
                table.insert_or_assign(option.Name, signedValue()); return;
            case DataType::UInt8: case DataType::UInt16: case DataType::UInt32: case DataType::UInt64: {
                const std::uint64_t value = unsignedValue();
                //if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                if (value > static_cast<std::uint64_t>(12345)) {
                    throw std::out_of_range("toml++ integers are signed 64-bit values");
                }
                table.insert_or_assign(option.Name, static_cast<std::int64_t>(value)); return;
            }
            case DataType::Float32: case DataType::Float64: {
                std::size_t used = 0; const double value = std::stod(raw, &used);
                if (used != raw.size()) { throw std::invalid_argument("invalid floating-point value"); }
                table.insert_or_assign(option.Name, value); return;
            }
            default: break;
            }
            if (IsArray(option.DataTypeValue)) {
                toml::array array;
                const DataType elementType = ElementType(option.DataTypeValue);
                for (const auto& value : Split(raw, ',')) {
                    CCommandOption scalar = option;
                    scalar.DataTypeValue = elementType;
                    toml::table temporary;
                    SetScalar(temporary, scalar, value);
                    auto node = temporary.get(option.Name);
                    if (node) { array.push_back(std::move(*node)); }
                }
                table.insert_or_assign(option.Name, std::move(array));
                return;
            }
            throw std::invalid_argument("Date/time command-line values are not supported; use a TOML command file");
        }

        static OptionsParseResult ParseOptionValues(const std::vector<std::string>& args, std::size_t begin,
            toml::table& table, const DefEntry& entry)
        {
            OptionsParseResult result;
            for (std::size_t i = begin; i < args.size(); ++i) {
                const std::string& argument = args[i];
                if (argument == HelpCommand) { continue; }
                const CCommandOption* option = entry.Def.GetOptionByName(argument);
                if (!option) {
                    result.Errors.push_back("Invalid argument: " + argument + "!");
                    if (i + 1 < args.size() && !entry.Def.GetOptionByName(args[i + 1])) {
                        result.Values.push_back({ nullptr, args[++i], false });
                    }
                    continue;
                }
                std::string value;
                if (i + 1 < args.size() && !entry.Def.GetOptionByName(args[i + 1]) && args[i + 1] != HelpCommand) {
                    value = args[++i];
                }
                else if (option->DataTypeValue == DataType::Boolean) {
                    value = "true";
                }
                else {
                    result.Errors.push_back("There is no value for option: '" + argument + "'!");
                    continue;
                }
                try {
                    SetScalar(table, *option, value);
                    result.Values.push_back({ option, value, true });
                }
                catch (const std::exception& error) {
                    result.Errors.push_back("Invalid value for option '" + argument + "': " + error.what());
                }
            }
            return result;
        }

        HelpWriter HelpWriter_;
        std::vector<std::string> Messages_;
        std::vector<DefEntry> Commands_;
    };

} // namespace drewco::cli

#endif // DREWCO_CLI_HPP



//// ================================================================================================
//// C++ version of the drewCo.CLI C# library.
//// ================================================================================================
//#include "drewCo.Logging.hpp"
//#include "toml.hpp"
//
//using TomlTable = toml::table;
//
//namespace drewCo::CLI {
//
//    // ================================================================================================
//    // Defines a command that can be used on the command line.
//    //class CCommandDef {
//    public:
//
//        std::string Name;
//        std::string HelpText;
//        std::string Alias;
//
//    };
//
//    // ================================================================================================
//    class ICommand {};
//
//
//    // ================================================================================================
//    struct DefEntry {
//        CCommandDef Def;
//        ICommand* (*Hydrate)(TomlTable* table) = nullptr;
//        int (*OnCommand)(void* obj) = nullptr;
//
//        //using HydrateFn = ICommand * (*)(TomlTable&);
//        //using OnCommandFn = int (*)(void*);
//    };
//
//    // ================================================================================================
//    class CParser {
//
//    public:
//        static constexpr int DEFAULT_ERROR_CODE = -1;
//        static constexpr char* HELP_COMMAND = "--help";
//        static constexpr char* VERSION_COMMAND = "--version";
//
//
//        int Execute(int argc, char* argv[]) {
//        }
//    };
//}