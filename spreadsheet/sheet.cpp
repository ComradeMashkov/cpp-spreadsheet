#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::IsPositionValid(Position pos, const std::string& error_message) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException(error_message);
    }
}

void Sheet::SetCell(Position pos, std::string text) {
    IsPositionValid(pos, "Failed to set a cell: invalid cell position"s);
    
    table_.resize(std::max(pos.row + 1, int(std::size(table_))));
    table_[pos.row].resize(std::max(pos.col + 1, int(std::size(table_[pos.row]))));
        
    if (!table_[pos.row][pos.col]) {
        table_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }
        
    table_[pos.row][pos.col]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    IsPositionValid(pos, "Failed to get a cell: invalid cell position"s);
    
    if (pos.row < int(std::size(table_)) && pos.col < int(std::size(table_[pos.row]))) {
        if (table_[pos.row][pos.col].get()->GetText() == "") {
            return nullptr;
        }
        
        return table_[pos.row][pos.col].get();
    }
    
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    IsPositionValid(pos, "Failed to get a cell: invalid cell position"s);
    
    if (pos.row < int(std::size(table_)) && pos.col < int(std::size(table_[pos.row]))) {
        if (table_[pos.row][pos.col].get()->GetText() == "") {
            return nullptr;
        }
        
        return table_[pos.row][pos.col].get();
    }
    
    return nullptr;
}

const Cell* Sheet::GetRegularCell(Position pos) const {
    IsPositionValid(pos, "Failed to get a cell: invalid cell position"s);
    
    if (pos.row < int(std::size(table_)) && pos.col < int(std::size(table_[pos.row]))) {
        return table_[pos.row][pos.col].get();
    }
    
    return nullptr;
}

Cell* Sheet::GetRegularCell(Position pos) {
    IsPositionValid(pos, "Failed to get a cell: invalid cell position"s);
    
    if (pos.row < int(std::size(table_)) && pos.col < int(std::size(table_[pos.row]))) {
        return table_[pos.row][pos.col].get();
    }
    
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    IsPositionValid(pos, "Failed to clear a cell: invalid cell position"s);
    
    if (pos.row < int(std::size(table_)) && pos.col < int(std::size(table_[pos.row]))) {
        if (table_[pos.row][pos.col]) {
            table_[pos.row][pos.col]->Clear();

            if (!table_[pos.row][pos.col]->IsReferenced()) {
                table_[pos.row][pos.col].reset();
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size result;
    
    for (int row = 0; row < int(std::size(table_)); ++row) {
        for (int col = (int(std::size(table_[row])) - 1); col >= 0; --col) {
            if (table_[row][col]) {
                if (!table_[row][col]->GetText().empty()) {
                    result.rows = std::max(result.rows, row + 1);
                    result.cols = std::max(result.cols, col + 1);
                    break;
                }
            }
        }
    }
    
    return result;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row) {
        for (int col = 0; col < GetPrintableSize().cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            
            if (col < int(std::size(table_[row]))) {
                if (table_[row][col]) {
                    std::visit([&output] (const auto& obj) { output << obj; }, table_[row][col]->GetValue());
                }
            }
        }
        
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row) {
        for (int col = 0; col < GetPrintableSize().cols; ++col) {
            if (col) {
                output << '\t';
            }
            
            if (col < int(std::size(table_[row]))) {
                if (table_[row][col]) {
                    output << table_[row][col]->GetText();
                }
            }
        }
        
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}