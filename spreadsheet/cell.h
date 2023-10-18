#pragma once

#include "common.h"
#include "formula.h"
#include "FormulaAST.h"

class Sheet;

class Cell : public CellInterface {
private:
    class Impl;
    class TextImpl;
    class FormulaImpl;
public:
    Cell(Sheet& sheet);
    ~Cell() override;

    Cell(const Cell& other) = delete;
    Cell(Cell&& other);

    void Set(std::string text, Position pos);
    void Set(std::string text) override;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    //std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;

    bool IsCycle(const Position pos) const;
    bool IsCycle(const Position pos, std::unique_ptr<Impl>& impl) const;

    void ChangeDepends(Position pos);
    void ChangeWhereUsed();
    void DeleteHash();

private:
    class Impl{
        friend class Cell;
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual bool DeleteHash() = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
    };

    class EmptyImpl final : public Impl{
        friend class Cell;
    public:
        Value GetValue() const override;
        std::string GetText() const override;
        bool DeleteHash() override;
        std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl final : public Impl{
        friend class Cell;
    public:
        TextImpl(std::string text);
        Value GetValue() const override;
        std::string GetText() const override;
        bool DeleteHash() override;
        std::vector<Position> GetReferencedCells() const override;
    private:
        std::string text_;
    };

    class FormulaImpl final : public Impl{
        friend class Cell;
    public:
        FormulaImpl(std::string text, Sheet* sheet);
        Value GetValue() const override;
        std::string GetText() const override;
        bool DeleteHash() override;
        std::vector<Position> GetReferencedCells() const override;
    private:
        std::unique_ptr<Formula> formula_;
        Sheet* sheet_ = nullptr;
        Value VariantToVariant(const std::variant<double, FormulaError>& var) const;
        std::variant<std::monostate, double, FormulaError> hash_ = std::monostate();
    };

private:
    std::unique_ptr<Impl> impl_;
    Sheet* sheet_ = nullptr;
    std::vector<Position> depends_{};
    //std::vector<Cell*> used_in_;
    std::vector<Position> used_in_{};


private:
    const Sheet* GetSheet() const;
};
