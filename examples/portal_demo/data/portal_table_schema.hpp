#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using PortalFieldValue =
    std::variant<int, std::string>;

struct PortalReferenceSchema {
    std::string table;
    std::string field;
};

template <typename TModel>
struct PortalFieldSchema {
    std::string name;
    std::function<PortalFieldValue(const TModel&)> value;
    std::function<void(TModel&, const PortalFieldValue&)> setValue;
    bool key = false;
    bool unique = false;
    bool required = false;
    std::optional<PortalReferenceSchema> reference;
};

template <typename TModel>
class PortalTableSchema {
public:
    explicit PortalTableSchema(
        std::string name
    )
        : name_(std::move(name))
    {
    }

    static PortalTableSchema forModel(
        std::string name
    ) {
        return PortalTableSchema(
            std::move(name)
        );
    }

    PortalTableSchema& key(
        std::string name,
        int TModel::*member
    ) {
        fields_.push_back({
            .name = std::move(name),
            .value = getter(member),
            .setValue = setter(member),
            .key = true,
            .unique = true,
            .required = true
        });

        return *this;
    }

    PortalTableSchema& unique(
        std::string name,
        std::string TModel::*member
    ) {
        fields_.push_back({
            .name = std::move(name),
            .value = getter(member),
            .setValue = setter(member),
            .unique = true,
            .required = true
        });

        return *this;
    }

    PortalTableSchema& required(
        std::string name,
        std::string TModel::*member
    ) {
        fields_.push_back({
            .name = std::move(name),
            .value = getter(member),
            .setValue = setter(member),
            .required = true
        });

        return *this;
    }

    PortalTableSchema& required(
        std::string name,
        int TModel::*member
    ) {
        fields_.push_back({
            .name = std::move(name),
            .value = getter(member),
            .setValue = setter(member),
            .required = true
        });

        return *this;
    }

    PortalTableSchema& reference(
        std::string name,
        int TModel::*member,
        std::string table,
        std::string field
    ) {
        fields_.push_back({
            .name = std::move(name),
            .value = getter(member),
            .setValue = setter(member),
            .required = true,
            .reference = PortalReferenceSchema{
                .table = std::move(table),
                .field = std::move(field)
            }
        });

        return *this;
    }

    PortalTableSchema& reference(
        std::string name,
        std::string TModel::*member,
        std::string table,
        std::string field
    ) {
        fields_.push_back({
            .name = std::move(name),
            .value = getter(member),
            .setValue = setter(member),
            .required = true,
            .reference = PortalReferenceSchema{
                .table = std::move(table),
                .field = std::move(field)
            }
        });

        return *this;
    }

    const std::string& name() const {
        return name_;
    }

    const std::vector<PortalFieldSchema<TModel>>& fields() const {
        return fields_;
    }

private:
    std::string name_;
    std::vector<PortalFieldSchema<TModel>> fields_;

    static std::function<PortalFieldValue(const TModel&)>
    getter(int TModel::*member) {
        return [member](const TModel& model) {
            return model.*member;
        };
    }

    static std::function<PortalFieldValue(const TModel&)>
    getter(std::string TModel::*member) {
        return [member](const TModel& model) {
            return model.*member;
        };
    }

    static std::function<void(TModel&, const PortalFieldValue&)>
    setter(int TModel::*member) {
        return [member](TModel& model, const PortalFieldValue& value) {
            model.*member = std::get<int>(value);
        };
    }

    static std::function<void(TModel&, const PortalFieldValue&)>
    setter(std::string TModel::*member) {
        return [member](TModel& model, const PortalFieldValue& value) {
            model.*member = std::get<std::string>(value);
        };
    }
};