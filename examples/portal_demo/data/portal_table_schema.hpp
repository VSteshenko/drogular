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
    std::string displayField;
};

template <typename TModel>
struct PortalFieldSchema {
    std::string name;
    std::string displayName;
    std::function<PortalFieldValue(const TModel&)> value;
    std::function<void(TModel&, const PortalFieldValue&)> setValue;
    bool key = false;
    bool unique = false;
    bool required = false;
    std::optional<PortalReferenceSchema> reference;
};

template <typename TModel>
class PortalTableSchema;

template <typename TModel>
class PortalFieldBuilder {
public:
    PortalFieldBuilder(
        PortalTableSchema<TModel>& table,
        PortalFieldSchema<TModel>& field
    )
        : table_(table),
          field_(field)
    {
    }

    PortalFieldBuilder& key() {
        field_.key = true;
        field_.unique = true;
        field_.required = true;
        return *this;
    }

    PortalFieldBuilder& required() {
        field_.required = true;
        return *this;
    }

    PortalFieldBuilder& unique() {
        field_.unique = true;
        field_.required = true;
        return *this;
    }

    PortalFieldBuilder& reference(
        std::string table,
        std::string field,
        std::string displayField
    )
    {
        field_.required = true;
        field_.reference = PortalReferenceSchema{
            .table = std::move(table),
            .field = std::move(field),
            .displayField = std::move(displayField)
        };

        return *this;
    }

    PortalFieldBuilder& displayName(
        std::string name
    )
    {
        field_.displayName = std::move(name);
        return *this;
    }

private:
    PortalTableSchema<TModel>& table_;
    PortalFieldSchema<TModel>& field_;
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

    PortalFieldBuilder<TModel> field(
        std::string name,
        int TModel::*member
    ) {
        auto refName = std::move(name);
        fields_.push_back({
            .name = refName,
            .displayName = refName,
            .value = getter(member),
            .setValue = setter(member)
        });

        return PortalFieldBuilder<TModel>(
            *this,
            fields_.back()
        );
    }

    PortalFieldBuilder<TModel> field(
        std::string name,
        std::string TModel::*member
    ) {
        auto refName = std::move(name);
        fields_.push_back({
            .name = refName,
            .displayName = refName,
            .value = getter(member),
            .setValue = setter(member)
        });

        return PortalFieldBuilder<TModel>(
            *this,
            fields_.back()
        );
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