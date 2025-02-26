#include <algorithm>
#include <array>

#include "Rule.h"
#include "Types.h"


// ***Base Rule implementation***

void Rule::setID(int ID){
    this->ID = ID;
}

void Rule::print(){
    throw std::runtime_error("Not implemented yet");
}

int Rule::getID(){
    return ID;
}

void Rule::setRuleString(std::string str){
    this->rulestring = str;
}

double Rule::getConfidence(int nUnseen, bool exact){
    if (exact){
        return confWeight * ((double) cpredicted/((double) predicted + (double)nUnseen)); 
    }else{
        return confWeight *((double) sampledCpredicted/((double) sampledPredicted + (double)nUnseen)); 
    }
    
}

double Rule::getConfidence(bool exact){
    if (exact){
        return confWeight * ((double) cpredicted/((double) predicted + (double)numUnseen)); 
    }else{
        return confWeight *((double) sampledCpredicted/((double) sampledPredicted + (double)numUnseen)); 
    }
    
}

int Rule::getBranchingFactor(){
    return branchingFactor;
}

void Rule::setBranchingFactor(int val){
    branchingFactor = val;
}


void Rule::setStats(int _predicted, int _cpredicted, bool exact){
    if (exact){
        cpredicted = _cpredicted;
        predicted = _predicted;
    }else{
        sampledCpredicted = _cpredicted;
        sampledPredicted = _predicted;
    }
}

std::array<int,2> Rule::getStats(bool exact){
    if (exact){
        return {predicted, cpredicted};
    }else{
        return {sampledPredicted, sampledCpredicted};
    }
}

void Rule::setTrackInMaterialize(bool val){
    trackInMaterialize = val;
}

std::string Rule::getRuleString(){
    throw std::runtime_error("Use computeString instead.");
}

long long Rule::getBodyHash(){
    throw std::runtime_error("Not implemented yet");

}
void Rule::computeBodyHash(){
    throw std::runtime_error("Not implemented yet");
}

std::string Rule::computeRuleString(Index* index){
    throw std::runtime_error("Not implemented yet");

}

int Rule::getTargetRel(){
   return targetRel;

}
 void Rule::materialize(TripleStorage& triples, std::unordered_set<Triple>& preds){
    throw std::runtime_error("Not implemented yet");
}

std::vector<int>& Rule::getRelations() {
    return relations;
}

std::vector<bool>& Rule::getDirections() {
    return directions;
}

bool Rule::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults,  ManySet filterSet){
    throw std::runtime_error("Not implemented yet.");
}
bool Rule::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults,  ManySet filterSet){
    throw std::runtime_error("Not implemented yet.");
}


void Rule::setPredictHead(bool ind){
   throw std::runtime_error("Not implemented yet.");
}
void Rule::setPredictTail(bool ind){
    throw std::runtime_error("Not implemented yet.");
}

void Rule::setConfWeight(double weight){
    confWeight = weight;
}

void Rule::setNumUnseen(int val){
    numUnseen = val;
}

bool Rule::predictTriple(int tail, int head, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings)
{
  throw std::runtime_error("Not implemented yet.");
}


void Rule::searchCurrTargetGroundings(
		int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
		int targetEntity, std::vector<int>& rels, std::vector<bool>& dirs, std::vector<Triple>& currentGroundings,
		RuleGroundings* groundings, bool& reachedTarget,  bool invertGrounding
)
{
    int currRel = rels[currAtomIdx];
    int* begin;
    int length;

    if (reachedTarget && !groundings){
        return;
    }
    dirs[currAtomIdx-1] ? triples.getTforHR(currEntity, currRel, begin, length) : triples.getHforTR(currEntity, currRel, begin, length);
    if (currAtomIdx == rels.size()-1){
        // next entities
        for (int i=0; i<length; i++){
            int ent = begin[i];
            // IO and target checking
            if (substitutions.find(ent)==substitutions.end() && targetEntity==ent){
                reachedTarget = true;
                
                // we only track the groundings if groundings is given
                if (groundings){
                    Triple triple;
                    if (dirs[currAtomIdx-1]){
                        triple = {currEntity, currRel, ent};
                    }else{
                        triple = {ent, currRel, currEntity};
                    }
                    currentGroundings.push_back(triple);
                    if (!invertGrounding){
                        (*groundings)[this].push_back(currentGroundings);
                    }else{
                        std::vector<Triple> reverseGr = currentGroundings;
                        std::reverse(reverseGr.begin(), reverseGr.end());
                        (*groundings)[this].push_back(reverseGr);
                    }
                    
                    // we have to pop here (for substitutions we dont add anything so we dont erase)
                    currentGroundings.pop_back();
                    // now continue the loop, we want to find all the groundings
                }else{
                    // if we are not tracking groundings, we can stop as we know
                    // that the rule predicted the target triple, thats all we care about
                    return;

                }
            }
        }
    }else{
        if (branchingFactor>0 && length>branchingFactor){
            return;
        }
        //next entities
        for (int i=0; i<length; i++){
            int ent = begin[i];
            if (substitutions.find(ent)==substitutions.end()){
                substitutions.insert(ent);
                // we only track the groundings if groundings is given
                if (groundings){
                    Triple triple;
                    if (dirs[currAtomIdx-1]){
                        triple = {currEntity, currRel, ent};
                    }else{
                        triple = {ent, currRel, currEntity};
                    }
                    currentGroundings.push_back(triple);
                }
                searchCurrTargetGroundings(currAtomIdx+1, ent, substitutions, triples, targetEntity, rels, dirs, currentGroundings, groundings, reachedTarget, invertGrounding);
                substitutions.erase(ent);
                if (groundings){
                    currentGroundings.pop_back();
                }
            }
        }
    }
}

// ***RuleB implementation*** 

RuleB::RuleB(std::vector<int>& relations, std::vector<bool>& directions) {
    if(relations.size() != (directions.size() + 1)) {
        throw std::invalid_argument("'Directions' size should be one less than 'relations' size in construction of RuleB");
    }
    if(relations.size() < 2) {
        throw std::invalid_argument("Cannot construct a RuleB with no body atom.");
    }		
	this->relations = relations;
    this->directions = directions;	
    this->targetRel = relations.front();   

     // used for predicting heads
    this->_relations = relations;
    std::reverse(_relations.begin()+1, _relations.end());
    this->_directions = directions;
    std::reverse(_directions.begin(), _directions.end());
    _directions.flip();   
}

