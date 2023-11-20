#pragma once

#include "common.h"
#include "formula.h"


#include <functional>
#include <optional>

class Impl;
class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet, Position pos);
    ~Cell();

    void Set(std::string text) override;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;

private:
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    Position pos_;
    mutable std::optional<Value> cache_;
    std::vector<Position> referenced_cells_;

    bool CheckGraphDependences(const std::vector<Position> &);
    void UpdateGraphDependences();
    void CacheInvalidation();
};
