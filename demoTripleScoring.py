import c_clause
import numpy as np

train = "./data/wnrr/train.txt"
filter = "./data/wnrr/valid.txt"
target = "./data/wnrr/test.txt"
rules = "./data/wnrr/anyburl-rules-c5-3600"

num_top_rules = 10

options = {
        # scoring/ranking options
        "aggregation_function": "maxplus",
        "collect_explanations": "true", #set true to track groundings and rules; can be obtained with scorer.get_explanation(bool)
        "num_top_rules": str(num_top_rules), #stop scoring a triple after it was predicted by the higehst num_top_rules;
                                             #score will not be affected under maxplus aggregation, but number of rules/groundings
                                             # when tracked will be   
        # rule options 
        "rule_b_max_branching_factor": "-1",
        "use_zero_rules": "false",
        "rule_zero_weight":"0.01",
        "use_u_c_rules": "true",
        "use_b_rules": "true",
        "use_u_d_rules": "true",
        "rule_u_d_weight":"0.01",
        "use_u_xxc_rules": "false",
        "use_u_xxd_rules": "false",
}

loader = c_clause.DataHandler(options)
loader.load_data(train, filter, target)
loader.load_rules(rules)


scorer = c_clause.PredictionHandler(options)
## you can also input np.array with idx's or list of string triples
scorer.score_triples("./data/wnrr/test.txt", loader)

## false --> idx's are returned (set index if you want your own)
idx_scores = scorer.get_scores(False)
## true --> strings are returned
str_scores = scorer.get_scores(True)

## true,false as above
## explanation is a tuple with 3 elements
## 0: list of target (input) triples
## 1: list of list of rules (at position i, a list of rules that predicted target i)
## 2: list of list of list of groundings
idx_explanations = scorer.get_explanations(False)
str_explanations = scorer.get_explanations(True)

# list of triples 
targets = idx_explanations[0]
# rules[i] is a list of rules that predict triples targets[i]
rules = idx_explanations[1]
# groundings[i][j] is a list of groundings for rule j of target i
# every grounding itself is a list of triples; every triple itself is a list of strings/tuples
groundings = idx_explanations[2]


explanations = str_explanations
## list of string rules rule_idx[i] gives back rule string of rule i
rule_idx = loader.rule_index()

entity_map = loader.entity_map()
relation_map = loader.relation_map()

for i in range(len(explanations[0])):
    print("-----------------------------------------------")
    print(f"Target triple: {explanations[0][i]}")
    for j in range(len(explanations[1][i])):
        print("Rule:")
        print(explanations[1][i][j])
        print("Groundings:")
        ctr = 0
        for grounding in explanations[2][i][j]:
            print("Next grounding")
            ## each grounding is a list of triples where a triple is a list
            print(grounding)