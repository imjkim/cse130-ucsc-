# Tests for split on correct behavior for ./split -
# Test should output the same as ./rsplit binary in order to pass
# I made this test, because my implementation did not result the same output as rsplit when I did "./split test_files/frankenstein.txt -", as it would still take in the inputs when it is not supposed to.
text="test_files/frankenstein.txt -"
diff <(./split $text ) <(./rsplit $text ) > /dev/null