std::string RuleB::computeRuleString(Index* index){
    // internal represention of backend matches string anytime string representation, simple
    std::string out;
    out += index->getStringOfRelId(relations[0]);
    // head
    out += "(" + std::string(1, _cfg_prs_anyTimeVars[0])
               + "," + std::string(1, _cfg_prs_anyTimeVars.back()) 
               + ")" 
               +  _cfg_prs_ruleSeparator;
    // body
    for (int i=1; i<relations.size(); i++){
        out += index->getStringOfRelId(relations[i]);
        std::string var1 = std::string(1, _cfg_prs_anyTimeVars[i-1]);
        std::string var2 =  (i<relations.size()-1) ? std::string(1, _cfg_prs_anyTimeVars[i]): std::string(1, _cfg_prs_anyTimeVars.back());
        std::string atom = directions[i-1] ? "(" + var1 + "," + var2 + ")" : "(" + var2 + "," + var1 + ")";

        if (i==relations.size()-1){
             out += atom;
        }else{
            out += atom + _cfg_prs_atomSeparator;
        }
    }
    return out;
}

void RuleB::materialize(TripleStorage& triples, std::unordered_set<Triple>& preds){
    RelNodeToNodes* relNtoN = nullptr;
     // first body atom is (v1,v2)
    if (directions[0]){
         relNtoN =  &triples.getRelHeadToTails();
    // first body atom is (v2,v1)    
    }else{
         relNtoN =  &triples.getRelTailToHeads();
    }
    // first body relation
    auto it = relNtoN->find(relations[1]);
    if (!(it==relNtoN->end())){
        NodeToNodes& NtoN = it->second;
        // start branches of the DFS search
        // every entity e that satisfies b1(e,someY) [or b1(someX, e)]
        for (auto const& pair: NtoN){
            const int& e = pair.first;
            Nodes closingEntities;
            std::set<int> substitutions = {e};
            searchCurrGroundings(1, e, substitutions, triples, closingEntities, relations, directions);
            for (const int& cEnt:  closingEntities){
                Triple triple = {e, targetRel, cEnt};
                auto isNew = preds.insert(triple);
                // add to count if this triple is predicted for the first time
                if (trackInMaterialize && isNew.second){
                    predicted+=1;
                    if (triples.contains(triple[0], triple[1], triple[2])){
                        cpredicted += 1;
                    }
                }
            }
        }
    }
}

bool RuleB::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){ 
    Nodes closingEntities;
    std::set<int> substitutions = {head};
    searchCurrGroundings(1, head, substitutions, triples, closingEntities, relations, directions);
    bool madePred = false;
    for (const int& cEnt: closingEntities){ 
        if (!filterSet.contains(cEnt)){
            tailResults.insertRule(cEnt, this);
            madePred = true;
        }
    }
    return madePred;
                
           
}

bool RuleB::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults,  ManySet filterSet){
    Nodes closingEntities;
    std::set<int> substitutions = {tail};
    searchCurrGroundings(1, tail, substitutions, triples, closingEntities, _relations, _directions);
    bool madePred = false;
    for (const int& cEnt: closingEntities){
        if (!filterSet.contains(cEnt)){
                headResults.insertRule(cEnt, this);
                madePred = true;
        }
    }
    return madePred;
}

// recursive DFS from a startpoint currEntity, one substitution of the first body atom
void RuleB::searchCurrGroundings(
			int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
            Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs
		)
{
    int currRel = rels[currAtomIdx];
    int* begin;
    int length;

    dirs[currAtomIdx-1] ? triples.getTforHR(currEntity, currRel, begin, length) : triples.getHforTR(currEntity, currRel, begin, length);
    if (currAtomIdx == rels.size()-1){
        // next entities
        for (int i=0; i<length; i++){
            int ent = begin[i];
            // respect object identity constraint, stop if violated
            if (substitutions.find(ent)==substitutions.end()){
                    closingEntities.insert(ent);
            }
        }
    }else{
        if (branchingFactor>0 && length>branchingFactor){
            return;
        }
        for (int i=0; i<length; i++){
            int ent = begin[i];
            if (substitutions.find(ent)==substitutions.end()){
                substitutions.insert(ent);
                searchCurrGroundings(currAtomIdx+1, ent, substitutions, triples, closingEntities, rels, dirs);
                substitutions.erase(ent);
            }
        }
    }
}

bool RuleB::predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings)
{   

    bool reachedTarget = false;
    std::set<int> substitutions = {head};
    std::vector<Triple> currGroundings;
    searchCurrTargetGroundings(1, head, substitutions, triples, tail, relations, directions, currGroundings, groundings, reachedTarget);
    // for simplicity we just use QueryResults here 
    // we have to remember when calling from the outside what the triple is
    if (reachedTarget){
        qResults.insertRule(tail, this);
    }
    return reachedTarget;
}

// ***RuleC implementation*** 

RuleC::RuleC(std::vector<int>& relations, std::vector<bool>& directions, bool& leftC, std::array<int,2>& constants) {
    if(relations.size() != (directions.size() + 1)) {
        throw std::invalid_argument("'Directions' size should be one less than 'relations' size in construction of RuleC");
    }
    if(relations.size() < 2) {
        throw std::invalid_argument("Cannot construct a RuleC with no body atom.");
    }		
	this->relations = relations;
    this->directions = directions;
    this->leftC = leftC;	
    this->constants = constants;
    this->targetRel = relations.front();

    // used for rules where leftC=false and predicting heads
    this->_relations = relations;
    std::reverse(_relations.begin()+1, _relations.end());
    this->_directions = directions;
    std::reverse(_directions.begin(), _directions.end());
    _directions.flip();   
}


