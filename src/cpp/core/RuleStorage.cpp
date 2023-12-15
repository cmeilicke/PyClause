#include "RuleStorage.h"
#include "Index.h"
#include "Globals.h"
#include "Rule.h"
#include "Types.h"

#include <fstream>
#include <string>


RuleStorage::RuleStorage(std::shared_ptr<Index> index, std::shared_ptr<RuleFactory> ruleFactory){
    this->ruleFactory = ruleFactory;
    this->index = index;
}


// reads format outputted by AnyBURL
void RuleStorage::readAnyTimeFormat(std::string path, bool exact){
    int currID = 0;
    int currLine = 0;
	std::string line;
	std::ifstream file(path);
    if (verbose){
        std::cout<<"Loading rules from " + path <<std::endl;
    }
	if (file.is_open()) {
		while (!util::safeGetline(file, line).eof()){
            if (currLine%1000000==0 && verbose && currLine>0){
                std::cout<<"...parsed "<<currLine<<" rules "<<std::endl;
            }
            bool added = addAnyTimeRule(line, currID, false);
            if (added){
                currID += 1;
            }
            currLine += 1;
        }
        std::cout<<"Loaded "<<currID<<" rules."<<std::endl;
    }else{
         throw std::ios_base::failure("Could not open rule file: " + path + " is the path correct?");
    }
}

void RuleStorage::readAnyTimeFromVec(std::vector<std::string>& ruleStrings, bool exact){
    int currID = 0;
    for (int i=0; i<ruleStrings.size(); i++){
         if (i%1000000==0 && verbose && i>0){
                std::cout<<"...serialized "<<i<<" rules "<<std::endl;
        }
        std::string stringLine = ruleStrings[i];
        bool added = addAnyTimeRule(stringLine, currID, exact);
        if (added){
            currID += 1;
        }
    }
    std::cout<<"Loaded "<<currID<<" rules."<<std::endl;
} 


bool RuleStorage::addAnyTimeRule(std::string ruleLine, int id , bool exact){
    // expects a line: predicted\t cpredicted\tconf\trulestring
	std::vector<std::string> splitline = util::split(ruleLine, '\t');
    std::string ruleString = splitline[3];
    if (splitline.size()!=4){
        throw std::runtime_error("Could not parse this rule because of format: " + ruleLine);
    }
    int num_preds = std::stoi(splitline[0]);
    int num_true = std::stoi(splitline[1]);
    std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(ruleString, num_preds, num_true);
    if (rule){
        rule->setID(id);
        rule->setStats(num_preds, num_true, exact);
        rule->setRuleString(ruleString);
        relToRules[rule->getTargetRel()].insert(&(*rule)); //same as insert(rule.get())
        rules.push_back(std::move(rule));
        return true;
    } else {
        return false;
    }
}


std::set<Rule*, compareRule>&  RuleStorage::getRelRules(int relation){
    return relToRules[relation];
}



std::vector<std::unique_ptr<Rule>>& RuleStorage::getRules(){
    return rules;
 }