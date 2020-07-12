
//
//  main.cpp
//  Computer Architecture
//
//  Created by Ramy ElGendi on 26/11/19.
//  Copyright © 2019 Ramy ElGendi. All rights reserved.
//

// -----------------------
// Libraries Declaration:
// -----------------------
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
using namespace std;
// -----------------------
// Variables Declaration:
// -----------------------
string ADDRESS = "/Users/ramyelgendi/Desktop/Computer Architecture/Computer Architecture/Files/assembly_instructions.txt";
string SIM_RES = "/Users/ramyelgendi/Desktop/Computer Architecture/Computer Architecture/Files/simulation_results.txt";
string SIM_RES_NONEG ="/Users/ramyelgendi/Desktop/Computer Architecture/Computer Architecture/Files/simulation_results_2.txt";
int EMPTY = -1;
int countrob = 0, countrob1 =0;
bool flag;
// -----------------------
// Structs Declaration:
// -----------------------
struct ins_format{
    string op;
    int j; // Index of RS1
    int k; // Index of RS2
    int number; // Instruction place
    int res; // Reservation Station
    int rd; // Destination Register
    int imm; //Imm storage
    int PC; // PC
    int taken;
    int predict;
    float takenper = 0;
    float predictper =0;
    int issue= EMPTY; //Issue Cycle
    int execute_begin= EMPTY; // Execution cycle being time
    int execute_end= EMPTY; // Execution cycle end time
    int write= EMPTY; // Write back cycle
    int committed= EMPTY; // Write back cycle
    double result; //Output
    bool remove;
    int res_num; // Reservation Station Index
    string ready;
    //int rob = 0;
    ins_format(int number,string op,int rd, int j, int k): // Assigning from file inputs
    number(number),op(op), j(j), k(k), rd(rd) {};
    
    void display(){ // Printing the instructions entered from file with register indicies
        cout<<"      Instruction "<<number<<" ➜ "<<op<<" "<<rd<<" "<<j<<" "<<k<<"\n";
    }
};
struct res_format{
    string op;
    string name;
    int num; //in order of the RES in the RES vector
    int ex_count=0; // Counting execution cycles
    double Vj=EMPTY,Vk=EMPTY; // Source Operands
    int Qj=EMPTY,Qk=EMPTY; // Reservation Station Index
    int A_offset=0; //
    int A_reg=EMPTY; //
    int ins=EMPTY; //which instruction is being executed
    int rob =0;
    bool busy=false;
    bool done=false;
    res_format(){};
    res_format(string name, int num): name(name),num(num){};
};
// -----------------------
// Functions Declaration:
// -----------------------
bool checkfile(string); // Make sure the file exists and is empty
bool RemoveEmpty(string,string); // Removes all Empty signs (-1) from text file
bool Process_File(vector<ins_format>&,string&, float&, int&, int&, int&,int&,int&,int&,int&,int&);
bool Simulation(int,vector<ins_format>&, int&, int&, float&, float&);
void PrintTable1(ofstream&, vector<ins_format>,vector<res_format>,vector<double>,unordered_map<int,int>, int&, int&); // Print each cycle
void PrintTable2(ofstream&, vector<ins_format>,int, float&, float&); // Print cycles summary
void issue(ofstream&,int,vector<ins_format>&,vector<res_format>&,unordered_map<int,int>&,vector<double>, int&);
void execute(ofstream&, unordered_map<string,int>, int,vector<ins_format>&, vector<res_format>&, unordered_map<int,int>& ,vector<double>, unordered_map<int,double>&, int&, int&, float&);
void writeResult(ofstream& , int& ,int , vector<ins_format>& , vector<res_format>& ,unordered_map<int,int>& ,vector<double>& ,unordered_map<int,double>& );

// -----------------------

