#include "DataHandler.h"

#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>


 DataHandler::DataHandler(std::map<std::string, std::string> options){
    index = std::make_shared<Index>();
    data = std::make_unique<TripleStorage>(index);
    target = std::make_unique<TripleStorage>(index);
    filter = std::make_unique<TripleStorage>(index);
    ruleFactory = std::make_shared<RuleFactory>(index);
    setRuleOptions(options, *ruleFactory);
    rules = std::make_unique<RuleStorage>(index, ruleFactory);
 }


void DataHandler::loadData(std::string path){
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }
    data->read(path, true); 
    loadedData = true;
}

void DataHandler::loadData(std::string path, std::string filterPath){
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }
    data->read(path, false); 

    if (filterPath!=""){
        filter->read(filterPath, false);
        data->loadCSR();
        filter->loadCSR();
    }else{
        data->loadCSR();
    }
    loadedData = true;
}

void DataHandler::loadData(std::vector<std::array<int, 3>> triples){
    std::cout<< "Loading raw integer triples..." << "\n";
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }
    // 'no-label' index needed
    // right now we assume a sequential node index from range [0, max_entity_id], same for relations
    // e.g. if triples = [[1, 0, 15000], [1, 237, 3]]
    //      we construct a node index [0, 15000] and a relation index of [0, 237]
    int max_entity_id = 0;
    int max_relation_id = 0;
    for (auto& triple : triples){
        if (triple[0] > max_entity_id) {
            max_entity_id = triple[0];
        }
        if (triple[1] > max_relation_id) {
            max_relation_id = triple[1];
        }
        if (triple[2] > max_entity_id) {
            max_entity_id = triple[2];
        }
    }
    std::cout << "Constructing 'no-label' index using " << max_entity_id << " entities and " << max_relation_id << "relations...\n";
    for (int i = 0; i < max_entity_id + 1; i++){
        // need some label, cannot do empty string for each, using str(idx)
        std::string lbl = std::to_string(i);
	    index->addNode(lbl);
    }
    for (int i = 0; i < max_relation_id + 1; i++){
        std::string lbl = std::to_string(i);
	    index->addRelation(lbl); 
    }

    data->read(triples, true); 
    loadedData = true;
}

void DataHandler::loadData(std::vector<std::array<std::string, 3>> triples){
    std::cout<< "Loading string triples..." << "\n";    
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }
    data->read(triples, true); 
    loadedData = true;
}

bool DataHandler::getLoadedData(){
    return loadedData;
}

bool DataHandler::getLoadedRules(){
    return loadedRules;
}


void DataHandler::loadDatasets(std::string targetPath, std::string trainPath, std::string filterPath){
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }

    std::cout<<"Load data... \n";


    target->read(targetPath, false);
    if (filterPath!=""){
        // can also just call with true here
        filter->read(filterPath, false);
        
    }
    // index constructed after train/data is loaded
    // loads CSR directly
    // sets loadedData
    loadData(trainPath);
    target->loadCSR();
    if(filterPath!=""){
        filter->loadCSR();
    }
}


void DataHandler::setOptions(std::map<std::string, std::string> options){
    setRuleOptions(options, *ruleFactory);
}


void DataHandler::loadRules(std::string path){
    if (!loadedData){
         throw std::runtime_error("Please load the data first with the the Handlers load data functionality.");
    }
    rules->readAnyTimeFormat(path, true);
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
        {"rule_zero_weight", [this](std::string val) { RuleZ::zConfWeight = std::stod(val); }},
        {"rule_u_d_weight", [this](std::string val) { RuleD::dConfWeight = std::stod(val); }},
        {"rule_b_max_branching_factor", [this](std::string val) { RuleB::branchingFaktor = std::stoi(val); }},
        {"use_zero_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleZ(util::stringToBool(val));}},
        {"use_u_c_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleC(util::stringToBool(val));}},
        {"use_b_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleB(util::stringToBool(val));}},
        {"use_u_d_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleD(util::stringToBool(val));}},
        {"use_u_xxc_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXc(util::stringToBool(val));}},
        {"use_u_xxd_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXd(util::stringToBool(val));}},
        {"rule_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val));}}
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


std::unique_ptr<std::vector<Triple>> DataHandler::loadTriplesToVec(std::string path){

    auto triples = std::make_unique<std::vector<Triple>>();

    // Open the file
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    // Read the file line by line
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string element;
        Triple triple;

        std::getline(iss, element, '\t');
        triple[0] = index->getIdOfNodestring(element);

        std::getline(iss, element, '\t');
        triple[1] = index->getIdOfRelationstring(element);

        std::getline(iss, element, '\t');
        triple[2] = index->getIdOfNodestring(element);
        if (!iss.fail() || iss.eof()) {
            triples->push_back(triple);
        }else{
            throw std::runtime_error("Error while reading a file with Triples please check that every line follows tab separeated: head relation tail format. ");
        }
    }

    return std::move(triples);
}






