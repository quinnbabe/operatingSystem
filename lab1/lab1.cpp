/*
 Title: Two pass linker
 Description:a.input:1)definition list(defcount,defcount pairs(S,R)),
                     2)use list(usecount, usecount symbols)
                     3)program text(codecount,codecount pairs(type,instr)where type can be I,A,R,E)
             b. Pass One parses the input and verifies the correct syntax and determines the base address for each
                        module and the absolute address for each defined symbol, storing the latter in a symbol table.
             c. Pass Two again processes the input and uses the base addresses and the symbol table created in pass one to              generate the actual output by relocating relative addresses and resolving external references.
 Writer:Ying Qu
 Date:2014.2.17
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>

using namespace std;

FILE * pFile;
char mystring[2000];//to copy the files character into array mystring
char *currentToken;//the position of the current token which is being processed
int numOfRow=0;//the number of lines
int numOfColumn=0;//the offset
char newstring[2000];//to make a copy of mystring
int defcount=0;//the number of defcount
int usecount=0;//the number of usecount
int codecount=0;//the number of codecount
int n; // the number returned by function getFileByToken()
bool isDelimiter = false;//to determine whether it is a delimiter
int symbolNum=0;//the number of symbol
bool uselessNum =false;
//to make a mark that if a symbol is multiple timess defined, the value except the first on will be useless number
int modNum=0;//the number of moduler
int modOffset[50];//the offset of moduler
int allCodecount=0;//the sum of codecount by all modulers
char uselist[200][17];//
int uselistUsed[200];//to check whether the uselist's symbol is used
int mapNum=0;//the number of the map(000,001...)
bool symbolFound =false;//to determine whether there is a symbol been matched
char firstChar;//to take the opcode off the 4-digit

//to store the symbol table's symbols‘ attributes in a struct which consists of symbols' name, address, information,
//and which module the symbol is been used and the last one useSymbol to determine whether the symbol has been used at least
//once of the whole program

struct symbol{
    //string name;
    char name[17];
    int addr;
    char info[200];
    int mod;
    bool useSymbol;
};

symbol symbolTable[256];//symbol table should support at least 256 symbols

//to firstly get the file's input by line
int getFileByLine(){
    
    if(pFile == NULL) perror("Error opening file");
    else{
        fgets(mystring, 2000, pFile);
        strcpy(newstring, mystring);
    }
    return !feof(pFile);
}

//to split the string into tokens
int getFileByToken(){

  char *pch;
    pch =strtok(NULL," \t\n\r");
    if (pch != NULL){
        currentToken = pch;
        isDelimiter=false;
        for(int i=numOfColumn;;i++){
            if((newstring[i]==' ')||(newstring[i]=='\n')||(newstring[i]=='\t')||(newstring[i]=='\r')){
                isDelimiter= true;
                continue;
            }
            else{
                if(isDelimiter==true){
                    numOfColumn = i;
                    break;
                }
            }
        }
        return 1;
    }
    
//to open the file
    while(!feof(pFile)){
        if(!getFileByLine()){
            
            return 0;
        };
        
        numOfRow++;
        numOfColumn = 0;
        pch = strtok(mystring," \t\n\r");
        if (pch != NULL){
            currentToken = pch;
            isDelimiter=false;
            for(int i=0;;i++){
                if(newstring[i]==' '||newstring[i]=='\n'||newstring[i]=='\t'||newstring[i]=='\r'){
                    continue;
                }
                else{
                    numOfColumn = i;
                    break;
                }
            }
            return 1;
        }
    }
    return 0;
 }
//error detection
void parseError(int errcode) {
      string errstr[] = {
        "NUM_EXPECTED",  // Number expect
        "SYM_EXPECTED",   // Symbol Expected
        "ADDR_EXPECTED",   // Addressing Expected
        "SYM_TOLONG",    // Symbol Name is to long
        "TO_MANY_DEF_IN_MODULE",  //
        "TO_MANY_USE_IN_MODULE",    //
        "TO_MANY_INSTR"       //
    };
    
    printf("Parse Error line %d offset %d: %s\n", numOfRow, numOfColumn+1, errstr[errcode].c_str());
    // notice that the column should be added by one according to the lab's requirement
    return;
}

//to determine whether the token is a number
int isNum(){
    for(int i=0; ;i++){
        if (currentToken[i]== 0){return 1;}
        else if (currentToken[i]>='0'&& currentToken[i] <= '9'){continue;}
        else {return 0;}
    }
}

//to determine whether the token is a symbol
int isSysmbol(){

    if (!((currentToken[0]>='a'&& currentToken[0] <= 'z')||(currentToken[0]>='A'&& currentToken[0] <= 'Z'))){return 0;}
  
     //to make a loop to check the token iis began with alpha characters followed by optional alphanumerical characters
    for(int i=1; ;i++){
        
        if(i==16){
            //if token[16] is not the end of the token then this symbol is over 16 characters
            if(currentToken[16]!=0){
                  parseError(3);//the symbol name is too long,over 16 characters
                }
            else{return 1;}
        }
        else if(currentToken[i]==0){return 1;}
        else if((currentToken[i]>='a'&& currentToken[i] <= 'z')||(currentToken[i]>='A'&& currentToken[i] <= 'Z')||(currentToken[i]>='0'&&currentToken[i] <= '9')){continue;}
        else {return 0;}
    }
    
}

int main(int argc, char *argv[]){
    
    //pFile = fopen("/Users/Quinnbabe/Desktop/lab/input-19" , "r");
    pFile=fopen(argv[1], "r");
    
    modOffset[0]=0; //to initialise the offset of the moduler to be 0
    for(int i=0;i<=256;i++)
    {
        symbolTable[i].useSymbol=false;
    }
//Pass One
    while(1){
        //read defcount
        
        n= getFileByToken();//get the first token's return value
        if(n==0){
            break;
        }
        if(isNum()==0){
            
            parseError(0);//the first token is expected to be a number as the defcount
            return 0;
        }
        defcount= atoi(currentToken);//to transfer the token from character to number
        if (defcount>16){
            parseError(4);//a deflist can support upto 16 definitions
            return 0;
        }
        //read definition list
        for(int i=0;i<defcount;i++){
            uselessNum =false;
            n= getFileByToken();
            //to get the next token's return value, the first element of the defcount pair
            if(n==0){
                parseError(1);
                //if the program only has one number, it will raised an error since we can not find the symbols
                return 0;
            }
            if(isSysmbol()==0){
                parseError(1);
                //the rest of the definition list should be symbols
                return 0;
            }
            for(int j=0;j<symbolNum;j++){
                if(strcmp(currentToken,symbolTable[j].name)==0){
                    
                    strcpy(symbolTable[j].info,"Error: This variable is multiple times defined; first value used");
                    //to compare the current token with the symbol table's symbols one by one to check
                    //whether the symboll is defined multipletimes,
                    //if yes, change the boolean number of uselessNum to be true
                    //so that we can use the value give in the first definition
                    uselessNum=true;
                    
                }
                
            }
            
            if(uselessNum==false){
                strcpy(symbolTable[symbolNum].name,currentToken);
                //if the symbol is not appeared before, add it to the symbol table
            }
            n=getFileByToken();//to get the next token, the second element of the defcount pair
            if(n==0){
                parseError(0);//it might be missing
                return 0;
            }
            if(isNum()==0){
                parseError(0);//it should be a number
                return 0;
            }
            if(uselessNum==false){
                //to update the symboltable's address and moduler number
                symbolTable[symbolNum].addr= modOffset[modNum]+atoi(currentToken);
                symbolTable[symbolNum].mod =modNum;
                symbolNum++;
            }
        }
       //read usecount
        n= getFileByToken();
        if(n==0){
            parseError(0);//number expect
            return 0;
        }
        if(isNum()==0){
            parseError(0);//number expect
            return 0;
        }
        usecount= atoi(currentToken);
        if (usecount>16){parseError(5);//uselist should support 16 definitions but no more
            return 0;
        }
        //read use list
        for(int i=0;i<usecount;i++){
            n= getFileByToken();
            if(n==0){
                parseError(1);//symbol expected
                return 0;
            }
            if(isSysmbol()==0){
                parseError(1);//symbol expected
                return 0;
            }
        }
        //read codecount
        n= getFileByToken();
        if(n==0){
            parseError(0);//number expect
            return 0;
        }
        if(isNum()==0){
            parseError(0);//number expect
            return 0;
        }
        codecount= atoi(currentToken);
        allCodecount= allCodecount+codecount;//to update the sum of the codecount
        if(allCodecount>512){
            parseError(6);
            //number instructions are unlimited but they are limited to the machine size, which is 512 words in this lab
            return 0;
        }
        modOffset[modNum+1]=modOffset[modNum]+codecount;//to update the moduler's offset
        for(int i=0;i<symbolNum;i++){
            if(symbolTable[i].addr>=modOffset[modNum+1]){
                printf("Warning: Module %d: %s to big %d (max=%d) assume zero relative\n",modNum+1,symbolTable[i].name,symbolTable[i].addr,codecount-1);
                //if an address appearing in a definition exceeds the module, treat the address as zero
                symbolTable[i].addr= modOffset[modNum];
            }
        }
        
        //read program text
        for(int i=0;i<codecount;i++){
              n= getFileByToken();
                if(n==0){
                    parseError(2);//addressing expected
                    return 0;
            }
    if(strcmp("A",currentToken)!=0&&strcmp("I",currentToken)!=0&&strcmp("E",currentToken)!=0&&strcmp("R",currentToken)!=0){
                //compare the token with A I E R
                parseError(2);//addressing expected
                  return 0;
            }
            
            n= getFileByToken();
            if(n==0){
                parseError(0);//number expect
                return 0;
            }
            if(isNum()==0){
                parseError(0);//number expect
                return 0;
            }
        }
        modNum++;
    }
    printf("Symbol Table\n");
    for(int i=0;i<symbolNum;i++){
        printf("%s=%d %s\n",symbolTable[i].name,symbolTable[i].addr,symbolTable[i].info);
    //print the symbol table out
    }
    
    //Pass Two
    
    numOfRow=0;
    numOfColumn=0;
    modNum=0;
    rewind(pFile);//to make the pointer back to the begin of the file
    printf("\nMemory Map\n");
    int tokenNum = 0;
    while(1){
        // to make dead cycle until we can not get any token
        for(int i=0;i<200;i++){
            uselist[i][0]=0;
            uselistUsed[i]=0;
        }
        //read defcount
        n=getFileByToken();
        if(n==0){
            break;
        }
     
        defcount= atoi(currentToken);
        
        //read definition list
        for(int i=0;i<defcount;i++){
          //to get the defcount pair
            getFileByToken();

            getFileByToken();
        
        }
        //read usecount
        getFileByToken();
       
        usecount= atoi(currentToken);
        
        //read use list
        for(int i=0;i<usecount;i++){
            getFileByToken();
            strcpy(uselist[i],currentToken);
        }
        //read codecount
        
        getFileByToken();

        codecount= atoi(currentToken);
       // allCodecount= allCodecount+codecount;
      
       // modOffset[modNum+1]=modOffset[modNum]+codecount;
        
        //read program text
        for(int i=0;i<codecount;i++){
            getFileByToken();
            
            switch(currentToken[0]){
                    
                //an absolute address is unchanged
                case('A'):
                
                    getFileByToken();
                    tokenNum=atoi(currentToken);
                    firstChar=tokenNum/1000+'0';//to get the opcode of the token
                    
                    if(tokenNum%1000>511){//since it begins with 0
                //to compare the oprand with 512, to check whether the absolute address exceeds the size of the machine
                            printf("%03d: %c000 ",mapNum,firstChar);
                            printf("Error: Absolute address exceeds machine size; zero used\n" );
                       
                        }
                        else{
                            printf("%03d: %04d\n",mapNum,tokenNum);
                          
                        }
                    break;
                    
                    // an immediate operand is unchanged
                    //there is no opcode(leftmost digit) and operand(rightmost 3 digits) is all four digits
                case('I'):
                    
                    getFileByToken();
                    
                    tokenNum=atoi(currentToken);
                    
                    if(tokenNum>9999){
                        tokenNum=9999;
                        printf("%03d: %04d",mapNum,tokenNum);
                        printf(" Error: Illegal immediate value; treated as 9999\n");
                       //if more than 4 numerical digits, convert the value to 9999
                 
                    }
                    else{
                       printf("%03d: %04d\n",mapNum,tokenNum);
                    
                    }
                    break;
               
                //a relative address is relocated by replacing the relative address with the absolute address of that
               //relative address after the modules global address has been determined
                case('R'):
                    getFileByToken();
                    tokenNum=atoi(currentToken);
                    firstChar=tokenNum/1000+'0';
                    if(tokenNum>9999){
                        tokenNum=9999;
                        printf("%03d: %04d",mapNum,tokenNum);
                        printf(" Error: Illegal opcode; treated as 9999\n");
                        //if an illegal opcode is encountered, convert the <opcode,operand> to 9999
                      
                    }
                    else if (tokenNum%1000>codecount-1){
                        tokenNum=modOffset[modNum];
                        printf("%03d: %c%03d",mapNum,firstChar,tokenNum);
                        printf(" Error: Relative address exceeds module size; zero used\n");
        //If a relative address exceeds the size of the module, and use the module relative value zero
                    }
                    else {
                        tokenNum= tokenNum + modOffset[modNum];//to update the address
                        printf("%03d: %04d\n",mapNum,tokenNum);
                    }
                    break;
                    
              //an external address is an index into the uselist.
            //identify to which global address the symbol is assigned and then replace the address.
                case('E'):
                    getFileByToken();
                    tokenNum=atoi(currentToken);
                    symbolFound=false;
                    firstChar=tokenNum/1000+'0';
                    if (tokenNum%1000>=usecount){
                        printf("%03d: %04d",mapNum,tokenNum);
                        printf(" Error: External address exceeds length of uselist; treated as immediate\n" );
                        
            //If an external address is too large to reference an entry in the use list
            //treat the address as immediate.
                    }
                    else {
                        uselistUsed[tokenNum%1000]=1;
                        for(int i=0;i<symbolNum;i++){
                            
                            if(strcmp(uselist[tokenNum%1000],symbolTable[i].name)==0){
                                symbolFound =true;
                                symbolTable[i].useSymbol =true;
                                tokenNum=(currentToken[0]-'0')*1000+symbolTable[i].addr;//to replace the operand
                               
                            }
                        }
                        if(symbolFound==false){
                            printf("%03d: %c000",mapNum,firstChar);
                            printf(" Error: %s is not defined; zero used\n", uselist[tokenNum%1000]);
                    //If a symbol is used in an E-instruction but not defined, print an error message and use the value zero
                        }
                        else{
                            printf("%03d: %04d\n",mapNum,tokenNum);
                        }
                    }
                    break;
                    
                //if no AEIR case appears, the default condition goes to
                default:printf("No matching instruction type\n");
                 
            }
            mapNum++;
        }
        // use check
        //If a symbol is defined but not used, print a warning message and continue.
        for(int i=0;i<usecount;i++){
            if( uselistUsed[i]==0){
            printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n",modNum+1,uselist[i]);
            }
        }
        modNum++;
    }
    
    //symbol check
    //If a symbol appears in a use list but it not actually used in the module ,print a warning message and continue.
    printf("\n");
    for(int i=0;i<symbolNum;i++){
        if(symbolTable[i].useSymbol ==false){
        printf("Warning: Module %d: %s was defined but never used\n", symbolTable[i].mod+1,symbolTable[i].name);
        }
    }
    fclose(pFile);
    return 0;
}