//
//  main.cpp
//  QTS Algorithm
//
//  Created by 唐健恆 on 2021/8/11.
//  Copyright © 2021 Alvin. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <cfloat>
#include "date.h"
#include "portfolio.h"

using namespace std;
using namespace filesystem;

#define EXPNUMBER 50
#define ITERNUMBER 10000
#define PARTICLENUMBER 10
#define FUNDS 10000000.0
#define DELTA 0.0004
#define QTSTYPE 2 //QTS 0, GQTS 1, GNQTS 2
#define TRENDLINETYPE 0 //linear 0, quadratic 1
#define MODE "train" //train, test, exhaustive, B&H, specify

string FILE_DIR = "0813_0.0004_GN_LN";
string DATA_FILE_DIR = "DJI_30";


string* vectorToArray(vector<string> &data_vector){
    string *d = new string[data_vector.size()];
    for(int j = 0; j < data_vector.size(); j++){
        d[j] = data_vector[j];
    }
    return d;
}

string** vectorToArray(vector<vector<string>> &data_vector){
    string **d = new string*[data_vector.size()];
    for(int j = 0; j < data_vector.size(); j++){
        d[j] = vectorToArray(data_vector[j]);
    }
    return d;
}

string*** vectorToArray(vector<vector<vector<string>>> &data_vector){
    string ***d = new string**[data_vector.size()];
    for(int j = 0; j < data_vector.size(); j++){
        d[j] = vectorToArray(data_vector[j]);
    }
    return d;
}

bool readData(string filename, vector<vector<string>> &data_vector, int &size, int &day_number) {
    cout << filename << endl;
    ifstream inFile(filename, ios::in);
    string line;
    vector< vector<string> > temp;

    if (!inFile) {
        cout << "Open file failed!" << endl;
        exit(1);
    }
    while (getline(inFile, line)) {
        istringstream delime(line);
        string s;
        vector<string> line_data;
        while (getline(delime, s, ',')) {
            if (s != "\r") {
                s.erase(remove(s.begin(), s.end(), '\r'), s.end());
                line_data.push_back(s);
            }
        }
        temp.push_back(line_data);
    }
    inFile.close();
    
    size = temp[0].size();
    day_number = temp.size() - 1;
    data_vector = temp;

    return true;
}

void createStock(Stock* stock_list, int size, int day_number, string **data) {
    for (int j = 0; j < size; j++) {
        stock_list[j].idx = j;
        stock_list[j].init(day_number);
        stock_list[j].company_name = data[0][j];
        for (int k = 1; k < day_number + 1; k++) {
            stock_list[j].price_list[k - 1] = atof(data[k][j].c_str());
        }
    }
}



