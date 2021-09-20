const std::string Input = "-i";
const std::string Output = "-o";
const std::string Find = "-f";
const std::string Naive = "-n";
const std::string NaiveParallel = "-np";
const std::string InducedSorting = "-is";
const std::string TryCompression = "-tc";
const std::string useLCP = "-lcp";
const std::string SubstringWidth = "-sw";
const std::string FilterNewline = "-fn";

bool parse_flag(const std::string& flag_str){
   return flag_str.size();
}
bool parse_flag(const bool& flag){
   return flag;
}




class Modes{
public:
   Modes(const unsigned& argc, char**& argv)
   : modes(8, "")
   {
      modes[0] = "SA_DATA";
      modes[1] = InducedSorting;
      std::string short_input_arg1 = "";
      std::string short_input_arg2 = "";
      for(unsigned i = 1 ; i < argc ; ++i){
         std::string arg = std::string(argv[i]);
         
         if(arg == Output){
            if(++i < argc)
               modes[0] = std::string(argv[i]);
            else{
               std::cerr << "ERROR : no argument for mode ''-o''\n";
               exit(1);
            }
         }
         else if(arg == Input){
            if(++i < argc)
               modes[3] = std::string(argv[i]);
            else{
               std::cerr << "ERROR : no argument for mode ''-i''\n";
               exit(1);
            }
         }
         else if(arg == Find){
            if(++i < argc)
               modes[2] = std::string(argv[i]);
            else{
               std::cerr << "ERROR : no argument for mode ''-f''\n";
               exit(1);
            }
         }
         else if(arg == SubstringWidth){
            if(++i < argc)
               modes[4] = std::string(argv[i]);
            else{
               std::cerr << "ERROR : no argument for mode ''-sw''\n";
               exit(1);
            }
         }
         else if(arg == Naive || arg == NaiveParallel || arg == InducedSorting)
            modes[1] = arg;
         else if(arg == TryCompression)
            modes[5] = " ";
         else if(arg == useLCP)
            modes[6] = " ";
         else if(arg == FilterNewline)
            modes[7] = " ";
         else{
            if(!short_input_arg1.size())
               short_input_arg1 = arg;
            else{
               if(!short_input_arg2.size())
                  short_input_arg2 = arg;
               else{
                  std::cerr << "ERROR : Too many arguments without mode-specification\n!";
                  exit(1);
               }
            }
         }
      }
      check_short_input(argc, argv, short_input_arg1, short_input_arg2);
   }
   
   bool shortInput() const {
      return short_input;
   };
   
   std::string operator[](const std::string& key) const {
      if(key == "OutFile")
         return modes[0];
      
      if(key == "InFile")
         return modes[3];
      
      if(key == "Infix")
         return modes[2];
      
      if(key == "Algorithm")
         return modes[1];
      
      if(key == "SubstringWidth")
         return modes[4];
      
      if(key == "TryCompression")
         return modes[5];
      
      if(key == "useLCP")
         return modes[6];
      
      if(key == "FilterNewline")
         return modes[7];
      std::cerr << "ERROR : Key to Modes unknown.\n";
      exit(1);
   }
   
private:
   void check_short_input(const unsigned& argc, 
                          char**& argv,
                          const std::string& arg1Inp,
                          const std::string& arg2Inf){
      if(!modes[3].size()){
         short_input = true;
         /*if(argc == 2){
            
            modes[3] = std::string(argv[1]);
         }
         else */
         if(2 <= argc && argc < modes.size()){
            if(!arg1Inp.size()){
               std::cerr << "no input how to page\n";
               exit(1);
            }
            modes[3] = arg1Inp;
            modes[2] = arg2Inf;
         }
         else{
            std::cerr << "how to page\n";
            exit(1);
         }
      }
      else{
         short_input = false;
      }
   }
   bool short_input;
   std::vector<std::string> modes;
};