int main() {
    vector<ins_format> ins_vector;
    
    string op;// File Inputs
    
    int instruction_count=0; // Number of instructions in the file
    int j, k, rd, index,number = 0;
    int predict = 0;
    int imm;
    int taken = 0;
    float takenper = 0;
    float predictper = 0;
    if(!checkfile(ADDRESS)) //  Checking that file exists and is not empty
        return 0;
    
    cout<<"List of instructions uploaded forom assembly_instructions.txt:\n\n";
    if(!Process_File(ins_vector,op,predictper,predict, imm, rd,j,k,index,number,instruction_count)) // Getting inputs into array of structs
        return 0;
    
    cout<<"\n• There is a total of ["<<instruction_count<<"] instructions uploaded from the file.\n";
    
    if(Simulation(instruction_count,ins_vector, taken, predict, predictper, takenper)) {
        cout<< "\n✓ File: simulation_results.txt created successfuly! ✓ \n";
        cout<< "\n- Results:  \n";
        for(int i=0;i<instruction_count;i++)
            cout<<"      Result of Instruction "<<ins_vector[i].number<<" is "<<ins_vector[i].result<<".\n";
    } else {
        cout<<"\n Simulation Failed! \n";
    }
    string line;
    
    if(RemoveEmpty(SIM_RES,SIM_RES_NONEG))
        cout<<"\n- Simulation Completed! \n\n";
    
    return 0;
}

bool RemoveEmpty(string inaddr,string outaddr) {
    ifstream file (inaddr);
    ofstream file2 (outaddr);
    string sent;
    while (getline (file, sent))
    {
        for(int i=0;i<sent.size();i++)
        {
            if(sent[i-1] == ' ' && sent[i] == '-' && sent[i+1] == '1') {
                sent[i] = ' ';
                sent[i+1] = ' ';
            }
        }
        file2 << sent<<"\n";
    }
    file.close();
    file2.close();
    return true;
}

bool Simulation(int instruction_count,vector<ins_format>& ins_vector, int &taken,int & predict, float&predictper, float& takenper) {
    // INITIALIZINGS:
    vector<double> reg_vector{0,1,2,3,4,5,6,7}; // Initialize Registers
    vector<res_format> res_vector{ // Reservation Stations
        res_format("ADD/SUB/ADDI1",0),res_format("ADD/SUB/ADDI2",1),res_format("ADD/SUB/ADDI3",2),
        res_format("MULT1",3),res_format("MULT2",4),
        res_format("LW1",5),res_format("LW2",6),res_format("SW1",7),res_format("SW2",8),
        res_format("NAND1",9),res_format("NAND2",10), res_format("JMP1", 11), res_format("JMP2", 12), res_format("JALR1", 13), res_format("JALR2", 14), res_format("RET1", 15), res_format("RET2", 16), res_format("ADDI1", 17), res_format("ADDI2", 18), res_format("ADDI3", 19), res_format("BEQ1", 20), res_format("BEQ2", 21)
    };
    unordered_map<int,int> Reg_status({ // Initializing Registers Statuses
        {0,EMPTY},{1,EMPTY},{2,EMPTY},
        {3,EMPTY},{4,EMPTY},{5,EMPTY},
        {6,EMPTY},{7,EMPTY}
    });
    unordered_map<int,double> memory({{0,0},{1,1},{2,2},{3,3},{4,4},{5,5}}); // Loading the memory
    
    // Setting the amount of needed cycles per instruction
    unordered_map<string,int> cycles_needed;
    cycles_needed["ADD"]=2;cycles_needed["SUB"]=2;cycles_needed["ADDI"]=2;
    cycles_needed["LW"]=3;
    cycles_needed["SW"]=3;
    cycles_needed["NAND"]=1;
    cycles_needed["MULT"]=10;
    cycles_needed["BEQ"]=1;
    cycles_needed["JMP"]=1;
    cycles_needed["JALR"]=1;
    cycles_needed["RET"]=1;
    
    ofstream sim_file;
    sim_file.open(SIM_RES);
    
    int wb_counter=0;
    int cycle_number=1;
    int PC = 0;
    while(wb_counter!=instruction_count){
        
        sim_file<<"------------\n";
        sim_file<<"  CYCLE "<<cycle_number<<"\n";
        sim_file<<"------------\n";
        
        // The 3 Stages
        issue(sim_file, cycle_number,ins_vector,res_vector,Reg_status,reg_vector, PC);
        issue(sim_file, cycle_number,ins_vector,res_vector,Reg_status,reg_vector, PC);
        execute(sim_file,cycles_needed,cycle_number,ins_vector,res_vector,Reg_status,reg_vector,memory, taken, PC, takenper);
        execute(sim_file,cycles_needed,cycle_number,ins_vector,res_vector,Reg_status,reg_vector,memory, taken, PC, takenper);
        writeResult(sim_file, wb_counter,cycle_number,ins_vector,res_vector,Reg_status,reg_vector,memory);
        writeResult(sim_file, wb_counter,cycle_number,ins_vector,res_vector,Reg_status,reg_vector,memory);
        cycle_number++; // Incrementing Cycle counter
        // cout << cycle_number;
        PrintTable1(sim_file,ins_vector, res_vector,reg_vector,Reg_status, taken, predict);
        
        //        cout << i
        
    }
    PrintTable2(sim_file,ins_vector,instruction_count, takenper, predictper);
    sim_file.close();
    
    return true;
}

