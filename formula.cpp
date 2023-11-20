#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {}
    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        } catch (const FormulaError& exc) {
            return exc;
        }
    }
    std::string GetExpression() const override {
        std::ostringstream result;
        ast_.PrintFormula(result);
        return result.str();
    }

    virtual std::vector<Position> GetReferencedCells() const override {
        auto cells = ast_.GetCells(); // must return sorted list
        std::vector<Position> result{cells.begin(), cells.end()};
        auto last_unique = std::unique(result.begin(), result.end());
        result.erase(last_unique, result.end());
        return result;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
