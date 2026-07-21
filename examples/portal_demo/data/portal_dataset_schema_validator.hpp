#pragma once

#include "portal_dataset.hpp"
#include "portal_dataset_validation_result.hpp"
#include "portal_schema.hpp"
#include "portal_table_schema.hpp"

#include <set>
#include <string>
#include <vector>

class PortalDatasetSchemaValidator {
public:
    static PortalDatasetValidationResult validate(
        const PortalDataset& dataset
    ) {
        PortalDatasetValidationResult result;

        validateTable(
            result,
            PortalSchema::users(),
            dataset.users()
        );

        validateTable(
            result,
            PortalSchema::roles(),
            dataset.roles()
        );

        validateTable(
            result,
            PortalSchema::projects(),
            dataset.projects()
        );

        validateTable(
            result, PortalSchema::projectTypes(), dataset.projectTypes());
        validateTable(
            result, PortalSchema::departments(), dataset.departments());
        validateTable(
            result, PortalSchema::departmentMembers(), dataset.departmentMembers());

        validateReferences(
            result,
            PortalSchema::users(),
            dataset.users(),
            dataset
        );

        validateReferences(
            result, PortalSchema::projects(), dataset.projects(), dataset);
        validateReferences(
            result, PortalSchema::departments(), dataset.departments(), dataset);
        validateReferences(
            result, PortalSchema::departmentMembers(), dataset.departmentMembers(), dataset);

        return result;
    }

private:
    template <typename TModel>
    static void validateTable(
        PortalDatasetValidationResult& result,
        const PortalTableSchema<TModel>& schema,
        const std::vector<TModel>& rows
    ) {
        for (const auto& field : schema.fields()) {
            if (field.key) {
                validatePositive(
                    result,
                    schema.name(),
                    field,
                    rows
                );
            }

            if (field.required) {
                validateRequired(
                    result,
                    schema.name(),
                    field,
                    rows
                );
            }

            if (field.unique) {
                validateUnique(
                    result,
                    schema.name(),
                    field,
                    rows
                );
            }
        }
    }

    template <typename TModel>
    static void validatePositive(
        PortalDatasetValidationResult& result,
        const std::string& table,
        const PortalFieldSchema<TModel>& field,
        const std::vector<TModel>& rows
    ) {
        for (const auto& row : rows) {
            const auto value =
                field.value(row);

            if (std::holds_alternative<int>(value) &&
                std::get<int>(value) <= 0) {
                result.addError(
                    table + "." + field.name +
                    " must be positive"
                );
            }
        }
    }

    template <typename TModel>
    static void validateRequired(
        PortalDatasetValidationResult& result,
        const std::string& table,
        const PortalFieldSchema<TModel>& field,
        const std::vector<TModel>& rows
    ) {
        for (const auto& row : rows) {
            const auto value =
                field.value(row);

            if (std::holds_alternative<int>(value) &&
                std::get<int>(value) <= 0) {
                result.addError(
                    table + "." + field.name +
                    " is required"
                );
            }

            if (std::holds_alternative<std::string>(value) &&
                std::get<std::string>(value).empty()) {
                result.addError(
                    table + "." + field.name +
                    " is required"
                );
            }
        }
    }

    template <typename TModel>
    static void validateUnique(
        PortalDatasetValidationResult& result,
        const std::string& table,
        const PortalFieldSchema<TModel>& field,
        const std::vector<TModel>& rows
    ) {
        std::set<PortalFieldValue> values;

        for (const auto& row : rows) {
            const auto value =
                field.value(row);

            if (!values.insert(value).second) {
                result.addError(
                    table + "." + field.name +
                    " must be unique"
                );
            }
        }
    }

    template <typename TModel>
    static void validateReferences(
        PortalDatasetValidationResult& result,
        const PortalTableSchema<TModel>& schema,
        const std::vector<TModel>& rows,
        const PortalDataset& dataset
    ) {
        for (const auto& field : schema.fields()) {
            if (!field.reference.has_value()) {
                continue;
            }

            for (const auto& row : rows) {
                const auto value =
                    field.value(row);

                if (!referenceExists(
                        *field.reference,
                        value,
                        dataset
                    )) {
                    result.addError(
                        schema.name() + "." + field.name +
                        " references missing " +
                        field.reference->table + "." +
                        field.reference->field
                    );
                }
            }
        }
    }

    static bool referenceExists(
        const PortalReferenceSchema& reference,
        const PortalFieldValue& value,
        const PortalDataset& dataset
    ) {
        if (reference.table == "users" &&
            reference.field == "id") {
            return contains(
                dataset.users(),
                value,
                [](const PortalUser& user) {
                    return PortalFieldValue(user.id);
                }
            );
        }

        if (reference.table == "roles" &&
            reference.field == "code") {
            return contains(
                dataset.roles(),
                value,
                [](const PortalRole& role) {
                    return PortalFieldValue(role.code);
                }
            );
        }

        if ((reference.table == "projectTypes" || reference.table == "project_types") &&
            reference.field == "id") {
            return contains(
                dataset.projectTypes(),
                value,
                [](const PortalProjectType& type) {
                    return PortalFieldValue(type.id);
                }
            );
        }

        if (reference.table == "departments" && reference.field == "id") {
            return contains(dataset.departments(), value, [](const PortalDepartment& item) { return PortalFieldValue(item.id); });
        }

        return false;
    }

    template <typename TModel, typename TValue>
    static bool contains(
        const std::vector<TModel>& rows,
        const PortalFieldValue& value,
        TValue getter
    ) {
        for (const auto& row : rows) {
            if (getter(row) == value) {
                return true;
            }
        }

        return false;
    }
};