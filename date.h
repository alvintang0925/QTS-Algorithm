#include <string>
#include <ctime>
class Date {
public:
    tm date = { 0 };
    int slide_number;
    int train_range;
    std::string getYear();
    std::string getQ();
    std::string getMon();
    std::string getDate();
    Date getRangeEnd(int);
    void slide();
    void slide(int);
    bool operator>=(Date&);
};



std::string Date::getYear() {
    return std::to_string((this->date.tm_year) + 1900);
}

std::string Date::getQ() {
    return "Q" + std::to_string((this->date.tm_mon / 3) + 1);
}

std::string Date::getMon() {
    if (this->date.tm_mon > 8) {
        return std::to_string((this->date.tm_mon) + 1);
    }
    else {
        return "0" + std::to_string((this->date.tm_mon) + 1);
    }
}

std::string Date::getDate(){
    return this -> getYear() + this -> getMon();
}

Date Date::getRangeEnd(int range) {
    tm temp = this->date;
    temp.tm_mon += range;
    time_t temp2 = mktime(&temp);
    tm* temp3 = localtime(&temp2);
    Date temp4;
    temp4.date = *temp3;
    return temp4;
}

void Date::slide() {
    tm temp = this->date;
    temp.tm_mon += this->slide_number;
    time_t temp2 = mktime(&temp);
    tm* temp3 = localtime(&temp2);
    this->date = *temp3;
}

void Date::slide(int range) {
    tm temp = this->date;
    temp.tm_mon += range;
    time_t temp2 = mktime(&temp);
    tm* temp3 = localtime(&temp2);
    this->date = *temp3;
}

bool Date::operator>=(Date& that){
    
    if (this -> date.tm_year > that.date.tm_year) {
        return true;
    }
    if ((this -> date.tm_year == that.date.tm_year) && (this -> date.tm_mon >= that.date.tm_mon)) {
        return true;
    }
    return false;
}
