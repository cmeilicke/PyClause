#include "DataHandler.h"

#include <functional>
#include <vector>


 DataHandler::DataHandler(std::map<std::string, std::string> options){
    index = std::make_shared<Index>();
    data = std::make_unique<TripleStorage>(index);
    target = std::make_unique<TripleStorage>(index);
    filter = std::make_unique<TripleStorage>(index);
    ruleFactory = std::make_shared<RuleFactory>(index);
    setRuleOptions(options, *ruleFactory);
    rules = std::make_unique<RuleStorage>(index, ruleFactory);
 }

bool DataHandler::getLoadedData(){
    return loadedData;
}

bool DataHandler::getLoadedRules(){
    return loadedRules;
}


void DataHandler::setOptions(std::map<std::string, std::string> options){
    setRuleOptions(options, *ruleFactory);
}


void DataHandler::loadRules(std::string path){
    if (!loadedData){
         throw std::runtime_error("Please load the data first with the the Handlers load data functionality.");
    }
    rules->readAnyTimeFormat(path, false);
    loadedRules = true;
}


void DataHandler::loadRules(std::vector<std::string> ruleStrings){
    if (!loadedData){
         throw std::runtime_error("Please load the data first with the the Handlers load data functionality.");
    }
    rules->readAnyTimeFromVec(ruleStrings, false);
    loadedRules = true;
}


std::unordered_map<std::string, int>& DataHandler::getNodeToIdx(){
    return index->getNodeToIdx();
}
std::unordered_map<std::string, int>& DataHandler::getRelationToIdx(){
    return index->getRelationToIdx();
}


void DataHandler::subsEntityStrings(std::map<std::string, std::string>& newNames){
        index->subsEntityStrings(newNames);

    }
void DataHandler::subsRelationStrings(std::map<std::string, std::string>& newNames){
        index->subsRelationStrings(newNames);
    }



void DataHandler::setRuleOptions(std::map<std::string, std::string> options, RuleFactory& ruleFactory){
    

    // rule options:  individual rule options and options of which rules to use
     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    std::vector<OptionHandler> handlers = {
        // Z
        {"load_zero_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleZ(util::stringToBool(val));}},
        {"z_weight", [&ruleFactory](std::string val) {ruleFactory.setZconfWeight(std::stod(val));}},
        {"z_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "z");}},
        {"z_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "z");}},
        {"z_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "z");}},
        {"z_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "z");}},
        // C
        {"load_u_c_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleC(util::stringToBool(val));}},
        {"c_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "c");}},
        {"c_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "c");}},
        {"c_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "c");}},
        {"c_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "c");}},
        // B
        {"load_b_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleB(util::stringToBool(val));}},
        {"b_max_branching_factor", [&ruleFactory](std::string val) {ruleFactory.setBbranchingFactor(std::stoi(val));}},
        {"b_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "b");}},
        {"b_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "b");}},
        {"b_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "b");}},
        {"b_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "b");}},
        // D
        {"load_u_d_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleD(util::stringToBool(val));}},
        {"d_weight", [&ruleFactory](std::string val) {ruleFactory.setDconfWeight(std::stod(val));}},
        {"d_max_branching_factor", [&ruleFactory](std::string val) {ruleFactory.setDbranchingFactor(std::stoi(val));}},
        {"d_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "d");}},
        {"d_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "d");}},
        {"d_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "d");}},
        {"d_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "d");}},
        // XXc
        {"load_u_xxc_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXc(util::stringToBool(val));}},
        {"xxc_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "xxc");}},
        {"xxc_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "xxc");}},
        {"xxc_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "xxc");}},
        {"xxc_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "xxc");}},
        // XXd
        {"load_u_xxd_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXd(util::stringToBool(val));}},
        {"xxd_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "xxd");}},
        {"xxd_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "xxd");}},
        {"xxd_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "xxd");}},
        {"xxd_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "xxd");}},
        
    };

    for (auto& handler : handlers) {
        auto opt = options.find(handler.name);
        if (opt != options.end()) {
            if (verbose){
                std::cout<< "Setting option "<<handler.name<<" to: "<<opt->second<<std::endl;
            }
            handler.setter(opt->second);
        }
    }
}

std::vector<std::string> DataHandler::getRuleIdx(){
    if (!loadedRules){
        throw std::runtime_error("You cannot obtain a rule index before you loaded rules into the laoder.");
    }
    std::vector<std::unique_ptr<Rule>>& allRules = rules->getRules();
    std::vector<std::string> out(allRules.size());
    for (int i=0; i<allRules.size(); i++){
        Rule* rule = allRules[i].get();
        if (rule->getID() != i){
            throw std::runtime_error("A rule's idx does not match its position. This is an internal; error check the backend.");
        }
        out.at(i) = rule->computeRuleString(index.get());
    }
    return out;
}


TripleStorage& DataHandler::getData(){
    return *data;
}
TripleStorage& DataHandler::getFilter(){
    return *filter;
}
TripleStorage& DataHandler::getTarget(){
    return *target;

}
RuleStorage& DataHandler::getRules(){
    return *rules;
}

RuleFactory& DataHandler::getRuleFactory(){
    return *ruleFactory;
}


std::shared_ptr<Index> DataHandler::getIndex(){
    return index;
}

// loads a file with tab separated string (token) triples
std::unique_ptr<std::vector<Triple>> DataHandler::loadTriplesToVec(std::string path){

    auto triples = std::make_unique<std::vector<Triple>>();

    // Open the file
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    // Read the file line by line
    std::string line;


    while (!util::safeGetline(file, line).eof()){
		std::istringstream iss(line);
		std::vector<std::string> results = util::split(line, '\t');
		if (results.size() != 3) {
			throw std::runtime_error("Error while reading a file with Triples please check that every line follows tab separeated: head relation tail format. ");
		}
		Triple triple;
        //TODO catch error here when unseen entity appears; for more informative error handling
        triple[0] = index->getIdOfNodestring(results[0]);
        triple[1] = index->getIdOfRelationstring(results[1]);
        triple[2] = index->getIdOfNodestring(results[2]);

        if (!iss.fail() || iss.eof()) {
            triples->push_back(triple);
        }else{
            throw std::runtime_error("Error while reading a file with Triples please check that every line follows tab separeated: head relation tail format. ");
        }
	}
	file.close();
    return std::move(triples);
}


void DataHandler::setNodeIndex(std::vector<std::string>& idxToNode){
    if (loadedData){
        throw std::runtime_error("You can only set an entity index before you loaded data.");
    }
        index->setNodeIndex(idxToNode);

}
void DataHandler::setRelIndex(std::vector<std::string>& idxToRel){
    if (loadedData){
        throw std::runtime_error("You can only set a relation index before you loaded data.");
    }
    index->setRelIndex(idxToRel);


}