std::string RuleC::computeRuleString(Index* index){
    // we parse into the anytime string representation for !leftC and all length=1 rules it is the same as the internatl represention
    // for 
    std::string out;
    out += index->getStringOfRelId(relations[0]);

    if (!leftC){
        out += "(" + std::string(1,_cfg_prs_anyTimeVars[0]) 
                   + "," + index->getStringOfNodeId(constants[0])
                   + ")"
                   + _cfg_prs_ruleSeparator;
    
        // body
        for (int i=1; i<relations.size(); i++){
            if (i<relations.size()-1){
                out += index->getStringOfRelId(relations[i]);
                std::string var1 = std::string(1, _cfg_prs_anyTimeVars[i-1]);
                std::string var2 = std::string(1, _cfg_prs_anyTimeVars[i]);
                std::string atom = directions[i-1] ? "(" + var1 + "," + var2 + ")" : "(" + var2 + "," + var1 + ")";
                out += atom + _cfg_prs_atomSeparator;
            }
            // last atom contains constant
            if (i==relations.size()-1){
                out += index->getStringOfRelId(relations[i]);
                std::string var1 = std::string(1, _cfg_prs_anyTimeVars[i-1]);
                std::string var2 = index->getStringOfNodeId(constants[1]);
                std::string atom = directions[i-1] ? "(" + var1 + "," + var2 + ")" : "(" + var2 + "," + var1 + ")";
                out += atom;
            }
        }
    } else{
        out += "(" +  index->getStringOfNodeId(constants[0])
                    + "," + std::string(1,_cfg_prs_anyTimeVars.back())
                    + ")"
                    + _cfg_prs_ruleSeparator;
        
        // okay the anytime string representations - compared to the internal represenation - 
        // reverses the order of the body then flips dirs AND then uses the variable ordering YABCD..X instead of 
        // XABCD..Y , so we can readily use _relations and _directions and also have to flip the variable ordering

        // old ordering 
        std::string newOrder = _cfg_prs_anyTimeVars;
        newOrder[0] = _cfg_prs_anyTimeVars.back();
        newOrder.back() = _cfg_prs_anyTimeVars[0];

        // thats it, now we just do the same as above with _relations, _directions.
        // For readability we just repeat the black above here
        for (int i=1; i<_relations.size(); i++){
            if (i<_relations.size()-1){
                out += index->getStringOfRelId(_relations[i]);
                std::string var1 = std::string(1, newOrder[i-1]);
                std::string var2 = std::string(1, newOrder[i]);
                std::string atom = _directions[i-1] ? "(" + var1 + "," + var2 + ")" : "(" + var2 + "," + var1 + ")";
                out += atom + _cfg_prs_atomSeparator;
            }
            // last atom contains constant
            if (i==_relations.size()-1){
                out += index->getStringOfRelId(_relations[i]);
                std::string var1 = std::string(1, newOrder[i-1]);
                std::string var2 = index->getStringOfNodeId(constants[1]);
                std::string atom = _directions[i-1] ? "(" + var1 + "," + var2 + ")" : "(" + var2 + "," + var1 + ")";
                out += atom;
            }
        }
    }
    return out;
}


//DFS search where the starting point is the grounded body atom
// in the rule representation here this is the last body atom if leftC=false and first body atom if leftC=true
void RuleC::materialize(TripleStorage& triples, std::unordered_set<Triple>& preds){
    // if left head variable is grounded we start with with the first body atom it contains the second constant
    // if right is grounded we start with last body atom which then contains the second constant
    std::vector<int>& rels = leftC ? relations : _relations;
    std::vector<bool>& dirs = leftC ? directions: _directions;
    RelNodeToNodes* relNtoN = nullptr;
    if (dirs[0]){
        relNtoN =  &triples.getRelHeadToTails();   
    }else{
        relNtoN =  &triples.getRelTailToHeads();
    }
    auto it = relNtoN->find(rels[1]);
    if (it!=relNtoN->end()){
        NodeToNodes& NtoN = it->second;
        if (NtoN.count(constants[1])>0){
            Nodes closingEntities;
            // we enforce OI for both the constants, this is consistent with B rules
            std::set<int> substitutions = {constants[0], constants[1]};
            searchCurrGroundings(1, constants[1], substitutions, triples, closingEntities, rels, dirs);
            for (const int& cEnt:  closingEntities){
                bool isNew;
                Triple triple;
                if (leftC){
                    triple = {constants[0], targetRel, cEnt};
                    isNew = preds.insert(triple).second;
                }else{
                    triple = {cEnt, targetRel, constants[0]};
                    isNew = preds.insert(triple).second;
                }
                // if triple is predicted for the first time track stats
                if (trackInMaterialize && isNew){
                    predicted += 1;
                    if (triples.contains(triple[0], triple[1], triple[2])){
                        cpredicted += 1;
                    }

                }
            }
        }
    }
}

// contrary to B rules we let the DFS run starting from the grounded constants of the rules body
// and not from the grounded entity in the query 
bool RuleC::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
    // can only predict my constant in the grounded direction
    if (leftC && head!=constants[0]){
        return false;
    }

    if (directions.size()==1){
        return predictL1TailQuery(head, triples, tailResults, filterSet);
    }

    std::vector<int>& rels = leftC ? relations : _relations;
    std::vector<bool>& dirs = leftC ? directions: _directions;

    int* begin;
    int length;
    dirs[0] ? triples.getTforHR(constants[1], rels[1], begin, length) : triples.getHforTR(constants[1], rels[1], begin, length);
    if (length>0){
        Nodes closingEntities;
        std::set<int> substitutions = {constants[0], constants[1]};
        searchCurrGroundings(1, constants[1], substitutions, triples, closingEntities, rels, dirs);
        bool madePred = false;
        for (const int& cEnt: closingEntities){
            // the rule is grounded at the tail so it can only predict this grounding
            // and the closing entity must be the head grounding in this case
            if (!leftC && cEnt == head && !filterSet.contains(constants[0])){
                tailResults.insertRule(constants[0], this);
                return true;
            }else if(leftC && !filterSet.contains(cEnt)){
                tailResults.insertRule(cEnt, this);
                madePred = true;
            }
        }
        return madePred;
    }
    return false;
}


bool RuleC::predictL1TailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
    RelationalCSR* csr = triples.getCSR();

    // h(X,c) <-- b1(d,X) or h(X,c) <-- b1(X,d)
    // we want to only look up the body for X=head by looking from X=head to d and then predict c
    if (!leftC){

        if (filterSet.contains(constants[0])){
            return false;
        }
        int* begin;
        int length;
        int bodyRel = this->relations[1];
        directions[0] ? triples.getTforHR(head, bodyRel, begin, length) :  triples.getHforTR(head, bodyRel, begin, length);
        int* end = begin + length;
        if (std::binary_search(begin, end, constants[1]) && constants[0]!=head){
            tailResults.insertRule(constants[0], this);
            return true;
        }
    }else{
         // h(c,Y) <-- b1(d,Y) or h(c,Y) <-- b1(Y,d) , predict all Y=y that ground the body
        int* begin;
        int length;
        int bodyRel = this->relations[1];
        directions[0] ? triples.getTforHR(constants[1], bodyRel, begin, length) :  triples.getHforTR(constants[1], bodyRel, begin, length);
        bool predicted = false;
        for (int i=0; i<length; i++){
            int cand = begin[i];
            if (!filterSet.contains(cand) && cand!=constants[0]){
                tailResults.insertRule(cand, this);
                predicted = true;
            }
        }
        return predicted;
    }
    return false;

}


