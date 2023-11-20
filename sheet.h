#pragma once

#include "common.h"

#include <functional>
#include <vector>
#include <memory>
#include <unordered_set>

class DependencyGraph {
public:
    struct PositionHasher {
        size_t operator()(const Position& pos) const;
    };

    const std::unordered_set<Position, DependencyGraph::PositionHasher> & GetAdjacency(Position) const;
    void AddAdjacency(Position target, Position adding);
    void RemoveAdjacency(Position target, Position removing);

    const std::unordered_set<Position, DependencyGraph::PositionHasher> & GetDependency(Position) const;
    void AddDependency(Position target, Position adding);
    void RemoveDependency(Position target, Position removing);
private:
    // На что ссылается каждая ячейка
    std::unordered_map<Position, std::unordered_set<Position, PositionHasher>, PositionHasher> adjacency_;
    // Какие ячейки ссылаются на текущую
    std::unordered_map<Position, std::unordered_set<Position, PositionHasher>, PositionHasher> dependency_;
};

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    // Можете дополнить ваш класс нужными полями и методами
    DependencyGraph & GetDependencyGraph();

private:
    // Можете дополнить ваш класс нужными полями и методами
    std::vector<std::vector<std::unique_ptr<CellInterface>>> cells_; // vector_rows, row is vector_cols
    Size printable_size_ = {0,0};
    DependencyGraph graph_;

    bool CheckPosition(Position pos) const;
    void ShrinkPrintableArea();
    void PrintEachCell(std::ostream& output, const std::function<void(const CellInterface*)>& CellHandler) const;
};
