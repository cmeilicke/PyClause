"""
This example illustrates how to read and evaluate a ranking that has been stored previously.
Please run demoEval1.py first to create this ranking.
"""

from clause.util.utils import get_base_dir
from clause.data.triples import TripleSet

from clause.rule.rules import RuleSet
from clause.rule.ruleparser import RuleReader

train_path = f"{get_base_dir()}/data/wnrr/train.txt"
test_path = f"{get_base_dir()}/data/wnrr/test.txt"
rules_path = f"{get_base_dir()}/data/wnrr/anyburl-rules-c5-3600"

train = TripleSet(train_path)
test = TripleSet(test_path, train.index)

train.write_as_problog("local/problog/pl-wn18rr-train.txt")
# test.write_as_problog("local/problog/pl-wn18rr-test.txt")

rs = RuleSet(train.index)

rr = RuleReader(rs)
rules = rr.read_file(rules_path)

# rs.write("local/problog/pl-wn18rr-rules-b3.txt", output_format="Problog")









