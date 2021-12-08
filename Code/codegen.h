#ifndef CODEGEN_H
#define CODEGEN_H

#include <iostream>
#include <fstream>
#include <string>
#include "emitcode.cpp"

FILE* code; // pointer to testfile.tm, needed by emitcode

// Defines all TM instructions
enum Instruction {
                    NONE,                                   // Default, not a valid instruction
                    LDC, LDA, LD, ST, JNZ, JZR, JMP,        // REGISTER TO MEMORY INSTRUCTIONS (RA)
                    HALT, NOP, IN, INB, INC_tm, OUT, OUTB,     // REGISTER ONLY INSTRUCTIONS (RO)
                    OUTC, OUTNL, ADD_tm, SUB_tm, MUL, DIV_tm,        // ...
                    MOD_tm, AND_tm, OR_tm, XOR, NOT_tm, NEG, SWP, RND,  // ...
                    TLT, TLE, TEQ, TNE, TGE, TGT, SLT, SGT, // TEST INSTRUCTIONS (RO)
                    MOV, SET, CO, COA,                      // BLOCK MEMORY TO MEMORY INSTRUCTIONS (RO)
                    LIT,                                    // LITERAL INSTRUCTIONS
                };

enum LType {DEFAULT, STR_LIT, NUM_LIT, BOOL_LIT}; // denotes what type of literal this is

/*
    Describes all internal data associated with a single "instruction" or line of 3 address code and provides
    any necessary helper functions.
*/
class TMInstruction {
    private:
        int address;                // program memory address
        Instruction instruction = NONE;    // the instruction to be executed
        int addrs[3];               // 3 address part of "3 Address Code"
        std::string comment;        // stores a comment for this line
        LType ltype = DEFAULT;      // if this is not default, it stores the type of literal
        union literal {
            std::string* str_lit;// if this is one of the LIT commands, put a string lit here
            int* int_lit;        // also for a LIT command, but for int
            bool* bool_lit;      // for LIT command
        } literal;

    public:
        std::string instructionToStr(); // convert an instruction code into a string output
        TMInstruction(int line, Instruction ins, int addr1, int addr2, int addr3, std::string comment);
        TMInstruction(int line, Instruction ins, LType ltype, void* lit_ptr, std::string comment);
        TMInstruction(std::string);
        TMInstruction();
        std::string to_string();    // converts this TM instruction into a single 3 address code to go in output

        TMInstruction* head; // TMInstruction above this in the doubly linked list, NULL if true head of list
        TMInstruction* tail; // TMInstruction below, NULL if true tail of list
};

/*
    Provides functionality to analyze a valid AST and convert it into 3 address TM code and output to a .tm file.
*/
class CodeGenerator {
    private:
        std::ofstream* tm_output;   // reference to 3 address output file
        int pmem;                   // store the address of the next instruction here
        int main_loc;               // stores the address in program memory for main
        int main_ret_loc;           // store the pmem address of the main return stmt, used in init

        int loffset;
        int goffset;
        int toffset;

        bool isCallChild = false;   // if set true, then nodes below call are params
        // Setup functions
        void init();                // sets up all static header comments and cookie-cutter code before tree traversal
        // Doubly linked list functions
        TMInstruction* prog_head;        // reference to the head/first line of 3 address code
        TMInstruction* prog_tail;        // reference to the tail/last line of 3 address code
        void addLine(TMInstruction*);    // add new line to end of program
        // wrappers for addLine that create a new TMInstruction and add it to the list
        void addLine(std::string);
        void addLine(int, Instruction, int, int, int, std::string);
        void addLine(int, Instruction, LType, void*, std::string);
        void insertLine(TMInstruction*, TMInstruction*); // insert new line after a specific line
        TMInstruction* removeLine(TMInstruction*); // remove a line from the list while keeping lines before and after
        // Tree traversal and code gen functions
        void generateCode(ASTreeNode*);       // traverse the tree and convert into 3 address code, first pass
        void generateInitCode(ASTreeNode*); // generate code snips for this node type that don't change between programs
        void generatePostProc(ASTreeNode*);   // generate the code that comes after a node
        void generateInit();                    // final function called, places vector to main and halt
        // generate DeclK
        void generateFunc(ASTreeNode*);
        void generateVar(ASTreeNode*);
        void generateParam(ASTreeNode*);
        // generate StmtK
        void generateCompound(ASTreeNode*);
        void generateReturn(ASTreeNode*);
        void generateCall(ASTreeNode*);

        // Tree Traversal Cleanup
        void genFunCleanup(ASTreeNode*);
        void genReturnCleanup(ASTreeNode*);
        void genCallCleanup(ASTreeNode*);

        void appendToFile(); // write all lines of code to the output file in order

    public:
        CodeGenerator(ASTreeNode*);     // Generate code based on the AST passed to the generator
        void writeToFile(std::ofstream&, std::string);  // appends a string to the output file
        void printCode();           // print each line of code
        // Test functions
        void testCGEN_INSERT();
        void testCGEN_REMOVE();
};

#endif
