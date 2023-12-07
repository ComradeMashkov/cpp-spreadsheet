#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    : impl_(std::make_unique<EmptyImpl>())
    , sheet_(sheet) {  
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> tmp_impl;

    if (text.empty()) {
        tmp_impl = std::make_unique<EmptyImpl>();
    }
    else if (text.size() >= 2 && text.at(0) == FORMULA_SIGN) {
        tmp_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    }
    else {
        tmp_impl = std::make_unique<TextImpl>(std::move(text));
    }

    // Cyclic dependencies 
    const Impl& tmp_impl_cyclic = *tmp_impl;
    const auto tmp_referenced_cells = tmp_impl_cyclic.GetReferencedCells();

    if (!tmp_referenced_cells.empty()) {
        std::set<const Cell*> references;
        std::set<const Cell*> entries;
        std::vector<const Cell*> entries_vec;

        for (auto position : tmp_referenced_cells) {
            references.insert(sheet_.GetRegularCell(position));
        }

        entries_vec.push_back(this);

        while (!entries_vec.empty()) {
            const Cell* current = entries_vec.back();
            entries_vec.pop_back();
            entries.insert(current);

            if (references.find(current) == references.end()) {
                for (const Cell* dep : current->dependent_cells_) {
                    if (entries.find(dep) == entries.end()) {
                        entries_vec.push_back(dep);
                    }
                }
            }

            else {
                throw CircularDependencyException("Circular dependency has been evaluated");
            }
        }

        impl_ = std::move(tmp_impl);
    }

    else {
        impl_ = std::move(tmp_impl);
    }
    // Cyclic dependencies 

    // Updating references
    for (Cell* ref : referenced_cells_) {
        ref->dependent_cells_.erase(this);
    }

    referenced_cells_.clear();

    for (const auto& position : impl_->GetReferencedCells()) {
        Cell* ref = sheet_.GetRegularCell(position);

        if (!ref) {
            sheet_.SetCell(position, "");
            ref = sheet_.GetRegularCell(position);
        }

        referenced_cells_.insert(ref);
        ref->dependent_cells_.insert(this);
    }
    // Updating references

    // Cache invalidation
    InvalidateAllCache(true);

}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

bool Cell::IsReferenced() const {
    return !dependent_cells_.empty();
}

void Cell::InvalidateAllCache(bool flag = false) {
    if (impl_->HasCache() || flag) {
        impl_->InvalidateCache();

        for (Cell* dep : dependent_cells_) {
            dep->InvalidateAllCache();
        }
    }
}

bool Cell::Impl::HasCache() {
    return true;
}

void Cell::Impl::InvalidateCache() {

}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}
 
Cell::TextImpl::TextImpl(std::string text) 
    : text_(std::move(text)) {
}

Cell::Value Cell::TextImpl::GetValue() const {
    if (text_.empty()) {
        throw std::logic_error("Cell is empty");
    } 
    else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);
    } 
    return text_;        
}
 
std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet_interface) 
    : formula_interface_(ParseFormula(text.substr(1)))
    , sheet_(sheet_interface) {
}
 
Cell::Value Cell::FormulaImpl::GetValue() const {             
    if (!cache_) {
        cache_ = formula_interface_->Evaluate(sheet_);
    }    

    return std::visit([] (auto& tmp) { return Value(tmp); }, *cache_);
}
 
std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_interface_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_interface_->GetReferencedCells();
}

bool Cell::FormulaImpl::HasCache() {
    return cache_.has_value();
}

void Cell::FormulaImpl::InvalidateCache() {
    cache_.reset();
}