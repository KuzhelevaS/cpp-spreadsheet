#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!CheckPosition(pos)) {
        if (static_cast<int>(cells_.size()) <= pos.row) {
            cells_.resize(pos.row + 1);
        }
        if (static_cast<int>(cells_[pos.row].size()) <= pos.col) {
            cells_[pos.row].resize(pos.col + 1);
        }
        cells_[pos.row][pos.col] = std::make_unique<Cell>(*this, pos);
    }
    cells_[pos.row][pos.col]->Set(text);
    printable_size_.rows = std::max(pos.row + 1, printable_size_.rows);
    printable_size_.cols = std::max(pos.col + 1, printable_size_.cols);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (CheckPosition(pos)) {
        return cells_[pos.row][pos.col].get();
    } else {
        return nullptr;
    }
}
CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>(const_cast<const Sheet*>(this)->GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
    if (CheckPosition(pos)) {
        cells_[pos.row][pos.col] = nullptr;
    }
    ShrinkPrintableArea();
}

Size Sheet::GetPrintableSize() const {
    return printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    auto print_text = [&output](const CellInterface* cell) {
        auto value = cell->GetValue();
        if (std::holds_alternative<double>(value)) {
            output << std::get<double>(value);
        } else if (std::holds_alternative<std::string>(value)) {
            output << std::get<std::string>(value);
        }else {
            output << std::get<FormulaError>(value);
        }
    };
    PrintEachCell(output, print_text);
}
void Sheet::PrintTexts(std::ostream& output) const {
    auto print_text = [&output](const CellInterface* cell) {
        output << cell->GetText();
    };
    PrintEachCell(output, print_text);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

bool Sheet::CheckPosition(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException(
            std::string("Position is incorrect: ")
            + std::to_string(pos.row) + std::to_string(pos.col));
    }
    return static_cast<int>(cells_.size()) > pos.row
        && static_cast<int>(cells_.at(pos.row).size()) > pos.col
        && cells_[pos.row][pos.col];
}

void Sheet::PrintEachCell(std::ostream& output, const std::function<void(const CellInterface*)>& CellHandler) const{
    for (int i = 0; i < printable_size_.rows; ++i) {
        for (int j = 0; j < printable_size_.cols; ++j) {
            if (j != 0) {
                output << '\t';
            }
            if (j < static_cast<int>(cells_[i].size()) && cells_[i][j]) {
                CellHandler(cells_[i][j].get());
            }
        }
        output << '\n';
    }
}

void Sheet::ShrinkPrintableArea() {
    int max_row = -1;
    int max_col = -1;
    for (int row = 0; row < static_cast<int>(cells_.size()); ++row) {
        for (int col = 0; col < static_cast<int>(cells_[row].size()); ++col) {
            if (cells_[row][col]) {
                if (max_row < row) {
                    max_row = row;
                }
                if (max_col < col) {
                    max_col = col;
                }
            }
        }
    }
    printable_size_.rows = max_row + 1;
    printable_size_.cols = max_col + 1;
}

DependencyGraph & Sheet::GetDependencyGraph() {
    return graph_;
}

size_t DependencyGraph::PositionHasher::operator()(const Position& pos) const {
    return pos.row * 37 + pos.col;
}

const std::unordered_set<Position, DependencyGraph::PositionHasher> & DependencyGraph::GetAdjacency(Position position) const {
    if (adjacency_.count(position)) {
        return adjacency_.at(position);
    }
    static std::unordered_set<Position, PositionHasher> empty;
    return empty;
}

void DependencyGraph::AddAdjacency(Position target, Position adding) {
    adjacency_[target].insert(adding);
}

void DependencyGraph::RemoveAdjacency(Position target, Position removing) {
    if (adjacency_.count(target)) {
        adjacency_.at(target).erase(removing);
    }
}

const std::unordered_set<Position, DependencyGraph::PositionHasher> & DependencyGraph::GetDependency(Position position) const {
    if (dependency_.count(position)) {
        return dependency_.at(position);
    }
    static std::unordered_set<Position, PositionHasher> empty;
    return empty;
}

void DependencyGraph::AddDependency(Position target, Position adding) {
    dependency_[target].insert(adding);
}
void DependencyGraph::RemoveDependency(Position target, Position removing) {
    if (dependency_.count(target)) {
        dependency_.at(target).erase(removing);
    }
}
