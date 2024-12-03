from subprocess import check_call

import hmmer


def test_press():
    assert check_call([hmmer.path(hmmer.hmmpress), "-h"]) == 0