void PrintTable1(ofstream& sim_file, vector<ins_format> ins_format, vector<res_format> res_vector,vector<double> reg_vector,unordered_map<int,int> Reg_status, int &taken, int&predict ) {
    
    for(int i=0;i<=70;i++)
        sim_file<<"-";
    sim_file<<"\n";
    
    sim_file<<setw(13)<<
    "Name     "<<" | "<<setw(5)<<
    "Busy"<<" | "<<setw(5)<<
    "Op"<<" | "<<setw(5)<<
    "Vj"<<" | "<<setw(5)<<
    "Vk"<<" | "<<setw(5)<<
    "Qj"<<" | "<<setw(5)<<
    "Qk"<<" | "<<setw(5)<<
    "Address"<<"\n";
    
    for(int i=0;i<=70;i++)
        sim_file<<"-";
    sim_file<<"\n";
    
    
    for(int i=0; i<res_vector.size();i++){
        sim_file<<setw(13)<<
        res_vector[i].name<<" | "<<setw(5)<<
        res_vector[i].busy<<" | "<<setw(5)<<
        res_vector[i].op<<" | "<<setw(5)<<
        res_vector[i].Vj<<" | "<<setw(5)<<
        res_vector[i].Vk<<" | "<<setw(5)<<
        res_vector[i].Qj<<" | "<<setw(5)<<
        res_vector[i].Qk<<" | "<<
        res_vector[i].A_offset<<"+R"<<
        res_vector[i].A_reg<<"\n";
    }
    for(int i=0;i<=70;i++)
        sim_file<<"=";
    sim_file<<"\n";
    
    
    // Printing Second Table
    for(int i=0;i<=15;i++)
        sim_file<<"-";
    sim_file<<"\n";
    
    sim_file <<"REGISTER STATUS"<<endl;
    
    for(int i=0;i<=15;i++)
        sim_file<<"-";
    sim_file<<"\n";
    
    for(int i=0; i<reg_vector.size();i++){
        sim_file<<i<<"  | "<<setw(5)<<
        //                reg_vector[i]<<" | "<<setw(5)<<
        "Qi = ";
        
        if (Reg_status[i]>EMPTY)
            sim_file<<res_vector[Reg_status[i]].name.c_str()<<"\n";
        else
            sim_file<<"\n";
    }
    
    for(int i=0;i<=15;i++)
        sim_file<<"=";
    sim_file<<"\n";
    
    for(int i=0;i<=40;i++)
        sim_file<<"-";
    sim_file<<"\n";
    
    sim_file << setw(15)<<"ROB"<<endl;
    for(int i=0;i<=42;i++)
        sim_file<<"-";
    sim_file<<"\n";
    sim_file<<
    "    #   "<<" | "<<setw(5)<<
    "Type"<<" | "<<setw(5)<<
    "Dest"<<" | "<<setw(5)<<
    "Value"<<" | "<<setw(5)<<
    "Ready"<<" | "<<setw(5)<<"\n";
    
    for(int i=0;i<=40;i++)
        sim_file<<"-";
    sim_file<<"\n";
    for (int i=0; i<8;i++)
    {
        if(taken && !predict){
            res_vector[i].rob = EMPTY;
            ins_format[i].op = EMPTY;
            ins_format[i].rd = EMPTY;
            ins_format[i].ready = EMPTY;
        }
        if ((res_vector[i].rob > 0)&&(!ins_format[i].remove))
        {
            
            sim_file<<setw(5)<<
            res_vector[i].rob <<"    |"<<setw(5)<<
            ins_format[i].op <<"  | "<<setw(5)<<
            ins_format[i].rd<<" | "<<setw(5);
            if (ins_format[i].ready == "Yes")
            {
                sim_file << ins_format[i].number <<" | " << setw(5) <<
                ins_format[i].ready <<" | "<<setw(5) << "\n";
                //ins_format[i].remove = true;
               
            }
            else
            {
                sim_file << "  " <<" | " << setw(5) <<
                ins_format[i].ready <<" | "<<setw(5) << "\n";
            }
        }
        else
        {
            sim_file <<setw(5) << (i+1) <<"    | "<<setw(5)
            <<"      | "<<setw(5) <<"      | "<<setw(5)
            <<"      | "<<setw(5)<< "      | " << "\n";
        }
        //        sim_file<<setw(42)<< "\n";
    }
}
void PrintTable2(ofstream& sim_file, vector<ins_format> ins_vector,int instruction_count, float& takenper, float& predictper) {
    
    for(int i=0;i<=85;i++)
        sim_file<<"=";
    sim_file<<"\n";
    
    sim_file <<"                                Summary\n";
    
    for(int i=0;i<=85;i++)
        sim_file<<"-";
    sim_file<<"\n";
    sim_file<<setw(10)<<
    "Ins"<<"  |  "<<setw(10)<<
    "Issue"<<"  |  "<<setw(10)<<
    "Ex start"<<"  |  "<<setw(10)<<
    "Ex end"<<"  |  "<<setw(10)<<
    "Write"<<"  |  "<<setw(10)<<
    "Committed"<<"\n";
    
    for(int i=0;i<=85;i++)
        sim_file<<"-";
    sim_file<<"\n";
    
    for(int i=0;i<instruction_count;i++){
        sim_file<<setw(10)<<
        ins_vector[i].op.c_str()<<"  |  "<<setw(10)<<
        ins_vector[i].issue<<"  |  "<<setw(10)<<
        ins_vector[i].execute_begin<<"  |  "<<setw(10)<<
        ins_vector[i].execute_end<<"  |  "<<setw(10)<<
        ins_vector[i].write<<"  |  "<<setw(10)<<
        ins_vector[i].committed<<"\n";
    }
    for(int i=0;i<=85;i++)
        sim_file<<"=";
    sim_file<<"\n";
    
    sim_file<< "The misprediction percentange is: " << ((takenper/predictper)*100);
    
}

