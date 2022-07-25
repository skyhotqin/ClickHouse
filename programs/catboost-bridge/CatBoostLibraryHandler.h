#pragma once

#include <Columns/ColumnFixedString.h>
#include <Columns/ColumnString.h>
#include <Columns/ColumnVector.h>
#include <Columns/ColumnsNumber.h>
#include <Columns/IColumn.h>
#include <Common/SharedLibrary.h>

#include "CatBoostLibraryAPI.h"

namespace DB
{

// TODO error handling, e.g. library path does not exist, model path does not exist etc.
// TODO apply stuff done to library-bridge also to odbc bridge
// TODO needs serialization/deserialization wrapper
// TODO check that the library is initialized

// TODO revert back to main/mainClickHouse

/// Abstracts access to the CatBoost shared library.
class CatBoostLibraryHandler
{
    /// Holds pointers to CatBoost library functions
    struct APIHolder
    {
        explicit APIHolder(SharedLibrary & lib);

        // NOLINTBEGIN(readability-identifier-naming)
        CatBoostLibraryAPI::ModelCalcerCreateFunc ModelCalcerCreate;
        CatBoostLibraryAPI::ModelCalcerDeleteFunc ModelCalcerDelete;
        CatBoostLibraryAPI::GetErrorStringFunc GetErrorString;
        CatBoostLibraryAPI::LoadFullModelFromFileFunc LoadFullModelFromFile;
        CatBoostLibraryAPI::CalcModelPredictionFlatFunc CalcModelPredictionFlat;
        CatBoostLibraryAPI::CalcModelPredictionFunc CalcModelPrediction;
        CatBoostLibraryAPI::CalcModelPredictionWithHashedCatFeaturesFunc CalcModelPredictionWithHashedCatFeatures;
        CatBoostLibraryAPI::GetStringCatFeatureHashFunc GetStringCatFeatureHash;
        CatBoostLibraryAPI::GetIntegerCatFeatureHashFunc GetIntegerCatFeatureHash;
        CatBoostLibraryAPI::GetFloatFeaturesCountFunc GetFloatFeaturesCount;
        CatBoostLibraryAPI::GetCatFeaturesCountFunc GetCatFeaturesCount;
        CatBoostLibraryAPI::GetTreeCountFunc GetTreeCount;
        CatBoostLibraryAPI::GetDimensionsCountFunc GetDimensionsCount;
        // NOLINTEND(readability-identifier-naming)
    };

public:
    CatBoostLibraryHandler(
        const std::string & library_path,
        const std::string & model_path);

    ~CatBoostLibraryHandler();

    ColumnPtr evaluate(const ColumnRawPtrs & columns) const;

private:
    SharedLibraryPtr library;
    APIHolder api;

    CatBoostLibraryAPI::ModelCalcerHandle * model_calcer_handle;

    size_t float_features_count;
    size_t cat_features_count;
    size_t tree_count;

    template <typename T>
    static void placeColumnAsNumber(const IColumn * column, T * buffer, size_t features_count);

    static void placeStringColumn(const ColumnString & column, const char ** buffer, size_t features_count);

    static PODArray<char> placeFixedStringColumn(const ColumnFixedString & column, const char ** buffer, size_t features_count);

    template <typename T>
    static ColumnPtr placeNumericColumns(const ColumnRawPtrs & columns, size_t offset, size_t size, const T** buffer);

    static std::vector<PODArray<char>> placeStringColumns(const ColumnRawPtrs & columns, size_t offset, size_t size, const char ** buffer);

    template <typename Column>
    static void calcStringHashes(const Column * column, size_t ps, const int ** buffer, const CatBoostLibraryHandler::APIHolder & api);

    static void calcIntHashes(size_t column_size, size_t ps, const int ** buffer, const CatBoostLibraryHandler::APIHolder & api);

    static void calcHashes(const ColumnRawPtrs & columns, size_t offset, size_t size, const int ** buffer, const CatBoostLibraryHandler::APIHolder & api);

    void fillCatFeaturesBuffer(const char *** cat_features, const char ** buffer, size_t column_size) const;

    ColumnFloat64::MutablePtr evalImpl(const ColumnRawPtrs & columns, bool cat_features_are_strings) const;
};

using CatBoostLibraryHandlerPtr = std::shared_ptr<CatBoostLibraryHandler>;

}
