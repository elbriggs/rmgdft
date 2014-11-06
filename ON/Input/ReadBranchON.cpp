#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
namespace po = boost::program_options;
#include <iostream> 
#include <fstream>
#include <sstream>
#include <iterator>
#include <string> 
#include <cfloat> 
#include <climits> 
#include <unordered_map>
#include <set>
#include <list>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>
#include "MapElements.h"
#include "transition.h"
#include "make_conf.h"
#include "const.h"
#include "rmgtypedefs.h"
#include "params.h"
#include "typedefs.h"
#include "RmgException.h"
#include "RmgInputFile.h"
#include "InputOpts.h"



/**********************************************************************

    read other stuff for ON branch

    cfile        = name of the file containing the kpoint info
    InputMap     = Control Map. May not be needed by all atomic input
                   drivers but is useful for reading the RMG format.
    
**********************************************************************/

namespace Ri = RmgInput;

void ReadBranchON(char *cfile, CONTROL& lc, std::unordered_map<std::string, InputKey *>& InputMap)
{

    std::string line_delims = "^\n";
    std::string whitespace_delims = " \n\t";
    std::vector<std::string> Kpoints;
    std::unordered_map<std::string, InputKey *> NewMap;
    int nkpts;

    RmgInputFile If(cfile, NewMap);

    


    If.RegisterInputKey("start_mode", NULL, &lc.runflag, "",
                     CHECK_AND_TERMINATE, REQUIRED, start_mode,
                     "Type of run. Choices are \"Random Start\", \"Restart From File\", \"FIREBALL Start\", \"Gaussian Start\", or \"Restart TDDFT\".\n", 
                     "start_mode must be one of  \"Random Start\", \"Restart From File\", \"FIREBALL Start\", \"Gaussian Start\", or \"Restart TDDFT\". Terminating.\n");

    If.RegisterInputKey("freeze_orbital_step", &lc.freeze_orbital_step, 0, 100000, 90, 
                     CHECK_AND_FIX, OPTIONAL, 
                     "freeze orbital after this step", 
                     "");

    If.RegisterInputKey("mg_method", NULL, &lc.mg_method, "Pulay",
                     CHECK_AND_TERMINATE, REQUIRED, mg_method,
                     "mixing type for orbitals  \"Steepest Descent\", \"Pulay\", or \"KAIN\".\n", 
                     "start_mode must be one of \"Steepest Descent\", \"Pulay\", or \"KAIN\". Terminating.\n");

    If.RegisterInputKey("mg_steps", &lc.mg_steps, 0, 100, 2, 
                     CHECK_AND_FIX, OPTIONAL, "", "");

    If.RegisterInputKey("do_maovable_orbital_centers", &lc.movingCenter, false, "");

    If.RegisterInputKey("movable_orbital_centers_steps", &lc.movingSteps, 1, 10000, 40, 
                     CHECK_AND_FIX, OPTIONAL, "", "");

    If.RegisterInputKey("number_of_orbitals", &lc.num_states, 1, INT_MAX, 4, 
                     CHECK_AND_FIX, OPTIONAL, "", "");
    If.RegisterInputKey("number_of_atoms", &lc.num_states, 1, INT_MAX, 1, 
                     CHECK_AND_FIX, OPTIONAL, "", "");


    If.LoadInputKeys();
}