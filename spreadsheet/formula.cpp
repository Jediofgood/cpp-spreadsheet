#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>


using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}


// Реализуйте следующие методы:
Formula::Formula(std::string expression)
    :ast_(ParseFormulaAST(expression))
{}

Formula::Value Formula::Evaluate(const SheetInterface& sheet) const{
    auto Functor = [&sheet](Position pos){
        return sheet.GetCell(pos);
    };

    try {
        //return ast_.Evaluate();
        return ast_.Execute(Functor);
    } catch (FormulaError& er) {
        return er;
    }
}

std::string Formula::GetExpression() const{
    //return ast_.GetExpression();
    std::stringstream str;
    ast_.PrintFormula(str);
    return str.str();
}

std::vector<Position> Formula::GetReferencedCells() const {
    const std::forward_list<Position>& list = ast_.GetCells();
    std::vector<Position> res(list.begin(), list.end());

    res.erase(std::unique(res.begin(), res.end()), res.end());

    return res;
}


std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    } catch (const std::exception&) {
        throw FormulaException("");
    }
}