bool RuleC::predictL1HeadQuery(int tail, TripleStorage& triples, QueryResults& headResults,  ManySet filterSet){
    RelationalCSR* csr = triples.getCSR();
    // h(c,Y) <-- b1(d,Y) or h(c,Y) <-- b1(Y,d)
    // we want to only look up the body for Y=tail by looking from Y=tail to d and then predict c
    if (leftC){
         if(filterSet.contains(constants[0])){
            return false;
        }
        int* begin;
        int length;
        int bodyRel = this->relations[1];
        directions[0] ? csr->getHforTREfficient(tail, bodyRel, begin, length) :  csr->getTforHREfficient(tail, bodyRel, begin, length);
        int* end = begin + length;
        if (std::binary_search(begin, end, constants[1]) && constants[0]!=tail){
            headResults.insertRule(constants[0], this);
            return true;
        }
    }else{
        // h(X,c) <-- b1(d,X) or h(X,c) <-- b1(X,d) we want to predict all x=X that ground the body
        // tail==constants[0] is already checked
        int* begin;
        int length;
        int bodyRel = this->relations[1];
        directions[0] ? csr->getHforTREfficient(constants[1], bodyRel, begin, length) :  csr->getTforHREfficient(constants[1], bodyRel, begin, length);
        bool predicted = false;
        for (int i=0; i<length; i++){
            int cand = begin[i];
            if (!filterSet.contains(cand) && cand!=constants[0]){
                headResults.insertRule(cand, this);
                predicted = true;
            }
        }
        return predicted;
    }
    return false;

}



bool RuleC::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults,  ManySet filterSet){
    // can only predict my constant in the grounded direction
    if (!leftC && tail!=constants[0]){
        return false;
    }

    if (directions.size()==1){
        return predictL1HeadQuery(tail, triples, headResults, filterSet);
    }

    // the internal representation for the rules is
    // h(c,Y) <-- b1(d,A), b2(A,Y)
    // h(X,c) <-- b1(X,A), b2(A,d)
    // the DFS always starts from the first body atom so for the second rule (leftC==false)
    // we turn everything around: b2(A,d),b1(X,A) starting with current_entity=d
    std::vector<int>& rels = leftC ? relations : _relations;
    std::vector<bool>& dirs = leftC ? directions: _directions;


    int* begin;
    int length;
    dirs[0] ? triples.getTforHR(constants[1], rels[1], begin, length) : triples.getHforTR(constants[1], rels[1], begin, length);
    if (length>0){
        Nodes closingEntities;
        std::set<int> substitutions = {constants[0], constants[1]};
        searchCurrGroundings(1, constants[1], substitutions, triples, closingEntities, rels, dirs);
        bool madePred = false;
        for (const int& cEnt: closingEntities){
            // the rule is grounded at the head so it can only predict this grounding
            // and the closing entity must be the tail grounding in this case
            if (leftC && cEnt == tail && !filterSet.contains(constants[0])){
                headResults.insertRule(constants[0], this);
                return true;
            }else if(!leftC && !filterSet.contains(cEnt)){
                headResults.insertRule(cEnt, this);
                madePred = true;
            }
        }
            return madePred;
    }
    return false;
}

bool RuleC::predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings){
   
    if (leftC && head!=constants[0]){
        return false;
    }else if (!leftC && tail!=constants[0]){
        return false;
    }

    std::vector<int>& rels = leftC ? relations : _relations;
    std::vector<bool>& dirs = leftC ? directions: _directions;

    int* begin;
    int length;
    dirs[0] ? triples.getTforHR(constants[1], rels[1], begin, length) : triples.getHforTR(constants[1], rels[1], begin, length);
    bool reachedTarget = false;
    std::set<int> substitutions = {constants[0], constants[1]};
    std::vector<Triple> currGroundings;
    int targetEnt = leftC ? tail : head;
    // h(c,Y) <-- b1(d,A), b2(A,Y)
    // h(X,c) <-- b1(X,A), b2(A,d)
    // for both directions we start with the atom with the constant
    // so we have to invert groundings in both cases (given we want to return AnyBURL order) aka the true in last input param
    searchCurrTargetGroundings(1, constants[1], substitutions, triples, targetEnt, rels, dirs, currGroundings, groundings, reachedTarget, true);
    // for simplicity we just use QueryResults here; this also holds for U_c rules 
    // we have to remember when calling from the outside what the triple is
    if (reachedTarget){
        qResults.insertRule(tail, this);
    }
    return reachedTarget;
}

bool RuleC::predictL1Triple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings){
    return false;
}

// same implementation as in RuleB except that rels dirs is used depending on leftC s.t.
// directions can be handled
void RuleC::searchCurrGroundings(
			int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
            Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs
		)
{
    int currRel = rels[currAtomIdx];
    int* begin;
    int length;

    dirs[currAtomIdx-1] ? triples.getTforHR(currEntity, currRel, begin, length) : triples.getHforTR(currEntity, currRel, begin, length);
    // last atom
    if (currAtomIdx == rels.size()-1){
        // next entities
        for (int i=0; i<length; i++){
            int ent = begin[i];
            // respect object identity constraint, stop if violated
            if (substitutions.find(ent)==substitutions.end()){
                    closingEntities.insert(ent);
            }
        }
    }else{
         //next entities
         for (int i=0; i<length; i++){
            int ent = begin[i];
            if (substitutions.find(ent)==substitutions.end()){
                substitutions.insert(ent);
                searchCurrGroundings(currAtomIdx+1, ent, substitutions, triples, closingEntities, rels, dirs);
                substitutions.erase(ent);
            }
        }
    }
}


// ***RuleZ implementation*** 



RuleZ::RuleZ(int& relation, bool& leftC, int& constant) {
    this->relation=relation;
    this->targetRel=relation;
    this->leftC = leftC;
    this->constant = constant;
}