void outputFile(Portfolio& portfolio, string file_name) {
    ofstream outfile;
    outfile.open(file_name, ios::out);
    outfile << setprecision(15);

    outfile << "Iteration," << ITERNUMBER << endl;
    outfile << "Element number," << PARTICLENUMBER << endl;
    outfile << "Delta," << DELTA << endl;
    outfile << "Exp times," << EXPNUMBER << endl << endl;
    
    outfile << "Init funds," << portfolio.funds << endl;
    outfile << "Final funds," << portfolio.total_money[portfolio.day_number - 1] << endl;
    outfile << "Real award," << portfolio.total_money[portfolio.day_number - 1] - portfolio.funds << endl << endl;
    
    outfile << "MMD," << portfolio.MDD << endl;
    outfile << "PF," << portfolio.PF << endl << endl;
    
    outfile << "m," << portfolio.m << endl;
    outfile << "Daily_risk," << portfolio.daily_risk << endl;
    outfile << "Trend," << portfolio.trend << endl << endl;
    
    if (TRENDLINETYPE == 0) {
        portfolio.countQuadraticYLine();
        double sum = 0;
        for (int k = 0; k < portfolio.day_number; k++) {
            double Y;
            Y = portfolio.getQuadraticY(k + 1);
            sum += (portfolio.total_money[k] - Y) * (portfolio.total_money[k] - Y);
        }
        double c = (portfolio.getQuadraticY(portfolio.day_number) - portfolio.getQuadraticY(1)) / (portfolio.day_number - 1);
        double d = sqrt(sum / (portfolio.day_number));
        
        outfile << "Quadratic trend line," << portfolio.a << "x^2 + " << portfolio.b << "x + " << FUNDS << endl << endl;
        outfile << "Quadratic m," << c << endl;
        outfile << "Quadratic daily risk," << d << endl;
        if(c < 0){
            outfile << "Quadratic trend," << c * d << endl << endl;
        }else{
            outfile << "Quadratic trend," << c / d << endl << endl;
        }
    }
    else {
        outfile << "Quadratic trend line," << portfolio.a << "x^2 + " << portfolio.b << "x + " << FUNDS << endl << endl;
        double x = 0;
        double y = 1;
        double sum = 0;
        for (int k = 0; k < portfolio.day_number - 1; k++) {
            x += (k + 2) * (portfolio.total_money[k + 1] - portfolio.funds);
            y += (k + 2) * (k + 2);
        }

        double c = x / y;
        for (int k = 0; k < portfolio.day_number; k++) {
            double Y;
            Y = c * (k + 1) + portfolio.funds;
            sum += (portfolio.total_money[k] - Y) * (portfolio.total_money[k] - Y);
        }
        double d = sqrt(sum / (portfolio.day_number));

        outfile << "Linear m," << c << endl;
        outfile << "Linear daily risk," << d << endl;
        if(c < 0){
            outfile << "Linear trend," << c * d << endl << endl;
        }else{
            outfile << "Linear trend," << c / d << endl << endl;
        }
    }

    outfile << "Best generation," << portfolio.gen << endl;
    outfile << "Best experiment," << portfolio.exp << endl;
    outfile << "Best answer times," << portfolio.answer_counter << endl << endl;

    outfile << "Stock number," << portfolio.stock_number << endl;
    outfile << "Stock#,";
    for (int j = 0; j < portfolio.stock_number; j++) {
        outfile << portfolio.constituent_stocks[portfolio.stock_id_list[j]].company_name << ",";
    }
    outfile << endl;

    outfile << "Number,";
    for (int j = 0; j < portfolio.stock_number; j++) {
        outfile << portfolio.investment_number[j] << ",";
    }
    outfile << endl;

    outfile << "Distribue funds,";
    for (int j = 0; j < portfolio.stock_number; j++) {
        outfile << portfolio.getDMoney() << ",";
    }
    outfile << endl;

    outfile << "Remain funds,";
    for (int j = 0; j < portfolio.stock_number; j++) {
        outfile << portfolio.remain_fund[j] << ",";
    }
    outfile << endl;

    for (int j = 0; j < portfolio.day_number; j++) {
        outfile << "fs( " << j+1 << "),";
        for (int k = 0; k < portfolio.stock_number; k++) {
            outfile << (portfolio.constituent_stocks[portfolio.stock_id_list[k]].price_list[j] * portfolio.investment_number[k]) + portfolio.remain_fund[k] << ",";
        }
        outfile << portfolio.total_money[j] << endl;
    }
    outfile << endl;
    outfile.close();
}

void recordCPUTime(double START, double END, string file_name){
    double total_time = (END - START) / CLOCKS_PER_SEC;
    ofstream outfile_time;
    outfile_time.open(file_name, ios::out);
    outfile_time << "total time: " << total_time << " sec" << endl;
    outfile_time.close();
}

void createDir(string file_dir, string type, string mode){
    
    create_directory(file_dir);
    create_directory(file_dir + "/" + type);
    create_directory(file_dir + "/" + type + "/" + mode);
    
}

