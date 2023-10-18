#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <memory>
#include <utility>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::ThrowOnNoValidPos(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
}

void Sheet::Resize(Position pos) {
    if (table_.size() < size_t(pos.row + 1)) {

        for (int i = table_.size(); i < pos.row + 1; ++i) {
            table_.push_back(std::vector<std::unique_ptr<Cell>>(size_.cols));
        }

    }
    if (table_[pos.row].size() < size_t(pos.col + 1)) {
        for (std::vector<std::unique_ptr<Cell>>& col : table_) {
            col.resize(pos.col + 1);
        }

    }
}

bool Sheet::CheckPrintSize(Position pos) const {
    if (size_.cols < pos.col + 1) return false;
    if (size_.rows < pos.row + 1) return false;
    return true;
}

void Sheet::AddToCatalogue(Position pos) {
    counter_col_[pos.col].emplace(pos.row);
    counter_row_[pos.row].emplace(pos.col);

    if (size_.cols < pos.col + 1) {
        size_.cols = pos.col + 1;
    }
    if (size_.rows < pos.row + 1) {
        size_.rows = pos.row + 1;
    }
}

void Sheet::DeleteFromCatalogue(Position pos) {
    counter_col_[pos.col].erase(pos.row);
    counter_row_[pos.row].erase(pos.col);

    ResizePrintZone();
}

void Sheet::SetCell(Position pos, std::string text) {
    ThrowOnNoValidPos(pos);
    Resize(pos);

    Cell cell(*this);
    cell.Set(text, pos);

    table_[pos.row][pos.col] = std::make_unique<Cell>(std::move(cell));
    AddToCatalogue(pos);
}

CellInterface* Sheet::MakeDepends(Position pos){
    ThrowOnNoValidPos(pos);
    Resize(pos);
    if(table_[pos.row][pos.col] != nullptr)
        return table_[pos.row][pos.col].get();
    else{
        table_[pos.row][pos.col] = std::make_unique<Cell>(Cell(*this));
        return table_[pos.row][pos.col].get();
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    ThrowOnNoValidPos(pos);
    if (CheckPrintSize(pos)){
        return table_[pos.row][pos.col].get();
    }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    ThrowOnNoValidPos(pos);
    if (CheckPrintSize(pos)){
        return table_[pos.row][pos.col].get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    ThrowOnNoValidPos(pos);
    if (CheckPrintSize(pos)) {
        table_[pos.row][pos.col].release();
        DeleteFromCatalogue(pos);
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintIfCanValue(std::ostream& output, const Cell* cell) const {
    if (cell) {
        std::visit(PrinterValue{ output }, cell->GetValue());
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    //for (const std::vector<std::unique_ptr<Cell>>& row : table_) {
    for (int col = 0; col < size_.rows; ++col) {
        const std::vector<std::unique_ptr<Cell>>& row = table_.at(col);

        if (row.begin() != row.end())
        {
            auto iter = row.begin();
            PrintIfCanValue(output, iter->get());
            for (auto it = iter + 1; it < row.begin() + size_.cols; ++it) {
                output << '\t';
                PrintIfCanValue(output, it->get());
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {

    for (int col = 0; col < size_.rows; ++col) {
        const std::vector<std::unique_ptr<Cell>>& row = table_.at(col);

        if (row.begin() != row.end())
        {
            auto iter = row.begin();
            PrintIfCanText(output, iter->get());
            for (auto it = iter + 1; it < row.begin() + size_.cols; ++it) {
                output << "\t";
                PrintIfCanText(output, it->get());
            }
        }
        output << '\n';
    }
}

void PrinterValue::operator()(double value) const {
    out << value;
}
void PrinterValue::operator()(const std::string& value) const {
    out << value;
}
void PrinterValue::operator()(FormulaError value) const {
    out << value;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::PrintIfCanText(std::ostream& output, const Cell* cell) const {
    if (cell) {
        output << cell->GetText();
    }
}

void Sheet::ResizePrintZone() {
    while (counter_col_[size_.cols - 1].size() == 0 && size_.cols != 0) {
        --size_.cols;
    }
    while (counter_row_[size_.rows - 1].size() == 0 && size_.rows != 0) {
        --size_.rows;
    }
}

const Cell* Sheet::GetWorkerCell(Position pos) const{
    ThrowOnNoValidPos(pos);
    if (CheckPrintSize(pos)){
        return table_[pos.row][pos.col].get();
    }
    return nullptr;
}