std::string RuleZ::computeRuleString(Index* index){
    std::string out;
    out += index->getStringOfRelId(relation);
    if (!leftC){
        out += "(" + std::string(1,_cfg_prs_anyTimeVars[0]) 
                   + "," + index->getStringOfNodeId(constant)
                   + ")"
                   + _cfg_prs_ruleSeparator;
    }else{
        out += "(" +  index->getStringOfNodeId(constant)
                   + "," + std::string(1,_cfg_prs_anyTimeVars.back())
                   + ")"
                   + _cfg_prs_ruleSeparator;
    }
    return out;

}


bool RuleZ::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet){
    if (leftC && !filterSet.contains(constant)){
        headResults.insertRule(constant, this);
        return true;
    }
    return false;
}

bool RuleZ::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
     if (!leftC && !filterSet.contains(constant)){
        tailResults.insertRule(constant, this);
        return true;
    }
    return false;
}


bool RuleZ::predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings){
    if (leftC && head!=constant){
        return false;
    } else if(!leftC && tail!=constant){
        return false;
    }

    if (groundings){
        (*groundings)[this].push_back({});
        qResults.insertRule(tail, this);
        return true;
    }else
        qResults.insertRule(tail, this);
        return true;
}


void RuleZ::materialize(TripleStorage& triples, std::unordered_set<Triple>& preds){
    // predict c when h(c,X)<-- given all h(--, a) in train and vice versa
    RelNodeToNodes& relNtoN = leftC ? triples.getRelTailToHeads() : triples.getRelHeadToTails();
    //TODO optimize
    auto it = relNtoN.find(relation);
    if (it!=relNtoN.end()){
        NodeToNodes& NtoN = it->second;
        for (auto const& pair: NtoN){
            bool isNew;
            Triple triple;
            // source node 
            const int& e = pair.first;
            if (leftC){
                triple = {constant, relation, e};
                isNew = preds.insert(triple).second;
            }else{
                triple = {e, relation, constant};
                isNew = preds.insert(triple).second;
            }
            if (trackInMaterialize && isNew){
                predicted += 1;
                if (triples.contains(triple[0], triple[1], triple[2])){
                    cpredicted += 1;
                }
            }
        }
    }
}


// ***RuleD implementation*** 
// a rule with a constant and a dangling atom
// h(X,d) <-- b1(X,A), b2(A,B), b3(B,C)
//  leftC=false, relations=[h, b1, b2, b3], directions=[1,1,1]
// h(d,Y) <-- b1(A,B), b2(B,C), b3(C,Y)
// leftC=true, relations=[h, b1, b2, b3], directions=[1,1,1]
// h(d,Y) <-- b1(A,B), b2(C,D), b3(Y,C)
// leftC=true, relations=[h, b1, b2, b3], directions=[1,0,0]

RuleD::RuleD(std::vector<int>& relations, std::vector<bool>& directions, bool& leftC, int constant) {

    if(relations.size() != (directions.size() + 1)) {
        throw std::invalid_argument("'Directions' size should be one less than 'relations' size in construction of RuleD");
    }
    if(relations.size() < 2) {
        throw std::invalid_argument("Cannot construct a RuleD with no body atom.");
    }		
    this->directions = directions;
    this->relations=relations;
    this->targetRel=relations[0];
    this->leftC = leftC;
    this->constant = constant;

    // used for rules where leftC=false
    this->_relations = relations;
    std::reverse(_relations.begin()+1, _relations.end());
    this->_directions = directions;
    std::reverse(_directions.begin(), _directions.end());
    _directions.flip();   
}

std::string RuleD::computeRuleString(Index* index){
    // we parse into the anytime string representation for !leftC and all length=1 rules it is the same as the internatl represention
    std::string out;
    out += index->getStringOfRelId(relations[0]);

    if (!leftC){
        out += "(" + std::string(1,_cfg_prs_anyTimeVars[0]) 
                   + "," + index->getStringOfNodeId(constant)
                   + ")"
                   + _cfg_prs_ruleSeparator;
    
        // body
        for (int i=1; i<relations.size(); i++){
                out += index->getStringOfRelId(relations[i]);
                std::string var1 = std::string(1, _cfg_prs_anyTimeVars[i-1]);
                std::string var2 = std::string(1, _cfg_prs_anyTimeVars[i]);
                std::string atom = directions[i-1] ? "(" + var1 + "," + var2 + ")" : "(" + var2 + "," + var1 + ")";

                if (i==relations.size()-1){
                    out += atom;
                }else{
                    out += atom + _cfg_prs_atomSeparator;
                }
                
        }
    } else{
        out += "(" +  index->getStringOfNodeId(constant)
                    + "," + std::string(1,_cfg_prs_anyTimeVars.back())
                    + ")"
                    + _cfg_prs_ruleSeparator;
        
        // same as in U_c: the anytime string representations - compared to the internal represenation - 
        // reverses the order of the body then flips dirs AND then uses the variable ordering YABCD..X instead of 
        // XABCD..Y , so we can readily use _relations and _directions and also have to flip the variable ordering

        // old ordering 
        std::string newOrder = _cfg_prs_anyTimeVars;
        newOrder[0] = _cfg_prs_anyTimeVars.back();
        newOrder.back() = _cfg_prs_anyTimeVars[0];

        // thats it, now we just do the same as above with _relations, _directions.
        // For readability we just repeat the black above here
        for (int i=1; i<_relations.size(); i++){
            out += index->getStringOfRelId(_relations[i]);
            std::string var1 = std::string(1, newOrder[i-1]);
            std::string var2 = std::string(1, newOrder[i]);
            std::string atom = _directions[i-1] ? "(" + var1 + "," + var2 + ")" : "(" + var2 + "," + var1 + ")";
            if (i==relations.size()-1){
                out += atom;
            }else{
                out += atom + _cfg_prs_atomSeparator;
            }    
        }
    }
    return out;
}


