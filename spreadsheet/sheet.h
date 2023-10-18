#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_set>

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

    const Cell* GetWorkerCell(Position pos) const;

    // Можете дополнить ваш класс нужными полями и методами
    CellInterface* MakeDepends(Position);

private:
    std::vector<std::vector<std::unique_ptr<Cell>>> table_;

    std::unordered_map<int, std::unordered_set<int>> counter_col_;
    std::unordered_map<int, std::unordered_set<int>> counter_row_;

    Size size_;

    void AddToCatalogue(Position pos);
    void DeleteFromCatalogue(Position pos);
    void ThrowOnNoValidPos(Position pos) const;
    void Resize(Position pos);
    void ResizePrintZone();
    bool CheckPrintSize(Position pos) const;
    void PrintIfCanText(std::ostream& output, const Cell* cell) const;
    void PrintIfCanValue(std::ostream& output, const Cell* cell) const;

};

struct PrinterValue {
    std::ostream& out;

    explicit PrinterValue(std::ostream& os)
        :out(os)
    {}

    void operator()(double value) const;
    void operator()(const std::string& value) const;
    void operator()(FormulaError value) const;
};