void preSet(string mode, Date& current_date, Date& finish_date, int SLIDETYPE, string& TYPE) {
    string STARTYEAR;
    string STARTMONTH;
    string ENDYEAR;
    string ENDMONTH;
    int slide_number;
    int train_range;

    switch (SLIDETYPE) {
    case 0:
        STARTYEAR = "2009";
        STARTMONTH = "12";
        ENDYEAR = "2019";
        ENDMONTH = "11";
        TYPE = "M2M";
        train_range = 1;
        slide_number = 1;
        break;
    case 1:
        STARTYEAR = "2009";
        STARTMONTH = "10";
        ENDYEAR = "2019";
        ENDMONTH = "9";
        TYPE = "Q2M";
        train_range = 3;
        slide_number = 1;
        break;
    case 2:
        STARTYEAR = "2009";
        STARTMONTH = "10";
        ENDYEAR = "2019";
        ENDMONTH = "7";
        TYPE = "Q2Q";
        train_range = 3;
        slide_number = 3;
        break;
    case 3:
        STARTYEAR = "2009";
        STARTMONTH = "7";
        ENDYEAR = "2019";
        ENDMONTH = "6";
        TYPE = "H2M";
        train_range = 6;
        slide_number = 1;
        break;
    case 4:
        STARTYEAR = "2009";
        STARTMONTH = "7";
        ENDYEAR = "2019";
        ENDMONTH = "4";
        TYPE = "H2Q";
        train_range = 6;
        slide_number = 3;
        break;
    case 5:
        STARTYEAR = "2009";
        STARTMONTH = "7";
        ENDYEAR = "2019";
        ENDMONTH = "1";
        TYPE = "H2H";
        train_range = 6;
        slide_number = 6;
        break;
    case 6:
        STARTYEAR = "2009";
        STARTMONTH = "2";
        ENDYEAR = "2018";
        ENDMONTH = "12";
        TYPE = "Y2M";
        train_range = 12;
        slide_number = 1;
        break;
    case 7:
        STARTYEAR = "2009";
        STARTMONTH = "1";
        ENDYEAR = "2018";
        ENDMONTH = "10";
        TYPE = "Y2Q";
        train_range = 12;
        slide_number = 3;
        break;
    case 8:
        STARTYEAR = "2009";
        STARTMONTH = "7";
        ENDYEAR = "2018";
        ENDMONTH = "7";
        TYPE = "Y2H";
        train_range = 12;
        slide_number = 6;
        break;
    case 9:
        STARTYEAR = "2009";
        STARTMONTH = "1";
        ENDYEAR = "2018";
        ENDMONTH = "1";
        TYPE = "Y2Y";
        train_range = 12;
        slide_number = 12;
        break;
    case 10:
        STARTYEAR = "2009";
        STARTMONTH = "1";
        ENDYEAR = "2018";
        ENDMONTH = "12";
        TYPE = "M#";
        if(mode == "test"){
            train_range = 12;
        }else{
            train_range = 1;
        }
        slide_number = 1;
        break;
    case 11:
        STARTYEAR = "2009";
        STARTMONTH = "1";
        ENDYEAR = "2018";
        ENDMONTH = "10";
        TYPE = "Q#";
        if(mode == "test"){
            train_range = 12;
        }else{
            train_range = 3;
        }
        slide_number = 3;
        break;
    case 12:
        STARTYEAR = "2009";
        STARTMONTH = "1";
        ENDYEAR = "2018";
        ENDMONTH = "7";
        TYPE = "H#";
        if(mode == "test"){
            train_range = 12;
        }else{
            train_range = 6;
        }
        slide_number = 6;
        break;
    }

    current_date.date.tm_year = atoi(STARTYEAR.c_str()) - 1900;
    current_date.date.tm_mon = atoi(STARTMONTH.c_str()) - 1;
    current_date.date.tm_mday = 1;
    current_date.slide_number = slide_number;
    current_date.train_range = train_range;
    
    finish_date.date.tm_year = atoi(ENDYEAR.c_str()) - 1900;
    finish_date.date.tm_mon = atoi(ENDMONTH.c_str()) - 1;
    finish_date.date.tm_mday = 1;
    finish_date.slide_number = slide_number;
    finish_date.train_range = train_range;
    
    if(mode == "test"){
        current_date.slide(train_range);
        finish_date.slide(train_range);
    }
}

void setWindow(string mode, string &start_date, string &end_date, int &start_index, int &end_index, Date current_date, Date finish_data, string* data_copy, string **data, int day_number, int &range_day_number){
    bool sw = false;//判斷是否找到開始日期
    int flag = 0;//判斷是否找到結束月份
    for(int j = 0; j < day_number; j++){
        string temp1 = current_date.getYear() + "-" + current_date.getMon();
        string temp2;
        if(mode == "test"){
            temp2 = current_date.getRangeEnd(current_date.slide_number - 1).getYear() + "-" + current_date.getRangeEnd(current_date.slide_number - 1).getMon();
        }else{
            temp2 = current_date.getRangeEnd(current_date.train_range - 1).getYear() + "-" + current_date.getRangeEnd(current_date.train_range - 1).getMon();
        }
        
        if(!sw && temp1 == data_copy[j]){
            start_date = data[j+1][0];
            start_index = j + 1;
            sw = true;
        }
        
        if(sw){
            if(flag == 1 && temp2 != data_copy[j]){
                end_date = data[j][0];
                end_index = j;
                flag = 0;
                break;
            }
            if(flag == 0 && temp2 == data_copy[j]){
                flag = 1;
            }
        }
    }
    
    if(flag == 1){
        end_date = data[day_number][0];
        end_index = day_number;
    }
    range_day_number = end_index - start_index + 1;
}

