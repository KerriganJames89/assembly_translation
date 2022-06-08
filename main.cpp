
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <map>
#include <bitset>
#include <utility>
using namespace std;


class Assembly 
{ 
  
  public: 
  vector<string> instructions = {"add", "nand", "lw", "sw", "beq", "jalr", "halt", "noop", ".fill"};
  vector<vector<string>> DataTable;
  vector< pair <string,int>> AddressTable;
  std::vector< pair <string,int> > SymbolTable; 
  vector<std::string> input_lines;
  string file_name;
  string outfile_name;

  Assembly(string file_name, string outfile_name)
  {
    this->file_name = file_name;
    this->outfile_name = outfile_name;
  }
  
  //Tokenizes data from infile into strings; each instruction is parsed and stored in its own vector.
  void populate_DataTable()
  {
    ifstream file(file_name);
    ofstream outfile(outfile_name);
    string line;

    if(!file.is_open()) {
      cout << "error in opening input file\n";
      exit(1);
    }

    if(!outfile.is_open()) 
    {
    cout << "error in creating output file\n";
    exit(1);
    }

    //Opens target file and tokenizes the data, line by line
    while (getline(file, line))
    {
      int check_symbol = 1;
      istringstream iss(line);
      vector<string> tokens;
      copy(
        istream_iterator<string>(iss),
        istream_iterator<string>(),
        back_inserter(tokens)
      );

      //Sorts memory locations by label or instruction type
      for(int i = 0; i < instructions.size(); i++)
      {
        if (tokens[0] == instructions[i])
        {
          check_symbol = 0;
          break;
        }
      }

      //Stores lables into a vector
      if (check_symbol == 1)
      {
        SymbolTable.push_back(make_pair(tokens[0], DataTable.size()));
      }
      
      DataTable.push_back(tokens);
    }

    //Checks if there are any duplicate lables
    {
      std::sort(SymbolTable.begin(), SymbolTable.end(), [](std::pair<string, int> a, std::pair<string, int> b) 
      {
          return a.first < b.first;
      });

      auto it = std::adjacent_find(SymbolTable.begin(), SymbolTable.end(), [](std::pair<string, int> &a, std::pair<string, int> &b) 
      {
          return a.first == b.first;
      });

      if(it != SymbolTable.end()) 
      {
        cout<<"Bad label/Duplicate label; exiting";
        exit(1);
      } 
    }

    //Symbol table display test
    // for(auto it = SymbolTable.cbegin(); it != SymbolTable.cend(); ++it)
    // {
    //   std::cout << it->first << " " << it->second << "\n";
    // }

    //Fetches each instruction and decodes their fields
    for(int index = 0; index < DataTable.size(); index++)
    {
      int pos = 0;
      
      for(auto &s : SymbolTable) 
      {
        if(s.first == DataTable[index][0]) 
        {
          pos = 1;
        }
      }

      //Function which builds the decoded instruction
      read_Instructions(index, pos);
    }

    //Binary Display test
    for(auto s : AddressTable) 
    {
      cout << s.first <<endl;
    }

    //Writes instructions to output files
    for(auto s : AddressTable) 
    {
      if(s.second == 0)
      {
        outfile<<bitset<25>(s.first).to_ullong()<<endl;
      }

      else
      {
        outfile<<s.first<<endl;
      }
    }
  }

  //Selects instruction type by comparing to a vector of strings
  void read_Instructions(int i, int p)
  {
    int selection = 0;
    
    for (auto& it : instructions) 
    {
      if(DataTable[i][p] == it)
      {
        break;
      }
      selection++;
    } 

    switch (selection) 
    {
    case 0 ... 1:
      R_type(i, p);
      break;
    case 2 ... 4:
      I_type(i, p);
      break;
    case 5:
      J_type(i, p);
      break;
    case 6 ... 7:
      O_type(i, p);
      break;
    case 8:
      Fill(i, p);
      break;
    default:
    cout<<"Exception caught; exiting";
      exit(1);
    }
  }

