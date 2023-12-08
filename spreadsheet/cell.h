#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <set>

class Sheet;

class Cell : public CellInterface {
private:
    class Impl;

public:
    Cell(Sheet& sheet);
    ~Cell();

    void CheckCyclicDependencies(std::unique_ptr<Impl> tmp_impl);
    void UpdateReferences();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;
    void InvalidateAllCache(bool flag);

private:
    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;

        virtual std::vector<Position> GetReferencedCells() const;

        virtual bool HasCache();
        virtual void InvalidateCache();
        
        virtual ~Impl() = default;
    };
    
    class EmptyImpl final : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };
    
    class TextImpl final : public Impl {
    public:
        explicit TextImpl(std::string text);
        Value GetValue() const override;
        std::string GetText() const override;
        
    private:
        std::string text_;
    };
    
    class FormulaImpl final : public Impl {
    public:
        explicit FormulaImpl(std::string text, SheetInterface& sheet_interface);
        Value GetValue() const override;
        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

        bool HasCache() override;
        void InvalidateCache() override;
        
    private:
        mutable std::optional<FormulaInterface::Value> cache_;
        std::unique_ptr<FormulaInterface> formula_interface_;
        SheetInterface& sheet_;
    };

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;

    std::set<Cell*> dependent_cells_;
    std::set<Cell*> referenced_cells_;
};