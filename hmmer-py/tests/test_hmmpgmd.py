from subprocess import check_call

import hmmer


def test_hmmpgmd():
    assert check_call([hmmer.path(hmmer.hmmpgmd), "-h"]) == 0
