#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <tao/json.hpp>

std::string to_hex(unsigned int dec)
{
    char data[100];

    sprintf(data, "%08x", dec);
    return std::string(data);
}

int main(int argc, char** argv)
{
    std::string filename = argv[1];
    std::ofstream fout( (filename+".in").c_str() );
    std::ofstream fans( (filename+".out").c_str() );
    
    int totalcase = argc - 2;
    fout << totalcase << std::endl;
    fans << totalcase << std::endl;
    
    for(int i=2;i<argc;++i)
    {
        tao::json::value v = tao::json::parse_file(argv[i]);
        
        std::cout << "# " << v["height"] << std::endl
                  << "hash: " << v["hash"] << std::endl
                  << "ver: " << v["ver"] << std::endl
                  << "prev: " << v["prev_block"] << std::endl
                  << "mkrt: " << v["mrkl_root"] << std::endl
                  << "ntime: " << v["time"] << std::endl
                  << "nbits: " << v["bits"] << std::endl
                  << "nonce: " << v["nonce"] << std::endl;
        
        fout << to_hex(v["ver"].get_unsigned()) << std::endl;
        fout << v["prev_block"].get_string() << std::endl;
        fout << to_hex(v["time"].get_unsigned()) << std::endl;
        fout << to_hex(v["bits"].get_unsigned()) << std::endl;
        
        std::cout << "tx: " << v["tx"].get_array().size() << std::endl; 
        fout << v["tx"].get_array().size() << std::endl;
        for(int j=0;j<v["tx"].get_array().size();++j)
        {
            std::cout << "  " << j+1 << ": " << v["tx"][j]["hash"] << std::endl;
            fout << v["tx"][j]["hash"].get_string() << std::endl;
        }
        
        
        fans << v["hash"].get_string() << std::endl;
    }
    
    fout.close();

    return 0;
}