bool Process_File(vector<ins_format>& ins_vector,string& op, float&predictper ,int&predict,int& imm, int& rd,int& j,int& k,int& index,int& number, int& instruction_count) {
    string instruction; // Whole line of instruction from file
    string rd_s,j_s,k_s; // File Inputs
    
    ifstream file(ADDRESS);
    
    while(getline(file,instruction)){ // Reading every line in input file
        istringstream file_s(instruction);
        file_s>>op>>rd_s>>j_s>>k_s;
        
        if(op=="LW"||op=="SW")
            index=0;
        else
            index=1;
        if(op == "ADD" || op == "SUB" || op == "NAND" || op == "MUL"){
            rd=atoi(rd_s.substr(1).c_str());
            j=atoi(j_s.substr(index).c_str());
            k=atoi(k_s.substr(1).c_str());
            if ((rd>7 || rd<0) || (j>7 || j<0) || (k>7 || k<0)) { // Making sure the registers are from 0 to 7
                cout<< "Error, simulation allows registers from 0 to 7 only.\n";
                return false;;
            }
            
        }
        if(op == "BEQ" || op== "ADDI"){
            rd=atoi(rd_s.substr(1).c_str());
            j=atoi(j_s.substr(index).c_str());
            imm=atoi(k_s.substr(1).c_str());
            if(op == "BEQ" && imm < 0){
                predict = 1;
                predictper++;
            }
        }
        if(op == "JMP"){
            rd=EMPTY;
            j=EMPTY;
            imm=atoi(rd_s.substr(1).c_str());

        }
        if(op == "JALR"){
            rd=EMPTY;
            j=atoi(rd_s.substr(1).c_str());
            k=EMPTY;
        }
        if(op == "RET"){
            rd=EMPTY;
            j=EMPTY;
            k=EMPTY;
        }
        
        // Adding the converted to the vector array in instruction format structure
        ins_format ins(number,op,rd,j,k);
        ins_vector.push_back(ins);
        ins.display(); // Printing the instructions on the console
        number++; // incrementing the position of each instruction
        instruction_count++; // incrementing the instruction count
        
    }
    file.close();
    
    return true;
}

