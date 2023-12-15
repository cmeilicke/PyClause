#ifndef RULEFACTORY_H
#define RULEFACTORY_H


#include "Rule.h"
#include "Index.h"

#include <memory>
#include <string>





class RuleFactory
{
public:
    RuleFactory(std::shared_ptr<Index> index);
    std::unique_ptr<Rule> parseAnytimeRule(std::string rule);

    std::unique_ptr<Rule>parseUXXrule(std::vector<std::string> headBody);

    void parseAtom(const std::string& input, strAtom& atom);
    void parseSymAtom(strAtom& inputAtom, symAtom& symA);

   
    void setCreateRuleB(bool ind);
    void setCreateRuleC(bool ind);
    void setCreateRuleZ(bool ind);
    void setCreateRuleD(bool ind);
    void setCreateRuleXXd(bool ind);
    void setCreateRuleXXc(bool ind);
    void setNumUnseen(int val, std::string type);

    void setBbranchingFactor(int val);
    void setDbranchingFactor(int val);
    void setDconfWeight(double val);
    void setZconfWeight(double val);

private:
    std::shared_ptr<Index> index;

    // ***Rule options***
    bool createRuleZ = true;
    double ZconfWeight = 1.0;
    int ZnumUnseen = 5;


    bool createRuleB = true;
    int BbranchingFactor = -1;
    int BnumUnseen = 5;

    bool createRuleC = true;
    int CnumUnseen = 5;

    bool createRuleD = true;
    double DconfWeight = 1.0;
    int DbranchingFactor = -1;
    int DnumUnseen = 5;


    bool createRuleXXd = true;
    int XXDnumUnseen = 5;

    bool createRuleXXc = true;
    int XXCnumUnseen = 5;
};


#endif