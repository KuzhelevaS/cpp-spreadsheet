#include "cell.h"

#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

class Impl {
public:
    virtual ~Impl() = default;
    virtual CellInterface::Value GetValue(const SheetInterface& sheet) const = 0;
    virtual std::string GetText() const = 0;
};

class EmptyImpl : public Impl {
public:
    CellInterface::Value GetValue(const SheetInterface&) const override {
        return "";
    }
    std::string GetText() const override {
        return "";
    }
};

class TextImpl : public Impl {
public:
    TextImpl(std::string value) : text_(std::move(value)), value_(SetValue()) {}
    CellInterface::Value GetValue(const SheetInterface&) const override {
        return std::string(value_);
    }
    std::string GetText() const override {
        return text_;
    }
private:
    std::string text_ = "";
    std::string value_;

    std::string SetValue() {
        if (text_[0] == ESCAPE_SIGN) {
            return text_.substr(1);
        } else {
            return text_;
        }
    }
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::unique_ptr<FormulaInterface> formula)
        : formula_(std::move(formula)), text_(std::string("=") + formula_->GetExpression()) {}

    CellInterface::Value GetValue(const SheetInterface& sheet) const override {
        auto value = formula_->Evaluate(sheet);
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        }  else {
            return std::get<FormulaError>(value);
        }
    }

    std::string GetText() const override {
        return text_;
    }
private:
    std::unique_ptr<FormulaInterface> formula_;
    std::string text_ = "";
};

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet, Position pos)
    :impl_(std::make_unique<EmptyImpl>(EmptyImpl()))
    , sheet_(sheet), pos_(pos) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text == GetText()) {
        return;
    }

    bool isFormula = text[0] == FORMULA_SIGN && text.size() != 1;
    if (isFormula) {
        auto formula = ParseFormula(text.substr(1));
        auto referenced_cells = formula->GetReferencedCells();
        if (!CheckGraphDependences(referenced_cells)) {
            throw CircularDependencyException(
                std::string("Found circular at ") + pos_.ToString()
                + std::string("with formula") + text );
        }
        referenced_cells_ = std::move(referenced_cells);
        impl_ = std::make_unique<FormulaImpl>(std::move(formula));
    } else {
        impl_ = std::make_unique<TextImpl>(text);
        referenced_cells_.clear();
    }

    UpdateGraphDependences();
    CacheInvalidation();
    cache_.reset();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>(EmptyImpl());
    cache_.reset();
    referenced_cells_.clear();
    UpdateGraphDependences();
    CacheInvalidation();
}

Cell::Value Cell::GetValue() const {
    if (!cache_.has_value()) {
        cache_ = impl_->GetValue(*static_cast<SheetInterface*>(&sheet_));
    }

    return cache_.value();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return referenced_cells_;
}

bool Cell::IsReferenced() const {
    return !referenced_cells_.empty();
}

bool Cell::CheckGraphDependences(const std::vector<Position> & referenced_cells) {
    const auto & graph = sheet_.GetDependencyGraph();

    std::stack<Position> cell_stack;
    for (auto pos : referenced_cells) {
        if (pos == pos_) {
            return false;
        }
        cell_stack.push(pos);
    }

    std::unordered_set<Position, DependencyGraph::PositionHasher> cell_set;
    cell_set.insert(pos_);

    while (cell_stack.size() > 0) {
        Position pos = cell_stack.top();
        cell_stack.pop();
        for (auto position : graph.GetAdjacency(pos)) {
            if (cell_set.count(position)) {
                return false;
            }
            cell_stack.push(position);
        }
    }

    for (auto pos : graph.GetDependency(pos_)) {
        cell_stack.push(pos);
    }

    while (cell_stack.size() > 0) {
        Position pos = cell_stack.top();
        cell_stack.pop();
        for (auto position : graph.GetDependency(pos)) {
            if (cell_set.count(position)) {
                return false;
            }
            cell_stack.push(position);
        }
    }

    return true;
}

void Cell::UpdateGraphDependences() {
    auto & graph = sheet_.GetDependencyGraph();

    std::vector<Position> removing;
    removing.reserve(graph.GetAdjacency(pos_).size());
    for (auto pos : graph.GetAdjacency(pos_)) {
        graph.RemoveDependency(pos, pos_);
        removing.push_back(pos);
    }
    // удаляем отдельным циклом, чтобы избежать инвалидации итераторов
    for (auto pos : removing) {
        graph.RemoveAdjacency(pos_, pos);
    }

    for (auto pos : referenced_cells_) {
        graph.AddDependency(pos, pos_);
        graph.AddAdjacency(pos_, pos);
    }
}

void Cell::CacheInvalidation() {
    auto & graph = sheet_.GetDependencyGraph();
    std::stack<Position> cell_stack;
    for (auto pos : graph.GetDependency(pos_)) {
        cell_stack.push(pos);
    }
    while (cell_stack.size() > 0) {
        Position pos = cell_stack.top();
        cell_stack.pop();
        Cell * cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
        // если у этой ячейки кэш уже инвалидирован, значит у ссылающихся на нее тоже
        if (cell->cache_.has_value()) {
            cell->cache_.reset();
            for (auto position : graph.GetDependency(pos)) {
                cell_stack.push(position);
            }
        }
    }
}