bool checkfile(string address) {
    ifstream file(address);
    if (!file) {      // file is not open
        cout<<"File does not exist. \n";
        return false;
    }
    file.seekg(0, ios::end);
    if (file.tellg() == 0) { // file is empty
        cout<<"File is empty. \n";
        return false;
    }
    file.close();
    return true;  // file is open and not empty
}

// ==========================================
//          THREE STAGES FUNCTIONS
// ==========================================


void issue(ofstream& sim_file, int cycle_number, vector<ins_format>& ins_vector, vector<res_format>& res_vector, unordered_map<int,int>& Reg_status,vector<double> reg_vector, int& PC){
    int res=EMPTY;
    //    bool flag;
    //cout << countrob  << "   " << endl;
    if (countrob < 7)
        flag = true;
    else
        flag = false;
    //find the first instruction that has not been issued
    for(int i=0;i<ins_vector.size() && flag;i++) {
        if(ins_vector[i].issue==EMPTY) {
            if (i<ins_vector.size())
            {
                countrob++;
                countrob1++;
                // cout << i << "here" << endl;
                //  cout << countrob1;
                ins_vector[i].ready = "NO";
                ins_vector[i].remove= false;
                res_vector[i].rob= (countrob1 % 7)+1;
                //cout << res_vector[i].rob << endl;
                string curr_op = ins_vector[i].op;
                if ((curr_op=="LW") || (curr_op=="SW")) {
                    PC++;
                    for (int r:{5,6,7,8}) {
                        if (!res_vector[r].busy) {
                            res=r;
                            
                            ins_vector[i].issue = cycle_number;
                            sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                            ins_vector[i].res=res;
                            res_vector[res].busy = true;
                            res_vector[res].ins=i;
                            res_vector[res].op=ins_vector[i].op;
                            //ins_vector[i].rob= countrob1;
                            //set the address value of RS and Qi
                            res_vector[res].A_offset = ins_vector[i].j;
                            res_vector[res].A_reg = ins_vector[i].k;
                            if(curr_op=="LW") Reg_status[ins_vector[i].rd]=res;
                            break;
                        }
                        
                    }
                }
                else {
                    if ((curr_op=="ADD") || (curr_op=="SUB")) {
                        PC++;
                        for (int r:{0,1,2}) {
                           
                            if (!res_vector[r].busy) {
                                res=r;
                                ins_vector[i].issue = cycle_number;
                                sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                                ins_vector[i].res=res;
                                // countrob1++;
                                res_vector[res].ins=i;
                                res_vector[res].op=ins_vector[i].op;
                                res_vector[res].busy = true;
                                
                                Reg_status[ins_vector[i].rd] = res_vector[res].num;
                                
                                break;
                            }
                            
                        }
                    }
                    if (curr_op=="MULT") {
                        PC++;
                        for (int r:{3,4}) {
                            
                            if (!res_vector[r].busy) {
                                res=r;
                                ins_vector[i].issue = cycle_number;
                                sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                                // countrob1++;
                                //ins_vector[i].rob= countrob1;
                                ins_vector[i].res=res;
                                res_vector[res].ins=i;
                                res_vector[res].op=ins_vector[i].op;
                                res_vector[res].busy = true;
                                Reg_status[ins_vector[i].rd]=res_vector[res].num;
                                
                                break;
                            }
                            
                        }
                    }
                    if (curr_op=="NAND") {
                        PC++;
                        for (int r:{9,10}) {
                            
                            if (!res_vector[r].busy) {
                                res=r;
                                ins_vector[i].issue = cycle_number;
                                sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                                ins_vector[i].res=res;
                                res_vector[res].ins=i;
                                //countrob1++;
                                //ins_vector[i].rob= countrob1;
                                res_vector[res].op=ins_vector[i].op;
                                res_vector[res].busy = true;
                                Reg_status[ins_vector[i].rd]=res_vector[res].num;
                                
                                break;
                            }
                            
                        }
                    }
                    if (curr_op=="JMP") {
                        PC++;
                        for (int r:{11}) {
                            
                            if (!res_vector[r].busy) {
                                res=r;
                                ins_vector[i].issue = cycle_number;
                                sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                                ins_vector[i].res=res;
                                res_vector[res].ins=i;
                                //countrob1++;
                                //ins_vector[i].rob= countrob1;
                                res_vector[res].op=ins_vector[i].op;
                                res_vector[res].busy = true;
                                Reg_status[ins_vector[i].rd]=res_vector[res].num;
                                
                                break;
                            }
                            
                        }
                    }
                    if (curr_op=="BEQ") {
                        PC++;
                        for (int r:{12}) {
                            
                            if (!res_vector[r].busy) {
                                res=r;
                                ins_vector[i].issue = cycle_number;
                                sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                                ins_vector[i].res=res;
                                res_vector[res].ins=i;
                                //countrob1++;
                                //ins_vector[i].rob= countrob1;
                                res_vector[res].op=ins_vector[i].op;
                                res_vector[res].busy = true;
                                Reg_status[ins_vector[i].rd]=res_vector[res].num;
                                
                                
                                break;
                            }
                            
                        }
                    }
                    if (curr_op=="JALR") {
                        PC++;
                        for (int r:{13}) {
                            
                            if (!res_vector[r].busy) {
                                res=r;
                                ins_vector[i].issue = cycle_number;
                                sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                                ins_vector[i].res=res;
                                res_vector[res].ins=i;
                                //countrob1++;
                                //ins_vector[i].rob= countrob1;
                                res_vector[res].op=ins_vector[i].op;
                                res_vector[res].busy = true;
                                Reg_status[ins_vector[i].rd]=res_vector[res].num;
                                
                                break;
                            }
                            
                        }
                    }
                    if (curr_op=="ADDI") {
                        PC++;
                        for (int r:{14,15}) {
                            
                            if (!res_vector[r].busy) {
                                res=r;
                                ins_vector[i].issue = cycle_number;
                                sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                                ins_vector[i].res=res;
                                res_vector[res].ins=i;
                                //countrob1++;
                                //ins_vector[i].rob= countrob1;
                                res_vector[res].op=ins_vector[i].op;
                                res_vector[res].busy = true;
                                Reg_status[ins_vector[i].rd]=res_vector[res].num;
                                
                                break;
                            }
                            
                        }
                    }
                    if (curr_op=="RET") {
                        PC++;
                        for (int r:{16}) {
                            
                            if (!res_vector[r].busy) {
                                res=r;
                                ins_vector[i].issue = cycle_number;
                                sim_file<<"  Issued instruction "<<i+1<<" in cycle "<<cycle_number<<endl;
                                ins_vector[i].res=res;
                                res_vector[res].ins=i;
                                //countrob1++;
                                //ins_vector[i].rob= countrob1;
                                res_vector[res].op=ins_vector[i].op;
                                res_vector[res].busy = true;
                                Reg_status[ins_vector[i].rd]=res_vector[res].num;
                                
                                break;
                            }
                            
                        }
                    }
                    //check if operand is available for Vj and Vk, otherwise set Qj and Qk
                    if(res>EMPTY) {
                        if (Reg_status[ins_vector[i].j]<0 || ins_vector[i].rd==ins_vector[i].j) {
                            res_vector[res].Vj = reg_vector[ins_vector[i].j];
                            
                        } else {
                            res_vector[res].Qj = Reg_status[ins_vector[i].j];
                        }
                        if (Reg_status[ins_vector[i].k]<0 || ins_vector[i].rd==ins_vector[i].k) {
                            res_vector[res].Vk = reg_vector[ins_vector[i].k];
                            
                        } else {
                            res_vector[res].Qk = Reg_status[ins_vector[i].k];
                        }
                    }
                    
                }
                break;
            }
        }
    }
    
}