  //R-type instruction builder
  void R_type(int i, int p)
  {
    string buffer = "";

      if(DataTable[i][p] == "add")
    {
      buffer = "000";
    }

    else{buffer = "001";}

    int reg1 = check_value(i, p+1, 0, buffer);
    int reg2 = check_value(i, p+2, 0, buffer);
    int reg3 = check_value(i, p+3, 0, buffer);

    bitset<3> reg1_bits(reg1);
    bitset<3> reg2_bits(reg2);
    bitset<3> reg3_bits(reg3);


    buffer = buffer + reg1_bits.to_string();
    buffer = buffer + reg2_bits.to_string();
    buffer = buffer + "0000000000000";
    buffer = buffer + reg3_bits.to_string();
    
    AddressTable.push_back(make_pair(buffer, 0));
  }

  //I-type instruction builder
  void I_type(int i, int p)
  {
    string buffer = "";

    if(DataTable[i][p] == "beq")
    {
      buffer = "100";
    }

    else if(DataTable[i][p] == "sw")
    {
      buffer = "011";
    }

    else
    {
      buffer = "010";
    }

    
    int reg1 = check_value(i, p+1, 0, buffer);
    int reg2 = check_value(i, p+2, 0, buffer);
    int offset = check_value(i, p+3, 0, buffer);


    bitset<3> reg1_bits(reg1);
    bitset<3> reg2_bits(reg2);
    bitset<16> offset_bits(offset);

    buffer = buffer + reg1_bits.to_string();
    buffer = buffer + reg2_bits.to_string();
    buffer = buffer + offset_bits.to_string();

    AddressTable.push_back(make_pair(buffer, 0));
  }

  //J-type instruction builder
  void J_type(int i, int p)
  {
    string buffer = "101";

    int reg1 = check_value(i, p+1, 0, buffer);
    int reg2 = check_value(i, p+2, 0, buffer);

    bitset<3> reg1_bits(reg1);
    bitset<3> reg2_bits(reg2);

    buffer = buffer + reg1_bits.to_string();
    buffer = buffer + reg2_bits.to_string();
    buffer = buffer + "0000000000000000";

    AddressTable.push_back(make_pair(buffer, 0));
  }

  ////O-type instruction builder
  void O_type(int i, int p)
  {
    string buffer = "";

    if(DataTable[i][p] == "halt")
    {
      buffer = "110";
    }

    else
    {
      buffer = "111";
    }

    buffer = buffer + "0000000000000000000000";

    AddressTable.push_back(make_pair(buffer, 0));

  }

  //.fill instruction builder
  void Fill(int i, int p)
  {
    int x = check_value(i, p + 1, 1, "fill");
    AddressTable.push_back(make_pair(to_string(x), 1));
  }

  //Reads each instruction field for label positions and exits if something is found undefined
  int check_value(int i, int p, int f, string str)
  {
    int ret;

    for(auto &s : SymbolTable) 
    { 
      if(s.first == DataTable[i][p]) 
      { 
        if(str == "100")
        { 
          return (s.second - i - 1);
        }

        return s.second;
      }
    }

    try 
    {
      ret = stoi(DataTable[i][p]);

      } 
        catch (const std::exception &e) 
      {   
        cout<<"Exception caught (bad label or incorrect instruction); exiting";
        exit(1);
      }

    return ret;
  }

  template <int N>
  size_t least_significant_bit(const std::bitset<N> &bt)
  {
    for(size_t i = 0; i < bt.size(); ++i)
    {
        if(bt.test(i))
            return i;
    }
  }
}; 

int main(int argc, char *argv[])
{
  if (argc != 3) 
  {
    printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
        argv[0]);
    exit(1);
  }

  Assembly assembly(argv[1], argv[2]);
  // Assembly assembly("test");
  assembly.populate_DataTable();
  exit(0);
}