void RuleD::materialize(TripleStorage& triples, std::unordered_set<Triple>& preds){
    RelNodeToNodes* relNtoN = nullptr;
     // first body atom is (v1,v2)
    if (directions[0]){
         relNtoN =  &triples.getRelHeadToTails();
    // first body atom is (v2,v1)    
    }else{
         relNtoN =  &triples.getRelTailToHeads();
    }
     // first body relation
    auto it = relNtoN->find(relations[1]);
    if (!(it==relNtoN->end())){
        NodeToNodes& NtoN = it->second;
         // start branches of the DFS search
         // every entity e that satisfies b1(e,someY) [or b1(someX, e)]
         for (auto const& pair: NtoN){
                const int& e = pair.first;
                Nodes closingEntities;
                std::set<int> substitutions = {e, constant};

                if (e==constant){
                    continue;
                }
                searchCurrGroundings(1, e, substitutions, triples, closingEntities, relations, directions);
                for (const int& cEnt:  closingEntities){
                    Triple triple;
                    if (!leftC){
                        triple = {e, targetRel, constant};
                    }else{
                        triple = {constant, targetRel, cEnt};
                    }
                    auto isNew = preds.insert(triple);
                    // add to count if this triple is predicted for the first time
                    if (trackInMaterialize && isNew.second){
                        predicted+=1;
                        if (triples.contains(triple[0], triple[1], triple[2])){
                            cpredicted += 1;
                        }
                    }
                }
            }
    }
}



void RuleD::searchCurrGroundings(
		int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
		Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs
)
{
    int currRel = rels[currAtomIdx];
    int* begin;
    int length;

    dirs[currAtomIdx-1] ? triples.getTforHR(currEntity, currRel, begin, length) : triples.getHforTR(currEntity, currRel, begin, length);
    if (currAtomIdx == rels.size()-1){
        // next entities
        for (int i=0; i<length; i++){
            int ent = begin[i];
            // respect object identity constraint, stop if violated
            if (substitutions.find(ent)==substitutions.end()){
                    closingEntities.insert(ent);
            }
        }
    }else{
        if (branchingFactor>0 && length>branchingFactor){
            return;
        }
        for (int i=0; i<length; i++){
            int ent = begin[i];
            if (substitutions.find(ent)==substitutions.end()){
                substitutions.insert(ent);
                searchCurrGroundings(currAtomIdx+1, ent, substitutions, triples, closingEntities, rels, dirs);
                substitutions.erase(ent);
            }
        }
    }
}

bool RuleD::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet){
    // h(X,d) <-- b1(X,A), b2(A,B), b3(B,C)
    //  leftC=false, relations=[h, b1, b2, b3], directions=[1,1,1]
    // h(d,Y) <-- b1(A,B), b2(B,C), b3(C,Y)
    // leftC=true, relations=[h, b1, b2, b3], directions=[1,1,1]
    // h(d,Y) <-- b1(A,B), b2(C,D), b3(Y,C)
    // leftC=true, relations=[h, b1, b2, b3], directions=[1,0,0]
    if (!leftC && tail!=constant){
        return false;
    }

    if (leftC && filterSet.contains(constant)){
        return false;
    }
    if (directions.size()==1){
        return predictL1HeadQuery(tail, triples, headResults, filterSet);
    }
    // can only predict constant
    if (leftC){
        std::set<int> substitutions = {constant, tail};
        Nodes closingEntities;
        searchCurrGroundings(1, tail, substitutions, triples, closingEntities, _relations, _directions);
        if (closingEntities.size()>0){
            // filtering is checked above already
            headResults.insertRule(constant, this);
            //done, can only predict one thing
            return true;
        }
    }else{
        RelNodeToNodes* relNtoN = nullptr;
        if (directions[0]){
            relNtoN =  &triples.getRelHeadToTails();
          
        }else{
            relNtoN =  &triples.getRelTailToHeads();
        }
        // first body relation
        auto it = relNtoN->find(relations[1]);
        bool predicted = false;
        if (!(it==relNtoN->end())){
            NodeToNodes& NtoN = it->second;
            for (auto const& pair: NtoN){
                const int& e = pair.first;
                Nodes closingEntities;
                std::set<int> substitutions = {e, constant};
                if (e==constant){
                    continue;
                }
                searchCurrGroundings(1, e, substitutions, triples, closingEntities, relations, directions);
                if (closingEntities.size()>0 && !filterSet.contains(e)){
                    headResults.insertRule(e, this);
                    predicted = true;
                }
            }
        }
        return predicted;
    }
}


bool RuleD::predictL1HeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet){
    // h(X,c) <-- b1(X,A)
    // h(c,Y) <-- b1(A,Y)
    if (leftC){
        int bodyRel = relations[1];
        int* begin;
        int length;
        if (constant==tail){
            return false;
        }
        directions[0] ? triples.getHforTR(tail, bodyRel, begin, length) : triples.getTforHR(tail, bodyRel, begin, length);
        if (length>0){
            // filtering is checked  already
            headResults.insertRule(constant, this);
            return true;
        } else {
            return false;
        }
    }else { 
        Index* index = triples.getIndex();
        bool predicted = false;
        for (int i=0; i<index->getNodeSize(); i++){
            int bodyRel = relations[1];
            int* begin;
            int length;
            directions[0] ? triples.getTforHR(i, bodyRel, begin, length) : triples.getHforTR(i, bodyRel, begin, length);
            if (length>0 && !filterSet.contains(i) && i!=constant){
                 headResults.insertRule(i, this);
                 predicted = true;
            }
        }
        return predicted;
    }
}  

    

bool RuleD::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
    // h(X,c) <-- b1(X,A), b2(A,B), b3(B,C)
    //  leftC=false, relations=[h, b1, b2, b3], directions=[1,1,1]
    // h(c,Y) <-- b1(A,B), b2(B,C), b3(C,Y)
    // leftC=true, relations=[h, b1, b2, b3], directions=[1,1,1]
    // h(c,Y) <-- b1(A,B), b2(C,D), b3(Y,C)
    // leftC=true, relations=[h, b1, b2, b3], directions=[1,0,0]
     if (leftC && head!=constant){
        return false;
    }
    if (!leftC && filterSet.contains(constant)){
        return false;
    }

     if (directions.size()==1){
        return predictL1TailQuery(head, triples, tailResults, filterSet);
    }

    // can only predict constant for the tail
    if (!leftC){
        std::set<int> substitutions = {constant, head};
        Nodes closingEntities;
        searchCurrGroundings(1, head, substitutions, triples, closingEntities, relations, directions);
        if (closingEntities.size()>0){
            // filtering is checked above already
            tailResults.insertRule(constant, this);
            //done, can only predict one thing
            return true;
        }
    } else {
        
        // we start from the last atom (using _relations _directions)
        RelNodeToNodes* relNtoN = nullptr;
        if (_directions[0]){
            relNtoN =  &triples.getRelHeadToTails();
          
        }else{
            relNtoN =  &triples.getRelTailToHeads();
        }
        // first body relation
        auto it = relNtoN->find(_relations[1]);
        bool predicted = false;
        if (!(it==relNtoN->end())){
            NodeToNodes& NtoN = it->second;
            for (auto const& pair: NtoN){
                const int& e = pair.first;
                Nodes closingEntities;
                std::set<int> substitutions = {e, constant};
                if (e==constant){
                    continue;
                }
                searchCurrGroundings(1, e, substitutions, triples, closingEntities, _relations, _directions);
                if (closingEntities.size()>0 && !filterSet.contains(e)){
                    tailResults.insertRule(e, this);
                    predicted = true;
                }
            }
        }
        return predicted;
    }
}