void execute(ofstream& sim_file, unordered_map<string,int> cycles_needed, int cyclenum, vector<ins_format>& ins_vector, vector<res_format>& res_vector, unordered_map<int,int>& Reg_status,vector<double> reg_vector, unordered_map<int,double>& memory, int& taken, int& PC, float& takenper){
    for(int r=0;r<res_vector.size();r++){
        if(res_vector[r].busy){
            ins_format* instruction=&ins_vector[res_vector[r].ins];
            if(cyclenum>instruction->issue && instruction->execute_end<0){
                if((instruction->op!="LW")&&(instruction->op!="SW")){
                    if((!::isnan(res_vector[r].Vj)) && (!::isnan(res_vector[r].Vk))){
                        if (instruction->execute_begin < 0) {
                            instruction->execute_begin = cyclenum;
                            sim_file<< "  Started executing instruction " << res_vector[r].ins + 1 << endl;
                            if (instruction->op == "ADD")
                                instruction->result = res_vector[r].Vj + res_vector[r].Vk;
                            if (instruction->op == "SUB")
                                instruction->result = res_vector[r].Vj - res_vector[r].Vk;
                            if (instruction->op == "MULT")
                                instruction->result = res_vector[r].Vj * res_vector[r].Vk;
                            if (instruction->op == "NAND")
                                instruction->result = !(res_vector[r].Vj && res_vector[r].Vk);
                            if(instruction->op == "ADDI")
                                instruction->result = res_vector[r].Vj + res_vector[r].Vk;
                            if(instruction->op == "BEQ" && res_vector[r].Vj == res_vector[r].Vk){
                                taken = 1;
                                takenper++;
                                instruction->result = PC + 1 + res_vector[r].Vk;
                                PC = PC + 1 + res_vector[r].Vk;
                            }
                            if(instruction->op == "JMP"){
                                instruction->result =PC + 1 + res_vector[r].Vk;
                                PC = PC + 1 + res_vector[r].Vk;
                            }
                            if(instruction->op == "JALR"){
                                reg_vector[1] = PC + 1;
                                instruction->result = res_vector[r].Vj;
                                PC = res_vector[r].Vj;
                            }
                            if(instruction->op == "RET"){
                                instruction->result = reg_vector[1];
                                PC = reg_vector[1];
                            }
                        }
                        
                    }
                }
                
                if((instruction->op=="LW")||(instruction->op=="SW")){
                    if((Reg_status[instruction->k]<0) || (Reg_status[instruction->k]>EMPTY && ins_vector[res_vector[Reg_status[instruction->k]].ins].issue>=instruction->issue)){ //if Qi empty or if this ins was issued before the one completing Qi
                        if(instruction->execute_begin<0) {
                            instruction->execute_begin = cyclenum;
                            sim_file << "  Started executing instruction " << res_vector[r].ins + 1 << endl;
                            instruction->result = memory[res_vector[r].A_offset+(int)reg_vector[instruction->k]];
                        }
                    }
                }
                
                if((res_vector[r].ex_count<cycles_needed[instruction->op]) && (instruction->execute_begin>0)) {
                    res_vector[r].ex_count++;
                }
                if(res_vector[r].ex_count==cycles_needed[instruction->op] && !res_vector[r].done) {
                    res_vector[r].done=true;
                    instruction->execute_end=cyclenum;
                    sim_file<<"  Finished executing instruction "<<res_vector[r].ins+1<<endl;
                } else{
                    if((instruction->execute_begin>0) && (instruction->execute_begin<cyclenum)){
                        sim_file<<"  Currently executing instruction "<<res_vector[r].ins+1<<endl;
                    }
                }
                
            }
            
        }
    }
}

