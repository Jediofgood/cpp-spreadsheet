#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

#include "sheet.h"
#include "common.h"

using namespace std::literals;


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    :impl_(std::make_unique<EmptyImpl>()), sheet_(&sheet)
{

}

Cell::Cell(Cell&& other){
    //impl_ = std::move(other.impl_);
    impl_.swap(other.impl_);
    sheet_ = other.sheet_;
    depends_ = other.depends_;
    used_in_ = other.used_in_;
}

Cell::~Cell() {
    impl_.release();
}

void Cell::ChangeDepends(Position pos){
    //???
    FormulaImpl* impl = dynamic_cast<FormulaImpl*>(impl_.get());
    ///
    std::vector<Position> cells_list = impl->formula_.get()->GetReferencedCells();
    for(Position pos : cells_list){
        //depends_.push_back(cell);

        Cell* cell = dynamic_cast<Cell*>(sheet_->MakeDepends(pos));
        depends_.push_back(pos);
        cell->used_in_.push_back(pos);
    }
}

void Cell::DeleteHash(){

    if(impl_.get()->DeleteHash()){
        for(Position pos : used_in_){
            Cell* cell = dynamic_cast<Cell*>(sheet_->GetCell(pos));
            cell->DeleteHash();
        }
    }
    return;
}

//void Cell::Set(std::string text, Position pos){
//    if(IsCycle(pos)){
//        throw CircularDependencyException("");
//    }
//    Set(text);
//}

void Cell::Set(std::string text){
    return;
}

void Cell::Set(std::string text, Position pos) {
    if(impl_){
        Clear();
    }

    if(text.size() == 0){
        return;
    }
    else if(text.at(0) == FORMULA_SIGN && text.size() > 1){

        std::unique_ptr<Impl> impl = std::make_unique<FormulaImpl>(FormulaImpl(text.substr(1), sheet_));
        if(IsCycle(pos, impl)){
            throw CircularDependencyException("");
        }
        impl_ = std::move(impl);

        ChangeDepends(pos);

        return;
    } else {
        impl_ = std::make_unique<TextImpl>(TextImpl(text));
    }
}

bool Cell::IsCycle(const Position pos) const{
    const std::vector<Position> dep = impl_->GetReferencedCells();

    for(const Position d_pos : dep){
        if(d_pos == pos){
            return true;
        }

        //Мммм
        //const Cell* cell = dynamic_cast<const Cell*>(GetSheet()->GetCell(d_pos));
        const Cell* cell = sheet_->GetWorkerCell(d_pos);

        if(cell != nullptr){
            if(cell->IsCycle(pos)){
                return true;
            }
        }
    }
    return false;
}

bool Cell::IsCycle(const Position pos, std::unique_ptr<Impl>& impl) const{
    const std::vector<Position> dep = impl->GetReferencedCells();

    for(Position d_pos : dep){
        if(d_pos == pos){
            return true;
        }

        //const Cell* cell = dynamic_cast<const Cell*>(GetSheet()->GetCell(d_pos));
        const Cell* cell = sheet_->GetWorkerCell(d_pos);

        if(cell != nullptr){
            if(cell->IsCycle(pos)){
                return true;
            }
        }
    }
    return false;
}

void Cell::Clear() {
    impl_.release();
    impl_ = std::make_unique<EmptyImpl>(EmptyImpl());


    for(Position pos: depends_){
        Cell* cell = dynamic_cast<Cell*>(sheet_->GetCell(pos));
        cell->used_in_.erase(std::find(cell->used_in_.begin(), cell->used_in_.end(), pos));
    }

    depends_.clear();

    DeleteHash();

    for(Position pos : used_in_){
        Cell* cell = dynamic_cast<Cell*>(sheet_->GetCell(pos));
        cell->DeleteHash();
    }
}

std::vector<Position> Cell::GetReferencedCells() const{
    return impl_.get()->GetReferencedCells();
}


Cell::Value Cell::GetValue() const {
    return impl_.get()->GetValue();
}
std::string Cell::GetText() const {
    return impl_.get()->GetText();
}

//Imp;

Cell::TextImpl::TextImpl(std::string text)
    :text_(text)
{
}

Cell::FormulaImpl::FormulaImpl(std::string text, Sheet* sheet)
    //:formula_(std::make_unique<Formula>(text)), sheet_(sheet)
{
    try {
        formula_ = std::make_unique<Formula>(text);
    } catch (std::exception&) {
        throw FormulaException("");
    }
    //formula_ = std::make_unique<Formula>(text);
    sheet_ = (sheet);
}

std::string Cell::EmptyImpl::GetText() const {
    return ""s;
}

Cell::Value Cell::EmptyImpl::GetValue() const{
    return ""s;
}

std::string Cell::TextImpl::GetText() const{
    return text_;
}

Cell::Value Cell::TextImpl::GetValue() const{
    //if(text_[0] == '\''){
    if(text_[0] == ESCAPE_SIGN){
        return text_.substr(1);
    }
    return text_;
}

std::string Cell::FormulaImpl::GetText() const{
    return FORMULA_SIGN + formula_.get()->GetExpression();
}

Cell::Value Cell::FormulaImpl::GetValue() const{
    if(!std::holds_alternative<std::monostate>(hash_)){
        if(std::holds_alternative<double>(hash_)){
            return std::get<double>(hash_);
        }
        //if(std::holds_alternative<FormulaError>(hash_)){
            return std::get<FormulaError>(hash_);
        //}
    }
    else
    {
        std::variant<double, FormulaError> var = formula_.get()->Evaluate(*sheet_);
        if(std::holds_alternative<double>(var)){
            return std::get<double>(var);
        }
        else{
            return std::get<FormulaError>(var);
        }
    }
}

bool Cell::EmptyImpl::DeleteHash(){
    return false;
}

bool Cell::TextImpl::DeleteHash(){
    return false;
}

bool Cell::FormulaImpl::DeleteHash(){
    if(std::holds_alternative<std::monostate>(hash_)){
        return false;
    }
    hash_ = std::monostate();
    return true;
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const{
    return std::vector<Position>();
}
std::vector<Position> Cell::TextImpl::GetReferencedCells() const{
    return std::vector<Position>();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const{
    return formula_.get()->GetReferencedCells();
}

const Sheet* Cell::GetSheet() const{
    return sheet_;
}