bool RuleD::predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings){
    // h(X,c) <-- b1(X,A), b2(A,B), b3(B,C)
    //  leftC=false, relations=[h, b1, b2, b3], directions=[1,1,1]
    // h(c,Y) <-- b1(A,B), b2(B,C), b3(C,Y)
    // leftC=true, relations=[h, b1, b2, b3], directions=[1,1,1]
    // h(c,Y) <-- b1(A,B), b2(C,D), b3(Y,C)
    // leftC=true, relations=[h, b1, b2, b3], directions=[1,0,0]

    if (leftC && head!=constant){
        return false;
    }else if (!leftC && tail!=constant){
        return false;
    }

    bool reachedTarget = false;
    // for leftC we start with the first body atom and move to the last atom
    // if we hit a Y=tail(==targetEnt) we can stop (if not tracking groundings) 
    if (leftC){
        RelNodeToNodes* relNtoN = nullptr;
        if (directions[0]){
            relNtoN =  &triples.getRelHeadToTails();
          
        }else{
            relNtoN =  &triples.getRelTailToHeads();
        }

        auto it = relNtoN->find(relations[1]);
        if (!(it==relNtoN->end())){
            NodeToNodes& NtoN = it->second;
            for (auto const& pair: NtoN){
                const int& e = pair.first;
                std::set<int> substitutions = {e, constant};
                std::vector<Triple> currGroundings;
                searchCurrTargetGroundings(1, e, substitutions, triples, tail, relations, directions, currGroundings, groundings, reachedTarget, true);
                // stop after hitting tail once when we not track groundings
                if (!groundings && reachedTarget){
                    qResults.insertRule(tail, this);
                    return true;
                }
            }
        }
        if (reachedTarget){
            qResults.insertRule(tail, this);
        }
        return reachedTarget;
    // not leftC we move from last atom to first atom and see if we hit the head
    } else{
         RelNodeToNodes* relNtoN = nullptr;
         if (_directions[0]){
            relNtoN =  &triples.getRelHeadToTails();
          
        }else{
            relNtoN =  &triples.getRelTailToHeads();
        }

        auto it = relNtoN->find(_relations[1]);
        if (!(it==relNtoN->end())){
            NodeToNodes& NtoN = it->second;
            for (auto const& pair: NtoN){
                const int& e = pair.first;
                std::set<int> substitutions = {e, constant};
                std::vector<Triple> currGroundings;
                searchCurrTargetGroundings(1, e, substitutions, triples, head, _relations, _directions, currGroundings, groundings, reachedTarget, true);
                // stop after hitting tail once when we not track groundings
                if (!groundings && reachedTarget){
                    qResults.insertRule(tail, this);
                    return true;
                }
            }
        }
        if (reachedTarget){
            qResults.insertRule(tail, this);
        }
        return reachedTarget;

    }    
}


bool RuleD::predictL1TailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
    // h(X,c) <-- b1(X,A)
    // h(c,Y) <-- b1(A,Y)
     if (!leftC){
        int bodyRel = relations[1];
        int* begin;
        int length;
        //OI
        if (constant==head){
            return false;
        }
        !directions[0] ? triples.getHforTR(head, bodyRel, begin, length) : triples.getTforHR(head, bodyRel, begin, length);
        if (length>0){
            // filtering is checked  already
            tailResults.insertRule(constant, this);
            return true;
        } else {
            return false;
        }
    }else { 
        Index* index = triples.getIndex();
        bool predicted = false;
        for (int i=0; i<index->getNodeSize(); i++){
            int bodyRel = relations[1];
            int* begin;
            int length;
            !directions[0] ? triples.getTforHR(i, bodyRel, begin, length) : triples.getHforTR(i, bodyRel, begin, length);
            if (length>0 && !filterSet.contains(i) && i!=constant){
                 tailResults.insertRule(i, this);
                 predicted = true;
            }
        }
        return predicted;
    }
    
}

// RuleXXd

RuleXXd::RuleXXd(std::vector<int>& relations, std::vector<bool>& directions) {
    if(relations.size() > 2) {
        throw std::invalid_argument("'Cannot construct longer UXX_d rules-");
    }
    if (directions.size()!=1){
        throw std::invalid_argument("Something wrong with this UXXd rule.");

    }
    this->relations=relations; 
    this->directions=directions;
    this->targetRel=relations[0];

    // only used and set after a rule is contructed and parsed from AnyBURL rule files
    this->predictHead = true;
    this->predictHead = true;
}

void RuleXXd::setPredictHead(bool ind){
    this->predictHead = ind;
}
void RuleXXd::setPredictTail(bool ind){
    this->predictTail = ind;
}

std::string RuleXXd::computeRuleString(Index* index){

    std::string out;
    out += index->getStringOfRelId(relations[0]) + "(X,X)" + _cfg_prs_ruleSeparator + index->getStringOfRelId(relations[1]);
    out += directions[0] ?  "(" + std::string(1,_cfg_prs_anyTimeVars[0]) + "," + std::string(1,_cfg_prs_anyTimeVars[1]) + ")" :
                            "(" + std::string(1,_cfg_prs_anyTimeVars[1]) + "," + std::string(1,_cfg_prs_anyTimeVars[0]) + ")";
    return out;
}