void copyData(string *data_copy, string **data, int day_number){
    for(int j = 0; j < day_number; j++){
        data_copy[j] = data[j+1][0];
        data_copy[j].resize(7);
    }
}

void releaseData(vector<vector<string>> &price_data_vector, vector<vector<vector<string>>> &RSI_data_vector, string **price_data, string ***RSI_data, string *data_copy){
    for(int j = 0; j < price_data_vector.size(); j++){
        delete[] price_data[j];
        price_data_vector[j].clear();
    }
    delete[] price_data;
    price_data_vector.clear();
    
    for(int j = 0; j < RSI_data_vector.size(); j++){
        for(int k = 0; k < RSI_data_vector[j].size(); k++){
            delete[] RSI_data[j][k];
            RSI_data_vector[j][k].clear();
        }
        delete[] RSI_data[j];
        RSI_data_vector[j].clear();
    }

    delete[] RSI_data;
    RSI_data_vector.clear();
    delete[] data_copy;
}

string getPriceFilename(Date current_date, string mode, int SLIDETYPE, string TYPE) {
    Date temp;
    if(mode == "train"){
        switch (SLIDETYPE) {
        case 0://M2M
            TYPE = "M2M";
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "(" + current_date.getYear() + " Q1).csv";
            break;
        case 1://Q2M
            TYPE = "Q2M";
            temp = current_date.getRangeEnd(2);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "~" + temp.getYear() + "_" + temp.getMon() + "(" + current_date.getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "-" + temp.getMon() + "(" + current_date.getYear() + " Q1).csv";
            }
            break;
        case 2://Q2Q
            TYPE = "Q2Q";
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "(" + current_date.getYear() + " Q1).csv";
            break;
        case 3://H2M
            TYPE = "H2M";
            temp = current_date.getRangeEnd(5);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "~" + temp.getYear() + "_" + temp.getMon() + "(" + current_date.getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "-" + temp.getMon() + "(" + current_date.getYear() + " Q1).csv";
            }
            break;
        case 4://H2Q
            TYPE = "H2Q";
            temp = current_date.getRangeEnd(5);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "~" + temp.getYear() + "_" + temp.getQ() + "(" + current_date.getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "-" + temp.getQ() + "(" + current_date.getYear() + " Q1).csv";
            }
            break;
        case 5://H2H
            TYPE = "H2H";
            temp = current_date.getRangeEnd(5);
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "-" + temp.getQ() + "(" + current_date.getYear() + " Q1).csv";
            break;
        case 6://Y2M
            TYPE = "Y2M";
            temp = current_date.getRangeEnd(11);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "~" + temp.getYear() + "_" + temp.getMon() + "(" + current_date.getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "(" + current_date.getYear() + " Q1).csv";
            }
            break;
        case 7://Y2Q
            TYPE = "Y2Q";
            temp = current_date.getRangeEnd(11);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "~" + temp.getYear() + "_" + temp.getQ() + "(" + current_date.getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "(" + current_date.getYear() + " Q1).csv";
            }
            break;
        case 8://Y2H
            TYPE = "Y2H";
            temp = current_date.getRangeEnd(11);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "~" + temp.getYear() + "_" + temp.getQ() + "(" + current_date.getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "(" + current_date.getYear() + " Q1).csv";
            }
            break;
        case 9://Y2Y
            TYPE = "Y2Y";
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "(" + current_date.getYear() + " Q1).csv";
            break;
        case 10://M#
            TYPE = "M#";
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + to_string(atoi(current_date.getYear().c_str()) - 1) + "_" + current_date.getMon() + "(" + to_string(atoi(current_date.getYear().c_str()) - 1) + " Q1).csv";
            break;
        case 11://Q#
            TYPE = "Q#";
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + to_string(atoi(current_date.getYear().c_str()) - 1) + "_" + current_date.getQ() + "(" + to_string(atoi(current_date.getYear().c_str()) - 1) + " Q1).csv";
            break;
        case 12://H#
            TYPE = "H#";
            temp = current_date.getRangeEnd(5);
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + to_string(atoi(current_date.getYear().c_str()) - 1) + "_" + current_date.getQ() + "-" + temp.getQ() + "(" + to_string(atoi(current_date.getYear().c_str()) - 1) + " Q1).csv";
            break;
        }
    }else{
        switch (SLIDETYPE) {
        case 0://M2M
            TYPE = "M2M";
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            break;
        case 1://Q2M
            TYPE = "Q2M";
            temp = current_date.getRangeEnd(2);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "~" + temp.getYear() + "_" + temp.getMon() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "-" + temp.getMon() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            break;
        case 2://Q2Q
            TYPE = "Q2Q";
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            break;
        case 3://H2M
            TYPE = "H2M";
            temp = current_date.getRangeEnd(5);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "~" + temp.getYear() + "_" + temp.getMon() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "-" + temp.getMon() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            break;
        case 4://H2Q
            TYPE = "H2Q";
            temp = current_date.getRangeEnd(5);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "~" + temp.getYear() + "_" + temp.getQ() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "-" + temp.getQ() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            break;
        case 5://H2H
            TYPE = "H2H";
            temp = current_date.getRangeEnd(5);
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "-" + temp.getQ() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            break;
        case 6://Y2M
            TYPE = "Y2M";
            temp = current_date.getRangeEnd(11);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + "~" + temp.getYear() + "_" + temp.getMon() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            break;
        case 7://Y2Q
            TYPE = "Y2Q";
            temp = current_date.getRangeEnd(11);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "~" + temp.getYear() + "_" + temp.getQ() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            break;
        case 8://Y2H
            TYPE = "Y2H";
            temp = current_date.getRangeEnd(11);
            if (current_date.getYear() != temp.getYear()) {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getQ() + "~" + temp.getYear() + "_" + temp.getQ() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            else {
                return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + current_date.getYear() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            }
            break;
        case 9://Y2Y
            TYPE = "Y2Y";
            return "DJI_30/Y2Y/" + mode + "_" + current_date.getYear() + "(" + current_date.getRangeEnd(-1 * current_date.train_range).getYear() + " Q1).csv";
            break;
        case 10://M#
            TYPE = "M#";
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + to_string(atoi(current_date.getYear().c_str()) - 1) + "_" + current_date.getMon() + "(" + to_string(atoi(current_date.getRangeEnd(-1 * current_date.train_range).getYear().c_str()) - 1) + " Q1).csv";
            break;
        case 11://Q#
            TYPE = "Q#";
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + to_string(atoi(current_date.getYear().c_str()) - 1) + "_" + current_date.getQ() + "(" + to_string(atoi(current_date.getRangeEnd(-1 * current_date.train_range).getYear().c_str()) - 1) + " Q1).csv";
            break;
        case 12://H#
            TYPE = "H#";
            temp = current_date.getRangeEnd(5);
            return DATA_FILE_DIR + "/" + TYPE + "/" + mode + "_" + to_string(atoi(current_date.getYear().c_str()) - 1) + "_" + current_date.getQ() + "-" + temp.getQ() + "(" + to_string(atoi(current_date.getRangeEnd(-1 * current_date.train_range).getYear().c_str()) - 1) + " Q1).csv";
            break;
        }
    }
    return "";
}