void writeResult(ofstream& sim_file, int& WB,int cycle_number, vector<ins_format>& ins_vector, vector<res_format>& res_vector,unordered_map<int,int>& Reg_status,vector<double>& reg_vector,unordered_map<int,double>& memory){
    for(res_format &rs:res_vector){
        
        if(rs.busy) {
            ins_format *instruction = &ins_vector[rs.ins];
            if (rs.done && instruction->execute_end < cycle_number && instruction->write < 0) {
                if(rs.op!="SW") {
                    if(Reg_status[instruction->rd]>EMPTY) {
                        if (res_vector[Reg_status[instruction->rd]].name == rs.name)
                            reg_vector[instruction->rd] = instruction->result;
                    }
                    Reg_status[instruction->rd] = EMPTY;
                } else{
                    memory[instruction->rd]=instruction->result;
                }
                for (res_format &rs2:res_vector) {
                    if (rs2.Qj == rs.num) {
                        sim_file<<"writing to Vj of Reservation Station "<<rs2.num<<endl;
                        rs2.Qj = EMPTY;
                        rs2.Vj = instruction->result;
                    }
                    if (rs2.Qk == rs.num) {
                        rs2.Qk = EMPTY;
                        rs2.Vk = instruction->result;
                    }
                }
                
                instruction->write = cycle_number;
                if(instruction->op == "ADD" || instruction->op == "SUB" || instruction->op == "MULT" || instruction->op == "LW" || instruction->op == "NAND" || instruction->op == "SW" || instruction->op == "JALR" || instruction->op == "JMP" || instruction->op == "BEQ" ||instruction->op == "RET" || instruction->op == "ADDI")
                    
                    instruction->committed = cycle_number+1;
                else if(instruction->op == "SW")
                    sim_file<<"  Instruction "<<rs.ins+1<<" written on cycle "<<cycle_number<<endl;
                rs.op="";
                rs.done = false;
                rs.ins = EMPTY;
                rs.busy = false;
                rs.ex_count = 0;
                rs.Qj = EMPTY;
                rs.Qk = EMPTY;
                rs.Vj = EMPTY;
                rs.Vk = EMPTY;
                rs.A_reg = EMPTY;
                rs.A_offset = 0;
                
                WB++;
                
                if (instruction->committed == cycle_number+1)
                {
                    countrob--;
                   instruction->remove = true;
                }
                  instruction->ready = "Yes";
            
                
                
                break;
            }
            
        }
    }
}