bool RuleXXd::predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings){
    // h(X,X) <-- b(X,A)
    // h(X,X) <-- b(A,X)
    if (head!=tail){
        return false;
    }
    int length;
    int* begin;
    int bodyRel = this->relations[1];
    directions[0] ? triples.getTforHR(head, bodyRel, begin, length) :  triples.getHforTR(tail, bodyRel, begin, length); //head==tail anyways
    if (length>0 && !groundings){
        qResults.insertRule(tail, this);
        return true;
    }else if(length>0 && groundings){
        for (int i=0; i<length; i++){
            std::vector<Triple> oneGrounding;
            if (directions[0]){
                oneGrounding.push_back({head, relations[1], begin[i]});
            }else{
                oneGrounding.push_back({begin[i], relations[1], head}); //head==tail
            }
            (*groundings)[this].push_back(oneGrounding); 
        }
        qResults.insertRule(tail, this);
        return true;
    }else{
        return false;
    }
}


bool RuleXXd::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet){
    // h(X,X) <-- b(X,A)
    // h(X,X) <-- b(A,X)
    if (!predictHead){
        return false;
    }
    int length;
    int* begin;
    int bodyRel = this->relations[1];
    directions[0] ? triples.getTforHR(tail, bodyRel, begin, length) :  triples.getHforTR(tail, bodyRel, begin, length);
    if (length>0 && !filterSet.contains(tail)){
        headResults.insertRule(tail, this);
        return true;
    }
    return false;
}


bool RuleXXd::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
    // h(X,X) <-- b(X,A)
    // h(X,X) <-- b(A,X)
    if (!predictTail){
        return false;
    }
    int length;
    int* begin;
    
    int bodyRel = this->relations[1];
    directions[0] ? triples.getTforHR(head, bodyRel, begin, length) :  triples.getHforTR(head, bodyRel, begin, length);
    if (length>0 && !filterSet.contains(head)){
        tailResults.insertRule(head, this);
        return true;
    }
    return false;
}

void RuleXXd::materialize(TripleStorage& triples, std::unordered_set<Triple>& preds){
    // h(X,X) <-- b(X,A)
    // h(X,X) <-- b(A,X)
    Index* index = triples.getIndex();
    for (int i=0; i<index->getNodeSize(); i++){
        int bodyRel = relations[1];
        int* begin;
        int length;
        directions[0] ? triples.getTforHR(i, bodyRel, begin, length) : triples.getHforTR(i, bodyRel, begin, length);
        if (length>0){
            Triple triple = {i, targetRel, i};
            auto isNew = preds.insert(triple);
            if (trackInMaterialize && isNew.second){
                predicted+=1;
                if (triples.contains(triple[0], triple[1], triple[2])){
                    cpredicted += 1;
                }
            }
        }
    }
}



//RuleXXc

RuleXXc::RuleXXc(std::vector<int>& relations, std::vector<bool>& directions, int& constant) {
    if(relations.size() > 2) {
        throw std::invalid_argument("'Cannot construct longer UXX_d rules-");
    }
    if (directions.size()!=1){
        throw std::invalid_argument("Something wrong with this UXXd rule.");

    }
    this->relations=relations; 
    this->directions=directions;
    this->targetRel=relations[0];
    this->constant = constant;

    // only used and set after a rule is contructed and parsed from AnyBURL rule files
    this->predictHead = true;
    this->predictHead = true;
}

std::string RuleXXc::computeRuleString(Index* index){

    std::string out;
    out += index->getStringOfRelId(relations[0]) + "(X,X)" + _cfg_prs_ruleSeparator + index->getStringOfRelId(relations[1]);
    out += directions[0] ?  "(" + std::string(1,_cfg_prs_anyTimeVars[0]) + "," + index->getStringOfNodeId(constant) + ")" :
                            "(" +index->getStringOfNodeId(constant) + "," + std::string(1,_cfg_prs_anyTimeVars[0]) + ")";
    return out;
}

bool RuleXXc::predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings){
    // h(X,X) <-- b(X,d)
    // h(X,X) <-- b(d,X)
    if (head!=tail){
        return false;
    }
    int length;
    int* begin;
    int bodyRel = this->relations[1];
    directions[0] ? triples.getTforHR(head, bodyRel, begin, length) :  triples.getHforTR(tail, bodyRel, begin, length); //head==tail
    int* end = begin + length;
    if (std::binary_search(begin, end, constant)){
        if (groundings){
            std::vector<Triple> oneGrounding;
            if (directions[0]){
                oneGrounding.push_back({tail, bodyRel, constant});
            }else{
                oneGrounding.push_back({constant, bodyRel, tail});
            }
            (*groundings)[this].push_back(oneGrounding);
            qResults.insertRule(tail, this);
            return true;
        }else{
            qResults.insertRule(tail, this);
            return true;
        }
    }
    return false;
}



bool RuleXXc::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet){
    // h(X,X) <-- b(X,d)
    // h(X,X) <-- b(d,X)
    if (!predictHead){
        return false;
    }
    int length;
    int* begin;
    int bodyRel = this->relations[1];
    directions[0] ? triples.getTforHR(tail, bodyRel, begin, length) :  triples.getHforTR(tail, bodyRel, begin, length);
    int* end = begin + length;
    if (std::binary_search(begin, end, constant) && !filterSet.contains(tail)){
        headResults.insertRule(tail, this);
        return true;
    }
    return false;
}


bool RuleXXc::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
    // h(X,X) <-- b(X,d)
    // h(X,X) <-- b(d,X)
    if (!predictTail){
        return false;
    }
    int length;
    int* begin;
    int bodyRel = this->relations[1];
    directions[0] ? triples.getTforHR(head, bodyRel, begin, length) :  triples.getHforTR(head, bodyRel, begin, length);
    int* end = begin + length;
    if (std::binary_search(begin, end, constant) && !filterSet.contains(head)){
        tailResults.insertRule(head, this);
        return true;
    }
    return false;
}


void RuleXXc::setPredictHead(bool ind){
    this->predictHead = ind;
}
void RuleXXc::setPredictTail(bool ind){
    this->predictTail = ind;
}


void RuleXXc::materialize(TripleStorage& triples, std::unordered_set<Triple>& preds){
    // h(X,X) <-- b(X,d)
    // h(X,X) <-- b(d,X)
    int bodyRel = relations[1];
    int* begin;
    int length;
    directions[0] ? triples.getHforTR(constant, bodyRel, begin, length) : triples.getTforHR(constant, bodyRel, begin, length);
    for (int i=0; i<length; i++){
        Triple triple = {begin[i], targetRel, begin[i]};
        auto isNew = preds.insert(triple);
        if (trackInMaterialize && isNew.second){
            predicted+=1;
            if (triples.contains(triple[0], triple[1], triple[2])){
                cpredicted += 1;
            }
        }
    }
}