string getOutputFilePath(Date current_date, string mode, string file_dir, string type){
    return file_dir + "/" + type + "/" + mode + "/" + mode + "_" + current_date.getYear() + "_" + current_date.getMon() + ".csv";
}

void initial(double *b, int size) {
    for (int j = 0; j < size; j++) {
        b[j] = 0.5;
    }
}

void initPortfolio(Portfolio *p, int size, int day_number, Stock *stock_list){
    for(int j = 0; j < PARTICLENUMBER; j++){
        p[j].init(size, day_number, FUNDS, stock_list);
    }
}

void initPortfolio(Portfolio *p){
    for(int j = 0; j < PARTICLENUMBER; j++){
        p[j].init();
    }
}

void genPortfolio(Portfolio* portfolio_list, Stock* stock_list, int portfolio_number, double *beta_, int n, int i) {
    for (int j = 0; j < portfolio_number; j++) {
        portfolio_list[j].exp = n + 1;
        portfolio_list[j].gen = i + 1;
        portfolio_list[j].stock_number = 0;
        for (int k = 0; k < portfolio_list[j].size; k++) {
            double r = (double)rand() / (double)RAND_MAX;
            if (r > beta_[k]) {
                portfolio_list[j].data[k] = 0;
            }
            else {
                portfolio_list[j].data[k] = 1;
            }
        }
        
        for(int k = 0; k < portfolio_list[j].size; k++){
            if(portfolio_list[j].data[k] == 1){
                portfolio_list[j].stock_id_list[portfolio_list[j].stock_number] = k;
                portfolio_list[j].stock_number++;
            }
        }
    }
}

void gen_testPortfolio(Portfolio* portfolio_list, Stock* stock_list, int portfolio_number, string **data, string *myTrainData, int myTrainData_size) {
    for (int j = 0; j < portfolio_number; j++) {
        for(int k = 0; k < portfolio_list[j].size; k++){
            for(int h = 0; h < myTrainData_size; h++){
                if(data[0][k+1] == myTrainData[h]){
                    portfolio_list[j].data[k] = 1;
                    portfolio_list[j].stock_id_list[portfolio_list[j].stock_number] = k;
                    portfolio_list[j].stock_number++;
                    break;
                }
            }
        }
    }
}

void gen_testPortfolio(Portfolio* portfolio_list, Stock* stock_list, int portfolio_number, string **data, Portfolio &temp_portfolio) {
    for (int j = 0; j < portfolio_number; j++) {
        for(int k = 0; k < portfolio_list[j].size; k++){
            if(k == temp_portfolio.stock_id_list[portfolio_list[j].stock_number] && portfolio_list[j].stock_number < temp_portfolio.stock_number){
                portfolio_list[j].data[k] = 1;
                portfolio_list[j].stock_id_list[portfolio_list[j].stock_number] = k;
                portfolio_list[j].stock_number++;
            }else{
                portfolio_list[j].data[k] = 0;
            }
            
        }
    }
}


void capitalLevel(Portfolio* portfolio_list, int portfolio_number) {
    for (int j = 0; j < portfolio_number; j++) {
        for (int k = 0; k < portfolio_list[j].stock_number; k++) {
            portfolio_list[j].investment_number[k] = portfolio_list[j].getDMoney() / portfolio_list[j].constituent_stocks[portfolio_list[j].stock_id_list[k]].price_list[0];
            portfolio_list[j].remain_fund[k] = portfolio_list[j].getDMoney() - (portfolio_list[j].investment_number[k] * portfolio_list[j].constituent_stocks[portfolio_list[j].stock_id_list[k]].price_list[0]);
        }
//        portfolio_list[j].total_money[0] = funds;
        for (int k = 0; k < portfolio_list[j].day_number; k++) {
            portfolio_list[j].total_money[k] = portfolio_list[j].getRemainMoney();
            for (int h = 0; h < portfolio_list[j].stock_number; h++) {
                portfolio_list[j].total_money[k] += portfolio_list[j].investment_number[h] * portfolio_list[j].constituent_stocks[portfolio_list[j].stock_id_list[h]].price_list[k];
            }
            if(portfolio_list[j].total_money[k] > portfolio_list[j].capital_highest_point){
                portfolio_list[j].capital_highest_point = portfolio_list[j].total_money[k];
            }
            double DD = (portfolio_list[j].capital_highest_point - portfolio_list[j].total_money[k]) / portfolio_list[j].capital_highest_point;
            if(DD > portfolio_list[j].MDD){
                portfolio_list[j].MDD = DD;
            }
        }
    }
}

void capitalLevel(Portfolio* portfolio_list, int portfolio_number, Portfolio &result) {
    for (int j = 0; j < portfolio_number; j++) {
        for (int k = 0; k < portfolio_list[0].stock_number; k++) {
            portfolio_list[0].investment_number[k] = result.investment_number[k];
            portfolio_list[0].remain_fund[k] = result.remain_fund[k];
        }
//        portfolio_list[j].total_money[0] = funds;
        for (int k = 0; k < portfolio_list[j].day_number; k++) {
            portfolio_list[j].total_money[k] = portfolio_list[j].getRemainMoney();
            for (int h = 0; h < portfolio_list[j].stock_number; h++) {
                portfolio_list[j].total_money[k] += portfolio_list[j].investment_number[h] * portfolio_list[j].constituent_stocks[portfolio_list[j].stock_id_list[h]].price_list[k + 1];
            }
            if(portfolio_list[j].total_money[k] > portfolio_list[j].capital_highest_point){
                portfolio_list[j].capital_highest_point = portfolio_list[j].total_money[k];
            }
            double DD = (portfolio_list[j].capital_highest_point - portfolio_list[j].total_money[k]) / portfolio_list[j].capital_highest_point;
            if(DD > portfolio_list[j].MDD){
                portfolio_list[j].MDD = DD;
            }
        }
    }
}

void countTrend(Portfolio* portfolio_list, int porfolio_number, double funds) {
    
    for (int j = 0; j < porfolio_number; j++) {
        double sum = 0;
        if (TRENDLINETYPE == 0) {
            //portfolio_list[j].countQuadraticYLine();
            double x = 0;
            double y = 0;
            for (int k = 0; k < portfolio_list[j].day_number; k++) {
                x += (k + 1) * (portfolio_list[j].total_money[k] - funds);
                y += (k + 1) * (k + 1);
            }
            if (portfolio_list[j].stock_number != 0) {
                portfolio_list[j].m = x / y;
            }
            for (int k = 0; k < portfolio_list[j].day_number; k++) {
                double Y;
                Y = portfolio_list[j].getNormalY(k + 1);
                sum += (portfolio_list[j].total_money[k] - Y) * (portfolio_list[j].total_money[k] - Y);
            }
        }
        else if (TRENDLINETYPE == 1) {
            portfolio_list[j].countQuadraticYLine();
            for (int k = 0; k < portfolio_list[j].day_number; k++) {
                double Y;
                Y = portfolio_list[j].getQuadraticY(k + 1);
                sum += (portfolio_list[j].total_money[k] - Y) * (portfolio_list[j].total_money[k] - Y);
            }
            portfolio_list[j].m = (portfolio_list[j].getQuadraticY(portfolio_list[j].day_number) - portfolio_list[j].getQuadraticY(1)) / (portfolio_list[j].day_number - 1);
        }
        
        portfolio_list[j].daily_risk = sqrt(sum / (portfolio_list[j].day_number));

        if (portfolio_list[j].m < 0) {
            portfolio_list[j].trend = portfolio_list[j].m * portfolio_list[j].daily_risk;
        }
        else {
            portfolio_list[j].trend = portfolio_list[j].m / portfolio_list[j].daily_risk;
        }
    }
}

void recordGAnswer(Portfolio* portfolio_list, Portfolio& gBest, Portfolio& gWorst, Portfolio& pBest, Portfolio& pWorst) {
    pBest.copyP(portfolio_list[0]);
    pWorst.copyP(portfolio_list[PARTICLENUMBER - 1]);
    for (int j = 0; j < PARTICLENUMBER; j++) {
        if (pBest.trend < portfolio_list[j].trend) {
            pBest.copyP(portfolio_list[j]);
        }
        if (pWorst.trend > portfolio_list[j].trend) {
            pWorst.copyP(portfolio_list[j]);
        }
    }
    
    if (gBest.trend < pBest.trend) {
        gBest.copyP(pBest);
    }
    
    if (gWorst.trend > pWorst.trend) {
        gWorst.copyP(pWorst);
    }
}

void adjBeta(Portfolio& best, Portfolio& worst, double *beta_) {
    for (int j = 0; j < best.size; j++) {
        if (QTSTYPE == 2) {
            if (best.data[j] > worst.data[j]) {
                if (beta_[j] < 0.5) {
                    beta_[j] = 1 - beta_[j];
                }
                beta_[j] += DELTA;
            }
            else if (best.data[j] < worst.data[j]) {
                if (beta_[j] > 0.5) {
                    beta_[j] = 1 - beta_[j];
                }
                beta_[j] -= DELTA;
            }
        }
        else if (QTSTYPE == 1) {
            if (best.data[j] > worst.data[j]) {
                beta_[j] += DELTA;
            }
            else if (best.data[j] < worst.data[j]) {
                beta_[j] -= DELTA;
            }
        }
        else {
            if (best.data[j] > worst.data[j]) {
                beta_[j] += DELTA;
            }
            else if (best.data[j] < worst.data[j]) {
                beta_[j] -= DELTA;
            }
        }
    }
}

void recordExpAnswer(Portfolio& expBest, Portfolio& gBest) {
    if (expBest.trend < gBest.trend) {
        expBest.copyP(gBest);
        expBest.answer_counter = 1;
    }
    else if (expBest.trend == gBest.trend) {
        expBest.answer_counter++;
    }
}

void releaseArray(string **a, int length){
    for(int j = 0; j < length; j++){
        delete[] a[j];
    }
    delete [] a;
}

void releaseVector(vector<vector<string>> v){
    for(int j = 0; j < v.size(); j++){
        v[j].clear();
    }
    v.clear();
}



void startTrain(Portfolio &result, Stock *stock_list, int size, int range_day_number){
    
    double *beta_ = new double[size];
    Portfolio expBest(size, range_day_number, FUNDS, stock_list);
    Portfolio gBest(size, range_day_number, FUNDS, stock_list);
    Portfolio gWorst(size, range_day_number, FUNDS, stock_list);
    Portfolio pBest(size, range_day_number, FUNDS, stock_list);
    Portfolio pWorst(size, range_day_number, FUNDS, stock_list);
    Portfolio* portfolio_list = new Portfolio[PARTICLENUMBER];
    initPortfolio(portfolio_list, size, range_day_number, stock_list);
    
    for(int n = 0; n < EXPNUMBER; n++){
        cout << "___" << n << "___" << endl;
        gBest.init();
        gWorst.init();
        gBest.trend = 0;
        gWorst.trend = DBL_MAX;
        initial(beta_, size);
        for(int i = 0; i < ITERNUMBER; i++){
            pBest.init();
            pWorst.init();
            initPortfolio(portfolio_list);
            genPortfolio(portfolio_list, stock_list, PARTICLENUMBER, beta_, n, i);
            capitalLevel(portfolio_list, PARTICLENUMBER);
            countTrend(portfolio_list, PARTICLENUMBER, FUNDS);
            recordGAnswer(portfolio_list, gBest, gWorst, pBest, pWorst);
            adjBeta(gBest, pWorst, beta_);
        }
        recordExpAnswer(expBest, gBest);
    }
    
    expBest.print();
    delete[] portfolio_list;
    delete[] beta_;
    result.copyP(expBest);

}

void startTest(Portfolio &result, Date current_date, string company_name, string type, int range_day_number){
    
}

int main(int argc, const char * argv[]) {
    
    for(int s = 12; s >= 0; s--){
        
        srand(114);
        double START, END;
        START = clock();
        
        Date current_date;
        Date finish_date;
        string TYPE;
        string** data;
        string temp;
        vector<vector<string>> data_vector;
        int size;
        int day_number;
        
        preSet(MODE, current_date, finish_date, s, TYPE);
        
        createDir(FILE_DIR, TYPE, MODE);
        cout << TYPE << endl;
        
        do{
            readData(getPriceFilename(current_date, MODE, s, TYPE), data_vector, size, day_number);
            data = vectorToArray(data_vector);
            
            cout << "______" << TYPE << " : " << current_date.getDate() << " - " << current_date.getRangeEnd(current_date.train_range).getDate() << "______" << endl;
            
            Stock* stock_list = new Stock[size];
            createStock(stock_list, size, day_number, data);
            
            Portfolio result(size, day_number, FUNDS, stock_list);
            if(MODE == "train"){
                startTrain(result, stock_list, size, day_number);
            }else if(MODE == "test"){
//                startTest(result, current_date, company_list[c], TYPE, companyData, range_day_number);
            }else if(MODE == "exhaustive"){
//                startExhaustive(result, company_list[c], companyData, range_day_number);
            }else if(MODE == "B&H"){
//                startBH(result, company_list[c], companyData, range_day_number);
            }else if(MODE == "specify"){
//                startSpe(result, RSI_PARAMETER, company_list[c], companyData, range_day_number);
            }
            delete [] stock_list;
            releaseArray(data, day_number +1);
            releaseVector(data_vector);
            
            
            outputFile(result, getOutputFilePath(current_date, MODE, FILE_DIR, TYPE));
            current_date.slide();
        }while(finish_date >= current_date);
        
        END = clock();
        temp = FILE_DIR + "/" + TYPE + "/" + "time_" + MODE + ".txt";
        recordCPUTime(START, END, temp);
    }
    
    return 0;
}
